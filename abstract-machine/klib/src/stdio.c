#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char buf[512];
  int count = vsprintf(buf, fmt, ap);
  va_end(ap);
  return count; 
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int count = vsprintf(out, fmt, ap);
  va_end(ap);
  return count; 
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  int count = 0;
  while(*fmt)
  {
    if(*fmt == '%')
    {
      char c, ch;
      int width = 0;
      int pad_zero = 0;
      int d;
      int i;
      unsigned int u;
      int ifbreak = 1;
      char *s;
      char str[12];
      while(ifbreak) {
	c = *(++fmt);
	switch(c) {
	case '0':
	  pad_zero = 1;
	  i = 0;
	  while((*(fmt + 1)>'0')&&(*(fmt + 1)<='9')){str[i++]=*(++fmt);}
	  str[i] = '\0';
	  width = atoi(str);
	  break;	
        case 'u':
	  u = (unsigned int)va_arg(ap,int);
	  i = 0;
	  while(u!=0){str[i++] = (u%10)+'0';u/=10;}
	  if(pad_zero){
	    int num_zero = width - i;
	    while(num_zero-- > 0){out[count++]='0';}
	  }
	  while(i>0){out[count++]=str[--i];}
	  ifbreak = 0;
	  break;
	case 'c':
	  ch = va_arg(ap, int);
	  out[count++]=ch;
	  ifbreak = 0;
	  break;
	case 'x':
	case 'd':
	  d = va_arg(ap,int);
	  i = 0;
	  while(d!=0){str[i++] = (d%10)+'0';d/=10;}
	  if(pad_zero){
	    int num_zero = width - i;
	    while(num_zero-->0)out[count++]='0';
	  }
	  while(i>0){out[count++]=str[--i];}
	  ifbreak = 0;
	  break;
	case 's':
	  s = va_arg(ap, char *);
	  while(*s){out[count++]=*s++;}
	  ifbreak = 0;
	  break;
	}
      } 
    }
    else
    {
      out[count++] = *fmt;
    } 
    fmt ++;
  }
  out[count] = '\0';
  return count;

}
int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
