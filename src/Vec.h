#pragma once



typedef struct
{
    float x, y;
} Vec2f;


Vec2f vec2f(float x, float y);
Vec2f addVec(Vec2f a, Vec2f b);
Vec2f mulVec(Vec2f a, Vec2f b);
Vec2f subVec(Vec2f a, Vec2f b);
Vec2f divVec(Vec2f a, Vec2f b);


typedef struct
{
    int x, y;
} Vec2i;


Vec2i vec2i(int x, int y);
Vec2i addVec(Vec2i a, Vec2i b);
Vec2i mulVec(Vec2i a, Vec2i b);
Vec2i subVec(Vec2i a, Vec2i b);

typedef struct
{
    float x, y, z, w;
} Vec4f;
Vec4f div(const Vec4f& a, float scalar);
