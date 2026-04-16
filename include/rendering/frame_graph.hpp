#pragma once

#include "rendering/render_pass.hpp"

#include <vector>
#include <memory>

class frame_graph {
private:
    std::vector<std::unique_ptr<render_pass>> passes;

public:
    render_pass& add_pass(const char* name) {
        auto pass = std::make_unique<render_pass>(name);
        passes.push_back(std::move(pass));
        return *passes.back();
    }

    void read(render_pass& from, render_pass& to);
    void execute();
};