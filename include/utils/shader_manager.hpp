#pragma once

#include <raylib.h>

#include <cstring>
#include <sstream>
#include <string>
#include <unordered_map>

struct load_result {
    bool success;
    Shader shader;
};

class shader_manager {
private:
    std::unordered_map<std::string, Shader> shaders;

private:
    load_result try_load_shader(const char* vs_path, const char* fs_path);
    std::string resolve_includes(const char* path);
    load_result try_load_shader_with_includes(const char* vs_path, const char* fs_path);

public:
    shader_manager() = default;
    ~shader_manager();

    static shader_manager& instance();
    bool exists(const char* vs_path, const char* fs_path) const;
    load_result load(const char* vs_path, const char* fs_path);

    // Load shaders with #include resolution.
    // Searches for #include "filename" directives and resolves them
    // from the same directory as the including file.
    load_result load_with_includes(const char* vs_path, const char* fs_path);
};
