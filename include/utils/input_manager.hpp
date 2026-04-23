#pragma once

class input_manager {
private:
    bool show_mouse_ = false;

public:
    input_manager();
    void hide_mouse();
    void show_mouse();
};
