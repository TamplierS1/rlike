#include "vec.h"

Vec2 vec2(int x, int y)
{
    Vec2 vector = {x, y};
    return vector;
}

Vec2 vec_add(Vec2 vec1, Vec2 vec2)
{
    Vec2 result = {vec1.x + vec2.x, vec1.y + vec2.y};
    return result;
}

Vec2 vec_sub(Vec2 vec1, Vec2 vec2)
{
    Vec2 result = {vec1.x - vec2.x, vec1.y - vec2.y};
    return result;
}
