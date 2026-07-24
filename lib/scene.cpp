#include "scene.hpp"

scene::~scene() {
    // Unregister lights before objects are destroyed
    for (auto& obj : objects_) {
        if (auto* l = dynamic_cast<light*>(obj.get())) {
            lights_->remove(l);
        }
    }
}
