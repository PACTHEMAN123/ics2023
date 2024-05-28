#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;

uint32_t NDL_GetTicks() {
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  return tv.tv_usec;
}

int NDL_PollEvent(char *buf, int len) {
  assert(evtdev != -1);
  buf[0] = '\0';  
  return read(evtdev, buf, len);
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
  FILE *fb_fp = fopen("/proc/dispinfo", "r");
  fscanf(fb_fp, "WIDTH : %d\nHEIGHT: %d\n", w, h);
  screen_w = *w;
  screen_h = *h;
  //printf("%d %d", *w, *h);
  fclose(fb_fp);


}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  for(int i = 0; i < h; i++) {
    lseek(fbdev, x + (y + i) * screen_w, SEEK_SET);
    write(fbdev, pixels + i * w, w); 
  } 
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  evtdev = open("/dev/events", 0, 0);
  fbdev = open("/dev/fb", 0, 0);
  return 0;
}

void NDL_Quit() {
}
