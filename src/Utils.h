#pragma once

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <string>
#include <fstream>
#include <sstream>
#include "Vec.h"


// initial screen size
inline int SCREEN_WIDTH = 1500;    
inline int SCREEN_HEIGHT = 1000;
// seperate an 32 bits integer into 4 8 bits color (r g b a)
#define UNHEX(type, c) (type)((uint8_t)((c) >> 8 * 3) & (uint8_t)0xff)\
                      ,(type)((uint8_t)((c) >> 8 * 2) & (uint8_t)0xff)\
                      ,(type)((uint8_t)((c) >> 8)     & (uint8_t)0xff)\
                      ,(type)((uint8_t)((c))          & (uint8_t)0xff)

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
template<typename T>
inline T check(T data)
{
    if (!data) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }
    return data;
}
std::string readAllFile(const char* filepath);

void printShaderLog(GLuint shader);
void printProgramLog(int prog);
bool checkOpenGLError();
void MessageCallback(GLenum source,
                     GLenum type,
                     GLuint id,
                     GLenum severity,
                     GLsizei length,
                     const GLchar* message,
                     const void* userParam);
GLuint create_compiled_shader(const char* source, GLenum type);
GLuint create_program(const char** src, const GLenum* type, size_t n);

inline float squareDistance(Vec2f a, Vec2f b) {
    float t1 = b.x - a.x;
    float t2 = b.y - a.y;
    return t1 * t1 + t2 * t2;
}
inline Vec2f lerp(Vec2f a, Vec2f b, float t) {
    if (t <= .0f) return a;
    if (t >= 1.0f) return b;
    return vec2f(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t); 
}
inline Vec2f normalized(Vec2f x) {
    float len = x.x * x.x + x.y * x.y;
    return vec2f(x.x / sqrt(len), x.y / sqrt(len));
}
