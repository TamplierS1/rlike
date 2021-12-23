#include "error.h"

void err_error(const char* file, const char* function, int line, const char* format, ...)
{
    fprintf(stderr, "\n[line %d] Error in %s (func %s): ", line, file, function);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "\n");
}

void err_fatal(const char* file, const char* function, int line, const char* format, ...)
{
    fprintf(stderr, "\n[line %d] Fatal error in %s (func %s): ", line, file, function);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "\n");

    exit(EXIT_FAILURE);
}
