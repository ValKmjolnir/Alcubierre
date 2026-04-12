#include "raylib.h"
#include "raymath.h"

#include "object/star.hpp"
#include "lighting_system.hpp"
#include <cmath>

star::star()
    : point_light()
    , object()
    , color_r_(255)
    , color_g_(200)
    , color_b_(100)
    , color_alpha_(255)
{
    lighting_system::instance().add_light(std::shared_ptr<star>(this, [](star*){}));
    load_texture();
}

star::star(const Vector3& position, int r, int g, int b, int alpha)
    : point_light(position, { r / 255.0f, g / 255.0f, b / 255.0f }, 1.0f)
    , object()
    , color_r_(r)
    , color_g_(g)
    , color_b_(b)
    , color_alpha_(alpha)
{
    lighting_system::instance().add_light(std::shared_ptr<star>(this, [](star*){}));
    load_texture();
}

star::~star() {
    UnloadTexture(texture_);
    lighting_system::instance().remove_light(std::shared_ptr<star>(this, [](star*){}));
}

void star::load_texture() {
    Image img = GenImageGradientRadial(512, 512, 0.0f,
        Color {
            static_cast<unsigned char>(color_r_),
            static_cast<unsigned char>(color_g_),
            static_cast<unsigned char>(color_b_),
            static_cast<unsigned char>(color_alpha_)
        },
        BLANK
    );
    texture_ = LoadTextureFromImage(img);
    UnloadImage(img);
}

void star::set_color(int r, int g, int b, int alpha) {
    color_r_ = r;
    color_g_ = g;
    color_b_ = b;
    color_alpha_ = alpha;
    color_ = { r / 255.0f, g / 255.0f, b / 255.0f };

    UnloadTexture(texture_);
    load_texture();
}

void star::draw(const camera_3d& cam) const {
    DrawBillboard(cam.get_camera(), texture_, position_, 60.0f, WHITE);
}
