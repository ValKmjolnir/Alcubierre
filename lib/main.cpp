#include "window.hpp"

int main() {
    game_window window(1600, 800, "Alcubierre");

    while (!window.should_close()) {
        window.begin_drawing();
        window.clear_background(25, 25, 25);
        window.end_drawing();
    }

    return 0;
}
