#include "utils/shader_manager.hpp"

load_result shader_manager::try_load_shader(const char* vs_path, const char* fs_path) {
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

shader_manager::~shader_manager() {
    for (auto& [name, shader] : shaders) {
        printf("Unloading shader %s [%d]\n", name.c_str(), shader.id);
        UnloadShader(shader);
    }
    shaders.clear();
}

shader_manager& shader_manager::instance() {
    static shader_manager shm;
    return shm;
}

bool shader_manager::exists(const char* vs_path, const char* fs_path) const {
    const std::string name = std::string(vs_path) + ":" + std::string(fs_path);
    return shaders.find(name) != shaders.end();
}

load_result shader_manager::load(const char* vs_path, const char* fs_path) {
    const std::string name = std::string(vs_path) + ":" + std::string(fs_path);
    if (shaders.find(name) != shaders.end()) {
        return { true, shaders.at(name) };
    }

    auto res = try_load_shader(vs_path, fs_path);
    if (res.success) {
        shaders[name] = res.shader;
        return res;
    }

    return { false };
}