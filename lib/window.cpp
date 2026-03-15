#include "window.hpp"
#include "raylib.h"

game_window::game_window(int width, int height, const char* title)
    : width_(width), height_(height) {
    InitWindow(width_, height_, title);
    SetTargetFPS(60);
}

game_window::~game_window() {
    CloseWindow();
}

bool game_window::should_close() const {
    return WindowShouldClose();
}

void game_window::begin_drawing() {
    ::BeginDrawing();
}

void game_window::end_drawing() {
    ::EndDrawing();
}

void game_window::clear_background(int r, int g, int b) {
    ::ClearBackground((Color){static_cast<unsigned char>(r),
                               static_cast<unsigned char>(g),
                               static_cast<unsigned char>(b),
                               255});
}
