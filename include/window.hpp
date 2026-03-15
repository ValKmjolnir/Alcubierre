#pragma once

#include "raylib.h"
#include "raymath.h"

class game_window {
public:
    game_window(int width = 800, int height = 600, const char* title = "Game Window");
    ~game_window();

    bool should_close() const;
    void begin_drawing();
    void end_drawing();
    void clear_background(int r, int g, int b);
    
    // 3D camera methods
    void begin_mode_3d(const Camera3D& camera);
    void end_mode_3d();
    void draw_cube(const Vector3& position, float width, float height, float length, int r, int g, int b);
    void draw_grid(float spacing, int slices);

private:
    int width_;
    int height_;
};
