#include "Editor.h"
#include "EditorRenderer.h"
#include "Utils.h"
#include <assert.h>

inline Vec2f project_GL(Vec2f a) {
    return Vec2f {
        .x = a.x * 2.0f / SCREEN_WIDTH,
        .y = a.y * 2.0f / SCREEN_HEIGHT
    };
}

void EditorRenderer::render_text(const char* text, Vec2f pos, Vec4f fg, Vec4f bg) {
    size_t n = strlen(text);
    for (size_t i = 0; i < n; i++) {
        Vec4f back = bg;
        Vec4f fore = fg;
        int ch = (int)text[i];
        float c_width = (float)uv_pixel_cache[ch].width;
        buffers[buffer_count + i].uv = {
            .x = (float)uv_pixel_cache[ch].uv.x / cache_font_size.x,
            .y = 0
        }; 
        buffers[buffer_count + i].uv_size = {
            .x = (float)c_width / cache_font_size.x,
            .y = 1
        };
         
        buffers[buffer_count + i].pos = project_GL(pos);
        buffers[buffer_count + i].size = project_GL(Vec2f{c_width, (float)cache_font_size.y});
        buffers[buffer_count + i].fg = div(fore, 255.0f);
        buffers[buffer_count + i].bg = div(back, 255.0f);
        pos.x += c_width;
    }
    buffer_count += n; 
}

void EditorRenderer::render(const Editor* editor, float time) {
    assert(editor);
    glBindVertexArray(vao);
    
    GLuint timeLoc = glGetUniformLocation(program, "time");
    glUniform1f(timeLoc, time * 2.0f);

    Vec2f pos {-SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f}; // init at top left screen
    clear_buffer();
    for (const Editor::Line& line : editor->lines) {
        render_text(line.chars.data(), pos, Vec4f {UNHEX(0xffffffff)}, Vec4f {UNHEX(0x0)});
        pos.y -= cache_font_size.y; // opengl y axis point up
    }
    sync_buffer();
    draw_buffer();
    if (checkOpenGLError()) exit(1);
}

void EditorRenderer::sync_buffer() {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    sizeof(buffers),
                    buffers);
}

void EditorRenderer::draw_buffer() {
    glBindTexture(GL_TEXTURE_2D, cache_font_texture);
    glClearColor(.0f, .0f, .0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, buffer_count);
}

void EditorRenderer::clear_buffer() {
    buffer_count = 0;
}

void EditorRenderer::render_cursor(Vec2f pos) {
    assert(false && "unimplemented");
    (void)pos; 
}

void EditorRenderer::init_font_cache(const char* font_path, int font_size) {
    cache_font_size = {0, 0};
    font = check(TTF_OpenFont(font_path, font_size));
    for (size_t i = 32; i < 128; i++) {
        char temp[2] = {(char)i, 0};
        SDL_Surface* BGRA_surface = check(TTF_RenderText_Blended(font, temp, SDL_Color{UNHEX(0xffffffff)}));
        SDL_Surface* RGBA_surface = check(SDL_ConvertSurfaceFormat(BGRA_surface, SDL_PIXELFORMAT_RGBA32, 0));
        SDL_FreeSurface(BGRA_surface);
        
        uv_pixel_cache[i].uv = {
            .x = cache_font_size.x,
            .y = 0
        };
        uv_pixel_cache[i].width = RGBA_surface->w;
        cache_font_size.x += RGBA_surface->w;
        if (RGBA_surface->h > cache_font_size.y) {
            cache_font_size.y = RGBA_surface->h;
        }
        SDL_FreeSurface(RGBA_surface);
    }

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &cache_font_texture);
    glBindTexture(GL_TEXTURE_2D, cache_font_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 cache_font_size.x,
                 cache_font_size.y,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 NULL);
    for (size_t i = 32; i < 128; i++) {
        char temp[2] = {(char)i, 0};
        SDL_Surface* BGRA_surface = check(TTF_RenderText_Blended(font, temp, SDL_Color{UNHEX(0xffffffff)}));
        SDL_Surface* RGBA_surface = check(SDL_ConvertSurfaceFormat(BGRA_surface, SDL_PIXELFORMAT_RGBA32, 0));
        SDL_FreeSurface(BGRA_surface);

        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            (GLint)uv_pixel_cache[i].uv.x,
            (GLint)uv_pixel_cache[i].uv.y,
            (GLsizei)RGBA_surface->w,
            (GLsizei)RGBA_surface->h,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            RGBA_surface->pixels);
        SDL_FreeSurface(RGBA_surface);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}


EditorRenderer::EditorRenderer(const char* font_path, int font_size)
    : program {}, vao {}, vbo {}, buffer_count {}, camera {0, 0}, font {}, cache_font_texture {}, cache_font_size {}, uv_pixel_cache {}
{
    program = create_program(shader_files, shader_types, n_shaders);
    glUseProgram(program);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (checkOpenGLError()) exit(6);
    init_font_cache(font_path, font_size);
    if (checkOpenGLError()) exit(7);

    GLuint textureLoc = glGetUniformLocation(program, "cache_font_texture");
    glUniform1i(textureLoc, 0);

    if (checkOpenGLError()) exit(5);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 
                 sizeof(buffers),
                 NULL,
                 GL_DYNAMIC_DRAW);
    if (checkOpenGLError()) exit(4);
    for (size_t attr = 0; attr < COUNT_GLYPH_ATTR; attr++) {
        glEnableVertexAttribArray(attr);
        glVertexAttribPointer(
            attr,
            glyph_attr_defs[attr].comps,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Glyph),
            (void*)glyph_attr_defs[attr].offset);
        glVertexAttribDivisor(attr, 1); // modify the rate at which generic vertex attributes advance during instanced rendering
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    if (checkOpenGLError()) exit(2);
}

EditorRenderer::~EditorRenderer() {
    TTF_CloseFont(font);
}

