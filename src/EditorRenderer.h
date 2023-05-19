#pragma once
#include "Vec.h"



inline const char* shader_files[] {
    "./shaders/vShader.glsl",
    "./shaders/fShader.glsl"
};
inline constexpr GLenum shader_types[] {
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER       
};
#define SCROLL_SPEED 10
#define SCROLL_SENSITIVITY 50

inline constexpr const size_t n_shaders = sizeof(shader_files) / sizeof(shader_files[0]);
static_assert(n_shaders == sizeof(shader_types) / sizeof(shader_types[0]));

struct UV_cache {
    Vec2f uv;
    float width;  
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
    void render_cursor(const Editor* editor, float time);
    void set_cursor_to_mouse(Editor* editor, Vec2f mousePos);
    void init_font_cache(const char* font_path, int font_size);
    void push_buffer(Glyph glyph);
    void clear_buffer();
    void sync_buffer();
    void draw_buffer();
    void moveCamera(float delta);
    void addVel(Vec2f vel);

private:
    inline static Vec4f cursor_color { div(Vec4f {UNHEX(float, 0x888888ff)}, 255.0f) };
    inline static Vec4f text_on_cursor { div(Vec4f {UNHEX(float, 0xff)}, 255.0f) };
    inline static constexpr float cursor_draw_interval = .85; // draw/notdraw times in second
    
private:
    Vec2f camera;
    Vec2f camVelocity;

    bool onMoveInterpolated = true;
    GLuint program;
    GLuint vao;
    GLuint vbo;

    inline static Glyph buffers[BUFFER_INIT_CAP] {};
    size_t buffer_count;

    TTF_Font* font;
    GLuint cache_font_texture; 
    Vec2f cache_font_size;

    // store uv offset and width, in order
    // height will be the same for all chars which is store in size variable
    // offset is in pixel format ( from  0 to SCREEN_WIDTH or some things like that ), size in pixel too
    // so when pass to shader, divide them properly
    UV_cache uv_pixel_cache[128];
};
