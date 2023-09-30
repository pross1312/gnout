#include "Vec.h"
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
Vec2f divVec(Vec2f a, Vec2f b)
{
    return Vec2f {
        .x = a.x / b.x,
        .y = a.y / b.y
    };
}
Vec2i vec2i(int x, int y)
{
    Vec2i result = {
        .x = x,
        .y = y
    };
    return result;
}
Vec2i addVec(Vec2i a, Vec2i b)
{
    return Vec2i {
        .x = a.x + b.x,
        .y = a.y + b.y
    };
}
Vec2i mulVec(Vec2i a, Vec2i b)
{
    return Vec2i {
        .x = a.x * b.x,
        .y = a.y * b.y
    };
}
Vec2i subVec(Vec2i a, Vec2i b)
{
    return Vec2i {
        .x = a.x - b.x,
        .y = a.y - b.y
    };
}
Vec4f div(const Vec4f& a, float scalar) {
    return Vec4f {
        .x = a.x / scalar,
        .y = a.y / scalar,
        .z = a.z / scalar,
        .w = a.w / scalar,
    };
}