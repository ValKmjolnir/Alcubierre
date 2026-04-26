#include "ui/widget/textbox.hpp"

void textbox::draw() {
    DrawText(text.c_str(), x, y, fontsize, color);
}
