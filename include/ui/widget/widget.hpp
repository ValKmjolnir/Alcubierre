#pragma once

class widget {
protected:
    bool mouse_pressed = false;

public:
    virtual bool is_hovered(int, int) const { return false; }
    virtual void mouse_press_call_back() {}
    virtual void mouse_release_call_back() {}
    void handle_mouse_press() {
        mouse_pressed = true;
        mouse_press_call_back();
    }
    void handle_mouse_release() {
        mouse_pressed = false;
        mouse_release_call_back();
    }
    bool get_mouse_pressed() const { return mouse_pressed; }
};