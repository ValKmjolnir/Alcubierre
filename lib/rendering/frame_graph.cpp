#include "rendering/frame_graph.hpp"
#include "utils/draw_texture.hpp"

void frame_graph::set_enable(const char* name, bool enabled) {
    if (pass_index.find(name) == pass_index.end()) {
        return;
    }

    passes[pass_index.at(name)]->set_enabled(enabled);
}

bool frame_graph::enabled(const char* name) const {
    if (pass_index.find(name) == pass_index.end()) {
        return false;
    }

    return passes[pass_index.at(name)]->is_enabled();
}

void frame_graph::load() {
    result.load(target.get_width(), target.get_height());
    for (auto& pass: passes) {
        pass->load();
    }
}

void frame_graph::unload() {
    result.unload();
    for (auto& pass: passes) {
        pass->unload();
    }
}

texture_handle& frame_graph::execute(const RenderTexture2D& input,
                                     int width,
                                     int height) {
    texture_handle* tmp = nullptr;
    for (auto& pass: passes) {
        if (tmp) {
            tmp = &(pass->apply(tmp->get(), width, height));
        } else {
            tmp = &(pass->apply(input, width, height));
        }
    }

    if (tmp) {
        BeginTextureMode(result.get());
        draw_texture_to_specific_screen(tmp->get(), width, height);
        EndTextureMode();
    } else {
        BeginTextureMode(result.get());
        draw_texture_to_specific_screen(input, width, height);
        EndTextureMode();
    }
    return result;
}