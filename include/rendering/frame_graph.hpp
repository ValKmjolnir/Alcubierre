#pragma once

#include "rendering/render_target.hpp"
#include "rendering/render_pass.hpp"

#include <vector>
#include <memory>

class frame_graph {
private:
    render_target target;
    std::vector<std::unique_ptr<render_pass>> passes;

public:
    frame_graph(int w, int h): target(w, h) {}
    render_pass& add_pass(const char* name) {
        auto pass = std::make_unique<render_pass>(
            name,
            target.get_width(),
            target.get_height()
        );
        passes.push_back(std::move(pass));
        return *passes.back();
    }
    void load();
    void unload();
    void execute();
};