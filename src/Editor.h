#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <SDL2/SDL_ttf.h>
#include "Utils.h"
#include "Vec2.h"

#define INIT_LINE_SIZE 512
#define BOTTOM_SCROLL_BUFFER 100
#define RIGHT_SCROLL_BUFFER 100


class Editor
{
private:
    class Line
    {
    public:
        std::vector<char> chars; // stort text of a line
        size_t char_count;       // number of characters currently on line
        Line(size_t init_size): chars(init_size, '\0'), char_count(0) {}
        void insert_at_pos(size_t pos, const char* text, size_t n);
        void delete_all_after_pos(size_t pos);
        void delete_at_pos(size_t pos, size_t n);
    };

    std::vector<Line> lines;
    Vec2f text_origin {};
    TTF_Font* font;

    SDL_Color text_color{ UNHEX(0xffffffff) };
    SDL_Color cursor_color{ UNHEX(0xffffffff) };

    // cursor position in term of characters and lines
    size_t cursor_row = 0;
    size_t cursor_col = 0;

    int line_height = 0; // also cursor height
    int char_width = 0;
    float text_height = 0; // number of lines * line_height, update every times call new line
    float text_width = 0; // chars of max length line * char_width, update every time call insert after cursor 


public:
    Editor(const std::string& font_name, int font_size);
    void insert_at_cursor(const char* text, size_t n);
    // delete 1 line and move cursor to end of previous line if cursor is on that line
    // cursor_col will be set to zero to assure valid position
    void delete_line(size_t row);
    void delete_before_cursor(); // remove 1 character before cursor
    void delete_at_cursor();
    void new_line(); // add new line and set cursor to start of that line
    void split_to_new_line_at_cursor();


    // set cursor to mouse position (in pixel) 
    void set_cursor_to_mouse_postition(int x, int y);
    // set cursor to a position, do nothing if position is invalid
    // and adjust origin to show cursor too
    void set_cursor(size_t row, size_t col);
    // move cursor n chars/lines 
    void cursor_up(size_t n);
    void cursor_down(size_t n);
    void cursor_left(size_t n);
    void cursor_right(size_t n);
    void move_origin(float x, float y);



    // load and save file
    bool load(const char* file);
    bool save(const char* file);


    void draw_cursor(SDL_Renderer* renderer);
    // draw lines and cursor
    void draw(SDL_Renderer* renderer);


    ~Editor();
};
