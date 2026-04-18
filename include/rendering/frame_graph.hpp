#pragma once

#include "rendering/render_target.hpp"
#include "rendering/render_pass.hpp"

#include <vector>
#include <memory>
#include <unordered_map>

class frame_graph {
private:
    render_target target;
    std::unordered_map<std::string, int64_t> pass_index;
    std::vector<std::unique_ptr<render_pass>> passes;
    texture_handle result;

public:
    template<typename T>
    render_pass& add_pass(const char* name) {
        auto pass = std::make_unique<T>(
            name,
            target.get_width(),
            target.get_height()
        );
        pass_index[name] = passes.size();
        passes.push_back(std::move(pass));
        return *passes.back();
    }

    template<typename T>
    T& get_pass(const char* name) {
        if (pass_index.count(name) == 0) {
            throw std::runtime_error("pass not found");
        }
        return dynamic_cast<T&>(*passes[pass_index.at(name)]);
    }

public:
    void set_enable(const char* name, bool enabled);
    bool enabled(const char* name) const;

public:
    frame_graph(int w, int h): target(w, h) {}
    void load();
    void unload();
    texture_handle& execute(const RenderTexture2D& input, int width, int height);
};