#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#define MAX_INT_STR_LEN 100

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int uint_to_str(unsigned int n, char * str)
{
  char tmp[MAX_INT_STR_LEN];
  int i = 0;
  while(n > 0)
  {
    tmp[i ++] = n % 10 + '0';
    n /= 10;
  }
  int j = 0;
  while(i > 0)
  {
    str[j++] = tmp[--i];
  }
  str[j] = '\0';
  return j;
}
int int_to_str(int n, char * str)
{
  if(n > 0)
    return uint_to_str(n, str);
  char tmp[MAX_INT_STR_LEN];
  int i = 0;
  while(n != 0)
  {
    tmp[i++] = - (n % 10) + '0';
    n /= 10;
  }
  str[0] = '-';
  int j = 1;
  while(i > 0)
  {
    str[j++] = tmp[--i];
  }
  str[j] = '\0';
  return j;
}
int printf(const char *fmt, ...) {
  va_list list;
  va_start(list, fmt);
  const char * cur = fmt;
  int n = 0;
  while(*cur != '\0')
  {
    if(*cur == '%')
    {
      cur ++;
      if(*cur == 'd')
      {
        int i = va_arg(list, int);
        char str[MAX_INT_STR_LEN];
        int len = int_to_str(i, str);
        putstr(str);
        n += len;
        cur ++;
      }
      else if(*cur == 's')
      {
        char* s = va_arg(list, char*);
        putstr(s);
        n += strlen(s);
        cur ++;
      }
      else
      {
        char error[50] = "__ is not supported now!";
        error[0] = '%';
        error[1] = *cur;
        panic(error);
      }
    }
    else
    {
      putch(*cur);
      n ++;
      cur ++;
    }
  }
  va_end(list);
  return n;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
