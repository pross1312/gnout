#include "Editor.h"
#include "EditorRenderer.h"
#include "Utils.h"
#include <assert.h>



// this will rendered text at a specific postiion
// use it to draw editor text
#ifdef USE_SDL_RENDERER
inline void drawString(SDL_Renderer* renderer, TTF_Font* font, const char* text, Vec2f pos, SDL_Color fg, SDL_Color bg)
{
    SDL_Surface* text_surface = check(TTF_RenderText(font, text, fg, bg));
    SDL_Texture* text_texture = check(SDL_CreateTextureFromSurface(renderer, text_surface));
    SDL_FRect dst = {
        .x = pos.x,
        .y = pos.y,
        .w = (float)text_surface->w,
        .h = (float)text_surface->h
    };
    check(SDL_RenderCopyF(renderer, text_texture, NULL, &dst));
    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text_surface);
}

void Editor::draw_cursor(SDL_Renderer* renderer)
{
    assert(cursor_row < lines.size() && cursor_col <= lines[cursor_row].char_count);
    SDL_FRect cursor{
        .x = char_width * (float)cursor_col - text_origin.x,
        .y = line_height * (float)cursor_row - text_origin.y,
        .w = (float)char_width,
        .h = (float)line_height
    };


    // if cursor is in middle of line then draw char at cursor with cursor_color background and black fore ground
    if (cursor_col < lines[cursor_row].char_count) {
        char temp[2]{ lines[cursor_row].chars[cursor_col] };
        drawString(renderer, font, temp, vec2f(cursor.x, cursor.y), SDL_Color{ UNHEX(0x000000ff) }, cursor_color);
    }
    // else draw a rectangle at cursor position
    else {
        check(SDL_SetRenderDrawColor(renderer, cursor_color.r, cursor_color.g, cursor_color.b, cursor_color.a));
        check(SDL_RenderFillRectF(renderer, &cursor));
    }
}

void Editor::draw(SDL_Renderer* renderer)
{
    // do nothing if there's no lines to draw
    if (lines.size() == 0)
        return;
    // don't draw anything that won't be show
    for (size_t row = text_origin.y / line_height; row < lines.size(); row++) {
        if (lines[row].char_count > 0) {
            // draw text
            drawString(renderer, font, lines[row].chars.data(), subVec(vec2f(0, row * line_height), text_origin),
                       text_color, SDL_Color{ UNHEX(0x0) });
        }
        // don't draw anything that won't be shown
        if (row * line_height - text_origin.y >= SCREEN_HEIGHT)
            break;
    }
    draw_cursor(renderer);
}
#else // USE_OPENGL_RENDERER

void EditorRenderer::render_line(const char* text, Vec2f pos, SDL_Color color) {
    
    size_t n = strlen(text);
    float w = 0.0f;
    for (size_t i = 0; i < n; i++) {
        int c_width = uv_pixel_cache[(int)text[i]].second;
        buffers[buffer_count + i].uv = {
            .x = (float)uv_pixel_cache[(int)text[i]].first.x / cache_font_size.x,
            .y = 0
        }; 
        buffers[buffer_count + i].uv_size = {
            .x = (float)c_width / cache_font_size.x,
            .y = 1
        };
         
        buffers[buffer_count + i].pos = {
            .x = (pos.x + w) * 2.0f / SCREEN_WIDTH,
            .y = pos.y       * 2.0f / SCREEN_HEIGHT
        };
        buffers[buffer_count + i].size = {
            .x = c_width * 2.0f / SCREEN_WIDTH,
            .y = cache_font_size.y  * 2.0f / SCREEN_HEIGHT
        };
        buffers[buffer_count + i].fg = {
            .x = color.r / 255.0f,
            .y = color.g / 255.0f,
            .z = color.b / 255.0f,
            .w = color.a / 255.0f
        };
        w += c_width;
    }
    glBufferData(GL_ARRAY_BUFFER, 
                 sizeof(buffers),
                 buffers,
                 GL_DYNAMIC_DRAW);

    


    buffer_count += n; 
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cache_font_texture);

}
void EditorRenderer::render(const Editor* editor) {
    (void)editor;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cache_font_texture);

    render_line("Editor render", vec2f(.0f, .0f), SDL_Color {UNHEX(0x1da2d3ff)});
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, buffer_count);

}
void EditorRenderer::render_cursor(Vec2f pos) {
    assert(false && "unimplemented");
    (void)pos; 
}

void EditorRenderer::init_font_cache(const char* font_path, int font_size) {
    cache_font_size = {0, 0};
    font = check(TTF_OpenFont(font_path, font_size));
    for (size_t i = 32; i < 128; i++) {
        int advance, minx, maxx, miny, maxy, h;
        check(TTF_GlyphMetrics32(font, i, &minx, &maxx, &miny, &maxy, &advance));
        h = TTF_FontHeight(font); 
        uv_pixel_cache[i].first = {
            .x = cache_font_size.x,
            .y = 0
        };
        uv_pixel_cache[i].second = advance;
        cache_font_size.x += advance;
        if (h > cache_font_size.y) {
            cache_font_size.y = h;
        }
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
            (GLint)uv_pixel_cache[i].first.x,
            (GLint)uv_pixel_cache[i].first.y,
            (GLsizei)RGBA_surface->w,
            (GLsizei)RGBA_surface->h,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            RGBA_surface->pixels);
        SDL_FreeSurface(RGBA_surface);
    }

}

EditorRenderer::EditorRenderer(const char* font_path, int font_size) {
    program = create_program(shader_files, shader_types, n_shaders);
    glUseProgram(program);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    init_font_cache(font_path, font_size);
    glBindTexture(GL_TEXTURE_2D, 0); 
    GLuint textureLoc = glGetUniformLocation(program, "renderingTexture");
    glUniform1i(textureLoc, 0);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
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
    
}

EditorRenderer::~EditorRenderer() {
    TTF_CloseFont(font);
}

#endif // USE_SDL_RENDERER


