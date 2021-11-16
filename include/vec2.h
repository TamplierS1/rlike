#ifndef VEC2_H
#define VEC_H

#include <stdbool.h>

typedef struct
{
    int x;
    int y;
} Vec2;

Vec2 vec2(int x, int y);
Vec2 vec2_add(Vec2 vec1, Vec2 vec2);
Vec2 vec2_add_int(Vec2 vec1, int num);
Vec2 vec2_sub(Vec2 vec1, Vec2 vec2);
bool vec2_equals(Vec2 vec1, Vec2 vec2);

#endif  // VEC2_H
