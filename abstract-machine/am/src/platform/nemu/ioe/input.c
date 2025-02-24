#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t am_scancode = (uint32_t)inl(KBD_ADDR);
  kbd->keydown = ((am_scancode & KEYDOWN_MASK)?true:false);
  kbd->keycode = (am_scancode ? (am_scancode & 0xfff) : AM_KEY_NONE);
}
