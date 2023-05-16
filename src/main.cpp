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
GLuint create_texture(TTF_Font* font, const char* text, SDL_Color color, int index, int* w, int *h)
{
    SDL_Surface* BGRA_surface = check(TTF_RenderText_Blended(font, text, color));
    SDL_Surface* RGBA_surface = check(SDL_ConvertSurfaceFormat(BGRA_surface, SDL_PIXELFORMAT_RGBA32, 0));
    SDL_FreeSurface(BGRA_surface);

    GLuint texture;
    glActiveTexture(GL_TEXTURE0 + index);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 RGBA_surface->w,
                 RGBA_surface->h,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 (GLubyte*)RGBA_surface->pixels);
    
    glBindTexture(GL_TEXTURE_2D, 0); 
    *w = RGBA_surface->w;
    *h = RGBA_surface->h;
    SDL_FreeSurface(RGBA_surface);
    return texture;
}

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
// end init

   
    EditorRenderer renderer("./fonts/Ubuntu-B.ttf", 64);
// main loop
    SDL_Event event {};
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;

            }
        }

        glClearColor(.0f, .0f, .0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        renderer.render(NULL);

        SDL_GL_SwapWindow(window);
    }
// end loop


    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    return 0;
}

#ifdef USE_SDL_RENDERER
#define FPS 60

const float DELTA_TIME = (1.0f / FPS);
const float scroll_speed = 2000.0f;

int main(int argc, char** argv)
{
    const char* file_path; 
    if (argc > 1) {
        file_path = argv[1];
    }
    check(SDL_Init(SDL_INIT_VIDEO));
    check(TTF_Init());
    SDL_Window* window = check(SDL_CreateWindow("gnout",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI));
    SDL_Renderer* renderer = check(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));
    // event
    SDL_Event event;
    // use this cause of fixed width font size is easy to implement
    // will think about font problem in the future
    Editor editor("MitraMono.ttf", 30); 
    if (file_path) {
        // if can't load then set file_path then with normal editor
        if (!editor.load(file_path)) {
            // do something later maybe
        }
    }
    bool onCtrl = false;
    bool quit = false;
    while (!quit) {
        // update w and h in case user resize window for accurate drawing
        SDL_GetWindowSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT); 
        uint32_t start_tick = SDL_GetTicks(); // start tick for fps handle
        
        // event handle
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
                    editor.set_cursor_to_mouse_postition(x, y);
                }
            } break;

            case SDL_MOUSEWHEEL: {
                float x_vel = event.wheel.x;
                float y_vel = -event.wheel.y;
                editor.move_origin(x_vel * scroll_speed * DELTA_TIME, y_vel * scroll_speed * DELTA_TIME);
    
            } break;

            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {

                case SDLK_DELETE: {
                    editor.delete_at_cursor();
                } break;

                    // handle paste event
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
            } break;
            case SDL_KEYUP: {
                switch (event.key.keysym.sym) {
                case SDLK_LCTRL: case SDLK_RCTRL: {
                    onCtrl = false;
                } break;
                }
            } break;
            }


        // clear and draw screen
        check(SDL_SetRenderDrawColor(renderer, UNHEX(0x000000ff)));
        check(SDL_RenderClear(renderer));
        editor.draw(renderer);
        SDL_RenderPresent(renderer);
        }

        uint32_t interval = SDL_GetTicks() - start_tick;
        if (interval < 1000 / FPS) {
            SDL_Delay(1000 / FPS - interval);
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
#endif
