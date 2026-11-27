#include <raylib.h>

#include "ui/menu.hpp"
#include "window.hpp"

menu::menu(game_window& window) :
    window(window),
    panel(0, 0, 480, 800, Color { 0, 0, 0, 150 }) {}

void menu::add_new_button(int x,
                          int y,
                          int width,
                          int height,
                          button_operation op,
                          const char* text,
                          config_set_funcptr cb) {
    panel.add_new_button(x, y, width, height, op, text, cb);
}

void menu::add_back_button(int x, int y, int width, int height, const char* text) {
    button& back = panel.add_new_button(
        x, y, width, height, button_operation::NONE, text, nullptr);
    back.set_action([this] { set_show_menu(false); });
}

void menu::add_new_checkbox(int x,
                            int y,
                            int size,
                            config_get_funcptr gf,
                            config_set_funcptr cf) {
    panel.add_new_checkbox(x, y, size, gf, cf);
}

void menu::add_new_slider(int x,
                          int y,
                          int width,
                          int height,
                          int min_value,
                          int max_value,
                          config_get_int_funcptr gf,
                          config_set_int_funcptr cf) {
    panel.add_new_slider(x, y, width, height, min_value, max_value, gf, cf);
}

void menu::add_new_textbox(int x,
                           int y,
                           int width,
                           int height,
                           int fontsize,
                           Color color,
                           const char* text) {
    panel.add_new_textbox(x, y, width, height, fontsize, color, text);
}

void menu::add_new_selector(int x,
                            int y,
                            int width,
                            int height,
                            const resolution_option* options,
                            int option_count) {
    panel.add_new_selector(x, y, width, height, options, option_count, &window);
}

void menu::check_mouse_press() {
    panel.update_input();
}

void menu::draw(bool accept_input) {
    if (!show_menu_) {
        return;
    }

    int panel_width = int(window.width() * 0.31f);
    if (panel_width < 480) {
        panel_width = 480;
    }
    panel.set_size(panel_width, window.height());

    if (accept_input) {
        check_mouse_press();
    }
    panel.draw();
}

void menu::set_show_menu(bool flag) {
    show_menu_ = flag;
    if (!show_menu_) {
        panel.cancel_input();
    }
}
