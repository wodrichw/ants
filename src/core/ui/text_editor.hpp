#pragma once

#include <string>
#include <vector>

class SoftwareManager;

struct TextEditor {
    SoftwareManager& software_manager;
    std::vector<std::string> lines;

    TextEditor(SoftwareManager&);
    void open();
    void close();
    void go_to_text_x();
    void go_to_text_y();

    void move_to_start_line();
    void move_to_end_line();
    ushort line_length() const;
    void pan_to_cursor_x();
    void pan_to_cursor_y();
    void reset();

    bool on_left_edge() const;
    bool on_top_edge() const;
    bool on_right_edge() const;
    bool on_bottom_edge() const;

    void move_left();
    void move_up();
    void move_right();
    void move_down();

    void insert(char);
    void new_line();
    void backspace();

    ushort get_cursor_x() const;
    ushort get_cursor_y() const;

    ushort get_offset_x() const;
    ushort get_offset_y() const;

    private:
    ushort cursor_x = 0, cursor_y = 0;
    ushort pan_x = 0, pan_y = 0;
};
