Custom riscv bootloader written for the qemu generic riscv virtual platform

https://www.qemu.org/docs/master/system/target-riscv.html

https://www.qemu.org/docs/master/system/riscv/virt.html

Uses the NS16550 compatible UARTs for communicating with the system

## Adding virtio-gpu framebuffer support

Launch with: `qemu-system-riscv64 -machine virt -bios none -kernel kernel.elf -nographic -device virtio-gpu-device`

The `virtio-gpu-device` attaches to one of the virtio-mmio transport slots. The riscv-virt machine has 8 such slots (`virtio_mmio@...` in the device tree) at 0x10008000, 0x10009000, 0x1000A000, ..., 0x1000F000 (each 0x1000 apart). The GPU will occupy the first available one.

Implementation plan — write these yourself:

### 1. Virtio-MMIO transport driver

Probe each MMIO slot address until you find the GPU:

- **Reset**: write `0` to the `Status` register (offset `0x100`). Read it back until `0`.
- **Acknowledge**: write `ACKNOWLEDGE` (`0x01`) to `Status`.
- **Driver**: write `DRIVER` (`0x02`) to `Status`.
- **Negotiate features**: read `DeviceFeatures` (offset `0x010`), then write your accepted subset to `DriverFeatures` (offset `0x020`).
- **Feature OK**: write `FEATURES_OK` (`0x08`) to `Status`. Read `Status` back — if `FEATURES_OK` isn't set, the device didn't like your feature set.
- **Discover device ID**: read `DeviceID` (offset `0x00c`). `0x016` (22) is `VIRTIO_ID_GPU`.
- **Set up virtqueues**: you need at least the **controlq** (`index 0`) and **cursorq** (`index 1`). For each:
  1. Read `QueueNumMax` (offset `0x030`).
  2. Write the desired size to `QueueNum` (offset `0x038`).
  3. Allocate a 4-byte-aligned block in guest RAM for the descriptor table (`sizeof(virtq_desc) * n`), available ring, and used ring. Write its physical address to `QueueDescLow/High` (offsets `0x080`/`0x084`), same for `QueueAvail` and `QueueUsed`.
  4. Write `1` to `QueueReady` (offset `0x044`).
- **Finish**: write `DRIVER_OK` (`0x04`) to `Status`.

Register map reference: `#offset` from the MMIO base — [Virtio 1.2 spec, section 4.2.4](https://docs.oasis-open.org/virtio/virtio/v1.2/csd01/virtio-v1.2-csd01.html#x1-2570004.2.4).

### 2. Virtqueue descriptor submission

Each queue has three rings in guest RAM:

- **Descriptor table**: array of `struct virtq_desc { addr, len, flags, next }`. Each entry points to a buffer (guest physical address).
- **Available ring**: `struct virtq_avail { flags, idx, ring[] }`. The driver writes descriptor indices into `ring[]`, then advances `idx` (modulo queue size).
- **Used ring**: `struct virtq_used { flags, idx, ring[] }`. The device writes consumed descriptor indices here after processing. Poll `idx` to find new completions.

Submission sequence:
1. Fill a descriptor (or a chain for request/response) in the descriptor table.
2. Write the head descriptor index into `avail.ring[avail.idx % n]`.
3. Write a memory barrier (`fence`).
4. Increment `avail.idx`.
5. Write `QueueNotify` (offset `0x050`) with the queue index — ring the doorbell.
6. Poll `used.idx` until it advances, then read `used.ring[]` for the completed descriptor.

You'll need to write helper functions: `alloc_desc()`, `submit_avail()`, `poll_used()`.

### 3. Virtio-GPU protocol

The GPU speaks over the control queue. All commands share a common header:

```
struct virtio_gpu_ctrl_hdr {
    u32 type;
    u32 flags;
    u64 fence_id;
    u32 context_id;
    u32 padding;
};
```

Send a request descriptor (write-only for the device to read), then chain a response descriptor (read-only for the device to write). The response starts with the same header; check `type` for `VIRTIO_GPU_RESP_OK_NODATA`.

Commands you need (in order):

1. **`VIRTIO_GPU_CMD_GET_DISPLAY_INFO`** (`type = 0x0100`)
   - Response: `struct virtio_gpu_resp_display_info { hdr; struct virtio_gpu_display_one { r, x, y, w, h } pmodes[] }`.
   - Gives you the display resolution, e.g. 1024x768 (QEMU default).

2. **`VIRTIO_GPU_CMD_RESOURCE_CREATE_2D`** (`type = 0x0101`)
   - Request struct: `{ hdr; u32 resource_id; u32 format; u32 width; u32 height }`.
   - Pick a `resource_id` (start at 1), format `VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM` (`1`), and your width/height from step 1.
   - Response: just `OK_NODATA`.

3. **`VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING`** (`type = 0x0106`)
   - Request: `{ hdr; u32 resource_id; u32 nr_entries; struct virtio_gpu_mem_entry entries[] }`.
   - Each `virtio_gpu_mem_entry` has `{ u64 addr; u32 length; u32 padding }`.
   - Allocate one or more contiguous physical pages in guest RAM (4KB-aligned, at least `width * height * 4` bytes). Point the entries at those pages.
   - Response: `OK_NODATA`.

4. **`VIRTIO_GPU_CMD_SET_SCANOUT`** (`type = 0x010b`)
   - Request: `{ hdr; struct virtio_gpu_rect { u32 x, y, w, h }; u32 scanout_id; u32 resource_id }`.
   - `scanout_id` = 0, `resource_id` from step 2, `w`/`h` from step 1.
   - Response: `OK_NODATA`.

5. **Write pixels!** Write RGBA32 (`0xRRGGBBAA` in little-endian = B, G, R, A bytes) directly to your backing pages. Each row is `width * 4` bytes.

6. **`VIRTIO_GPU_CMD_RESOURCE_FLUSH`** (`type = 0x010c`)
   - Request: `{ hdr; struct virtio_gpu_rect { u32 x, y, w, h }; u32 resource_id; u32 padding }`.
   - Flush the whole rect (`x=0, y=0, w, h`) to push pixels to the display.
   - Response: `OK_NODATA`.

You'll want a higher-level API like `framebuffer_init()`, `framebuffer_write_pixel(x, y, color)`, `framebuffer_flush()`.

### 4. Update start.sh

```
qemu-system-riscv64 -machine virt -bios none -kernel kernel.elf -nographic -device virtio-gpu-device
```

Remove `-nographic` if you want both the QEMU display window and serial console, or keep it and add `-display sdl` (or `gtk`) to see the window.

### Reference

- Virtio 1.2 spec, sections 4.2 (MMIO transport) and 5.7 (GPU device)  
- QEMU source: `hw/display/virtio-gpu.c`  
- Linux source: `drivers/virtio/virtio_mmio.c` and `drivers/gpu/drm/virtio/`
