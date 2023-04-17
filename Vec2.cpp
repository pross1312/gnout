#include "Vec2.h"




Vec2f vec2f(float x, float y)
{
    Vec2f result = {
        .x = x,
        .y = y
    };
    return result;
}

Vec2f addVec(Vec2f a, Vec2f b)
{
    return Vec2f {
        .x = a.x + b.x,
        .y = a.y + b.y
    };
}
Vec2f mulVec(Vec2f a, Vec2f b)
{
    return Vec2f {
        .x = a.x * b.x,
        .y = a.y * b.y
    };
}
Vec2f subVec(Vec2f a, Vec2f b)
{
    return Vec2f {
        .x = a.x - b.x,
        .y = a.y - b.y
    };
}