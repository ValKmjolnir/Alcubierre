#pragma once

class game_config {
private:
    bool should_exit = false;
    bool enable_grid_draw = false;
    bool enable_debug_hud = false;

    int unapplied_max_fps = 80;
    int current_max_fps = 80;

public:
    void apply_maximum_fps();

public:
    bool get_should_exit() const { return should_exit; }
    bool get_enable_grid_draw() const { return enable_grid_draw; }
    bool get_enable_debug_hud() const { return enable_debug_hud; }
    int get_unapplied_max_fps() const { return unapplied_max_fps; }
    int get_applied_max_fps() const { return current_max_fps; }

public:
    static game_config& singleton();
    void set_should_exit(bool flag);
    void set_enable_grid_draw(bool flag);
    void set_enable_debug_hud(bool flag);
    void set_unapplied_max_fps(int fps);
};

typedef void (game_config::*config_set_funcptr)(bool);
typedef bool (game_config::*config_get_funcptr)() const;
typedef void (game_config::*config_set_int_funcptr)(int);
typedef int (game_config::*config_get_int_funcptr)() const;
