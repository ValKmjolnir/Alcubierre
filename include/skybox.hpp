#pragma once

#include <raylib.h>
#include <string>

class skybox {
public:
    skybox();
    ~skybox();

    // Draw the skybox
    void draw(const Camera3D& camera);

    // Check if skybox is loaded
    bool is_loaded() const;

    // Set the random seed for star generation
    void set_seed(float seed);

private:
    void init_mesh();
    void load_shader();
    void build_cubemap();
    void unload_cubemap();

    Mesh mesh_;
    Shader shader_;
    Material material_;
    Material default_material_;
    TextureCubemap cubemap_ {};
    bool is_loaded_;
    float seed_;
    int mvp_location_;
};
