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
    
    SDL_GLContext context = check(SDL_GL_CreateContext(window));
    SDL_GL_MakeCurrent(window, context);
    if (GLEW_OK != glewInit()) {
        fprintf(stderr, "Can't init glew\n");
        exit(EXIT_FAILURE);
    }
    if (checkOpenGLError()) {
        exit(10);
    }
// end init

    if (argc > 1) {
        // if can't load then set file_path then with normal editor
        if (editor.load(argv[1])) {
            file_path = argv[1];
        }
        else {

        }
    }
    EditorRenderer renderer("./fonts/Ubuntu-B.ttf", 32);

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
                assert(false && "Unimplemented");
                int x = 0, y = 0;
                uint32_t mouse_button = SDL_GetMouseState(&x, &y);
                if (mouse_button == SDL_BUTTON_LEFT) {

                }
            } break;

            case SDL_MOUSEWHEEL: {
                assert(false && "Unimplemented"); 
            } break;

            case SDL_KEYDOWN: {
                handle_keydown(event.key);
            } break;
            case SDL_KEYUP: {
                handle_keyup(event.key);
            } break;
            }
        }


        
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
