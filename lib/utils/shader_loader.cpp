#include "utils/shader_loader.hpp"

load_result try_load_shader(const char* vs_path, const char* fs_path) {
    const char* possible_directories[] = {
        "shaders",
        "build_cmake/shaders",
    };

    for (auto directory : possible_directories) {
        std::string real_vs_path = std::string(directory) + "/" + vs_path;
        std::string real_fs_path = std::string(directory) + "/" + fs_path;

        auto shader = LoadShader(real_vs_path.c_str(), real_fs_path.c_str());
        if (IsShaderValid(shader)) {
            return { true, shader };
        }
    }

    return { false };
}