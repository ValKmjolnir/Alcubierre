#include "scenes/scene.hpp"

void scene::set_lights_active(bool active) {
    for (auto& obj : objects_) {
        if (auto* l = dynamic_cast<light*>(obj.get())) {
            l->set_active(active);
        }
    }
}

void scene::set_flare_sources_visible(bool visible) {
    for (auto& obj : objects_) {
        if (obj->is_flare_source()) {
            obj->set_visible(visible);
        }
    }
}

scene::~scene() {
    // Unregister lights before objects are destroyed
    for (auto& obj : objects_) {
        if (auto* l = dynamic_cast<light*>(obj.get())) {
            lights_->remove(l);
        }
    }
}
