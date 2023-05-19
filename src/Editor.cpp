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

void Editor::insert_at_cursor(const char* text, size_t n)
{
    if (n == 0) // don't do anything if n == 0
        return;
    if (lines.size() == 0) // for whatever reason that we don't have any line, add 1 line
        add_new_line(0);
    assert(cursor.row < lines.size()); // and for whatever reason i have an invalid cursor.row, abort
    lines[cursor.row].insert_at_pos(cursor.col, text, n);
    cursor_right(n);
}

void Editor::add_new_line(size_t row) {
    assert(row <= lines.size() && "Out of range");
    lines.insert(lines.begin() + row, Line(INIT_LINE_SIZE));
}

void Editor::delete_before_cursor()
{
    assert(cursor.row < lines.size());
    if (cursor.col == 0 && cursor.row == 0) // do nothing if cursor is at start
        return;
    if (cursor.col == 0) {
        // set cursor to end of previous line
        // then append all text left of this line to previous line
        cursor_up(1);
        cursor.col = lines[cursor.row].char_count;
        lines[cursor.row].insert_at_pos(cursor.col, lines[cursor.row + 1].chars.data(), lines[cursor.row + 1].char_count);
        delete_line(cursor.row + 1);
    }
    else {
        lines[cursor.row].delete_at_pos(cursor.col - 1, 1); // remove 1 character before cursor
        cursor_left(1);
    }
}

void Editor::delete_at_cursor()
{
    assert(cursor.row < lines.size());
    // if cursor at end of line;
    // merge 2 lines if not last line
    // do nothing if last line
    if (cursor.col == lines[cursor.row].char_count) {
        if (cursor.row == lines.size() - 1)
            return;
        lines[cursor.row].insert_at_pos(cursor.col, lines[cursor.row + 1].chars.data(), lines[cursor.row + 1].char_count);
        // remove next line, add all text to this line, cursor will be zero
        // so set it back to current position
        delete_line(cursor.row + 1); // cursor won't be moved if its not on that line
    }
    else {
        lines[cursor.row].delete_at_pos(cursor.col, 1);
    }
}

// this function won't let cursor get out of text area
void Editor::set_cursor(size_t row, size_t col)
{
    if (lines.size() == 0) return;
    if (row > lines.size() - 1) row = lines.size() - 1;
    if (col > lines[row].char_count) col = lines[row].char_count;
    cursor.col = col;
    cursor.row = row;
}
// move up n lines
void Editor::cursor_up(size_t n)
{
    if (lines.size() == 0) return;
    if (cursor.row < n) {
        set_cursor(0, 0);
    }
    else {
        set_cursor(cursor.row - n, cursor.col > lines[cursor.row - n].char_count ? lines[cursor.row - n].char_count : cursor.col);
    }
}

// move down n lines
void Editor::cursor_down(size_t n)
{
    if (lines.size() == 0) return;
    if (cursor.row + n > lines.size() - 1) {
        set_cursor(lines.size()-1, lines[lines.size()-1].char_count);
    }
    else {
        set_cursor(cursor.row + n, cursor.col > lines[cursor.row + n].char_count ? lines[cursor.row + n].char_count : cursor.col);
    }
}

// move left n characters
// move to new line if necessary
void Editor::cursor_left(size_t n)
{
    if (lines.size() == 0) return;
    if (cursor.col < n) {
        if (cursor.row == 0)
            set_cursor(0, 0);
        else
            set_cursor(cursor.row - 1, lines[cursor.row - 1].char_count - (n - 1));
    }
    else
        set_cursor(cursor.row, cursor.col - n);

}

// move right n characters
// move to new line if necessary
void Editor::cursor_right(size_t n)
{
    if (lines.size() == 0) return;
    if (cursor.col + n > lines[cursor.row].char_count) {
        if (cursor.row == lines.size() - 1)
            set_cursor(cursor.row, lines[cursor.row].char_count);
        else
            set_cursor(cursor.row + 1, n - 1);
    }
    else
        set_cursor(cursor.row, cursor.col + n);

}

// delete line and move cursor if cursor is on that line 
void Editor::delete_line(size_t row)
{
    assert(row < lines.size());
    lines.erase(lines.begin() + row);
    if (cursor.row == row) {
        set_cursor(row == 0 ? 0 : row - 1, 0);
    }
}


void Editor::split_to_new_line_at_cursor()
{
    if (lines.size() == 0) {
        add_new_line(0);
    }
    else {
        add_new_line(cursor.row + 1);
        lines[cursor.row + 1].insert_at_pos(0, lines[cursor.row].chars.data() + cursor.col, lines[cursor.row].char_count - cursor.col);
        lines[cursor.row].delete_at_pos(cursor.col, lines[cursor.row].char_count - cursor.col); 
        set_cursor(cursor.row + 1, 0);
    }
}

std::string Editor::get_selected_text() {
    if (!has_selected_text()) return {};
    Editor::Cursor start;
    Editor::Cursor end;
    if (start_select.value() < cursor) {
        start = start_select.value();
        end = cursor;
    }
    else {
        end = start_select.value();
        start = cursor;
    }
    std::stringstream ss;
    if (start.row == end.row) {
        ss.write(lines[start.row].chars.data() + start.col, end.col - start.col);
    } else {
        ss.write(lines[start.row].chars.data() + start.col, lines[start.row].char_count - start.col);
        ss << '\n';
        for (size_t row = start.row + 1; row < end.row; row++) {
            ss << lines[row].chars.data() << '\n';
        }
        ss.write(lines[end.row].chars.data(), end.col);
    }
    return ss.str();
}

void Editor::delete_selected_text() {
    if (!has_selected_text()) return;
    Editor::Cursor start;
    Editor::Cursor end;
    if (start_select.value() < cursor) {
        start = start_select.value();
        end = cursor;
    }
    else {
        end = start_select.value();
        start = cursor;
    }
    if (start.row == end.row) {
        lines[start.row].delete_at_pos(start.col, end.col - start.col);
    }
    else {
        lines[start.row].delete_at_pos(start.col, lines[start.row].char_count - start.col);
        lines[start.row].insert_at_pos(lines[start.row].char_count, lines[end.row].chars.data() + end.col, lines[end.row].char_count - end.col);
        for (size_t row = end.row; row > start.row; row--) {
            delete_line(row); 
        }
        set_cursor(start.row, start.col);
    }
}

bool Editor::check_on_selection(size_t row, size_t col) const {
    if (!start_select.has_value())
        return false;
    Editor::Cursor start;
    Editor::Cursor end;
    if (start_select.value() < cursor) {
        start = start_select.value();
        end = cursor;
    }
    else {
        end = start_select.value();
        start = cursor;
    }

    if (start.row < row && row < end.row) return true;
    if (start.row == end.row) {
        if (row == start.row && start.col <= col && col < end.col) return true;
    }
    else {
        if (start.row == row && start.col <= col) return true;
        if (end.row == row && end.col > col) return true;
    }
    return false;
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
        line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) {
            return isprint(ch);
        }));
        line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) {
            return isprint(ch);
        }).base(), line.end());
        if (!line.empty()) {
            add_new_line(lines.size());
            lines[lines.size()-1].insert_at_pos(0, line.c_str(), line.size());
        }
    }
    set_cursor(0, 0);
    fin.close();
    return true;
}

