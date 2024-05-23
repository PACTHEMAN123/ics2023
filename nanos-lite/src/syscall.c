#include <common.h>
#include "syscall.h"
#include <fs.h>

int sys_write(int, void *, size_t);

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  //Log("syscall ID = %d", a[0]);
	 //printf("here %d, %d", (int)a[0], a[1]);
  switch (a[0]) {
    case SYS_exit: halt(a[1]);
    case SYS_yield: yield(); c->GPRx = 0; break;
    case SYS_open: c->GPRx = fs_open((const char *)(a[1]), (int)(a[2]), (int)(a[3])); break;
    case SYS_read: c->GPRx = fs_read((int)(a[1]), (void *)(a[2]), (size_t)(a[3])); break;
    case SYS_write: c->GPRx = sys_write((int)(a[1]), (void *)(a[2]), (size_t)(a[3])); break;
    case SYS_lseek: c->GPRx = fs_lseek((int)(a[1]), (size_t)(a[2]), (int)(a[3])); break;
    case SYS_close: c->GPRx = fs_close((int)(a[1])); break;
    case SYS_brk: c->GPRx = 0; break; 
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

int sys_write(int fd, void *buf, size_t count) {
  if(fd == 1|| fd == 2){
     for(size_t i = 0; i < count; i++)
       putch(((char *)buf)[i]);
     return count;
  }
  return fs_write(fd, buf, count); 
}
