#ifndef ERROR_H
#define ERROR_H

#include <stdlib.h>
#include <stdio.h>

#include <libtcod.h>

void fatal(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(EXIT_FAILURE);
}
#endif  // ERROR_H
