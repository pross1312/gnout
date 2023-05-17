#pragma once
#include "Vec.h"

#ifndef USE_SDL_RENDERER
inline const char* shader_files[] {
    "./shaders/vShader.glsl",
    "./shaders/fShader.glsl"
};
inline const GLenum shader_types[] {
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER       
};
inline const size_t n_shaders = 2;
static_assert(sizeof(shader_files) / sizeof(shader_files[0]) == sizeof(shader_types) / sizeof(shader_types[0]));
static_assert(n_shaders == sizeof(shader_files) / sizeof(shader_files[0]));

struct UV_cache {
    Vec2i uv;
    int width;  
};

struct Glyph {
    Vec2f uv;
    Vec2f uv_size;
    Vec2f pos;
    Vec2f size;
    Vec4f fg;
    Vec4f bg;
};

enum Glyph_Attr {
    GLYPH_ATTR_UV = 0,
    GLYPH_ATTR_UV_SIZE,
    GLYPH_ATTR_POS,
    GLYPH_ATTR_SIZE,
    GLYPH_ATTR_FG,
    GLYPH_ATTR_BG,
    COUNT_GLYPH_ATTR,
};

struct Glyph_Defs {
    size_t offset;
    size_t comps;
};

inline const Glyph_Defs glyph_attr_defs[COUNT_GLYPH_ATTR] {
    {.offset = offsetof(Glyph, uv), .comps = 2},
    {.offset = offsetof(Glyph, uv_size), .comps = 2},
    {.offset = offsetof(Glyph, pos), .comps = 2},
    {.offset = offsetof(Glyph, size), .comps = 2},
    {.offset = offsetof(Glyph, fg), .comps = 4},
    {.offset = offsetof(Glyph, bg), .comps = 4},
};
static_assert(COUNT_GLYPH_ATTR == 6 && "The amound of glyph attribute has changed");


#define BUFFER_INIT_CAP (1000 * 512)



class EditorRenderer
{
public:
    EditorRenderer(const char* font_path, int size);
    ~EditorRenderer();
    void render_text(const char* text, Vec2f pos, Vec4f fg, Vec4f bg);
    void render(const Editor* editor, float time);
    void render_cursor(Vec2f pos);
    void init_font_cache(const char* font_path, int font_size);
    void clear_buffer();
    void sync_buffer();
    void draw_buffer();
    

private:
    GLuint program;
    GLuint vao;
    GLuint vbo;

    inline static Glyph buffers[BUFFER_INIT_CAP] {};
    size_t buffer_count = 0;

    Vec2f camera;

    TTF_Font* font;
    GLuint cache_font_texture; 
    Vec2i cache_font_size;

    // store uv offset and width, in order
    // height will be the same for all chars which is store in size variable
    // offset is in pixel format ( from  0 to SCREEN_WIDTH or some things like that ), size in pixel too
    // so when pass to shader, divide them properly
    UV_cache uv_pixel_cache[128];
};

#else
class EditorRenderer
{
public:
    EditorRenderer(const char* font_path, int size);
    ~EditorRenderer();
    void render_line(SDL_Renderer* renderer, const char* text, Vec2f pos, SDL_Color color);
    void render(SDL_Renderer* renderer, const Editor* editor);
    void render_cursor(SDL_Renderer* renderer, Vec2f pos, SDL_Color color); 
    

private:
    size_t char_width, line_height;
    Vec2f text_origin;
    TTF_Font* font;
    SDL_Color text_color;
};

#endif
