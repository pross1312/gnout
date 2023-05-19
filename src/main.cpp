// HELLO FROM NEW GNOUT
#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>
#include <assert.h>
#include <string>
#include "Editor.h"
#include "EditorRenderer.h"
// TODO: learn opengl and use it to draw
// TODO: split render code of editor into an EditorRenderer
// TODO: delete word
// TODO: cursor blinking
// TODO: tab
// this is inserted by gnout
//
#define FPS 60
const float DELTA_TIME = (1.0f / FPS);
inline static Editor editor;
inline static const char* file_path;
void handle_keydown(SDL_KeyboardEvent& event);
void handle_keyup(SDL_KeyboardEvent& event);
int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    
    check(SDL_Init(SDL_INIT_VIDEO));
    check(TTF_Init());
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
// init stuff 
    SDL_Window* window = check(SDL_CreateWindow("gnout",
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                SCREEN_WIDTH, SCREEN_HEIGHT,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI));
    
    int gl_ver_major, gl_ver_minor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &gl_ver_major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &gl_ver_minor);
    printf("OpenGL %d.%d\n", gl_ver_major, gl_ver_minor);

    SDL_GLContext context = check(SDL_GL_CreateContext(window));
    SDL_GL_MakeCurrent(window, context);

    // if (glDrawArraysInstanced == NULL) {
    //     fprintf(stderr, "Support for EXT_draw_instanced is required!\n");
    //     exit(1);
    // }
    if (glDebugMessageCallback != NULL) {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, 0);
    }
    if (GLEW_OK != glewInit()) {
        fprintf(stderr, "Can't init glew\n");
        exit(EXIT_FAILURE);
    }
    EditorRenderer renderer("./fonts/Ubuntu-B.ttf", 32);
// end init
    if (argc > 1) {
        // if can't load then set file_path then with normal editor
        if (editor.load(argv[1])) {
            file_path = argv[1];
        }
        else {
        }
    }
// main loop
    SDL_Event event {};
    bool quit = false;
    while (!quit) {
        uint32_t start_tick = SDL_GetTicks(); // start tick for fps handle
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: {
                quit = true;
            } break;
            case SDL_TEXTINPUT: {
                const char* text = event.text.text;
                editor.insert_at_cursor(text, strlen(text));
            } break;
            case SDL_MOUSEBUTTONDOWN: {
                int x = 0, y = 0;
                uint32_t mouse_button = SDL_GetMouseState(&x, &y);
                if (mouse_button == SDL_BUTTON_LEFT) {
                    renderer.set_cursor_to_mouse(&editor, vec2f((float)x, (float)y));
                }
            } break;
            case SDL_WINDOWEVENT_RESIZED: {
                SCREEN_WIDTH = event.window.data1;
                SCREEN_HEIGHT = event.window.data2;
                glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
                
            } break;
            case SDL_MOUSEWHEEL: {
                Vec2f vel = normalized(vec2f(event.wheel.x, event.wheel.y));
                vel.x *= SCROLL_SENSITIVITY;
                vel.y *= SCROLL_SENSITIVITY;
                renderer.addVel(vel); 
            } break;
            case SDL_KEYDOWN: {
                handle_keydown(event.key);
            } break;
            case SDL_KEYUP: {
                handle_keyup(event.key);
            } break;
            }
        }
        
        renderer.moveCamera(DELTA_TIME);
        renderer.render(&editor, start_tick / 1000.0f);
        SDL_GL_SwapWindow(window);
        uint32_t interval = SDL_GetTicks() - start_tick;
        if (interval < 1000 / FPS) {
            SDL_Delay(1000 / FPS - interval);
        }
    }
// end loop
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    return 0;
}
bool onCtrl = false;
void handle_keydown(SDL_KeyboardEvent& event) {
    switch (event.keysym.sym) {
    case SDLK_DELETE: {
        editor.delete_at_cursor();
    } break;
    case SDLK_LCTRL: case SDLK_RSHIFT: {
        onCtrl = true;
    } break;
    case SDLK_s: {
        if (onCtrl) {
            if (file_path)
                editor.save(file_path);
            else
                assert(false && "Don't know yet");
        }
    } break;
    case SDLK_v: {
        if (onCtrl) {
            char* clip_board_text = SDL_GetClipboardText();
            if (clip_board_text == NULL)
                break;
            std::string_view sv(clip_board_text);
            // include line after line ...
            size_t start = 0;
            while (start < sv.size()) {
                size_t end = sv.find('\n', start);
                if (end == std::string::npos)
                    end = sv.size();
                editor.insert_at_cursor(clip_board_text + start, end - start);
                if (end != sv.size())
                    editor.new_line();
                start = end + 1;
            }
            SDL_free(clip_board_text);
        }
    } break;
        // handle navigate events.
    case SDLK_UP: {
        editor.cursor_up(1);
    } break;
    case SDLK_DOWN: {
        editor.cursor_down(1);
    } break;
    case SDLK_LEFT: {
        editor.cursor_left(1);
    } break;
    case SDLK_RIGHT: {
        editor.cursor_right(1);
    } break;
    case SDLK_RETURN: {
        editor.split_to_new_line_at_cursor();
    } break;
    case SDLK_BACKSPACE: {
        editor.delete_before_cursor();
    } break;
    }
}
void handle_keyup(SDL_KeyboardEvent& event) {
    switch (event.keysym.sym) {
    case SDLK_LCTRL: case SDLK_RCTRL: {
        onCtrl = false;
    } break;
    }
}
