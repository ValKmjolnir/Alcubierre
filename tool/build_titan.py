"""Generate the 10 km titan and export it as .glb.

Run on the Windows host:

    blender.exe --background --python D:\\code\\Alcubierre\\tool\\build_titan.py

Set PREVIEW=1 to also render turnaround stills into .qwen/tmp/.

Axes: geometry is authored in Blender's own frame -- nose along +Y, up along
+Z -- because the glTF exporter rewrites (x, y, z) as (x, z, -y).

Design language (phase 1, blockout only): a titan is not a scaled corvette.
It is a chain of districts -- arsenal, hangar, engine -- hung on one spine,
with the whole identity carried by a single macro form.  Here that form is the
twin claw: two 3 km prongs opening off the bow, braced by exposed trusses, with
the doomsday throat recessed at the root of the fork.  The gap between the
claws is the largest single feature on the hull and it is negative space,
which is what separates a 10 km ship from a 150 m one.

Length is exactly 1000 units so model_object(target_length=1000) scales by 1.0
and the hardpoint coordinates printed at the end map 1:1 into the engine.
"""

import math
import os
import sys

import bmesh
import bpy

HERE = os.path.dirname(os.path.abspath(__file__))
OUTPUT = os.path.normpath(os.path.join(HERE, "..", "assets", "models", "titan_py.glb"))
PREVIEW_DIR = os.path.normpath(os.path.join(HERE, "..", "build", "titan_preview"))

# Deep base hull with light plating bolted over it and bronze trim: a single
# light grey reads as one machined object, which is what the Ragnarok is not.
BASE = (0.22, 0.21, 0.20)
PLATE = (0.62, 0.60, 0.56)
BRONZE = (0.46, 0.30, 0.18)
DARK = (0.13, 0.13, 0.14)
THROAT = (0.52, 0.28, 0.17)

# ---------------------------------------------------------------------------
# mesh helpers, all in Blender axes (X right, Y along the hull, Z up)
# ---------------------------------------------------------------------------


def section(half_w, top, bottom, cx=0.0, cz=0.0):
    """Twelve-point cross-section in XZ: chamfered top, faceted flank, keeled belly."""
    return [
        (cx + half_w * 0.80, cz - bottom * 0.55),
        (cx + half_w, cz - bottom * 0.10),
        (cx + half_w, cz + top * 0.45),
        (cx + half_w * 0.72, cz + top * 0.82),
        (cx + half_w * 0.36, cz + top),
        (cx - half_w * 0.36, cz + top),
        (cx - half_w * 0.72, cz + top * 0.82),
        (cx - half_w, cz + top * 0.45),
        (cx - half_w, cz - bottom * 0.10),
        (cx - half_w * 0.80, cz - bottom * 0.55),
        (cx - half_w * 0.42, cz - bottom),
        (cx + half_w * 0.42, cz - bottom),
    ]


def section_spar(half_w, top, bottom, cx=0.0, cz=0.0):
    """Eight-point spar: vertical sides, flat top, keeled bottom.

    The default section rounds off under bevel and reads as a tube; claw chords
    need machined faces so the segments stay legible.
    """
    return [
        (cx + half_w, cz - bottom * 0.25),
        (cx + half_w, cz + top * 0.55),
        (cx + half_w * 0.55, cz + top),
        (cx - half_w * 0.55, cz + top),
        (cx - half_w, cz + top * 0.55),
        (cx - half_w, cz - bottom * 0.25),
        (cx - half_w * 0.45, cz - bottom),
        (cx + half_w * 0.45, cz - bottom),
    ]


def _cap_faces(rings, count):
    faces = []
    for r in range(len(rings) - 1):
        a = r * count
        b = (r + 1) * count
        for i in range(count):
            j = (i + 1) % count
            faces.append((a + i, a + j, b + j, b + i))
    faces.append(tuple(range(count - 1, -1, -1)))
    last = (len(rings) - 1) * count
    faces.append(tuple(range(last, last + count)))
    return faces


def loft(rings):
    """rings: [(y, [(x, z), ...]), ...] with equal point counts, closed at both ends."""
    verts = []
    for y, pts in rings:
        verts.extend((x, y, z) for x, z in pts)
    return verts, _cap_faces(rings, len(rings[0][1]))


def pod(cx, cz, y0, y1, half_w, top, bottom, end_scale=0.55, pinch=0.3):
    """Tapered volume along Y: full section in the middle, pinched at both ends."""
    length = y1 - y0
    p = length * pinch
    rings = []
    for y, s in ((y0, end_scale), (y0 + p, 1.0), (y1 - p, 1.0), (y1, end_scale)):
        rings.append((y, section(half_w * s, top * s, bottom * s, cx, cz)))
    return loft(rings)


def slab(cx, cz, y0, y1, half_w, half_t, end_scale=0.94, pinch=0.06):
    """Near-flat horizontal plate with chamfered rims."""
    return pod(cx, cz, y0, y1, half_w, half_t, half_t, end_scale, pinch)


def beam(points, half_w, top, bottom, scales):
    """Loft a section along an arbitrary polyline: claws, struts, braces."""
    pts = [tuple(p) for p in points]
    count = 12
    verts = []
    for i, p in enumerate(pts):
        if i == 0:
            t = _sub(pts[1], pts[0])
        elif i == len(pts) - 1:
            t = _sub(pts[-1], pts[-2])
        else:
            t = _sub(pts[i + 1], pts[i - 1])
        t = _norm(t)
        side = _norm(_cross((0.0, 0.0, 1.0), t))
        up = _norm(_cross(t, side))
        s = scales[i]
        for u, v in section(half_w * s, top * s, bottom * s):
            verts.append(
                (
                    p[0] + side[0] * u + up[0] * v,
                    p[1] + side[1] * u + up[1] * v,
                    p[2] + side[2] * u + up[2] * v,
                )
            )
    return verts, _cap_faces(pts, count)


def strut(p0, p1, s0, s1, fn=section):
    """Straight prism between two points; s = (half_w, top, bottom) at each end."""
    side, up = _frame(_sub(p1, p0))
    verts = []
    count = 0
    for p, s in ((p0, s0), (p1, s1)):
        pts = fn(s[0], s[1], s[2])
        count = len(pts)
        for u, v in pts:
            verts.append(
                (
                    p[0] + side[0] * u + up[0] * v,
                    p[1] + side[1] * u + up[1] * v,
                    p[2] + side[2] * u + up[2] * v,
                )
            )
    return verts, _cap_faces((p0, p1), count)


def build_claw(nodes, sections, tag, hull_mat, panel_mat, dark_mat):
    """One claw, welded from straight chords with bolted collars at every kink.

    A smooth taper along a curved path reads as an organic horn; Minmatar work
    is straight plate and exposed bracket, so the kinks stay visible.
    """
    for i in range(len(nodes) - 1):
        add_mesh("claw_%s_%d" % (tag, i),
                 *strut(nodes[i], nodes[i + 1], sections[i], sections[i + 1],
                        section_spar),
                 hull_mat, bevel=1.0)

    for i in range(1, len(nodes) - 1):
        d0 = _norm(_sub(nodes[i], nodes[i - 1]))
        d1 = _norm(_sub(nodes[i + 1], nodes[i]))
        d = _norm((d0[0] + d1[0], d0[1] + d1[1], d0[2] + d1[2]))
        s = sections[i]
        big = (s[0] * 1.18, s[1] * 1.18, s[2] * 1.18)
        add_mesh("claw_%s_joint_%d" % (tag, i),
                 *strut(_along(nodes[i], d, -9.0), _along(nodes[i], d, 9.0),
                        big, big, section_spar),
                 dark_mat)

    for i in range(len(nodes) - 1):
        for k, t in enumerate((0.30, 0.66)):
            s = _lerp_section(sections[i], sections[i + 1], t)
            band = (s[0] * 1.08, s[1] * 1.06, s[2] * 1.06)
            add_mesh("claw_%s_band_%d%d" % (tag, i, k),
                     *strut(_lerp3(nodes[i], nodes[i + 1], t - 0.05),
                            _lerp3(nodes[i], nodes[i + 1], t + 0.05),
                            band, band, section_spar),
                     panel_mat)


def bell(cx, cz, y0, y1, r0, r1, segments=16):
    """Engine nozzle: radius r0 at the throat, r1 at the lip."""
    verts = []
    for y, r in ((y0, r0), (y1, r1)):
        for i in range(segments):
            a = 2.0 * math.pi * i / segments
            verts.append((cx + r * math.cos(a), y, cz + r * math.sin(a)))
    count = segments
    faces = []
    for i in range(count):
        j = (i + 1) % count
        faces.append((i, j, count + j, count + i))
    faces.append(tuple(range(count - 1, -1, -1)))
    faces.append(tuple(range(count, count * 2)))
    return verts, faces


def disc(cx, cy, z0, z1, radius, segments=20):
    """Cylinder along Z, used for hardpoint sockets."""
    verts = []
    for z in (z0, z1):
        for i in range(segments):
            a = 2.0 * math.pi * i / segments
            verts.append((cx + radius * math.cos(a), cy + radius * math.sin(a), z))
    count = segments
    faces = []
    for i in range(count):
        j = (i + 1) % count
        faces.append((i, j, count + j, count + i))
    faces.append(tuple(range(count - 1, -1, -1)))
    faces.append(tuple(range(count, count * 2)))
    return verts, faces


def annulus(cx, cz, y0, y1, r_out, r_in, segments=32):
    """Washer-shaped drum around the Y axis, used for the doomsday aperture ring."""
    loop_defs = ((y0, r_out), (y0, r_in), (y1, r_in), (y1, r_out))
    verts = []
    for y, r in loop_defs:
        for i in range(segments):
            a = 2.0 * math.pi * i / segments
            verts.append((cx + r * math.cos(a), y, cz + r * math.sin(a)))
    faces = []
    for L in range(4):
        a = L * segments
        b = ((L + 1) % 4) * segments
        for i in range(segments):
            j = (i + 1) % segments
            faces.append((a + i, a + j, b + j, b + i))
    return verts, faces


def _sub(a, b):
    return (a[0] - b[0], a[1] - b[1], a[2] - b[2])


def _cross(a, b):
    return (
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    )


def _norm(a):
    n = math.sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]) or 1.0
    return (a[0] / n, a[1] / n, a[2] / n)


def _along(p, d, distance):
    return (p[0] + d[0] * distance, p[1] + d[1] * distance, p[2] + d[2] * distance)


def _lerp3(a, b, t):
    return (a[0] + (b[0] - a[0]) * t,
            a[1] + (b[1] - a[1]) * t,
            a[2] + (b[2] - a[2]) * t)


def _lerp_section(a, b, t):
    return (a[0] + (b[0] - a[0]) * t,
            a[1] + (b[1] - a[1]) * t,
            a[2] + (b[2] - a[2]) * t)


def _frame(t):
    t = _norm(t)
    side = _cross((0.0, 0.0, 1.0), t)
    if math.sqrt(side[0] * side[0] + side[1] * side[1] + side[2] * side[2]) < 1e-5:
        side = (1.0, 0.0, 0.0)
    side = _norm(side)
    return side, _norm(_cross(t, side))


# ---------------------------------------------------------------------------
# scene assembly
# ---------------------------------------------------------------------------


def material(name, rgb, roughness=0.66):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = (rgb[0], rgb[1], rgb[2], 1.0)
    bsdf.inputs["Roughness"].default_value = roughness
    bsdf.inputs["Metallic"].default_value = 0.0
    return mat


def add_mesh(name, verts, faces, mat, bevel=0.0, bevel_angle=35.0):
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(verts, [], faces)
    mesh.update()

    bm = bmesh.new()
    bm.from_mesh(mesh)
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    bm.to_mesh(mesh)
    bm.free()

    obj = bpy.data.objects.new(name, mesh)
    obj.data.materials.append(mat)
    bpy.context.collection.objects.link(obj)

    if bevel > 0.0:
        mod = obj.modifiers.new("bevel", "BEVEL")
        mod.width = bevel
        mod.segments = 1
        mod.limit_method = "ANGLE"
        mod.angle_limit = math.radians(bevel_angle)
    return obj


# Three districts with real gaps between them; the spine beams run through the
# gaps so the hull reads as a chain of volumes instead of one long body.
# One continuous hull: the Ragnarok neck narrows but never breaks into separate
# volumes.  Beam is ~1/11 of length, which is what kills the fat-tube read.
HULL_PROFILE = [
    (300.0, 30.0, 24.0, 21.0),
    (240.0, 36.0, 29.0, 25.0),
    (180.0, 41.0, 33.0, 28.0),
    (120.0, 45.0, 36.0, 31.0),
    (60.0, 43.0, 34.0, 29.0),
    (0.0, 30.0, 26.0, 22.0),
    (-60.0, 38.0, 32.0, 27.0),
    (-120.0, 45.0, 38.0, 31.0),
    (-190.0, 47.0, 40.0, 33.0),
    (-260.0, 44.0, 37.0, 31.0),
    (-320.0, 38.0, 32.0, 27.0),
    (-380.0, 24.0, 22.0, 18.0),
    (-440.0, 14.0, 13.0, 11.0),
    (-500.0, 4.0, 4.0, 4.0),
]


def _hull_at(y):
    for i in range(len(HULL_PROFILE) - 1):
        y0, hw0, top0, bot0 = HULL_PROFILE[i]
        y1, hw1, top1, bot1 = HULL_PROFILE[i + 1]
        if y1 <= y <= y0:
            t = (y - y1) / (y0 - y1)
            return (hw1 + (hw0 - hw1) * t,
                    top1 + (top0 - top1) * t,
                    bot1 + (bot0 - bot1) * t)
    return HULL_PROFILE[0][1:]

# (x, y, z, up, socket radius)
MOUNTS = [
    (0.0, 300.0, 4.0, 1.0, 20.0),
    (0.0, 200.0, 35.0, 1.0, 15.0),
    (0.0, -180.0, 41.0, 1.0, 15.0),
    (-26.0, -90.0, -34.0, -1.0, 10.0),
    (26.0, -230.0, -33.0, -1.0, 10.0),
    (-33.0, 160.0, 20.0, 1.0, 6.0),
    (33.0, 160.0, 20.0, 1.0, 6.0),
    (-35.0, -250.0, 24.0, 1.0, 6.0),
    (35.0, -250.0, 24.0, 1.0, 6.0),
]


def _hangar(panel_mat, dark_mat, side, y0, y1, z0, z1, ribs, x_out):
    """Recessed bay read: dark door plane, lips above and below, rib bars."""
    tag = "r" if side > 0 else "l"
    zc = (z0 + z1) * 0.5
    zh = (z1 - z0) * 0.5
    add_mesh("hangar_%s_door" % tag,
             *pod(side * (x_out + 0.5), zc, y0, y1, 1.6, zh, zh, 0.98),
             dark_mat)
    for k, cz in ((0, z1 + 4.0), (1, z0 - 4.0)):
        add_mesh("hangar_%s_lip_%d" % (tag, k),
                 *slab(side * (x_out - 1.0), cz, y0 - 6.0, y1 + 6.0, 6.0, 5.0, 0.96),
                 panel_mat)
    step = (y1 - y0) / float(ribs + 1)
    for i in range(ribs):
        y = y0 + step * (i + 1)
        add_mesh("hangar_%s_rib_%d" % (tag, i),
                 *pod(side * (x_out + 2.0), zc, y - 2.5, y + 2.5,
                      2.5, zh * 0.92, zh * 0.92, 0.9),
                 panel_mat)
    z = z0 + (z1 - z0) * 0.5
    add_mesh("hangar_%s_bar" % tag,
             *pod(side * (x_out + 4.0), z, y0 + 8.0, y1 - 8.0,
                  2.2, 3.0, 3.0, 0.9),
             panel_mat)


def build_hull(base_mat, plate_mat, bronze_mat, dark_mat, throat_mat):
    rings = [(y, section_spar(hw, top, bottom)) for y, hw, top, bottom in HULL_PROFILE]
    add_mesh("hull", *loft(rings), base_mat, bevel=1.0)

    # --- Split spear: two long blades closing on a point.
    for side, tag in ((-1.0, "l"), (1.0, "r")):
        add_mesh("spear_%s" % tag,
                 *strut((side * 15.0, 300.0, 4.0), (side * 6.0, 500.0, 2.0),
                        (9.0, 11.0, 9.0), (3.0, 5.0, 4.0), section_spar),
                 plate_mat, bevel=0.6)
        for k, t in enumerate((0.34, 0.66)):
            p0 = _lerp3((side * 15.0, 300.0, 4.0), (side * 6.0, 500.0, 2.0), t - 0.02)
            p1 = _lerp3((side * 15.0, 300.0, 4.0), (side * 6.0, 500.0, 2.0), t + 0.02)
            sec = _lerp_section((9.0, 11.0, 9.0), (3.0, 5.0, 4.0), t)
            band = (sec[0] * 1.5, sec[1] * 1.12, sec[2] * 1.12)
            add_mesh("spear_%s_band_%d" % (tag, k),
                     *strut(p0, p1, band, band, section_spar), bronze_mat)
    for i, y in enumerate((356.0, 416.0, 466.0)):
        add_mesh("spear_brace_%d" % i,
                 *beam([(-13.0, y, 4.0), (13.0, y, 3.0)],
                       3.0, 4.0, 3.0, (1.0, 1.0)), dark_mat)

    # --- Antenna arrays: long thin booms lying along the hull axis, bow and
    # stern.  They are rod thin -- at 24 km a 3 u rod is barely a pixel.
    for i, (x, y0, y1, z, r) in enumerate((
            (-24.0, 250.0, 430.0, 12.0, 1.5), (24.0, 250.0, 430.0, 12.0, 1.5),
            (-38.0, 236.0, 372.0, -2.0, 1.0), (38.0, 236.0, 372.0, -2.0, 1.0))):
        add_mesh("antenna_bow_%d" % i,
                 *strut((x, y0, z), (x, y1, z),
                        (r, r, r), (r * 0.5, r * 0.5, r * 0.5), section_spar),
                 plate_mat, bevel=0.1)
        add_mesh("antenna_bow_%d_root" % i,
                 *beam([(x, y0 + 6.0, z), (x * 0.55, y0 - 22.0, z * 0.6)],
                       1.6, 1.6, 1.6, (1.0, 1.0)), dark_mat)
        add_mesh("antenna_bow_%d_pad" % i,
                 *pod(x, z, y0 - 10.0, y0 + 10.0, 4.5, 4.5, 4.5, 0.8),
                 base_mat, bevel=0.2)
    for i, (x, y0, y1, z, r) in enumerate((
            (-18.0, -300.0, -476.0, 12.0, 1.4), (18.0, -300.0, -476.0, 12.0, 1.4),
            (-30.0, -290.0, -428.0, 0.0, 1.0), (30.0, -290.0, -428.0, 0.0, 1.0))):
        add_mesh("antenna_stern_%d" % i,
                 *strut((x, y0, z), (x, y1, z),
                        (r, r, r), (r * 0.5, r * 0.5, r * 0.5), section_spar),
                 plate_mat, bevel=0.1)
        add_mesh("antenna_stern_%d_root" % i,
                 *beam([(x, y0 - 6.0, z), (x * 0.6, y0 + 22.0, z * 0.6)],
                       1.6, 1.6, 1.6, (1.0, 1.0)), dark_mat)
        add_mesh("antenna_stern_%d_pad" % i,
                 *pod(x, z, y0 - 10.0, y0 + 10.0, 4.0, 4.0, 4.0, 0.8),
                 base_mat, bevel=0.2)

    # --- Deck plating: two long shallow runs, flush with the hull top.
    add_mesh("deck_fore", *pod(0.0, 26.0, 90.0, 270.0, 30.0, 7.0, 5.0, 0.9),
             plate_mat, bevel=0.5)
    add_mesh("deck_aft", *pod(0.0, 30.0, -290.0, -90.0, 30.0, 8.0, 5.0, 0.9),
             plate_mat, bevel=0.5)
    add_mesh("deck_step", *pod(0.0, 44.0, -250.0, -130.0, 17.0, 6.0, 4.0, 0.9),
             base_mat, bevel=0.4)

    # --- Longitudinal armour strips: long bright seams along the flanks and
    # the belly, the cheapest detail that still reads at 24 km.
    for side, tag in ((-1.0, "l"), (1.0, "r")):
        for i, (ya, yb, zc) in enumerate(((250.0, 70.0, 8.0), (-60.0, -180.0, 4.0),
                                         (-190.0, -310.0, 4.0))):
            xa = side * (_hull_at(ya)[0] + 1.5)
            xb = side * (_hull_at(yb)[0] + 1.5)
            sec = (2.2, 12.0, 12.0)
            add_mesh("strip_%s_%d" % (tag, i),
                     *strut((xa, ya, zc), (xb, yb, zc), sec, sec, section_spar),
                     plate_mat, bevel=0.2)
    add_mesh("keel_strip", *strut((0.0, 200.0, -31.0), (0.0, -300.0, -31.0),
                                  (3.0, 5.0, 5.0), (3.0, 5.0, 5.0), section_spar),
             plate_mat, bevel=0.2)

    # --- Emitter recessed between the spear roots.
    add_mesh("throat_ring", *annulus(0.0, 4.0, 290.0, 310.0, 21.0, 13.0), bronze_mat)
    add_mesh("throat_core", *bell(0.0, 4.0, 296.0, 302.0, 13.0, 11.0), throat_mat)

    # --- Flank hangars, unequal.
    _hangar(plate_mat, dark_mat, side=1, y0=-100.0, y1=-200.0,
            z0=-16.0, z1=16.0, ribs=3, x_out=45.0)
    _hangar(plate_mat, dark_mat, side=-1, y0=-70.0, y1=-150.0,
            z0=-20.0, z1=10.0, ribs=2, x_out=45.0)

    # --- Main bells on the aft block, collared in bronze.
    for i, x in enumerate((-17.0, 17.0)):
        for j, z in enumerate((11.0, -12.0)):
            add_mesh("nozzle_%d%d" % (i, j),
                     *bell(x, z, -318.0, -350.0, 9.5, 12.5), dark_mat)
            add_mesh("nozzle_%d%d_core" % (i, j),
                     *bell(x, z, -346.0, -351.0, 9.5, 9.5), throat_mat)
            add_mesh("nozzle_%d%d_collar" % (i, j),
                     *bell(x, z, -324.0, -312.0, 13.5, 12.5), bronze_mat)

    # --- Empty hardpoint sockets.
    for i, (x, y, z, up, radius) in enumerate(MOUNTS):
        add_mesh("mount_%d_ring" % i,
                 *disc(x, y, z, z + up * 2.5, radius), plate_mat)
        add_mesh("mount_%d_cap" % i,
                 *disc(x, y, z + up * 2.5, z + up * 4.0, radius * 0.62), dark_mat)


def build():
    bpy.ops.wm.read_factory_settings(use_empty=True)

    base_mat = material("base", BASE, roughness=0.72)
    plate_mat = material("plate", PLATE, roughness=0.62)
    bronze_mat = material("bronze", BRONZE, roughness=0.50)
    dark_mat = material("dark", DARK, roughness=0.80)
    throat_mat = material("throat", THROAT, roughness=0.45)

    build_hull(base_mat, plate_mat, bronze_mat, dark_mat, throat_mat)

    bpy.ops.export_scene.gltf(
        filepath=OUTPUT,
        export_format="GLB",
        export_apply=True,
    )
    print("wrote %s" % OUTPUT)

    meshes = [o for o in bpy.data.objects if o.type == "MESH"]
    verts = sum(len(o.data.vertices) for o in meshes)
    faces = sum(len(o.data.polygons) for o in meshes)
    print("objects=%d verts=%d faces=%d" % (len(meshes), verts, faces))

    xs, ys, zs = [], [], []
    for o in meshes:
        for v in o.data.vertices:
            xs.append(v.co.x)
            ys.append(v.co.y)
            zs.append(v.co.z)
    cx = (min(xs) + max(xs)) * 0.5
    cy = (min(ys) + max(ys)) * 0.5
    cz = (min(zs) + max(zs)) * 0.5
    length = max(ys) - min(ys)
    print("bbox x %.1f..%.1f  y %.1f..%.1f  z %.1f..%.1f  longest %.1f"
          % (min(xs), max(xs), min(ys), max(ys), min(zs), max(zs), length))

    scale = 1000.0 / length
    print("hardpoints (engine frame: centred, scaled to target_length=1000):")
    for i, (x, y, z, up, radius) in enumerate(MOUNTS):
        lx = (x - cx) * scale
        ly = (z - cz) * scale
        lz = (-(y - cy)) * scale
        print("  %d: { %.2ff, %.2ff, %.2ff }  %s  r=%.1f"
              % (i, lx, ly, lz, "up" if up > 0 else "down", radius * scale))


def render_previews():
    os.makedirs(PREVIEW_DIR, exist_ok=True)
    scene = bpy.context.scene
    scene.render.engine = "CYCLES"
    scene.cycles.device = "CPU"
    scene.cycles.samples = 24
    scene.cycles.use_denoising = True
    scene.render.resolution_x = 900
    scene.render.resolution_y = 506
    scene.render.film_transparent = False

    world = bpy.data.worlds.new("preview_world")
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs[0].default_value = (0.02, 0.025, 0.04, 1.0)
    world.node_tree.nodes["Background"].inputs[1].default_value = 1.0
    scene.world = world

    sun_data = bpy.data.lights.new("key", type="SUN")
    sun_data.energy = 4.0
    sun_data.angle = math.radians(8.0)
    sun = bpy.data.objects.new("key", sun_data)
    sun.rotation_euler = (math.radians(55.0), 0.0, math.radians(-35.0))
    bpy.context.collection.objects.link(sun)

    fill_data = bpy.data.lights.new("fill", type="SUN")
    fill_data.energy = 1.1
    fill_data.color = (0.55, 0.65, 0.9)
    fill = bpy.data.objects.new("fill", fill_data)
    fill.rotation_euler = (math.radians(115.0), 0.0, math.radians(150.0))
    bpy.context.collection.objects.link(fill)

    cam_data = bpy.data.cameras.new("cam")
    cam_data.angle = math.radians(38.0)
    cam_data.clip_start = 5.0
    cam_data.clip_end = 20000.0
    cam = bpy.data.objects.new("cam", cam_data)
    bpy.context.collection.objects.link(cam)
    scene.camera = cam

    views = [
        ("three_quarter", (-1480.0, 500.0, 390.0), (0.0, 60.0, 0.0), 38.0, 900, 506),
        ("top", (-900.0, 300.0, 1150.0), (0.0, 60.0, 0.0), 38.0, 900, 506),
        ("bow", (380.0, 1560.0, 260.0), (0.0, 60.0, 0.0), 38.0, 900, 506),
        ("claw", (-380.0, 700.0, 180.0), (-20.0, 380.0, 15.0), 40.0, 900, 506),
        ("side", (-1800.0, 60.0, 45.0), (0.0, 60.0, 0.0), 34.0, 900, 506),
        ("ingame", (-2380.0, 760.0, 150.0), (0.0, 60.0, 0.0), 45.0, 1600, 800),
    ]
    for name, eye, aim, fov, res_x, res_y in views:
        cam.location = eye
        cam_data.angle = math.radians(fov)
        scene.render.resolution_x = res_x
        scene.render.resolution_y = res_y
        direction = _vec2(aim, cam.location)
        cam.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()
        scene.render.filepath = os.path.join(PREVIEW_DIR, "titan_%s.png" % name)
        bpy.ops.render.render(write_still=True)
        print("rendered %s" % scene.render.filepath)


def _vec(x, y, z):
    from mathutils import Vector

    return Vector((x, y, z))


def _vec2(a, b):
    from mathutils import Vector

    return Vector(a) - Vector(b)


build()
if "--preview" in sys.argv:
    render_previews()
