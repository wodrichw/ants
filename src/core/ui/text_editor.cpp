#include "ui/text_editor.hpp"

#include "app/globals.hpp"
#include "ui/event_system.hpp"
#include "hardware/software_manager.hpp"

#include "spdlog/spdlog.h"

TextEditor::TextEditor(SoftwareManager& software_manager):
    software_manager(software_manager) {
        reset();
    }

void TextEditor::open() {
    SPDLOG_INFO("Opening the editor");
    software_manager.get_lines(lines);
    go_to_text_y();
    SPDLOG_TRACE("Successfully opened the editor");
}

void TextEditor::close() {
    SPDLOG_DEBUG("Closing the text editor");
    software_manager.add_lines(lines);

    reset();
    SPDLOG_INFO("The text editor closed successfully");
}

void TextEditor::go_to_text_x() {
    ushort length = line_length();
    SPDLOG_TRACE("Checking if cursor x is within text - cursor x: {} length: {}", cursor_x, length);

    if (cursor_x <= length) return;

    cursor_x = length;
    pan_to_cursor_x();
}

void TextEditor::go_to_text_y() {
    if (cursor_y >= lines.size()) {
        cursor_y = lines.size() - 1;
        pan_to_cursor_y();
    }
    go_to_text_x();
}

void TextEditor::move_to_start_line() {
    SPDLOG_DEBUG("Moving to start of line");
    cursor_x = 0;
    pan_to_cursor_x();
    SPDLOG_DEBUG("move_to_end_line - cursor_x: {}, cursor_y: {}", cursor_x, cursor_y);
}

void TextEditor::move_to_end_line() {
    SPDLOG_DEBUG("Moving to end of line");
    cursor_x = line_length();
    pan_to_cursor_x();
    SPDLOG_DEBUG("move_to_end_line - cursor_x: {}, cursor_y: {}", cursor_x, cursor_y);
}

void TextEditor::reset() {
    SPDLOG_DEBUG("Clearing the text editor lines");
    lines.clear();
    lines.push_back("");
}

ushort TextEditor::line_length() const {
    return lines.at(cursor_y).size();
}

void TextEditor::pan_to_cursor_x() {
    ushort x_pos = std::max(0, cursor_x - 2);
    if (x_pos < pan_x) {
        SPDLOG_TRACE("Panning to the left - pan x: {} -> {}", pan_x, cursor_x);
        pan_x = x_pos;
        return;
    }

    if (cursor_x >= (pan_x + globals::TEXTBOXWIDTH - 1)) {
        pan_x = cursor_x - globals::TEXTBOXWIDTH + 1;
        SPDLOG_TRACE("Panning to the right - pan x: {}", pan_x);
        return;
    }
}

void TextEditor::pan_to_cursor_y() {
    if (cursor_y < pan_y) {
        SPDLOG_TRACE("Panning up - pan y: {} -> {}", pan_y, cursor_y);
        pan_y = cursor_y;
        return;
    }

    if (cursor_y >= (pan_y + globals::TEXTBOXHEIGHT - 1)) {
        pan_y = cursor_y - globals::TEXTBOXHEIGHT + 1;
        SPDLOG_TRACE("Panning down - pan y: {}", pan_y);
        return;
    }
}

bool TextEditor::on_left_edge() const {
    return cursor_x == 0;
}

bool TextEditor::on_top_edge() const {
    return cursor_y == 0;
}

bool TextEditor::on_right_edge() const {
    return cursor_x == line_length();
}

bool TextEditor::on_bottom_edge() const {
    return cursor_y == (lines.size() - 1);
}

void TextEditor::move_left() {
    SPDLOG_TRACE("Moving cursor left");
    --cursor_x;
    pan_to_cursor_x();
}

void TextEditor::move_up() {
    SPDLOG_TRACE("Moving cursor up");
    --cursor_y;
    pan_to_cursor_y();
}

void TextEditor::move_right() {
    SPDLOG_TRACE("Moving cursor right");
    ++cursor_x;
    pan_to_cursor_x();
}

void TextEditor::move_down() {
    SPDLOG_TRACE("Moving cursor down");
    ++cursor_y;
    pan_to_cursor_y();
}

void TextEditor::insert(char ch) {
    std::string& line = lines[cursor_y];
    line.insert(line.begin() + cursor_x, ch);
    move_right();
}

void TextEditor::new_line() {
    std::string& line = lines[cursor_y];

    std::string new_line = line.substr(cursor_x);
    line = line.substr(0, cursor_x);

    move_down();
    move_to_start_line();

    lines.insert(lines.begin() + cursor_y, new_line);
}

void TextEditor::backspace() {
    if (!on_left_edge()) {
        move_left();
        lines[cursor_y].erase(cursor_x, 1);
        return;
    }

    if (!on_top_edge()) {
        std::string copy_line = lines[cursor_y];
        lines.erase(lines.begin() + cursor_y);
        move_up();
        move_to_end_line();
        lines[cursor_y].append(copy_line);
    }
}

ushort TextEditor::get_cursor_x() const { return cursor_x; }
ushort TextEditor::get_cursor_y() const { return cursor_y; }
ushort TextEditor::get_offset_x() const { return pan_x; }
ushort TextEditor::get_offset_y() const { return pan_y; }
