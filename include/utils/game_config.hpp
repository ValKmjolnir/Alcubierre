#pragma once

class game_config {
private:
    bool should_exit = false;
    bool enable_grid_draw = false;
    bool enable_debug_hud = false;

    int max_fps = 80;

public:
    bool get_should_exit() const { return should_exit; }
    bool get_enable_grid_draw() const { return enable_grid_draw; }
    bool get_enable_debug_hud() const { return enable_debug_hud; }
    int get_max_fps() const { return max_fps; }

public:
    static game_config& singleton();
    void set_should_exit(bool flag);
    void set_enable_grid_draw(bool flag);
    void set_enable_debug_hud(bool flag);
    void set_max_fps(int fps);
};

typedef void (game_config::*config_set_funcptr)(bool);
typedef bool (game_config::*config_get_funcptr)() const;
typedef void (game_config::*config_set_int_funcptr)(int);
typedef int (game_config::*config_get_int_funcptr)() const;
