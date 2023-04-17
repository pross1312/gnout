#define SDL_MAIN_HANDLED
#include <windows.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <string>
#include "Editor.h"

// TODO: save/load file
// TODO: delete word
// TODO: cursor blinking
// TODO: tab


int main(int argc, char** argv)
{
    const char* file_path = NULL;
    if (argc > 1) {
        file_path = argv[1];
    }

    check(SDL_Init(SDL_INIT_VIDEO));
    check(TTF_Init()); 
    SDL_Window* window = (SDL_Window*) check(SDL_CreateWindow("Text editor",
                                                SDL_WINDOWPOS_UNDEFINED,
                                                SDL_WINDOWPOS_UNDEFINED,
                                                1500, 1000,
                                                SDL_WINDOW_RESIZABLE));
    SDL_Renderer* renderer = (SDL_Renderer*) check(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));

    // event
    SDL_Event event;
    
    // use this cause of fixed width font size is easy to implement
    // will think about font problem in the future
    Editor editor("Caskaydia Cove Nerd Font Complete Mono ExtraLight.otf", 30);

    if (file_path) {
        // if can't load then set file_path to NULL and proceed with normal editor
        if (editor.load(file_path) == false) 
            file_path = NULL;
    }

    bool onShift = false;
    bool onCtrl  = false;


    bool quit = false;
    while (!quit) {
        // clear and draw screen
        check(SDL_SetRenderDrawColor(renderer, UNHEX(0x000000ff)));
        check(SDL_RenderClear(renderer));

        editor.draw(renderer);

        SDL_RenderPresent(renderer);


        // event handle
        while (SDL_PollEvent(&event)) {
            switch(event.type) {
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

            case SDL_KEYDOWN: {
                switch(event.key.keysym.sym) {
                
                case SDLK_DELETE: {
                    editor.delete_at_cursor();
                } break;


                // handle paste event
                case SDLK_LCTRL: case SDLK_RSHIFT: {
                    onCtrl = true;                    
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
                    editor.cursor_up();
                } break;

                case SDLK_DOWN: {
                    editor.cursor_down();
                } break;

                case SDLK_LEFT: {
                    editor.cursor_left();
                } break;
                
                case SDLK_RIGHT: {
                    editor.cursor_right();
                } break;

                case SDLK_RETURN: {
                    editor.split_to_new_line_at_cursor();
                } break;

                case SDLK_BACKSPACE: {
                    editor.delete_before_cursor();
                } break;

                case SDLK_F2: {
                    if (file_path)
                        editor.save(file_path);
                    else
                        assert(false && "Don't know yet");
                } break;

                }
            } break;

            case SDL_KEYUP: {
                switch(event.key.keysym.sym) {
                case SDLK_LCTRL: case SDLK_RCTRL: {
                    onCtrl = false;
                } break;

                }


            } break;

            }
        }

    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window); 
    SDL_Quit();
    return 0;
}