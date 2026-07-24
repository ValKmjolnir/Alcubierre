#include <chrono>

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
    using clock = std::chrono::high_resolution_clock;

    texture_handle* tmp = nullptr;
    for (auto& pass: passes) {
        if (!pass->is_enabled()) {
            continue;
        }

        clock::time_point t0;
        if (timing_enabled_) {
            t0 = clock::now();
        }

        if (tmp) {
            tmp = &(pass->apply(tmp->get(), width, height));
        } else {
            tmp = &(pass->apply(input, width, height));
        }

        if (timing_enabled_) {
            double elapsed = std::chrono::duration<double, std::milli>(
                clock::now() - t0).count();
            pass_timings_ms_[pass->get_name()] = elapsed;
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