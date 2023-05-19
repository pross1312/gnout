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

// cursor is just a special glyph and it will be put at the end of buffer
void EditorRenderer::render_cursor(const Editor* editor, float time) {
    static size_t old_row = editor->cursor_row, old_col = editor->cursor_col;
    static float next_change = time + cursor_draw_interval;
    static bool onDraw = true;
    if (editor->lines.size() == 0)
        return;
    Glyph cursor {};
    size_t row = editor->cursor_row;
    size_t col = editor->cursor_col;
    int ch = (int)'_';
    float c_width = uv_pixel_cache[ch].width;
    Vec4f color = onDraw ? cursor_color : Vec4f {.0f, .0f, .0f, .0f};
    if (time >= next_change) {
        color = !onDraw ? cursor_color : Vec4f {.0f, .0f, .0f, .0f};
        next_change = time + cursor_draw_interval;
        onDraw = !onDraw;
    }
    Vec2f cursor_pos {0, -cache_font_size.y * row};
    for (size_t c = 0; c < col; c++) {
        cursor_pos.x += uv_pixel_cache[(int)editor->lines[row][c]].width;
    }
    // if cursor is in the middle of the line, render a | instead of block
    if (editor->lines[row].char_count != col) {         
        c_width *= .30;
    }
    if (old_row != row || old_col != col) {
        // don't change when cursor is moving
        color = cursor_color;
        onDraw = true;
        next_change = time + cursor_draw_interval;
        onMoveInterpolated = false; // temporary disable it, after moving camera then enable it again
        old_row = row;
        old_col = col;
        float dx = 0.0f;
        float dy = 0.0f;
        if (abs(cursor_pos.x - camera.x) > SCREEN_WIDTH / 2.0f) {
            dx = abs(cursor_pos.x - camera.x) - SCREEN_WIDTH / 2.0f;
            dx *= (cursor_pos.x > camera.x ? 1.0f : -1.0f);
        } 
        if (abs(cursor_pos.y - camera.y) + (cursor_pos.y > camera.y ? 0 : cache_font_size.y) > SCREEN_HEIGHT / 2.0f) {
            dy = abs(cursor_pos.y - camera.y) + (cursor_pos.y > camera.y ? 0 : cache_font_size.y) - SCREEN_HEIGHT / 2.0f;
            dy *= (cursor_pos.y > camera.y ? 1.0f : -1.0f);
        }
        addVel(vec2f(dx, dy));
    }
    cursor_pos = subVec(cursor_pos, camera);
    cursor = {
        .uv      = divVec(uv_pixel_cache[ch].uv, cache_font_size),
        .uv_size = vec2f(c_width / cache_font_size.x, 1),
        .pos     = project_GL(cursor_pos),
        .size    = project_GL(Vec2f{c_width, (float)cache_font_size.y}),
        .fg      = color,
        .bg      = color,
    };
    push_buffer(cursor);
}

void EditorRenderer::addVel(Vec2f vel) {
    camVelocity = addVec(camVelocity, vel);
}

void EditorRenderer::moveCamera(float delta) {
    if (camVelocity.x == .0f && camVelocity.y == .0f)
        return;
    if (abs(camVelocity.x) < 1 && abs(camVelocity.y) < 1) {
        camVelocity.x = camVelocity.y = .0f;
    }
    else {
        if (!onMoveInterpolated) {
            camera = addVec(camera, camVelocity);
            camVelocity = vec2f(.0f, .0f);
            onMoveInterpolated = true;
        }
        else {
            // Vec2f normalizedVel = normalized(camVelocity);
            camera.x += camVelocity.x * delta * SCROLL_SPEED;
            camera.y += camVelocity.y * delta * SCROLL_SPEED;
            camVelocity.x -= camVelocity.x * delta * SCROLL_SPEED;
            camVelocity.y -= camVelocity.y * delta * SCROLL_SPEED;

        }
    }
}

void EditorRenderer::set_cursor_to_mouse(Editor* editor, Vec2f mousePos) {
    if (editor->lines.size() == 0) {
        editor->new_line();
    }
    else {
        mousePos.x += (camera.x - SCREEN_WIDTH / 2.0f);
        mousePos.y -= (camera.y + SCREEN_HEIGHT / 2.0f);
        if (mousePos.y < 0) mousePos.y = 0;
        if (mousePos.x < 0) mousePos.x = 0;
        size_t row = size_t(mousePos.y / cache_font_size.y);
        size_t col = 0;
        float w = .0f;
        for (col = 0; col < editor->lines[row].char_count; col++) {
            w += uv_pixel_cache[(int)editor->lines[row][col]].width;
            if (mousePos.x < w) break;
        }
        editor->set_cursor(row, col);
    }
}
void EditorRenderer::render_text(const char* text, Vec2f pos, Vec4f fg, Vec4f bg) {
    size_t n = strlen(text);
    for (size_t i = 0; i < n; i++) {
        Vec4f back = bg;
        Vec4f fore = fg;
        int ch = (int)text[i];
        float c_width = uv_pixel_cache[ch].width;
        push_buffer(Glyph {
            .uv      = divVec(uv_pixel_cache[ch].uv, cache_font_size),
            .uv_size = vec2f(c_width / cache_font_size.x, 1),
            .pos     = project_GL(pos),
            .size    = project_GL(Vec2f{c_width, (float)cache_font_size.y}),
            .fg      = div(fore, 255.0f),
            .bg      = div(back, 255.0f),
        });
        pos.x += c_width;
    }
}


void EditorRenderer::render(const Editor* editor, float time) {
    assert(editor);
    glBindVertexArray(vao);
    
    GLuint timeLoc = glGetUniformLocation(program, "time");
    glUniform1f(timeLoc, time * 2.0f);

    Vec2f pos {- camera.x, - camera.y}; // init at top left screen
    clear_buffer();
    for (size_t i = 0; i < editor->lines.size(); i++) {
        render_text(editor->lines[i].chars.data(), pos, Vec4f {UNHEX(float, 0xffffffff)}, Vec4f {UNHEX(float, 0x0)});
        pos.y -= cache_font_size.y; // opengl y axis point up
    }
    render_cursor(editor, time);
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

void EditorRenderer::push_buffer(Glyph glyph) {
    assert(buffer_count < BUFFER_INIT_CAP);
    buffers[buffer_count] = glyph;
    buffer_count++;
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


// cache all texture for drawable characters
void EditorRenderer::init_font_cache(const char* font_path, int font_size) {
    cache_font_size = {0, 0};
    font = check(TTF_OpenFont(font_path, font_size));
    for (size_t i = 32; i < 128; i++) {
        char temp[2] = {(char)i, 0};
        SDL_Surface* BGRA_surface = check(TTF_RenderText_Blended(font, temp, SDL_Color{UNHEX(uint8_t, 0xffffffff)}));
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
        SDL_Surface* BGRA_surface = check(TTF_RenderText_Blended(font, temp, SDL_Color{UNHEX(uint8_t, 0xffffffff)}));
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
    : camera {SCREEN_WIDTH / 2.0f, -SCREEN_HEIGHT / 2.0f}, camVelocity {.0f, .0f}, program {},
    vao {}, vbo {}, buffer_count {}, font {}, cache_font_texture {}, cache_font_size {}, uv_pixel_cache {}
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

