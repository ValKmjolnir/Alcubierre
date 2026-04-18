#include "rendering/frame_graph.hpp"

void frame_graph::load() {
    for (auto& pass: passes) {
        pass->load();
    }
}

void frame_graph::unload() {
    for (auto& pass: passes) {
        pass->unload();
    }
}

void frame_graph::execute() {
    // TODO
}