#include <sys/time.h>
#include <stdio.h>
#include <assert.h>
int main() {
  struct timeval tv;
  struct timezone tz;
  int sec = 1;
  while(1) {
    assert(gettimeofday(&tv, &tz) == 0);
    if(tv.tv_sec / (sec * 0.5)) {
      printf("sec: %ld hello from navy\n", tv.tv_sec);
      sec += 1;
    }
  }
  return 0;
}
