#pragma once
#include <assert.h>
#include <filesystem>
namespace fs = std::filesystem;
#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <SDL2/SDL_ttf.h>
#include <optional>
#include "Utils.h"
#include "Vec.h"


#define INIT_LINE_SIZE 512
#define BOTTOM_SCROLL_BUFFER 100
#define RIGHT_SCROLL_BUFFER 100

enum Mode {
    FILE_EXPLORER,
    TEXT,
};


class Editor
{
private:
    class Line
    {
    public:
        std::vector<char> chars; // stort text of a line
        size_t char_count;       // number of characters currently on line
        Line(size_t init_size): chars(init_size, '\0'), char_count(0) {}
        Line(const char* init_text) : chars(init_text, init_text + strlen(init_text) + 1), char_count(strlen(init_text)) {}
        void insert_at_pos(size_t pos, const char* text, size_t n);
        void delete_all_after_pos(size_t pos);
        void delete_at_pos(size_t pos, size_t n);
        char operator[] (size_t col) const { assert(col < char_count); return chars[col]; }
    };
    class Cursor {
    public:
        size_t row, col;
        Cursor():row {0}, col {0} {}
        Cursor(size_t r, size_t c): row{r}, col{c} {}
        bool operator==(const Cursor& b) const { return row == b.row && col == b.col; }
        bool operator!=(const Cursor& b) const { return !(*this == b); }
        bool operator<(const Cursor& b) const { return (row < b.row || (row == b.row && col < b.col)); }
    };
    class FileExplorer {
    public:
        fs::path current_path;
        std::vector<Line> entries;
        FileExplorer(const char* dir_path);
        void change_dir(fs::path dir_path);
        void open(size_t row);
    };
    char* current_file;
    Mode mode;
    FileExplorer file_explorer;
    std::vector<Line> text_lines;
    std::vector<Line>* lines;

    // cursor position in term of characters and lines
    Cursor cursor;
    std::optional<Cursor> start_select;

    friend class EditorRenderer;

public:
    Editor();
    ~Editor();
    void insert_at_cursor(const char* text, size_t n);
    // delete 1 line and move cursor to end of previous line if cursor is on that line
    // cursor_col will be set to zero to assure valid position
    void delete_line(size_t row);
    void delete_before_cursor(); // remove 1 character before cursor
    void delete_at_cursor();
    void add_new_line(size_t row);
    void split_to_new_line_at_cursor();
    void start_selection() { if (start_select.has_value()) return; start_select = cursor; }
    void end_selection() { start_select.reset(); }
    bool check_on_selection(size_t row, size_t col) const;
    bool has_selected_text() { return start_select.has_value() && start_select.value() != cursor; }
    void change_mode(Mode m);
    void open_file_at_cursor();
    void reset();
    Mode get_mode() { return mode; };
    std::string get_selected_text();
    void delete_selected_text();

    // set cursor to a position, do nothing if position is invalid
    // and adjust origin to show cursor too
    void set_cursor(size_t row, size_t col);
    // move cursor n chars/lines 
    void cursor_up(size_t n);
    void cursor_down(size_t n);
    void cursor_left(size_t n);
    void cursor_right(size_t n);
    // void move_origin(float x, float y);



    // load and save file
    bool load(const char* file);
    bool save();
};
