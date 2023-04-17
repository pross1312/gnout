#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <SDL2/SDL_ttf.h>
#include "Utils.h"
#include "cursor.h"
#include "Vec2.h"

inline const std::string FONT_DIR = "C:\\Windows\\Fonts\\";

#define LOW_ASCII 48
#define HIGH_ASCII 123

#define INIT_LINE_SIZE 1024


class Editor
{
private:
    class Line
    {
    public:
        std::vector<char> chars;
        size_t char_count;
        Line(size_t init_size) : chars(init_size, '\0'), char_count(0) {}
        // at the name suggesst
        void insert_at_pos(size_t pos, const char* text);
        void delete_all_after_pos(size_t pos);
        void delete_at_pos(size_t pos, size_t n);
    };

    std::vector<Line> lines;
    TTF_Font* font;
    SDL_Color text_color;



    // cursor position in term of characters and lines
    size_t cursor_row = 0;
    size_t cursor_col = 0;



    uint32_t cursor_color = 0xffffffff;

    int line_height = 0; // also cursor height
    int char_width = 0;
     
public:
    Editor(const std::string& font_name, int font_size);
    void insert_at_cursor(const char* text);
    void delete_before_cursor(); // remove 1 character before cursor
    void new_line(); // add new line and set cursor to start of that line
    void cursor_up();
    void cursor_down();
    void cursor_left();
    void cursor_right();
    void delete_at_cursor();

    // delete 1 line and move cursor to end of previous line
    // cursor_col will be set to zero to assure valid position
    void delete_line(size_t row); 
    
    void draw_cursor(SDL_Renderer* renderer);

    void draw(SDL_Renderer* renderer); 


    ~Editor();
};
