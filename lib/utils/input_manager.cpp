#include <raylib.h>

#include "utils/input_manager.hpp"

input_manager::input_manager(): show_mouse_(false) {
    DisableCursor();
    SetExitKey(KEY_NULL);
}

void input_manager::hide_mouse() {
    if (show_mouse_) {
        show_mouse_ = false;
        DisableCursor();
    }
}

void input_manager::show_mouse() {
    if (!show_mouse_) {
        show_mouse_ = true;
        EnableCursor();
    }
}
