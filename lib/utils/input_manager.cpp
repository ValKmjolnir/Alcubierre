#include <raylib.h>

#include "utils/input_manager.hpp"

input_manager::input_manager() {
    HideCursor();
    SetExitKey(KEY_NULL);
}

void input_manager::hide_mouse() {
    if (!IsCursorHidden()) {
        HideCursor();
    }
}

void input_manager::show_mouse() {
    if (IsCursorHidden()) {
        ShowCursor();
    }
}
