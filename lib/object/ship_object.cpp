#include "object/ship_object.hpp"

#include <raymath.h>

#include <cmath>

#include "window.hpp"

namespace {

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

ship_object::ship_object(game_window& win): window_(&win) {
    build();
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
        window_->draw_lit_mesh(p.mesh, p.transform, p.color.r, p.color.g, p.color.b,
                               p.spec_strength, p.shininess);
    }
}

bool ship_object::blocks_line(const Vector3& from, const Vector3& to) const {
    return ray_hits_box(from, to, -2.1f, -1.0f, -6.0f, 2.1f, 1.0f, -1.8f)
        || ray_hits_box(from, to, -3.35f, -1.05f, -1.8f, 3.35f, 1.25f, 2.6f)
        || ray_hits_box(from, to, -2.7f, -1.85f, 1.6f, 2.7f, -0.3f, 6.9f)
        || ray_hits_box(from, to, -0.35f, 1.0f, 0.2f, 0.35f, 2.35f, 2.6f);
}

void ship_object::add(Mesh mesh, const Matrix& transform, Color color,
                      float spec_strength, float shininess) {
    parts_.push_back(part { mesh, transform, color, spec_strength, shininess });
}

void ship_object::build() {
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
