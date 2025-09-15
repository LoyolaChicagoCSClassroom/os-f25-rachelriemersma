#ifndef RPRINTF_H
#define RPRINTF_H

#include <stdarg.h>

typedef void (*func_ptr)(int);
typedef char* charptr;

void esp_printf(const func_ptr f_ptr, charptr ctrl, ...);

#endif 

