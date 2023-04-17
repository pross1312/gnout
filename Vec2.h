#pragma once



typedef struct
{
    float x, y;
} Vec2f;


Vec2f vec2f(float x, float y);
Vec2f addVec(Vec2f a, Vec2f b);
Vec2f mulVec(Vec2f a, Vec2f b);
Vec2f subVec(Vec2f a, Vec2f b);
