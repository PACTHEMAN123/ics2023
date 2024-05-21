#include <common.h>
#include "syscall.h"
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  switch (a[0]) {
    case 0: halt(a[1]);
    case 1: yield(); c->GPRx = 0; break;
    case 4: if(a[1]==1||a[1]==2){char *buf = (char *)(a[2]);for(int i=0;i<a[3];i++)putch(buf[i]);c->GPRx=a[3];} break; 
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
