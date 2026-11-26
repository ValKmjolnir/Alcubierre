#include "ui/ui_window.hpp"

ui_window::ui_window(int x, int y, int width, int height, Color background) :
    x_(x), y_(y),
    width_(width), height_(height),
    background_(background) {}

void ui_window::set_title(const char* title) {
    title_ = title ? title : "";
}

void ui_window::set_visible(bool flag) {
    visible_ = flag;
    if (!visible_) {
        cancel_input();
    }
}

Rectangle ui_window::title_rect() const {
    return { float(x_), float(y_), float(width_), float(kTitleHeight) };
}

bool ui_window::is_hovered(int mouse_x, int mouse_y) const {
    const Vector2 pos { float(mouse_x), float(mouse_y) };
    const Rectangle rec { float(x_), float(y_), float(width_), float(height_) };
    return CheckCollisionPointRec(pos, rec);
}

bool ui_window::any_child_hovered(const Vector2& pos) const {
    for (const auto& child : children_) {
        if (child->is_hovered(int(pos.x), int(pos.y))) {
            return true;
        }
    }
    return false;
}

bool ui_window::captures_mouse() const {
    if (!visible_) {
        return false;
    }
    if (dragging_ || hovered_) {
        return true;
    }
    const Vector2 pos = GetMousePosition();
    return is_hovered(int(pos.x), int(pos.y));
}

void ui_window::set_size(int width, int height) {
    width_ = width;
    height_ = height;
    clamp_inside_screen();
}

void ui_window::move_to(int x, int y) {
    int max_x = GetScreenWidth() - width_;
    int max_y = GetScreenHeight() - height_;
    if (max_x < 0) {
        max_x = 0;
    }
    if (max_y < 0) {
        max_y = 0;
    }
    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }
    if (x > max_x) {
        x = max_x;
    }
    if (y > max_y) {
        y = max_y;
    }

    const int dx = x - x_;
    const int dy = y - y_;
    if (dx == 0 && dy == 0) {
        return;
    }

    for (auto& child : children_) {
        child->move(dx, dy);
    }
    x_ = x;
    y_ = y;
}

void ui_window::clamp_inside_screen() {
    move_to(x_, y_);
}

void ui_window::cancel_input() {
    if (hovered_) {
        hovered_->cancel_mouse_press();
        hovered_ = nullptr;
    }
    dragging_ = false;
}

void ui_window::update_input() {
    if (!visible_) {
        cancel_input();
        return;
    }

    const Vector2 pos = GetMousePosition();

    if (!IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        dragging_ = false;
        if (hovered_) {
            if (hovered_->is_hovered(int(pos.x), int(pos.y)) ||
                hovered_->commits_outside_release()) {
                hovered_->handle_mouse_release();
            } else {
                hovered_->cancel_mouse_press();
            }
            hovered_ = nullptr;
        }
        return;
    }

    if (dragging_) {
        move_to(int(pos.x) - drag_offset_x_, int(pos.y) - drag_offset_y_);
        return;
    }
    if (hovered_) {
        return;
    }

    if (draggable_ && !any_child_hovered(pos) && CheckCollisionPointRec(pos, title_rect())) {
        dragging_ = true;
        drag_offset_x_ = int(pos.x) - x_;
        drag_offset_y_ = int(pos.y) - y_;
        return;
    }

    hovered_ = nullptr;
    for (auto& child : children_) {
        if (child->is_hovered(int(pos.x), int(pos.y))) {
            hovered_ = child.get();
        }
    }
    if (hovered_) {
        hovered_->handle_mouse_press();
    }
}

void ui_window::draw() {
    if (!visible_) {
        return;
    }

    DrawRectangle(x_, y_, width_, height_, background_);

    if (!title_.empty()) {
        DrawRectangle(x_, y_, width_, kTitleHeight, Color { 0x00, 0xdd, 0xff, 110 });
        DrawText(title_.c_str(), x_ + 8, y_ + 6, 16, WHITE);
    }

    DrawRectangleLines(x_, y_, width_, height_, border_);

    BeginScissorMode(x_ + 1, y_ + 1, width_ - 2, height_ - 2);
    for (auto& child : children_) {
        child->draw();
    }
    EndScissorMode();
}

button& ui_window::add_new_button(int x,
                                  int y,
                                  int width,
                                  int height,
                                  button_operation op,
                                  const char* text,
                                  config_set_funcptr cb) {
    children_.push_back(std::make_unique<button>(
        x_ + x, y_ + y,
        width, height,
        Color { 0x00, 0xdd, 0xff, 175 },
        op,
        text,
        cb
    ));
    return static_cast<button&>(*children_.back());
}

checkbox& ui_window::add_new_checkbox(int x,
                                      int y,
                                      int size,
                                      config_get_funcptr gf,
                                      config_set_funcptr cf) {
    children_.push_back(std::make_unique<checkbox>(
        x_ + x, y_ + y, size, Color { 0x00, 0xdd, 0xff, 200 }, gf, cf));
    return static_cast<checkbox&>(*children_.back());
}

slider& ui_window::add_new_slider(int x,
                                  int y,
                                  int width,
                                  int height,
                                  int min_value,
                                  int max_value,
                                  config_get_int_funcptr gf,
                                  config_set_int_funcptr cf) {
    children_.push_back(std::make_unique<slider>(
        x_ + x, y_ + y, width, height,
        Color { 0x00, 0xdd, 0xff, 200 },
        min_value, max_value,
        gf, cf
    ));
    return static_cast<slider&>(*children_.back());
}

textbox& ui_window::add_new_textbox(int x,
                                    int y,
                                    int width,
                                    int height,
                                    int fontsize,
                                    Color color,
                                    const char* text) {
    children_.push_back(std::make_unique<textbox>(
        x_ + x, y_ + y, width, height, fontsize, color, text));
    return static_cast<textbox&>(*children_.back());
}

selector& ui_window::add_new_selector(int x,
                                      int y,
                                      int width,
                                      int height,
                                      const resolution_option* options,
                                      int option_count,
                                      game_window* window) {
    children_.push_back(std::make_unique<selector>(
        x_ + x, y_ + y, width, height,
        Color { 0x00, 0xdd, 0xff, 200 },
        options, option_count,
        window
    ));
    return static_cast<selector&>(*children_.back());
}
