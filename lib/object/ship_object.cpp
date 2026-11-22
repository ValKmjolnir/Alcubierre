#include "object/ship_object.hpp"

#include <raymath.h>

#include <cmath>

#include "window.hpp"

namespace {

// The corvette blockout was authored at 12.9 units overall; 15 units is 150 m.
constexpr float kCorvetteScale = 15.0f / 12.9f;

Matrix box_matrix(float w, float h, float l, const Vector3& pos, float rot_x_deg = 0.0f) {
    Matrix m = MatrixScale(w, h, l);
    m = MatrixMultiply(m, MatrixRotateX(rot_x_deg * DEG2RAD));
    return MatrixMultiply(m, MatrixTranslate(pos.x, pos.y, pos.z));
}

// GenMeshCylinder spans y in [0, height]; recenter, then +90 deg about X maps it onto +Z
Matrix cylinder_matrix(const Vector3& pos, float height) {
    Matrix m = MatrixTranslate(0.0f, -height * 0.5f, 0.0f);
    m = MatrixMultiply(m, MatrixRotateX(90.0f * DEG2RAD));
    return MatrixMultiply(m, MatrixTranslate(pos.x, pos.y, pos.z));
}

// Same, but -90 deg about X maps +Y onto -Z (guns pointing at the bow)
Matrix barrel_matrix(const Vector3& pos, float length) {
    Matrix m = MatrixTranslate(0.0f, -length * 0.5f, 0.0f);
    m = MatrixMultiply(m, MatrixRotateX(-90.0f * DEG2RAD));
    return MatrixMultiply(m, MatrixTranslate(pos.x, pos.y, pos.z));
}

// Cylinder centred on pos, its +Y axis rotated onto one of the six axis directions
Matrix axis_matrix(const Vector3& pos, float length, const Vector3& dir) {
    Matrix m = MatrixTranslate(0.0f, -length * 0.5f, 0.0f);
    if (dir.z > 0.5f)       m = MatrixMultiply(m, MatrixRotateX(90.0f * DEG2RAD));
    else if (dir.z < -0.5f) m = MatrixMultiply(m, MatrixRotateX(-90.0f * DEG2RAD));
    else if (dir.y < -0.5f) m = MatrixMultiply(m, MatrixRotateX(180.0f * DEG2RAD));
    else if (dir.x > 0.5f)  m = MatrixMultiply(m, MatrixRotateZ(-90.0f * DEG2RAD));
    else if (dir.x < -0.5f) m = MatrixMultiply(m, MatrixRotateZ(90.0f * DEG2RAD));
    return MatrixMultiply(m, MatrixTranslate(pos.x, pos.y, pos.z));
}

bool ray_hits_box(const Vector3& from, const Vector3& to,
                  float lo_x, float lo_y, float lo_z,
                  float hi_x, float hi_y, float hi_z) {
    const float origin[3] = { from.x, from.y, from.z };
    const float dir[3] = { to.x - from.x, to.y - from.y, to.z - from.z };
    const float lo[3] = { lo_x, lo_y, lo_z };
    const float hi[3] = { hi_x, hi_y, hi_z };

    float tmin = 0.0f;
    float tmax = 1.0f;
    for (int i = 0; i < 3; i++) {
        if (fabsf(dir[i]) < 1e-6f) {
            if (origin[i] < lo[i] || origin[i] > hi[i]) return false;
            continue;
        }
        float t1 = (lo[i] - origin[i]) / dir[i];
        float t2 = (hi[i] - origin[i]) / dir[i];
        if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
        if (t1 > tmin) tmin = t1;
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax) return false;
    }
    return tmax > 0.0f;
}

Mesh make_tapered_box(float front_hw, float front_hh,
                      float back_hw, float back_hh,
                      float length) {
    const float hz = length * 0.5f;

    const Vector3 a { -front_hw, -front_hh, -hz };
    const Vector3 b { front_hw, -front_hh, -hz };
    const Vector3 c { front_hw, front_hh, -hz };
    const Vector3 d { -front_hw, front_hh, -hz };
    const Vector3 e { -back_hw, -back_hh, hz };
    const Vector3 f { back_hw, -back_hh, hz };
    const Vector3 g { back_hw, back_hh, hz };
    const Vector3 h { -back_hw, back_hh, hz };

    const Vector3 quads[6][4] = {
        { a, d, c, b },
        { e, f, g, h },
        { a, b, f, e },
        { d, h, g, c },
        { a, e, h, d },
        { b, c, g, f },
    };

    Mesh mesh = { 0 };
    mesh.vertexCount = 24;
    mesh.triangleCount = 12;
    mesh.vertices = static_cast<float*>(MemAlloc(mesh.vertexCount * 3 * sizeof(float)));
    mesh.normals = static_cast<float*>(MemAlloc(mesh.vertexCount * 3 * sizeof(float)));
    mesh.texcoords = static_cast<float*>(MemAlloc(mesh.vertexCount * 2 * sizeof(float)));
    mesh.indices = static_cast<unsigned short*>(MemAlloc(mesh.triangleCount * 3 * sizeof(unsigned short)));

    int v = 0;
    int idx = 0;
    for (int q = 0; q < 6; q++) {
        const Vector3 normal = Vector3Normalize(Vector3CrossProduct(
            Vector3Subtract(quads[q][1], quads[q][0]),
            Vector3Subtract(quads[q][2], quads[q][0])));

        for (int k = 0; k < 4; k++) {
            mesh.vertices[v * 3 + 0] = quads[q][k].x;
            mesh.vertices[v * 3 + 1] = quads[q][k].y;
            mesh.vertices[v * 3 + 2] = quads[q][k].z;
            mesh.normals[v * 3 + 0] = normal.x;
            mesh.normals[v * 3 + 1] = normal.y;
            mesh.normals[v * 3 + 2] = normal.z;
            mesh.texcoords[v * 2 + 0] = (k == 1 || k == 2) ? 1.0f : 0.0f;
            mesh.texcoords[v * 2 + 1] = (k >= 2) ? 1.0f : 0.0f;
            v++;
        }

        const unsigned short base = static_cast<unsigned short>(q * 4);
        mesh.indices[idx++] = base + 0;
        mesh.indices[idx++] = base + 1;
        mesh.indices[idx++] = base + 2;
        mesh.indices[idx++] = base + 0;
        mesh.indices[idx++] = base + 2;
        mesh.indices[idx++] = base + 3;
    }

    UploadMesh(&mesh, false);
    return mesh;
}

} // namespace

ship_object::ship_object(game_window& win, hull_class cls, const Vector3& position, float yaw_deg)
    : window_(&win)
    , class_(cls)
    , position_(position)
    , yaw_deg_(yaw_deg) {
    const float scale = (cls == hull_class::corvette) ? kCorvetteScale : 1.0f;

    model_ = MatrixMultiply(
        MatrixMultiply(MatrixScale(scale, scale, scale), MatrixRotateY(yaw_deg * DEG2RAD)),
        MatrixTranslate(position.x, position.y, position.z));

    if (cls == hull_class::corvette) {
        build_corvette();
    } else {
        build_titan();
    }
}

ship_object::~ship_object() {
    for (auto& p : parts_) {
        UnloadMesh(p.mesh);
    }
}

void ship_object::draw(const camera_3d& cam, int window_height) const {
    (void)cam;
    (void)window_height;
    for (const auto& p : parts_) {
        lit_surface surface;
        surface.albedo = p.color;
        surface.spec_strength = p.spec_strength;
        surface.shininess = p.shininess;
        window_->draw_lit_mesh(p.mesh, MatrixMultiply(p.transform, model_), surface);
    }
}

bool ship_object::blocks_line(const Vector3& from, const Vector3& to) const {
    const Matrix inv = MatrixInvert(model_);
    const Vector3 a = Vector3Transform(from, inv);
    const Vector3 b = Vector3Transform(to, inv);

    if (class_ == hull_class::corvette) {
        return ray_hits_box(a, b, -2.1f, -1.0f, -6.0f, 2.1f, 1.0f, -1.8f)
            || ray_hits_box(a, b, -3.35f, -1.05f, -1.8f, 3.35f, 1.25f, 2.6f)
            || ray_hits_box(a, b, -2.7f, -1.85f, 1.6f, 2.7f, -0.3f, 6.9f)
            || ray_hits_box(a, b, -0.35f, 1.0f, 0.2f, 0.35f, 2.35f, 2.6f);
    }

    return ray_hits_box(a, b, -80.0f, -57.0f, -500.0f, 80.0f, 57.0f, -140.0f)
        || ray_hits_box(a, b, -108.0f, -57.0f, -140.0f, 108.0f, 57.0f, 140.0f)
        || ray_hits_box(a, b, -80.0f, -130.0f, 140.0f, 80.0f, 90.0f, 500.0f)
        || ray_hits_box(a, b, -60.0f, 55.0f, -380.0f, 60.0f, 200.0f, 240.0f)
        || ray_hits_box(a, b, -60.0f, -57.0f, 480.0f, 60.0f, 57.0f, 580.0f);
}

Vector3 ship_object::cockpit_eye() const {
    return Vector3Transform(Vector3 { 0.0f, 1.0f, -3.8f }, model_);
}

Vector3 ship_object::cockpit_forward() const {
    return Vector3Normalize(
        Vector3Transform(Vector3 { 0.0f, 0.0f, -1.0f }, MatrixRotateY(yaw_deg_ * DEG2RAD)));
}

void ship_object::add(Mesh mesh, const Matrix& transform, Color color,
                      float spec_strength, float shininess) {
    parts_.push_back(part { mesh, transform, color, spec_strength, shininess });
}

void ship_object::build_corvette() {
    const Color hull_light { 165, 170, 180, 255 };
    const Color hull_panel { 148, 153, 163, 255 };
    const Color hull_mid { 120, 126, 138, 255 };
    const Color hull_dark { 60, 64, 74, 255 };
    const Color nozzle_color { 32, 34, 40, 255 };
    const Color canopy_color { 35, 70, 95, 255 };
    const Color fin_color { 140, 144, 154, 255 };

    // Fore hull: z -6.0..-1.8, long taper down to a narrow nose
    add(make_tapered_box(1.1f, 0.55f, 2.1f, 1.0f, 4.2f),
        MatrixTranslate(0.0f, 0.0f, -3.9f), hull_mid, 1.8f, 12.0f);

    // Mid hull: three sections with slightly different cross sections
    add(make_tapered_box(2.1f, 1.0f, 2.3f, 1.05f, 1.2f),
        MatrixTranslate(0.0f, 0.0f, -1.2f), hull_light, 2.0f, 12.0f);
    add(make_tapered_box(2.3f, 1.05f, 2.2f, 1.0f, 1.8f),
        MatrixTranslate(0.0f, 0.0f, 0.3f), hull_panel, 2.0f, 12.0f);
    add(make_tapered_box(2.2f, 1.0f, 2.0f, 0.95f, 1.4f),
        MatrixTranslate(0.0f, 0.0f, 1.9f), hull_light, 2.0f, 12.0f);

    // Tail cone: z +2.6..+5.6, tapering to a blunt stern
    add(make_tapered_box(2.0f, 0.95f, 1.0f, 0.42f, 3.0f),
        MatrixTranslate(0.0f, 0.0f, 4.1f), hull_mid, 2.0f, 12.0f);

    // Engine pods hanging under the wings, nozzles at z +6.6
    for (int side = -1; side <= 1; side += 2) {
        const float x = 1.95f * side;
        add(GenMeshCylinder(0.72f, 5.2f, 24), cylinder_matrix(Vector3 { x, -1.12f, 4.2f }, 5.2f),
            hull_mid, 1.8f, 16.0f);
        add(GenMeshCylinder(0.5f, 0.6f, 24), cylinder_matrix(Vector3 { x, -1.12f, 6.6f }, 0.6f),
            nozzle_color, 0.8f, 8.0f);
    }

    // Wings over the pods
    for (int side = -1; side <= 1; side += 2) {
        add(GenMeshCube(1.0f, 1.0f, 1.0f),
            box_matrix(1.2f, 0.16f, 1.8f, Vector3 { 2.7f * side, -0.35f, 1.7f }),
            hull_mid, 1.8f, 16.0f);
        add(GenMeshCube(1.0f, 1.0f, 1.0f),
            box_matrix(0.22f, 0.22f, 1.2f, Vector3 { 3.28f * side, -0.35f, 1.7f }),
            hull_dark, 0.8f, 8.0f);
    }

    // Dorsal fin, leaning back
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(0.22f, 1.3f, 2.4f, Vector3 { 0.0f, 1.65f, 1.4f }, 14.0f),
        fin_color, 1.8f, 12.0f);

    // Canopy: glass box on a wider base frame
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(1.3f, 0.55f, 2.0f, Vector3 { 0.0f, 0.95f, -4.1f }),
        canopy_color, 2.5f, 20.0f);
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(1.45f, 0.16f, 1.5f, Vector3 { 0.0f, 0.7f, -3.9f }),
        hull_panel, 1.4f, 12.0f);

    // Dorsal spine blocks
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(0.6f, 0.26f, 1.3f, Vector3 { 0.0f, 1.1f, -0.5f }),
        hull_panel, 1.8f, 16.0f);
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(0.6f, 0.26f, 1.3f, Vector3 { 0.0f, 1.1f, 1.1f }),
        hull_panel, 1.8f, 16.0f);

    // Side blisters
    for (int side = -1; side <= 1; side += 2) {
        add(GenMeshCube(1.0f, 1.0f, 1.0f),
            box_matrix(0.55f, 0.4f, 2.2f, Vector3 { 2.35f * side, 0.05f, 0.7f }),
            hull_mid, 1.4f, 12.0f);
    }

    // Ventral strakes
    for (int side = -1; side <= 1; side += 2) {
        add(GenMeshCube(1.0f, 1.0f, 1.0f),
            box_matrix(0.12f, 0.3f, 2.8f, Vector3 { 0.95f * side, -1.05f, 0.6f }),
            hull_dark, 0.8f, 8.0f);
    }
}

void ship_object::build_titan() {
    const Color hull_light { 122, 127, 136, 255 };
    const Color hull_panel { 106, 111, 120, 255 };
    const Color hull_mid { 90, 95, 104, 255 };
    const Color hull_dark { 52, 56, 64, 255 };
    const Color deck_color { 74, 78, 86, 255 };
    const Color tower_color { 132, 137, 146, 255 };
    const Color nozzle_color { 38, 40, 46, 255 };
    const Color barrel_color { 66, 70, 78, 255 };
    const Color mouth_color { 26, 28, 34, 255 };

    const Vector3 up { 0.0f, 1.0f, 0.0f };
    const Vector3 fwd { 0.0f, 0.0f, -1.0f };
    const Vector3 down { 0.0f, -1.0f, 0.0f };
    const Vector3 right { 1.0f, 0.0f, 0.0f };

    // --- Hull loft: ram, two stepped shoulders, midships, flared stern ---
    add(make_tapered_box(22.0f, 18.0f, 50.0f, 34.0f, 50.0f),
        MatrixTranslate(0.0f, 0.0f, -475.0f), hull_light, 0.25f, 10.0f);
    add(make_tapered_box(50.0f, 34.0f, 70.0f, 50.0f, 120.0f),
        MatrixTranslate(0.0f, 0.0f, -390.0f), hull_mid, 0.25f, 10.0f);
    add(make_tapered_box(70.0f, 50.0f, 75.0f, 55.0f, 150.0f),
        MatrixTranslate(0.0f, 0.0f, -255.0f), hull_light, 0.25f, 10.0f);
    add(make_tapered_box(75.0f, 55.0f, 78.0f, 57.0f, 240.0f),
        MatrixTranslate(0.0f, 0.0f, -60.0f), hull_panel, 0.25f, 10.0f);
    add(make_tapered_box(78.0f, 57.0f, 72.0f, 53.0f, 160.0f),
        MatrixTranslate(0.0f, 0.0f, 140.0f), hull_light, 0.25f, 10.0f);
    add(make_tapered_box(72.0f, 53.0f, 64.0f, 48.0f, 160.0f),
        MatrixTranslate(0.0f, 0.0f, 300.0f), hull_mid, 0.25f, 10.0f);
    add(make_tapered_box(64.0f, 48.0f, 48.0f, 46.0f, 120.0f),
        MatrixTranslate(0.0f, 0.0f, 440.0f), hull_panel, 0.25f, 10.0f);

    // --- Armour seams: proud dark belts at the section joints ---
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(158.0f, 26.0f, 7.0f, Vector3 { 0.0f, 4.0f, -330.0f }),
        hull_dark, 0.15f, 10.0f);
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(164.0f, 26.0f, 7.0f, Vector3 { 0.0f, 4.0f, -40.0f }),
        hull_dark, 0.15f, 10.0f);
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(152.0f, 26.0f, 7.0f, Vector3 { 0.0f, 4.0f, 220.0f }),
        hull_dark, 0.15f, 10.0f);

    // --- Deck, dorsal spine, ventral keel ---
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(150.0f, 10.0f, 750.0f, Vector3 { 0.0f, 60.0f, 45.0f }),
        deck_color, 0.2f, 12.0f);
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(36.0f, 16.0f, 600.0f, Vector3 { 0.0f, 70.0f, 10.0f }),
        hull_mid, 0.3f, 12.0f);
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(18.0f, 12.0f, 400.0f, Vector3 { 0.0f, 82.0f, 40.0f }),
        hull_panel, 0.3f, 12.0f);
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(28.0f, 48.0f, 360.0f, Vector3 { 0.0f, -78.0f, 20.0f }),
        hull_dark, 0.2f, 10.0f);
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(12.0f, 34.0f, 150.0f, Vector3 { 0.0f, -112.0f, 90.0f }),
        hull_mid, 0.2f, 10.0f);

    // --- Command block: wide deck, offset tower, second mast ---
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(100.0f, 22.0f, 190.0f, Vector3 { 0.0f, 76.0f, 110.0f }),
        hull_light, 0.3f, 12.0f);
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(74.0f, 20.0f, 120.0f, Vector3 { 0.0f, 96.0f, 130.0f }),
        hull_panel, 0.3f, 12.0f);
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(46.0f, 34.0f, 66.0f, Vector3 { 14.0f, 122.0f, 145.0f }),
        tower_color, 0.35f, 12.0f);
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(26.0f, 12.0f, 32.0f, Vector3 { 14.0f, 146.0f, 145.0f }),
        hull_mid, 0.3f, 12.0f);
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(10.0f, 40.0f, 10.0f, Vector3 { -24.0f, 125.0f, 120.0f }),
        hull_dark, 0.1f, 8.0f);
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(6.0f, 42.0f, 6.0f, Vector3 { -24.0f, 165.0f, 120.0f }),
        hull_dark, 0.1f, 8.0f);
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(20.0f, 8.0f, 16.0f, Vector3 { -24.0f, 188.0f, 120.0f }),
        hull_mid, 0.3f, 12.0f);

    // --- Spinal lance overhanging the bow ---
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(26.0f, 20.0f, 130.0f, Vector3 { 0.0f, 78.0f, -300.0f }),
        hull_mid, 0.3f, 12.0f);
    add(GenMeshCylinder(6.0f, 190.0f, 12),
        axis_matrix(Vector3 { 0.0f, 78.0f, -450.0f }, 190.0f, fwd),
        barrel_color, 0.6f, 16.0f);
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(10.0f, 16.0f, 60.0f, Vector3 { 0.0f, 66.0f, -350.0f }),
        hull_dark, 0.2f, 10.0f);

    // --- Dorsal main turrets, superfiring ---
    add(GenMeshCylinder(26.0f, 12.0f, 16),
        axis_matrix(Vector3 { 0.0f, 71.0f, -190.0f }, 12.0f, up), hull_mid, 0.3f, 12.0f);
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(50.0f, 26.0f, 42.0f, Vector3 { 0.0f, 90.0f, -190.0f }),
        hull_light, 0.4f, 14.0f);
    for (int i = -1; i <= 1; i++) {
        add(GenMeshCylinder(2.6f, 70.0f, 10),
            axis_matrix(Vector3 { i * 16.0f, 90.0f, -246.0f }, 70.0f, fwd),
            barrel_color, 0.6f, 16.0f);
    }
    add(GenMeshCylinder(26.0f, 26.0f, 16),
        axis_matrix(Vector3 { 0.0f, 78.0f, -85.0f }, 26.0f, up), hull_mid, 0.3f, 12.0f);
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(50.0f, 26.0f, 42.0f, Vector3 { 0.0f, 104.0f, -85.0f }),
        hull_light, 0.4f, 14.0f);
    for (int i = -1; i <= 1; i++) {
        add(GenMeshCylinder(2.6f, 70.0f, 10),
            axis_matrix(Vector3 { i * 16.0f, 104.0f, -141.0f }, 70.0f, fwd),
            barrel_color, 0.6f, 16.0f);
    }

    // --- Ventral turrets, mirrored under the keel shoulders ---
    for (int side = -1; side <= 1; side += 2) {
        const float x = 42.0f * side;
        const float z = (side < 0) ? -150.0f : 110.0f;
        add(GenMeshCylinder(24.0f, 12.0f, 16),
            axis_matrix(Vector3 { x, -63.0f, z }, 12.0f, up), hull_mid, 0.3f, 12.0f);
        add(GenMeshCube(1.0f, 1.0f, 1.0f),
            box_matrix(46.0f, 26.0f, 40.0f, Vector3 { x, -82.0f, z }),
            hull_light, 0.4f, 14.0f);
        for (int i = -1; i <= 1; i++) {
            add(GenMeshCylinder(2.6f, 60.0f, 10),
                axis_matrix(Vector3 { x + i * 14.0f, -126.0f, z }, 60.0f, down),
                barrel_color, 0.6f, 16.0f);
        }
    }

    // --- Flank casemates: broadside guns set into the hull sides ---
    for (int side = -1; side <= 1; side += 2) {
        for (int k = 0; k < 2; k++) {
            const float z = (k == 0) ? -270.0f : 70.0f;
            add(GenMeshCube(1.0f, 1.0f, 1.0f),
                box_matrix(22.0f, 28.0f, 50.0f, Vector3 { 80.0f * side, 12.0f, z }),
                hull_panel, 0.35f, 12.0f);
            add(GenMeshCylinder(2.4f, 34.0f, 10),
                axis_matrix(Vector3 { 108.0f * side, 12.0f, z }, 34.0f,
                            Vector3 { 1.0f * side, 0.0f, 0.0f }),
                barrel_color, 0.5f, 14.0f);
        }
    }

    // --- Hangar mouths on both flanks ---
    for (int side = -1; side <= 1; side += 2) {
        add(GenMeshCube(1.0f, 1.0f, 1.0f),
            box_matrix(8.0f, 6.0f, 130.0f, Vector3 { 81.0f * side, 36.0f, -10.0f }),
            hull_mid, 0.25f, 12.0f);
        add(GenMeshCube(1.0f, 1.0f, 1.0f),
            box_matrix(8.0f, 6.0f, 130.0f, Vector3 { 81.0f * side, -6.0f, -10.0f }),
            hull_mid, 0.25f, 12.0f);
        add(GenMeshCube(1.0f, 1.0f, 1.0f),
            box_matrix(4.0f, 34.0f, 118.0f, Vector3 { 83.0f * side, 15.0f, -10.0f }),
            mouth_color, 0.1f, 8.0f);
    }

    // --- Radiator panels: dorsal and ventral blades, they carry the silhouette ---
    for (int side = -1; side <= 1; side += 2) {
        for (int k = 0; k < 2; k++) {
            const float z = (k == 0) ? -230.0f : -90.0f;
            add(GenMeshCube(1.0f, 1.0f, 1.0f),
                box_matrix(5.0f, 62.0f, 130.0f, Vector3 { 34.0f * side, 120.0f, z }),
                hull_panel, 0.3f, 12.0f);
            add(GenMeshCube(1.0f, 1.0f, 1.0f),
                box_matrix(14.0f, 10.0f, 90.0f, Vector3 { 34.0f * side, 92.0f, z }),
                hull_dark, 0.2f, 10.0f);
        }
    }
    for (int side = -1; side <= 1; side += 2) {
        for (int k = 0; k < 2; k++) {
            const float z = (k == 0) ? 200.0f : 320.0f;
            add(GenMeshCube(1.0f, 1.0f, 1.0f),
                box_matrix(5.0f, 68.0f, 110.0f, Vector3 { 34.0f * side, -90.0f, z }),
                hull_panel, 0.3f, 12.0f);
            add(GenMeshCube(1.0f, 1.0f, 1.0f),
                box_matrix(14.0f, 10.0f, 80.0f, Vector3 { 34.0f * side, -60.0f, z }),
                hull_dark, 0.2f, 10.0f);
        }
    }

    // --- Engine block on an exposed frame ---
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(120.0f, 96.0f, 34.0f, Vector3 { 0.0f, 0.0f, 508.0f }),
        hull_dark, 0.2f, 10.0f);
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(96.0f, 74.0f, 30.0f, Vector3 { 0.0f, 0.0f, 480.0f }),
        hull_mid, 0.3f, 12.0f);
    for (int side = -1; side <= 1; side += 2) {
        for (int k = 0; k < 2; k++) {
            add(GenMeshCube(1.0f, 1.0f, 1.0f),
                box_matrix(6.0f, 6.0f, 80.0f,
                           Vector3 { 46.0f * side, (k == 0) ? -38.0f : 38.0f, 440.0f }),
                hull_dark, 0.15f, 8.0f);
        }
    }
    for (int row = -1; row <= 1; row += 2) {
        for (int col = -1; col <= 1; col++) {
            add(GenMeshCylinder(15.0f, 46.0f, 16),
                axis_matrix(Vector3 { 38.0f * col, 26.0f * row, 548.0f }, 46.0f,
                            Vector3 { 0.0f, 0.0f, 1.0f }),
                nozzle_color, 0.15f, 8.0f);
        }
    }

    // --- Bow detail ---
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(26.0f, 16.0f, 26.0f, Vector3 { 0.0f, 4.0f, -505.0f }),
        hull_dark, 0.15f, 10.0f);
    add(GenMeshCube(1.0f, 1.0f, 1.0f),
        box_matrix(6.0f, 34.0f, 6.0f, Vector3 { 20.0f, 26.0f, -420.0f }),
        hull_mid, 0.1f, 8.0f);

    // --- Point-defence clusters ---
    for (int side = -1; side <= 1; side += 2) {
        for (int k = 0; k < 2; k++) {
            const float z = (k == 0) ? -230.0f : 280.0f;
            add(GenMeshCube(1.0f, 1.0f, 1.0f),
                box_matrix(12.0f, 12.0f, 16.0f, Vector3 { 74.0f * side, 42.0f, z }),
                hull_dark, 0.15f, 8.0f);
        }
    }
}
