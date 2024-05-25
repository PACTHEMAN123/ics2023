#include <stdio.h>
#include <assert.h>
#include <NDL.h>

int main() {
  int sec = 1;
  uint32_t usec;
  while(1) {
    usec = NDL_GetTicks();
    if((usec / 1000000) == sec) {
      printf("sec: %d hello from navy\n", sec);
      sec += 1;
    }
  }
  return 0;
}
