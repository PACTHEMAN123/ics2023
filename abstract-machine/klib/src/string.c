#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;
  size_t i;
  //printf("exec strlen");
  for ( i = 0 ; s[i] !='\0'; i++) {
    len ++; 
  }
  return len;
}

char *strcpy(char *dst, const char *src) {
  size_t i;
  //printf("exec strcpy");
  for (i = 0; src[i] != '\0'; i++)
    dst[i] = src[i];
  dst[i] = '\0';
  
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i;
  //printf("exec strncpy");
  for (i = 0; i < n && src[i] != '\0'; i++)
      dst[i] = src[i];
  for ( ; i < n; i++)
      dst[i] = '\0';

  return dst;
}

char *strcat(char *dst, const char *src) {
  size_t dst_len = strlen(dst);
  size_t i;
  //printf("exec strcat");
  for (i = 0 ; src[i] != '\0' ; i++)
      dst[dst_len + i] = src[i];
  dst[dst_len + i] = '\0';

  return dst;
}

int strcmp(const char *s1, const char *s2) {
  const unsigned char *p1 = (const unsigned char *) s1;
  const unsigned char *p2 = (const unsigned char *) s2;
  unsigned char c1, c2;
  //printf("exec strcmp\n");
  do{
    c1 = (unsigned char) *p1++;
    c2 = (unsigned char) *p2++;
    if (c1 == '\0'){ return c1 - c2; }
  }
  while (c1 == c2);

  return c1 - c2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  const unsigned char *p1 = (const unsigned char *) s1;
  const unsigned char *p2 = (const unsigned char *) s2;
  unsigned char c1, c2;
  //printf("exec strncmp");
  do{
    c1 = (unsigned char) *p1++;
    c2 = (unsigned char) *p2++;
  }
  while (c1 == c2 && --n > 0);

  return c1 - c2;

}

void *memset(void *s, int c, size_t n) {
  char bt = (char)c;
  char *ptr = (char *)s;
  size_t i;
  //printf("exec memset");
  for ( i = 0 ; i < n ; i++ ) {
    ptr[i] = bt;
  }
  return s; 
}

void *memmove(void *dst, const void *src, size_t n) {
  //printf("exec memmove");
  const char *psrc = (const char *)src;
  char *pdst = (char *)dst;
  if (pdst < psrc) {
    while (n--)
      *pdst++ = *psrc++;
  }
  else {
    const char *lasts = psrc + (n - 1);
    char *lastd = pdst + (n - 1);
    while (n--)
      *lastd-- = *lasts--;
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  //printf("exec memcpy");
  char *pd = (char *)out;
  const char *ps = (const char *)in;
  while (n--)
    *pd++ = *ps++;
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;
  unsigned char c1, c2; 
  while (n--) {
    c1 = *p1++;
    c2 = *p2++;
    if (c1 != c2) { return c1 - c2; } 
  }
  return 0;
}

#endif
