#pragma once

#include <vector>
#include <memory>

#include "lighting_system.hpp"
#include "object/object.hpp"

class scene {
private:
    lighting_system* lights_;
    std::vector<std::unique_ptr<object>> objects_;

public:
    explicit scene(lighting_system& lights): lights_(&lights) {}

    ~scene();

    scene(const scene&) = delete;
    scene& operator=(const scene&) = delete;

    template<typename T, typename... Args>
    T& add(Args&&... args) {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *ptr;

        // auto-register objects that are also lights
        if (auto* l = dynamic_cast<light*>(ptr.get())) {
            lights_->add(l);
        }

        objects_.push_back(std::move(ptr));
        return ref;
    }
};
