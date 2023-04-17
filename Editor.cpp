#include "Editor.h"
#include <assert.h>

// this will rendered text at a specific postiion
// use it to draw editor text
inline void drawString(SDL_Renderer* renderer, TTF_Font* font, const char* text, Vec2 pos, SDL_Color color)
{
    SDL_Surface* text_surface = (SDL_Surface*)check(TTF_RenderText_Solid(font, text, color));
    SDL_Texture* text_texture = (SDL_Texture*)check(SDL_CreateTextureFromSurface(renderer, text_surface));
    SDL_Rect dst = {
        .x = pos.x,
        .y = pos.y,
        .w = text_surface->w,
        .h = text_surface->h
    };
    check(SDL_RenderCopy(renderer, text_texture, NULL, &dst));
    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text_surface);
}

void Editor::Line::delete_all_after_pos(size_t pos)
{
    delete_at_pos(pos, char_count - pos); // call to delte at pos with n = number of charecters after pos
}

void Editor::Line::insert_at_pos(size_t pos, const char* text)
{
    if (pos > char_count)
        pos = char_count;

    size_t text_size = strlen(text);
    if (char_count + text_size > chars.size())
        chars.insert(chars.begin() + pos, text, text + text_size);
    else if (pos < char_count) {
        char* line_buffer = chars.data();
        memmove(line_buffer + pos + text_size, line_buffer + pos, char_count - pos);
        memcpy(line_buffer + pos, text, text_size);
    }
    else {
        memcpy(chars.data() + char_count, text, text_size);
    }
    char_count += text_size;
}

// delete n chars onward at position
void Editor::Line::delete_at_pos(size_t pos, size_t n)
{
    if (pos >= char_count)
        return;

    // shift text after pos left n position
    // after that memset to zero
    size_t n_chars_moved = char_count - pos - n;
    memmove(chars.data() + pos, chars.data() + pos + n, n_chars_moved);
    memset(chars.data() + pos + n_chars_moved, 0, n); 
    char_count -= n;
}

// init editor with 1 line, each line with INIT_LINE_SIZE characters
// text color is init to zero
// also calculate character width and height for cursor rendering
Editor::Editor(const std::string& font_name, int font_size): lines(1, Line(INIT_LINE_SIZE))
{
    const std::string font_path = FONT_DIR + font_name;
    font = (TTF_Font*)check(TTF_OpenFont(font_path.c_str(), font_size));
    text_color = {
        UNHEX(0xffffffff)
    };
    line_height = TTF_FontHeight(font);
    int minx, miny, maxx, maxy, advance;
    check(TTF_GlyphMetrics32(font, 'a', &minx, &maxx, &miny, &maxy, &advance));
    char_width = advance;
}

Editor::~Editor()
{
    TTF_CloseFont(font);
}

void Editor::insert_at_cursor(const char* text)
{
    assert(cursor_row < lines.size());
    lines[cursor_row].insert_at_pos(cursor_col, text);
    cursor_col += strlen(text);
}

void Editor::delete_before_cursor()
{
    assert(cursor_row < lines.size());
    if (cursor_col == 0 && cursor_row == 0) // do nothing if cursor is at start
        return;
    if (cursor_col == 0) {
        if (lines[cursor_row].char_count == 0) {
            delete_line(cursor_row);
            // delete line set cursor_row to previous line (current line after delete)
            // and set cursor_col to 0
            // set it back to end of that line
            cursor_col = lines[cursor_row].char_count; 
        }
        else {
            cursor_row -= 1;
            cursor_col = lines[cursor_row].char_count;
        }
    }
    else {
        lines[cursor_row].delete_at_pos(cursor_col - 1, 1); // remove 1 character before cursor
        cursor_col -= 1;
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
        lines[cursor_row].insert_at_pos(cursor_col, lines[cursor_row+1].chars.data());
        // remove next line, add all text to this line, cursor will be zero
        // so set it back to current position
        size_t temp = cursor_col;
        delete_line(cursor_row + 1);
        cursor_col = temp;
    }
    else {
        lines[cursor_row].delete_at_pos(cursor_col, 1);
    }
}

void Editor::cursor_up()
{
    if (cursor_row == 0)
        return;
    cursor_row -= 1;
    if (cursor_col > lines[cursor_row].char_count)
        cursor_col = lines[cursor_row].char_count;
}

void Editor::cursor_down()
{
    if (cursor_row == lines.size() - 1)
        return;
    cursor_row += 1;
    if (cursor_col > lines[cursor_row].char_count)
        cursor_col = lines[cursor_row].char_count;
}

void Editor::cursor_left()
{
    if (cursor_col == 0) {
        if (cursor_row == 0)
            return;
        cursor_row -= 1;
        cursor_col = lines[cursor_row].char_count;
    }
    cursor_col -= 1;
}

void Editor::cursor_right()
{
    if (cursor_col == lines[cursor_row].char_count) {
        if (cursor_row == lines.size() - 1)
            return;
        cursor_col = 0;
        cursor_row += 1;
    }
    cursor_col += 1;
}
// delete line and set row to previous line or 0 if delete first line
// delete line set column to a valid position (start of line)
void Editor::delete_line(size_t row)
{
    assert(row < lines.size());
    lines.erase(lines.begin() + row);
    if (row == 0) {
        cursor_row = 0;
    }
    else {
        cursor_row = row - 1;
    }
    cursor_col = 0;
}

void Editor::draw(SDL_Renderer* renderer)
{
    for (size_t row = 0; row < lines.size(); row++) {
        if (lines[row].char_count > 0) {
            // draw string return new cursor position in pixel
            drawString(renderer, font, lines[row].chars.data(), vec2(0, row * line_height), text_color);
        }
    }
    draw_cursor(renderer);
}

void Editor::new_line()
{
    // move all after cursor to new line (insert then delete)
    cursor_row++;
    lines.insert(lines.begin() + cursor_row, Line(INIT_LINE_SIZE));
    lines[cursor_row].insert_at_pos(0, lines[cursor_row - 1].chars.data() + cursor_col);
    lines[cursor_row - 1].delete_all_after_pos(cursor_col);
    cursor_col = 0;
}

void Editor::draw_cursor(SDL_Renderer* renderer)
{
    check(SDL_SetRenderDrawColor(renderer, UNHEX(cursor_color)));

    SDL_Rect cursor{
        .x = char_width * cursor_col,
        .y = cursor_row * line_height,
        .w = char_width,
        .h = line_height
    };

    check(SDL_RenderFillRect(renderer, &cursor));

    // redraw character at cursor with different color
    if (cursor_col < lines[cursor_row].char_count) {
        char temp[2]{ lines[cursor_row].chars[cursor_col] };
        drawString(renderer, font, temp, vec2(cursor.x, cursor.y), SDL_Color{ UNHEX(0x000000ff) });
    }
}
