#include "utils/shader_manager.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

static std::string read_file_content(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return "";
    std::stringstream buf;
    buf << file.rdbuf();
    return buf.str();
}

std::string shader_manager::resolve_includes(const char* path) {
    const char* possible_directories[] = {
        "shaders",
        "build_cmake/shaders",
    };

    // Find the file: try direct path first (for recursive calls),
    // then search standard directories
    std::string full_path;
    {
        std::ifstream direct(path);
        if (direct.good()) {
            full_path = path;
        }
    }
    if (full_path.empty()) {
        for (auto dir : possible_directories) {
            std::string candidate = std::string(dir) + "/" + path;
            std::ifstream test(candidate);
            if (test.good()) {
                full_path = candidate;
                break;
            }
        }
    }

    if (full_path.empty()) {
        return "";
    }

    // Extract base directory for resolving relative includes
    std::string base_dir;
    size_t slash = full_path.find_last_of("/\\");
    if (slash != std::string::npos) {
        base_dir = full_path.substr(0, slash);
    }

    std::string source = read_file_content(full_path);
    if (source.empty()) return "";

    std::string result;
    std::istringstream stream(source);
    std::string line;
    while (std::getline(stream, line)) {
        // Check for #include "filename" directive
        size_t inc_pos = line.find("#include \"");
        if (inc_pos != std::string::npos) {
            size_t start = inc_pos + 10; // length of '#include "'
            size_t end = line.find('"', start);
            if (end != std::string::npos) {
                std::string include_name = line.substr(start, end - start);

                // Search for include file: first relative to base_dir,
                // then in standard directories
                std::string inc_content;
                if (!base_dir.empty()) {
                    inc_content = resolve_includes(
                        (base_dir + "/" + include_name).c_str());
                }
                if (inc_content.empty()) {
                    inc_content = resolve_includes(include_name.c_str());
                }

                if (!inc_content.empty()) {
                    result += inc_content;
                    continue; // Skip the #include line itself
                } else {
                    std::cerr << "shader_manager: failed to resolve #include \""
                              << include_name << "\"" << std::endl;
                }
            }
        }
        result += line + "\n";
    }

    return result;
}

load_result shader_manager::try_load_shader_with_includes(const char* vs_path, const char* fs_path) {
    std::string vs_source = resolve_includes(vs_path);
    std::string fs_source = resolve_includes(fs_path);

    if (vs_source.empty() || fs_source.empty()) {
        return {false};
    }

    Shader shader = LoadShaderFromMemory(vs_source.c_str(), fs_source.c_str());
    if (IsShaderValid(shader)) {
        return {true, shader};
    }
    return {false};
}

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
    if (shaders.size()) {
        printf("shader_manager::~shader_manager(): unloading\n");
    }
    for (auto& [name, shader] : shaders) {
        printf("  - shader [%s]: id = [%d]\n", name.c_str(), shader.id);
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

load_result shader_manager::load_with_includes(const char* vs_path, const char* fs_path) {
    const std::string name = std::string(vs_path) + ":" + std::string(fs_path);
    if (shaders.find(name) != shaders.end()) {
        return { true, shaders.at(name) };
    }

    auto res = try_load_shader_with_includes(vs_path, fs_path);
    if (res.success) {
        shaders[name] = res.shader;
        return res;
    }

    // Fallback to normal loading
    return load(vs_path, fs_path);
}
