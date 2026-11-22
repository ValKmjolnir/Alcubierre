"""Generate a low-poly corvette and export it as .glb.

Run on the Windows host (Blender is not installed in WSL):

    blender.exe --background --python D:\\code\\Alcubierre\\tool\\build_ship.py

Axes: geometry is authored in Blender's own frame -- nose along +Y, up along
+Z -- because the glTF exporter rewrites (x, y, z) as (x, z, -y).  Authoring
in glTF axes directly makes the ship come out standing on its nose.

Every module is lofted from the same eight-point cross-section, so there are no
plain cuboids: a slab reads as unfinished from the three-quarter top view,
which is the angle the ship is actually seen from.

There is no bridge tower.  A deep-space hull has nowhere to look out of, so the
command spaces stay flush in the hull and the only vertical elements are thin
sensor blades.

Weapons are deliberately absent: the ship carries empty hardpoint sockets so
the engine can mount whatever the player fits later.  Those sockets are printed
at the end in glTF axes.

Length is 150 m, so the engine's model_object(..., target_length=15) maps it to
1 unit = 10 m.  Light values are chosen so that the engine's kAlbedoScale (0.6)
lands them near the hand-authored hulls.
"""

import math
import os

import bmesh
import bpy

OUTPUT = os.path.normpath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets", "models", "ship_py.glb")
)

HULL = (0.80, 0.81, 0.83)
PANEL = (0.42, 0.44, 0.47)
DARK = (0.16, 0.17, 0.19)

MOUNT_RADIUS = 3.0


# ---------------------------------------------------------------------------
# mesh helpers, all in Blender axes (X right, Y along the hull, Z up)
# ---------------------------------------------------------------------------

def _ring_faces(segments):
    faces = []
    for i in range(segments):
        j = (i + 1) % segments
        faces.append((i, j, segments + j, segments + i))
    faces.append(tuple(range(segments - 1, -1, -1)))
    faces.append(tuple(range(segments, segments * 2)))
    return faces


def disc(cx, cy, z0, z1, radius, segments=16):
    """Cylinder along Z, used for hardpoint sockets."""
    verts = []
    for z in (z0, z1):
        for i in range(segments):
            a = 2.0 * math.pi * i / segments
            verts.append((cx + radius * math.cos(a), cy + radius * math.sin(a), z))
    return verts, _ring_faces(segments)


def nozzle(cx, cz, y0, y1, radius, segments=12):
    """Cylinder along Y."""
    verts = []
    for y in (y0, y1):
        for i in range(segments):
            a = 2.0 * math.pi * i / segments
            verts.append((cx + radius * math.cos(a), y, cz + radius * math.sin(a)))
    return verts, _ring_faces(segments)


def section(half_w, top, bottom):
    """Eight-point cross-section in XZ: chamfered top corners, angled underside."""
    return [
        (half_w, -bottom * 0.5),
        (half_w * 0.92, top * 0.55),
        (half_w * 0.50, top),
        (-half_w * 0.50, top),
        (-half_w * 0.92, top * 0.55),
        (-half_w, -bottom * 0.5),
        (-half_w * 0.55, -bottom),
        (half_w * 0.55, -bottom),
    ]


def loft(rings):
    """rings: [(y, [(x, z), ...]), ...] with equal point counts."""
    verts = []
    for y, pts in rings:
        verts.extend((x, y, z) for x, z in pts)

    count = len(rings[0][1])
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
    return verts, faces


def spindle(cx, cz, cy, length, half_w, top, bottom, end_scale=0.45):
    """Tapered pod: full section in the middle, pinched at both ends."""
    y0 = cy - length * 0.5
    y1 = cy + length * 0.5
    pinch = length * 0.28

    rings = []
    for y, s in ((y0, end_scale), (y0 + pinch, 1.0), (y1 - pinch, 1.0), (y1, end_scale)):
        pts = [(x + cx, z + cz) for x, z in section(half_w * s, top * s, bottom * s)]
        rings.append((y, pts))
    return loft(rings)


# ---------------------------------------------------------------------------
# scene assembly
# ---------------------------------------------------------------------------

def material(name, rgb):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = (rgb[0], rgb[1], rgb[2], 1.0)
    bsdf.inputs["Roughness"].default_value = 0.65
    bsdf.inputs["Metallic"].default_value = 0.0
    return mat


def add_mesh(name, verts, faces, mat, bevel=0.0):
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
        mod.angle_limit = math.radians(35.0)
    return obj


def build():
    bpy.ops.wm.read_factory_settings(use_empty=True)

    hull_mat = material("hull", HULL)
    panel_mat = material("panel", PANEL)
    dark_mat = material("dark", DARK)

    # Hull: eight lofted sections, 150 m long, 34 m beam, nose at +Y.
    profile = [
        (75.0, 2.5, 2.5, 2.0),
        (62.0, 7.0, 6.0, 4.5),
        (45.0, 11.0, 9.0, 6.5),
        (20.0, 15.0, 11.0, 8.0),
        (-8.0, 17.0, 12.0, 9.0),
        (-38.0, 15.5, 11.0, 8.0),
        (-60.0, 12.5, 9.0, 6.0),
        (-75.0, 8.0, 7.0, 5.0),
    ]
    rings = [(y, section(hw, top, bottom)) for y, hw, top, bottom in profile]
    add_mesh("hull", *loft(rings), hull_mat, bevel=1.0)

    # Dorsal deck: one long tapered spine plus two flanking strakes, flush
    # enough that they read as layered plating from above rather than a tower.
    add_mesh("spine", *spindle(0.0, 11.0, -5.0, 100.0, 8.0, 5.0, 3.0), hull_mat, bevel=0.7)
    for side in (-1.0, 1.0):
        add_mesh(
            "strake_%s" % ("l" if side < 0 else "r"),
            *spindle(side * 10.0, 10.0, -15.0, 55.0, 4.0, 3.0, 2.0),
            panel_mat,
            bevel=0.5,
        )
    add_mesh("foredeck", *spindle(0.0, 9.0, 45.0, 40.0, 6.0, 3.0, 2.0), hull_mat, bevel=0.5)

    # Sensor blades instead of a bridge: thin vertical vanes, no conning tower.
    for side in (-1.0, 1.0):
        add_mesh(
            "blade_%s" % ("l" if side < 0 else "r"),
            *spindle(side * 7.0, 11.5, -50.0, 24.0, 1.2, 9.0, 2.5),
            dark_mat,
            bevel=0.0,
        )
    add_mesh("bow_blade", *spindle(0.0, 7.0, 55.0, 26.0, 1.0, 7.0, 3.0), dark_mat, bevel=0.0)

    # Shoulder armour: tapered wedges, not slabs.
    for side in (-1.0, 1.0):
        add_mesh(
            "shoulder_%s" % ("l" if side < 0 else "r"),
            *spindle(side * 14.5, 6.0, 5.0, 46.0, 4.0, 8.0, 6.0),
            hull_mat,
            bevel=0.6,
        )

    # Ventral keel blade.
    add_mesh("keel", *spindle(0.0, -12.0, -10.0, 62.0, 6.0, 6.0, 5.0), panel_mat, bevel=0.6)

    # Engine housing and three nozzles.
    add_mesh("engine_housing", *spindle(0.0, 0.0, -72.0, 16.0, 12.0, 7.0, 7.0), dark_mat, bevel=0.6)
    for i, x in enumerate((-8.0, 0.0, 8.0)):
        add_mesh("nozzle_%d" % i, *nozzle(x, 0.0, -80.0, -92.0, 4.6), dark_mat, bevel=0.0)

    # Empty hardpoints: a raised ring plus a darker socket cap.  z is the
    # surface each socket sits on; up = +1 dorsal, -1 ventral.
    mounts = [
        (0.0, 40.0, 12.0, 1.0),
        (0.0, 12.0, 16.0, 1.0),
        (-9.0, 20.0, 11.0, 1.0),
        (9.0, 20.0, 11.0, 1.0),
        (0.0, -30.0, 16.0, 1.0),
        (-6.0, 45.0, -6.5, -1.0),
        (6.0, 45.0, -6.5, -1.0),
    ]
    for i, (x, y, z, up) in enumerate(mounts):
        add_mesh("mount_%d_ring" % i, *disc(x, y, z, z + up * 1.4, MOUNT_RADIUS), panel_mat, bevel=0.0)
        add_mesh("mount_%d_cap" % i, *disc(x, y, z + up * 1.4, z + up * 2.2, MOUNT_RADIUS * 0.64), dark_mat, bevel=0.0)

    bpy.ops.export_scene.gltf(
        filepath=OUTPUT,
        export_format="GLB",
        export_apply=True,
    )
    print("wrote %s" % OUTPUT)

    print("hardpoints (glTF axes, metres, before model_object's centring/scale):")
    for i, (x, y, z, up) in enumerate(mounts):
        print("  %d: (%.1f, %.1f, %.1f)  facing %s" % (i, x, z, -y, "up" if up > 0 else "down"))


build()
