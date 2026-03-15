#include "window.hpp"
#include "raylib.h"

game_window::game_window(int width, int height, const char* title):
    width_(width), height_(height) {
    InitWindow(width_, height_, title);
    SetTargetFPS(120);
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
    const Color bgc = {
        static_cast<unsigned char>(r),
        static_cast<unsigned char>(g),
        static_cast<unsigned char>(b),
        255
    };
    ::ClearBackground(bgc);
}

void game_window::begin_mode_3d(const Camera3D& camera) {
    ::BeginMode3D(camera);
}

void game_window::end_mode_3d() {
    ::EndMode3D();
}

void game_window::draw_cube(const Vector3& position, float width, float height, float length, int r, int g, int b) {
    const Color color = {
        static_cast<unsigned char>(r),
        static_cast<unsigned char>(g),
        static_cast<unsigned char>(b),
        255
    };
    ::DrawCube(position, width, height, length, color);
    ::DrawCubeWires(position, width, height, length, MAROON);
}

void game_window::draw_grid(float spacing, int slices) {
    ::DrawGrid(slices, spacing);
}
