#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>

#ifndef NDEBUG
#define DEBUG_ERROR(msg)                                                            \
    do                                                                              \
    {                                                                               \
        fprintf(stderr, "\nIn file: %s\n[line %d] Error: %s\n", __FILE__, __LINE__, \
                (msg));                                                             \
        exit(1);                                                                    \
    } while (0)
#else
#define DEBUG_ERROR(msg)
#endif

#endif // ERROR_H
