#pragma once

#include <vector>
#include <memory>

#include "lighting_system.hpp"
#include "object/object.hpp"
#include "camera.hpp"

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

        if (auto* l = dynamic_cast<light*>(ptr.get())) {
            lights_->add(l);
        }

        objects_.push_back(std::move(ptr));
        return ref;
    }

    void update_all(float dt) {
        for (auto& obj : objects_) {
            obj->update(dt);
        }
    }

    void draw_all(const camera_3d& cam, int window_height) const {
        for (auto& obj : objects_) {
            obj->draw(cam, window_height);
        }
    }

    Vector2 draw_flares(const camera_3d& cam, int rt_width, int rt_height, int window_height) const {
        Vector2 origin { -1.0f, -1.0f };
        BeginBlendMode(BLEND_ADDITIVE);
        for (auto& obj : objects_) {
            if (!obj->is_flare_source()) {
                continue;
            }

            const Vector3 source = obj->flare_position();
            bool blocked = false;
            for (auto& other : objects_) {
                if (other.get() != obj.get() && other->blocks_line(cam.position(), source)) {
                    blocked = true;
                    break;
                }
            }
            if (blocked) {
                continue;
            }

            Vector2 screen = obj->draw_flare(cam, rt_width, rt_height, window_height);
            if (origin.x < 0.0f && screen.x >= 0.0f) {
                origin = Vector2 { screen.x / (float)rt_width, 1.0f - screen.y / (float)rt_height };
            }
        }
        EndBlendMode();
        return origin;
    }
};
