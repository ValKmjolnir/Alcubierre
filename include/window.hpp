#ifndef GAME_WINDOW_H
#define GAME_WINDOW_H

class game_window {
public:
    game_window(int width = 800, int height = 600, const char* title = "Game Window");
    ~game_window();

    bool should_close() const;
    void begin_drawing();
    void end_drawing();
    void clear_background(int r, int g, int b);

private:
    int width_;
    int height_;
};

#endif // GAME_WINDOW_H
