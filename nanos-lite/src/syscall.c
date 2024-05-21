#include <common.h>
#include "syscall.h"

int sys_write(int, void *, size_t);

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
	 //printf("here %d, %d", (int)a[0], a[1]);
  switch (a[0]) {
    case 0: halt(a[1]);
    case 1: yield(); c->GPRx = 0; break;
    case 4: c->GPRx = sys_write((int)(a[1]), (void *)(a[2]), (size_t)(a[3])); break; 
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

int sys_write(int fd, void *buf, size_t count) {
  if(fd == 1|| fd == 2){
     for(size_t i = 0; i < count; i++)
       putch(((char *)buf)[i]);
     return count;
  } 
  return -1;
}
