#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <cmath>

#include "object/star.hpp"
#include "lighting_system.hpp"

float smooth_maxf(float a, float b, float smooth) {
    // 1.0    ~ 5.0    ~ 10.0
    // smooth ~ medium ~ hard
    auto min_num = fminf(a, b);
    auto max_num = fmaxf(a, b);
    return max_num + logf(1.0f + expf((min_num - max_num) * smooth)) / smooth;
}

Color single_pixel(int x, int y, float center, const Color& base_color) {
    float dx = (x - center) / center;
    float dy = (y - center) / center;
    float dist = sqrtf(dx*dx + dy*dy);

    float intensity = 0.0f;
    float layer1 = 0.0f;
    float layer2 = 0.0f;
    float layer3 = 0.0f;
    if (dist <= 1.0f) {
        layer1 = powf(expf(-dist * 4.0f) * 1.0f, 1.4f) - expf(-4.0f) * 1.0f; // core
        layer2 = powf(expf(-dist * 2.0f) * 0.25f, 1.4f) - expf(-2.0f) * 0.25f;
        layer3 = powf(expf(-dist * 0.5f) * 0.125f, 1.4f) - expf(-0.5f) * 0.125f;
    }

    float absDx = fabsf(dx);
    float absDy = fabsf(dy);
    float starburst = 0;

    if (absDy < 0.08f) {
        float horizontalGlow = 1.0f - powf(absDx, 2.3f);
        float verticalSoftness = 1.0f - powf(absDy / 0.08f, 0.6f);
        starburst = horizontalGlow * verticalSoftness * 0.97f;
    }

    if (dist <= 1.0f) {
        intensity = smooth_maxf(fmaxf(fmaxf(layer1, layer2), layer3), starburst, 4.0f);
    } else {
        intensity = fmaxf(fmaxf(fmaxf(layer1, layer2), layer3), starburst);
    }
    intensity = fminf(intensity, 1.0f);

    float temp = 1.0f - dist * 0.3f;

    Color c;
    c.r = (unsigned char)(255 * temp * (base_color.r / 255.0f) * intensity);
    c.g = (unsigned char)(255 * temp * (base_color.g / 255.0f) * intensity);
    c.b = (unsigned char)(255 * temp * (base_color.b / 255.0f) * intensity);
    c.a = (unsigned char)(255 * temp * intensity);

    return c;
}

Image generate_layered_star(int size, const Color& base_color) {
    Image im = GenImageColor(size, size, BLANK);
    Color *pixels = (Color *)im.data;

    float center = size / 2.0f;
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            pixels[y * size + x] = single_pixel(x, y, center, base_color);
        }
    }

    return im;
}

star::star()
    : point_light()
    , object()
    , color_r_(255)
    , color_g_(200)
    , color_b_(100)
    , color_alpha_(255)
{
    lighting_system::instance().add(std::shared_ptr<star>(this, [](star*){}));
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
    lighting_system::instance().add(std::shared_ptr<star>(this, [](star*){}));
    load_texture();
}

star::~star() {
    UnloadTexture(texture_);
    lighting_system::instance().remove(std::shared_ptr<star>(this, [](star*){}));
}

void star::load_texture() {
    const auto base_color = Color {
        static_cast<unsigned char>(color_r_),
        static_cast<unsigned char>(color_g_),
        static_cast<unsigned char>(color_b_),
        static_cast<unsigned char>(color_alpha_)
    };

    Image img = generate_layered_star(512, base_color);
    texture_ = LoadTextureFromImage(img);
    UnloadImage(img);
}

void star::set_color(int r, int g, int b, int alpha) {
    color_r_ = r;
    color_g_ = g;
    color_b_ = b;
    color_alpha_ = alpha;

    // light color
    color_ = { r / 255.0f, g / 255.0f, b / 255.0f };

    UnloadTexture(texture_);
    load_texture();
}

void star::draw(const camera_3d& cam, int height, float scale) const {
    float dist = Vector3Distance(cam.position(), position_);
    if (dist < 0.001f) {
        dist = 0.001f;
    }
    float fov_factor = 2.0f * tanf(cam.fovy() * DEG2RAD / 2.0f);
    float final_scale = scale * (dist * fov_factor) / height;
    DrawBillboard(cam.get_camera(), texture_, position_, final_scale, WHITE);
}
