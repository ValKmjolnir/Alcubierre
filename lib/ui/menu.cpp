#include "ui/menu.hpp"

void menu::draw() {
    DrawRectangle(0, 0, window.width() / 2, window.height(), { 0, 0, 0, 150 });
    DrawText("Press M to close menu", 10, 10, 20, WHITE);
}