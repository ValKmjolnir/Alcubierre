#include <raylib.h>

#include "ui/menu.hpp"

void menu::add_new_button(int x,
                          int y,
                          int width,
                          int height,
                          button_operation op,
                          const char* text,
                          config_set_funcptr cb) {
    buttons.emplace_back(
        x, y,
        width, height,
        Color { 0x00, 0xdd, 0xff, 175 },
        op,
        text,
        cb
    );
}

void menu::add_new_checkbox(int x,
                            int y,
                            int size,
                            config_get_funcptr gf,
                            config_set_funcptr cf) {
    checkboxes.emplace_back(x, y, size, Color { 0x00, 0xdd, 0xff, 200 }, gf, cf);
}

void menu::add_new_slider(int x,
                          int y,
                          int width,
                          int height,
                          int min_value,
                          int max_value,
                          config_get_int_funcptr gf,
                          config_set_int_funcptr cf) {
    sliders.emplace_back(
        x, y, width, height,
        Color { 0x00, 0xdd, 0xff, 200 },
        min_value, max_value,
        gf, cf
    );
}

void menu::add_new_textbox(int x,
                           int y,
                           int width,
                           int height,
                           int fontsize,
                           Color color,
                           const char* text) {
    textboxes.emplace_back(x, y, width, height, fontsize, color, text);
}

void menu::add_new_selector(int x,
                            int y,
                            int width,
                            int height,
                            const resolution_option* options,
                            int option_count) {
    selectors.emplace_back(
        x, y, width, height,
        Color { 0x00, 0xdd, 0xff, 200 },
        options, option_count,
        &window
    );
}

void menu::set_show_menu(bool flag) {
    show_menu_ = flag;
    if (!show_menu_) {
        cancel_input();
    }
}

void menu::cancel_input() {
    if (hovered_widget) {
        hovered_widget->cancel_mouse_press();
        hovered_widget = nullptr;
    }
}

void menu::check_mouse_press() {
    const auto pos = GetMousePosition();

    if (!IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (hovered_widget) {
            // A press only turns into a click if the button is released on the
            // widget, unless the widget keeps the mouse for the whole drag.
            if (hovered_widget->is_hovered(static_cast<int>(pos.x), static_cast<int>(pos.y)) ||
                hovered_widget->commits_outside_release()) {
                hovered_widget->handle_mouse_release();
            } else {
                hovered_widget->cancel_mouse_press();
            }
            hovered_widget = nullptr;
        }
        return;
    }
    if (hovered_widget && hovered_widget->get_mouse_pressed()) {
        return;
    }

    hovered_widget = nullptr;
    for (auto& button : buttons) {
        if (button.is_hovered(pos.x, pos.y)) {
            hovered_widget = &button;
        }
    }
    for (auto& checkbox : checkboxes) {
        if (checkbox.is_hovered(pos.x, pos.y)) {
            hovered_widget = &checkbox;
        }
    }
    for (auto& selector : selectors) {
        if (selector.is_hovered(pos.x, pos.y)) {
            hovered_widget = &selector;
        }
    }
    for (auto& slider : sliders) {
        if (slider.is_hovered(pos.x, pos.y)) {
            hovered_widget = &slider;
        }
    }
    for (auto& textbox : textboxes) {
        if (textbox.is_hovered(pos.x, pos.y)) {
            hovered_widget = &textbox;
        }
    }

    if (!hovered_widget) {
        return;
    }

    if (!hovered_widget->get_mouse_pressed()) {
        hovered_widget->handle_mouse_press();
    }
}

void menu::draw() {
    if (!show_menu_) {
        return;
    }
    int panel_width = int(window.width() * 0.31f);
    if (panel_width < 480) {
        panel_width = 480;
    }
    DrawRectangle(0, 0, panel_width, window.height(), { 0, 0, 0, 150 });

    check_mouse_press();

    for (auto& button : buttons) {
        button.draw();
    }
    for (auto& checkbox : checkboxes) {
        checkbox.draw();
    }
    for (auto& selector : selectors) {
        selector.draw();
    }
    for (auto& slider : sliders) {
        slider.draw();
    }
    for (auto& textbox : textboxes) {
        textbox.draw();
    }
}
