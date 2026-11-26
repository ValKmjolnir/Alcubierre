#include "ui/widget/textbox.hpp"

void textbox::move(int dx, int dy) {
    x += dx;
    y += dy;
}

void textbox::draw() {
    DrawText(text.c_str(), x, y, fontsize, color);
}
