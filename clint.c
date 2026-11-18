//CLINT lives at 0x02000000 at 10MHz
//0x02000000   msip      (software-interrupt pokes, per hart — ignore for now)
//0x02004000   mtimecmp  (hart 0's compare value; hart 1 would be +8, etc.)
//0x0200BFF8   mtime     (the free-running counter)

volatile unsigned long *mtimecmp = (volatile unsigned long *)0x02004000;
volatile unsigned long *mtime = (volatile unsigned long *)0x0200BFF8;

unsigned long uptime() {
    volatile unsigned long *mtime = (volatile unsigned long *)0x0200BFF8;
    return *mtime;
}
