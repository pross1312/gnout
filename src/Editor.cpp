#include "Editor.h"
#include <assert.h>
#include <fstream>

void Editor::Line::delete_all_after_pos(size_t pos)
{
    delete_at_pos(pos, char_count - pos); // call to delte at pos with n = number of charecters after pos
}

void Editor::Line::insert_at_pos(size_t pos, const char* text, size_t n)
{
    if (n == 0) // don't do anything if n == 0
        return;
    if (pos > char_count) // only insert at most at end of line
        pos = char_count;


    // call insert of vector if not enough capacity
    if (char_count + n > chars.size())
        chars.insert(chars.begin() + pos, text, text + n);
    else {
        // if have enough capacity but cursor is in middle of line shift text right first 
        if (pos < char_count) {
            char* line_buffer = chars.data();
            memmove(line_buffer + pos + n, line_buffer + pos, char_count - pos);
        }
        // copy text into line at position
        memcpy(chars.data() + pos, text, n);
    }
    char_count += n;
}

// delete n chars onward at position
void Editor::Line::delete_at_pos(size_t pos, size_t n)
{
    if (pos >= char_count) // don't delete if pos is out of bound or maybe i should abort hmm..
        return;

    // shift text after pos left n position
    // after that memset to zero
    size_t n_chars_moved = char_count - pos - n;
    memmove(chars.data() + pos, chars.data() + pos + n, n_chars_moved);
    memset(chars.data() + pos + n_chars_moved, 0, n);
    char_count -= n;
}

// init editor with 0 line, each line with INIT_LINE_SIZE characters
// text color is init to zero
// also calculate character width and height for cursor rendering
Editor::Editor(const std::string& font_path, int font_size)
{
    font = check(TTF_OpenFont(font_path.c_str(), font_size));

    line_height = TTF_FontHeight(font);
    int minx, miny, maxx, maxy, advance;
    check(TTF_GlyphMetrics32(font, 'a', &minx, &maxx, &miny, &maxy, &advance));
    char_width = advance;
}

Editor::~Editor()
{
    TTF_CloseFont(font);
}

void Editor::insert_at_cursor(const char* text, size_t n)
{
    if (n == 0) // don't do anything if n == 0
        return;
    if (lines.size() == 0) // for whatever reason that we don't have any line, add 1 line
        new_line();
    assert(cursor_row < lines.size()); // and for whatever reason i have an invalid cursor_row, abort
    lines[cursor_row].insert_at_pos(cursor_col, text, n);
    cursor_right(n);
    if (lines[cursor_row].char_count * char_width > text_width)
        text_width = lines[cursor_row].char_count * char_width;
}

void Editor::new_line()
{
    lines.insert(lines.begin() + cursor_row + (cursor_row == lines.size() ? 0 : 1), Line(INIT_LINE_SIZE));

    // if increase cursor_row will be invalid for whatever reason then don't do it
    if (cursor_row + 1 < lines.size())
        set_cursor(cursor_row + 1, 0);
    else
        set_cursor(cursor_row, 0);

    text_height += line_height;
}

void Editor::delete_before_cursor()
{
    assert(cursor_row < lines.size());
    if (cursor_col == 0 && cursor_row == 0) // do nothing if cursor is at start
        return;
    if (cursor_col == 0) {
        // set cursor to end of previous line
        // then append all text left of this line to previous line
        cursor_up(1);
        cursor_col = lines[cursor_row].char_count;
        lines[cursor_row].insert_at_pos(cursor_col, lines[cursor_row + 1].chars.data(), lines[cursor_row + 1].char_count);
        delete_line(cursor_row + 1);
    }
    else {
        lines[cursor_row].delete_at_pos(cursor_col - 1, 1); // remove 1 character before cursor
        cursor_left(1);
    }
}

void Editor::delete_at_cursor()
{
    assert(cursor_row < lines.size());
    // if cursor at end of line;
    // merge 2 lines if not last line
    // do nothing if last line
    if (cursor_col == lines[cursor_row].char_count) {
        if (cursor_row == lines.size() - 1)
            return;
        lines[cursor_row].insert_at_pos(cursor_col, lines[cursor_row + 1].chars.data(), lines[cursor_row + 1].char_count);
        // remove next line, add all text to this line, cursor will be zero
        // so set it back to current position
        delete_line(cursor_row + 1); // cursor won't be moved if its not on that line
    }
    else {
        lines[cursor_row].delete_at_pos(cursor_col, 1);
    }
}

// set cursor to a position and recalculate origin if necessary
void Editor::set_cursor(size_t row, size_t col)
{
    if (row > lines.size() - 1 && col > lines[row].char_count)
        return;
    cursor_col = col;
    cursor_row = row;
    // check to move text to cursor if cursor is moved out of screen
    float cursor_y = cursor_row * line_height - text_origin.y;
    float cursor_x = cursor_col * char_width - text_origin.x;
    Vec2f vel {.x = 0, .y = 0};
    if (cursor_x < 0)
        vel.x = cursor_x;
    else if (cursor_x + char_width > SCREEN_WIDTH)
        vel.x = cursor_x + char_width - SCREEN_WIDTH;
    if (cursor_y < 0)
        vel.y = cursor_y;
    else if (cursor_y + line_height > SCREEN_HEIGHT)
        vel.y = cursor_y + line_height - SCREEN_HEIGHT;
    move_origin(vel.x, vel.y);
}

// move up n lines
void Editor::cursor_up(size_t n)
{
    if (cursor_row < n) {
        set_cursor(0, 0);
    }
    else {
        set_cursor(cursor_row - n, cursor_col > lines[cursor_row - n].char_count ? lines[cursor_row - n].char_count : cursor_col);
    }
}

// move down n lines
void Editor::cursor_down(size_t n)
{
    if (cursor_row + n > lines.size() - 1) {
        set_cursor(lines.size()-1, lines[lines.size()-1].char_count);
    }
    else {
        set_cursor(cursor_row + n, cursor_col > lines[cursor_row + n].char_count ? lines[cursor_row + n].char_count : cursor_col);
    }
}

// move left n characters
void Editor::cursor_left(size_t n)
{
    if (cursor_col < n) {
        if (cursor_row == 0)
            set_cursor(0, 0);
        else
            set_cursor(cursor_row - 1, lines[cursor_row - 1].char_count - (n - 1));
    }
    else
        set_cursor(cursor_row, cursor_col - n);

}

// move right n characters
void Editor::cursor_right(size_t n)
{
    if (cursor_col + n > lines[cursor_row].char_count) {
        if (cursor_row == lines.size() - 1)
            set_cursor(cursor_row, lines[cursor_row].char_count);
        else
            set_cursor(cursor_row + 1, n - 1);
    }
    else
        set_cursor(cursor_row, cursor_col + n);

}

// delete line and move cursor if cursor is on that line 
void Editor::delete_line(size_t row)
{
    assert(row < lines.size());
    lines.erase(lines.begin() + row);
    if (cursor_row == row) {
        set_cursor(row == 0 ? 0 : row - 1, 0);
    }
}


void Editor::split_to_new_line_at_cursor()
{
    size_t pre_pos = cursor_col; // temporary store position
    new_line(); // add new line
    // insert text after pre_pos to this new line
    lines[cursor_row].insert_at_pos(0, lines[cursor_row - 1].chars.data() + pre_pos, lines[cursor_row - 1].char_count - pre_pos);
    // delete all after pre_pos of previous line
    lines[cursor_row - 1].delete_all_after_pos(pre_pos);
}


bool Editor::save(const char* file)
{
    std::ofstream fout(file);
    if (!fout.is_open())
        return false;
    // simply iterate and write every line to file
    for (size_t row = 0; row < lines.size(); row++) {
        fout << lines[row].chars.data() << (row != lines.size() - 1 ? "\n" : "");
    }

    fout.close();
    return true;
}

bool Editor::load(const char* file)
{
    assert(lines.empty() && "Can only load to an empty editor.\n");
    std::ifstream fin(file);
    if (!fin.is_open())
        return false;
    std::string line;

    // read a line, if not empty then create a line and insert text in
    while (!fin.eof()) {
        std::getline(fin, line);
        if (!line.empty()) {
            new_line();
            insert_at_cursor(line.c_str(), line.size());
        }
    }
    if (lines.size() != 0)
        cursor_col = 0; // set cursor to end of line (end of text file) do only if there a line exist
    cursor_row = 0;
    fin.close();
    return true;
}


void Editor::set_cursor_to_mouse_postition(int x, int y)
{
    assert(x >= 0 && y >= 0 && "Invalid mouse position");
    size_t row = (size_t)((y + text_origin.y) / line_height);
    size_t col = (size_t)((x + text_origin.x) / char_width);
    if (lines.size() == 0)
        new_line();
    else {
        if (row >= lines.size()) {
            row = lines.size() - 1;
            col = lines[row].char_count;
        }
        else if (col >= lines[row].char_count)
            col = lines[row].char_count;
        set_cursor(row, col);
    }
}

void Editor::move_origin(float x, float y)
{
    Vec2f velocity {.x = x, .y = y};
    text_origin = addVec(text_origin, velocity);

    // check bound for origin (don't let we move text to far)
    if (SCREEN_HEIGHT - (text_height - text_origin.y) > BOTTOM_SCROLL_BUFFER)
        text_origin.y = text_height + BOTTOM_SCROLL_BUFFER - SCREEN_HEIGHT;
    if (SCREEN_WIDTH - (text_width - text_origin.x) > RIGHT_SCROLL_BUFFER)
        text_origin.x = text_width + RIGHT_SCROLL_BUFFER - SCREEN_WIDTH;
    if (text_origin.y < 0.0f)
        text_origin.y = 0.0f;
    if (text_origin.x < 0.0f)
        text_origin.x = 0.0f;
    

}

