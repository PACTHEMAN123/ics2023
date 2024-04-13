#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  int d;
  char *s;
  char str[12];  
  
  va_start(ap, fmt);
  while (*fmt) {
    if(*fmt == '%') {
      fmt ++;
      if(*fmt == 'd') {
	// demical number operation
	d = va_arg(ap, int);
	// int to string
	int i = 0;
	while (d != 0) {
	  str[i++] = (d % 10) + '0';
	  d /= 10;
	}
	while (i > 0) { *out++ = str[--i]; }
      } 
      else if(*fmt == 's') {
	// string operation
	s = va_arg(ap, char *);
	while(*s) {
	  *out++ = *s++ ;	
	}
      }
      else {
	fmt --; 
      }
    }
    else {
      *out++ = *fmt++ ;
    }
  }
  *out = '\0';
  va_end(ap);
  return strlen(out); 
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
