#define SDL_MAIN_HANDLED
#include <windows.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <string>
#include "Editor.h"



int main(int argc, char** argv)
{
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    assert(TTF_Init() == 0); 
    SDL_Window* window = (SDL_Window*) check(SDL_CreateWindow("Text editor",
                                                SDL_WINDOWPOS_UNDEFINED,
                                                SDL_WINDOWPOS_UNDEFINED,
                                                1500, 1000,
                                                SDL_WINDOW_RESIZABLE));

    SDL_Renderer* renderer = (SDL_Renderer*) check(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));

    // event
    SDL_Event event;
    
    // use this cause of fixed width
    Editor editor("Caskaydia Cove Nerd Font Complete Mono ExtraLight.otf", 30);


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

            case SDL_KEYDOWN: {
                switch(event.key.keysym.sym) {
                
                case SDLK_DELETE: {
                    editor.delete_at_cursor();
                } break;

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
                    editor.new_line();
                } break;

                case SDLK_BACKSPACE: {
                    editor.delete_before_cursor();
                } break;
                
                }
            } break;

            case SDL_TEXTINPUT: {
                editor.insert_at_cursor(event.text.text);
            } break;
            }
        }

    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window); 
    SDL_Quit();
    return 0;
}