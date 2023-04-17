#pragma once
#include "Vec2.h"



// seperate an 32 bits integer into 4 8 bits color (r g b a)
#define UNHEX(c) (uint8_t)((c) >> 8 * 3) & (uint8_t)0xff\
                ,(uint8_t)((c) >> 8 * 2) & (uint8_t)0xff\
                ,(uint8_t)((c) >> 8)     & (uint8_t)0xff\
                ,(uint8_t)((c))          & (uint8_t)0xff

// check for error code
inline void check(int code)
{
    if (code != 0) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }
}

// check for pointer
inline void* check(void* data)
{
    if (!data) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }
    return data;
}
