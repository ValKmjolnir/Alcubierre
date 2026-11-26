#pragma once

#include <raylib.h>

#include <memory>
#include <string>
#include <vector>

#include "ui/widget/button.hpp"
#include "ui/widget/checkbox.hpp"
#include "ui/widget/selector.hpp"
#include "ui/widget/slider.hpp"
#include "ui/widget/textbox.hpp"
#include "ui/widget/widget.hpp"
#include "utils/game_config.hpp"

class game_window;

class ui_window {
public:
    static constexpr int kTitleHeight = 26;

private:
    int x_ = 0;
    int y_ = 0;
    int width_ = 0;
    int height_ = 0;
    Color background_ { 0, 0, 0, 150 };
    Color border_ { 0, 0, 0, 0 };
    std::string title_;
    bool draggable_ = false;
    bool visible_ = true;

    bool dragging_ = false;
    int drag_offset_x_ = 0;
    int drag_offset_y_ = 0;

    std::vector<std::unique_ptr<widget>> children_;
    widget* hovered_ = nullptr;

private:
    Rectangle title_rect() const;
    bool any_child_hovered(const Vector2& pos) const;

public:
    ui_window(int x, int y, int width, int height, Color background);

    void set_title(const char* title);
    void set_draggable(bool flag) { draggable_ = flag; }
    void set_border(Color color) { border_ = color; }
    void set_visible(bool flag);
    bool get_visible() const { return visible_; }

    void set_size(int width, int height);
    void move_to(int x, int y);
    void clamp_inside_screen();

    bool is_hovered(int mouse_x, int mouse_y) const;
    // True while the pointer is over the window or a drag is in progress; the
    // camera uses this to stay out of the way of the UI.
    bool captures_mouse() const;

    void update_input();
    void cancel_input();
    void draw();

    button& add_new_button(int x,
                           int y,
                           int width,
                           int height,
                           button_operation op,
                           const char* text,
                           config_set_funcptr cb);
    checkbox& add_new_checkbox(int x,
                               int y,
                               int size,
                               config_get_funcptr gf,
                               config_set_funcptr cf);
    slider& add_new_slider(int x,
                           int y,
                           int width,
                           int height,
                           int min_value,
                           int max_value,
                           config_get_int_funcptr gf,
                           config_set_int_funcptr cf);
    textbox& add_new_textbox(int x,
                             int y,
                             int width,
                             int height,
                             int fontsize,
                             Color color,
                             const char* text);
    selector& add_new_selector(int x,
                               int y,
                               int width,
                               int height,
                               const resolution_option* options,
                               int option_count,
                               game_window* window);
};
