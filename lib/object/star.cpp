#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <cmath>

#include "object/star.hpp"

Color single_pixel(int x, int y, float center, const Color& base_color) {
    float dx = (x - center) / center;
    float dy = (y - center) / center;
    float dist = sqrtf(dx * dx + dy * dy);

    float intensity = 0.0f;
    float layer1 = 0.0f;
    float layer2 = 0.0f;
    float layer3 = 0.0f;
    if (dist <= 1.0f) {
        layer1 = powf(expf(-dist * 4.0f) * 1.0f, 1.4f) - expf(-4.0f) * 1.0f; // core
        layer2 = powf(expf(-dist * 2.0f) * 0.25f, 1.4f) - expf(-2.0f) * 0.25f;
        layer3 = powf(expf(-dist * 0.5f) * 0.125f, 1.4f) - expf(-0.5f) * 0.125f;
    }

    intensity = fmaxf(fmaxf(layer1, layer2), layer3);
    intensity = Clamp(intensity, 0.0f, 1.0f);

    float bump = expf(-(dist * dist) / (0.12f * 0.12f));
    intensity = Clamp(intensity * (1.0f + 0.15f * bump), 0.0f, 1.0f);

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

Image generate_flare_dot(int size, const Color& base_color) {
    Image im = GenImageColor(size, size, BLANK);
    Color *pixels = (Color *)im.data;

    float center = size / 2.0f;
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            float dx = (x - center + 0.5f) / center;
            float dy = (y - center + 0.5f) / center;
            float intensity = expf(-(dx * dx + dy * dy) * 6.0f);

            Color c = base_color;
            c.a = (unsigned char)(base_color.a * intensity);
            pixels[y * size + x] = c;
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
    load_texture();
    load_flare_texture();
}

star::star(const Vector3& position, int r, int g, int b, int alpha)
    : point_light(position, { r / 255.0f, g / 255.0f, b / 255.0f }, 1.0f)
    , object()
    , color_r_(r)
    , color_g_(g)
    , color_b_(b)
    , color_alpha_(alpha)
{
    load_texture();
    load_flare_texture();
}

star::~star() {
    UnloadTexture(texture_);
    UnloadTexture(flare_texture_);
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

void star::load_flare_texture() {
    const auto base_color = Color {
        static_cast<unsigned char>(color_r_),
        static_cast<unsigned char>(color_g_),
        static_cast<unsigned char>(color_b_),
        static_cast<unsigned char>(color_alpha_)
    };

    Image img = generate_flare_dot(64, base_color);
    flare_texture_ = LoadTextureFromImage(img);
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
    UnloadTexture(flare_texture_);
    load_flare_texture();
}

void star::draw(const camera_3d& cam, int window_height) const {
    float dist = Vector3Distance(cam.position(), position_);
    if (dist < 0.001f) {
        dist = 0.001f;
    }
    float fov_factor = 2.0f * tanf(cam.fovy() * DEG2RAD / 2.0f);
    float final_scale = scale_ * (dist * fov_factor) / window_height;

    // The sprite holds a constant ~55 deg apparent size. DrawBillboard queues
    // into the rlgl batch, which only flushes at EndMode3D -- after every
    // DrawMesh -- so the quad would paint over the ships whenever the star is
    // in front of the camera. Flush it here, depth-free: it is a backdrop.
    rlDisableDepthTest();
    rlDisableDepthMask();
    DrawBillboard(cam.get_camera(), texture_, position_, final_scale, WHITE);
    rlDrawRenderBatchActive();
    rlEnableDepthMask();
    rlEnableDepthTest();
}

Vector2 star::draw_flare(const camera_3d& cam, int rt_width, int rt_height, int window_height) const {
    const Vector3 forward = Vector3Subtract(cam.target(), cam.position());
    if (Vector3DotProduct(Vector3Subtract(position_, cam.position()), forward) <= 0.0f) {
        return Vector2 { -1.0f, -1.0f };
    }

    const Vector2 screen = GetWorldToScreenEx(position_, cam.get_camera(), rt_width, rt_height);
    const float size = flare_thickness_ * (float)rt_height;

    if (screen.x + size < 0.0f || screen.y + size < 0.0f ||
        screen.x - size > (float)rt_width || screen.y - size > (float)rt_height) {
        return Vector2 { -1.0f, -1.0f };
    }

    const Rectangle src = {
        0.0f, 0.0f,
        (float)flare_texture_.width, (float)flare_texture_.height
    };
    const Rectangle dst = {
        screen.x - size * 0.5f, screen.y - size * 0.5f, size, size
    };
    DrawTexturePro(flare_texture_, src, dst, Vector2 { 0.0f, 0.0f }, 0.0f, WHITE);

    return screen;
}
