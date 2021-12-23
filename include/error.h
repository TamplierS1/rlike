#ifndef ERROR_H
#define ERROR_H

#include <stdlib.h>
#include <stdio.h>

#include <libtcod.h>

typedef enum
{
    ERROR_JSON_PARSING,
    ERROR_JSON_DESERIALIZE,
    ERROR_FILE_IO,
    OK
} Error;

void err_error(const char* file, const char* function, int line, const char* format, ...);
void err_fatal(const char* file, const char* function, int line, const char* format, ...);

#endif  // ERROR_H
