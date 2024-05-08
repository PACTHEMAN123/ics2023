#include <am.h>
#include <nemu.h>
#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  int i;
  int h = (int)(inl(VGACTL_ADDR) >> 16);
  int w = (int)((inl(VGACTL_ADDR) << 16) >> 16);
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (i = 0; i < w * h; i++) fb[i] = i;
  outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = inw(VGACTL_ADDR + 2), .height = inw(VGACTL_ADDR),
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int i, j;
  int W = inw(VGACTL_ADDR + 2);
  int x = ctl->x;
  int y = ctl->y;
  int w = ctl->w;
  int h = ctl->h;
  uint32_t *p = (uint32_t *)(ctl->pixels);
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for(i = y; i < y + h; i++) {
    for(j = x; j < x + w; j++) {
      uint32_t px = *(p + w * (i-y) + (j-x));
      *(fb + j + i * W) = px;
    }
  } 
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
