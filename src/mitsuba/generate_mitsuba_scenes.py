"""
[DESCONTINUADO] Gera cenas XML para Mitsuba 3 equivalentes às cenas do VI-RT.

Substituído pelo exportador C++ (MitsubaExporter.{hpp,cpp}), que escreve os XML
a partir da MESMA Scene que o VI-RT renderiza — sem drift. Já NÃO é chamado pelo
render_all.sh; mantido apenas como referência histórica. Podes apagá-lo.

Uso: python3 generate_mitsuba_scenes.py [--scene NOME] [--output-dir DIR]
"""

import os
import argparse
import math

# Configuração global da câmara (igual ao main.cpp do VI-RT)
CAMERA = {
    "eye":    (0, 0.5, -5),
    "at":     (0, 0, 3),
    "up":     (0, 1, 0),
    "fov":    60.0,
    "width":  640,
    "height": 640,
    "spp":    218,
}

# ---------------------------------------------------------------------------
# Template base XML do Mitsuba 3
# ---------------------------------------------------------------------------
XML_HEADER = """<?xml version="1.0" encoding="utf-8"?>
<scene version="3.0.0">

    <!-- Integrador: path tracing directo (equivalente ao DistributedShader) -->
    <integrator type="direct">
        <integer name="emitter_samples" value="1"/>
        <integer name="bsdf_samples" value="1"/>
    </integrator>

    <!-- Câmara -->
    <sensor type="perspective">
        <float name="fov" value="{fov}"/>
        <string name="fov_axis" value="y"/>
        <transform name="to_world">
            <lookat origin="{ex},{ey},{ez}"
                    target="{ax},{ay},{az}"
                    up="{ux},{uy},{uz}"/>
        </transform>
        <sampler type="independent">
            <integer name="sample_count" value="{spp}"/>
        </sampler>
        <film type="hdrfilm">
            <integer name="width"  value="{w}"/>
            <integer name="height" value="{h}"/>
            <rfilter type="gaussian"/>
        </film>
    </sensor>

"""

XML_FOOTER = "\n</scene>\n"


# ---------------------------------------------------------------------------
# Helpers de geometria e material
# ---------------------------------------------------------------------------

def fmt(v):
    """Formata um float para string sem zeros desnecessários."""
    return f"{v:.6g}"

def sphere_xml(cx, cy, cz, r, bsdf_id):
    return f"""    <shape type="sphere">
        <point name="center" x="{fmt(cx)}" y="{fmt(cy)}" z="{fmt(cz)}"/>
        <float name="radius" value="{fmt(r)}"/>
        <ref id="{bsdf_id}"/>
    </shape>
"""

def triangle_xml(v1, v2, v3, bsdf_id):
    """v1/v2/v3 são tuplos (x,y,z)."""
    return f"""    <shape type="obj">
        <!-- triângulo inline via mesh -->
        <string name="filename" value="triangle_{bsdf_id}.obj"/>
        <ref id="{bsdf_id}"/>
    </shape>
"""

def triangle_inline_xml(v1, v2, v3, bsdf_id):
    """Triângulo usando ply inline (Mitsuba 3 suporta mesh com vértices inline)."""
    x1,y1,z1 = v1; x2,y2,z2 = v2; x3,y3,z3 = v3
    return f"""    <shape type="mesh">
        <string name="filename" value=""/>
        <!-- vértices inline não suportados: gerar .obj à parte -->
        <ref id="{bsdf_id}"/>
    </shape>
"""

def area_light_xml(power_r, power_g, power_b, v1, v2, v3, light_id):
    x1,y1,z1 = v1; x2,y2,z2 = v2; x3,y3,z3 = v3
    return f"""    <!-- AreaLight {light_id} -->
    <emitter type="area">
        <rgb name="radiance" value="{fmt(power_r)}, {fmt(power_g)}, {fmt(power_b)}"/>
    </emitter>
"""

def point_light_xml(r, g, b, px, py, pz):
    return f"""    <emitter type="point">
        <rgb name="intensity" value="{fmt(r)}, {fmt(g)}, {fmt(b)}"/>
        <point name="position" x="{fmt(px)}" y="{fmt(py)}" z="{fmt(pz)}"/>
    </emitter>
"""

def ambient_light_xml(r, g, b):
    return f"""    <emitter type="constant">
        <rgb name="radiance" value="{fmt(r)}, {fmt(g)}, {fmt(b)}"/>
    </emitter>
"""

# ---------------------------------------------------------------------------
# BSDFs Mitsuba equivalentes a cada BRDF do VI-RT
# ---------------------------------------------------------------------------

def diffuse_bsdf(mat_id, r, g, b):
    return f"""    <bsdf type="diffuse" id="{mat_id}">
        <rgb name="reflectance" value="{fmt(r)}, {fmt(g)}, {fmt(b)}"/>
    </bsdf>
"""

def phong_bsdf(mat_id, kd_r, kd_g, kd_b, ks_r, ks_g, ks_b, ns):
    """Phong via roughplastic (Mitsuba usa BRDF energeticamente correcto)."""
    # Mitsuba não tem Phong puro; usa roughplastic com GGX aproximado
    # roughness ≈ sqrt(2 / (ns+2))
    roughness = math.sqrt(2.0 / (ns + 2.0)) if ns > 0 else 1.0
    roughness = max(0.01, min(1.0, roughness))
    return f"""    <bsdf type="roughplastic" id="{mat_id}">
        <rgb name="diffuse_reflectance"  value="{fmt(kd_r)}, {fmt(kd_g)}, {fmt(kd_b)}"/>
        <rgb name="specular_reflectance" value="{fmt(ks_r)}, {fmt(ks_g)}, {fmt(ks_b)}"/>
        <float name="alpha" value="{fmt(roughness)}"/>
        <string name="distribution" value="ggx"/>
    </bsdf>
"""

def cook_torrance_bsdf(mat_id, kd_r, kd_g, kd_b, roughness, metallic):
    """Cook-Torrance via principled (Mitsuba 3.x) ou roughconductor/roughplastic."""
    if metallic > 0.5:
        # Metal: roughconductor com eta do ouro (aproximação)
        return f"""    <bsdf type="roughconductor" id="{mat_id}">
        <rgb name="specular_reflectance" value="{fmt(kd_r)}, {fmt(kd_g)}, {fmt(kd_b)}"/>
        <float name="alpha" value="{fmt(max(0.01, roughness))}"/>
        <string name="distribution" value="ggx"/>
        <string name="material" value="Au"/>
    </bsdf>
"""
    else:
        return f"""    <bsdf type="roughplastic" id="{mat_id}">
        <rgb name="diffuse_reflectance"  value="{fmt(kd_r)}, {fmt(kd_g)}, {fmt(kd_b)}"/>
        <float name="alpha" value="{fmt(max(0.01, roughness))}"/>
        <string name="distribution" value="ggx"/>
        <float name="int_ior" value="1.5"/>
    </bsdf>
"""

def oren_nayar_bsdf(mat_id, kd_r, kd_g, kd_b, sigma_rad):
    """Oren-Nayar: Mitsuba tem plugin nativo."""
    sigma_deg = math.degrees(sigma_rad) if sigma_rad < math.pi else sigma_rad * 30
    return f"""    <bsdf type="diffuse" id="{mat_id}">
        <!-- Mitsuba 3 não tem Oren-Nayar nativo; usar diffuse como aproximação -->
        <!-- sigma={fmt(sigma_rad)} rad -->
        <rgb name="reflectance" value="{fmt(kd_r)}, {fmt(kd_g)}, {fmt(kd_b)}"/>
    </bsdf>
"""

def ward_bsdf(mat_id, kd_r, kd_g, kd_b, ks_r, ks_g, ks_b, alpha_x, alpha_y):
    """Ward via roughconductor anisotrópico."""
    return f"""    <bsdf type="roughconductor" id="{mat_id}">
        <!-- Ward anisotrópico aproximado via GGX anisotrópico -->
        <rgb name="specular_reflectance" value="{fmt(ks_r)}, {fmt(ks_g)}, {fmt(ks_b)}"/>
        <float name="alpha_u" value="{fmt(max(0.01, alpha_x))}"/>
        <float name="alpha_v" value="{fmt(max(0.01, alpha_y))}"/>
        <string name="distribution" value="ggx"/>
    </bsdf>
"""

def ashikhmin_bsdf(mat_id, kd_r, kd_g, kd_b, ks_r, ks_g, ks_b, nu, nv):
    """Ashikhmin-Shirley via roughplastic anisotrópico."""
    # Converter nu/nv Phong para roughness GGX: alpha ≈ sqrt(2/(n+2))
    alpha_u = math.sqrt(2.0 / (nu + 2.0)) if nu > 0 else 1.0
    alpha_v = math.sqrt(2.0 / (nv + 2.0)) if nv > 0 else 1.0
    return f"""    <bsdf type="roughplastic" id="{mat_id}">
        <!-- Ashikhmin-Shirley: alpha_u={fmt(alpha_u)}, alpha_v={fmt(alpha_v)} -->
        <rgb name="diffuse_reflectance"  value="{fmt(kd_r)}, {fmt(kd_g)}, {fmt(kd_b)}"/>
        <rgb name="specular_reflectance" value="{fmt(ks_r)}, {fmt(ks_g)}, {fmt(ks_b)}"/>
        <float name="alpha" value="{fmt(max(0.01, (alpha_u+alpha_v)/2))}"/>
        <string name="distribution" value="ggx"/>
    </bsdf>
"""

# ---------------------------------------------------------------------------
# Gerador de ficheiros OBJ para triângulos (Mitsuba lê .obj)
# ---------------------------------------------------------------------------

def write_triangle_obj(filepath, v1, v2, v3):
    x1,y1,z1 = v1; x2,y2,z2 = v2; x3,y3,z3 = v3
    with open(filepath, "w") as f:
        f.write(f"v {fmt(x1)} {fmt(y1)} {fmt(z1)}\n")
        f.write(f"v {fmt(x2)} {fmt(y2)} {fmt(z2)}\n")
        f.write(f"v {fmt(x3)} {fmt(y3)} {fmt(z3)}\n")
        f.write("f 1 2 3\n")

def write_multi_triangle_obj(filepath, triangles):
    """Escreve múltiplos triângulos num único .obj."""
    with open(filepath, "w") as f:
        idx = 1
        for v1, v2, v3 in triangles:
            x1,y1,z1 = v1; x2,y2,z2 = v2; x3,y3,z3 = v3
            f.write(f"v {fmt(x1)} {fmt(y1)} {fmt(z1)}\n")
            f.write(f"v {fmt(x2)} {fmt(y2)} {fmt(z2)}\n")
            f.write(f"v {fmt(x3)} {fmt(y3)} {fmt(z3)}\n")
            f.write(f"f {idx} {idx+1} {idx+2}\n")
            idx += 3

def mesh_xml(obj_filename, bsdf_id):
    return f"""    <shape type="obj">
        <string name="filename" value="{obj_filename}"/>
        <ref id="{bsdf_id}"/>
    </shape>
"""

def emissive_mesh_xml(obj_filename, power_r, power_g, power_b):
    """Triângulo com emissão (AreaLight)."""
    return f"""    <shape type="obj">
        <string name="filename" value="{obj_filename}"/>
        <emitter type="area">
            <rgb name="radiance" value="{fmt(power_r)}, {fmt(power_g)}, {fmt(power_b)}"/>
        </emitter>
    </shape>
"""

# ---------------------------------------------------------------------------
# Definições das cenas (espelham o C++ do VI-RT)
# ---------------------------------------------------------------------------

def build_phong_sphere_scene(out_dir):
    """PhongSphereScene — 4 esferas Phong."""
    cam = CAMERA
    xml  = XML_HEADER.format(
        fov=cam["fov"],
        ex=cam["eye"][0], ey=cam["eye"][1], ez=cam["eye"][2],
        ax=cam["at"][0],  ay=cam["at"][1],  az=cam["at"][2],
        ux=cam["up"][0],  uy=cam["up"][1],  uz=cam["up"][2],
        spp=cam["spp"], w=cam["width"], h=cam["height"],
    )

    # Materiais
    xml += diffuse_bsdf("mat_diff",  0.6, 0.2, 0.2)
    xml += phong_bsdf("mat_rough",   0.2, 0.4, 0.7,  0.8, 0.8, 0.8,   5.0)
    xml += phong_bsdf("mat_mid",     0.2, 0.6, 0.2,  0.8, 0.8, 0.8,  50.0)
    xml += phong_bsdf("mat_shiny",   0.6, 0.4, 0.1,  0.9, 0.9, 0.9, 500.0)

    # Esferas
    xml += sphere_xml(-3, 0, 5, 0.8, "mat_diff")
    xml += sphere_xml(-1, 0, 5, 0.8, "mat_rough")
    xml += sphere_xml( 1, 0, 5, 0.8, "mat_mid")
    xml += sphere_xml( 3, 0, 5, 0.8, "mat_shiny")

    # Luzes
    xml += ambient_light_xml(0.05, 0.05, 0.05)
    xml += point_light_xml(300, 300, 300, -1, 2, 0)

    xml += XML_FOOTER
    return xml, {}   # {} = nenhum .obj extra necessário


def build_cook_torrance_sphere_scene(out_dir):
    """CookTorranceSphereScene — 4 metais."""
    cam = CAMERA
    xml  = XML_HEADER.format(
        fov=cam["fov"],
        ex=cam["eye"][0], ey=cam["eye"][1], ez=cam["eye"][2],
        ax=cam["at"][0],  ay=cam["at"][1],  az=cam["at"][2],
        ux=cam["up"][0],  uy=cam["up"][1],  uz=cam["up"][2],
        spp=cam["spp"], w=cam["width"], h=cam["height"],
    )

    r = 0.3
    xml += cook_torrance_bsdf("mat_gold",   1.00, 0.71, 0.29, r, 1.0)
    xml += cook_torrance_bsdf("mat_copper", 0.95, 0.64, 0.54, r, 1.0)
    xml += cook_torrance_bsdf("mat_silver", 0.95, 0.93, 0.88, r, 1.0)
    xml += cook_torrance_bsdf("mat_iron",   0.56, 0.57, 0.58, r, 1.0)

    xml += sphere_xml(-3, 0, 3, 0.8, "mat_gold")
    xml += sphere_xml(-1, 0, 3, 0.8, "mat_copper")
    xml += sphere_xml( 1, 0, 3, 0.8, "mat_silver")
    xml += sphere_xml( 3, 0, 3, 0.8, "mat_iron")

    # setCTLighting: 2 AreaLights + 1 PointLight + ambient + chão
    # AreaLights viram emissive meshes
    objs = {}
    tris_key = [
        ((-1.5,5,2),(1.5,5,2),(1.5,5,4)),
        ((-1.5,5,2),(1.5,5,4),(-1.5,5,4)),
    ]
    for i, (v1,v2,v3) in enumerate(tris_key):
        fname = f"key_light_{i}.obj"
        write_triangle_obj(os.path.join(out_dir, fname), v1, v2, v3)
        objs[fname] = True
        xml += emissive_mesh_xml(fname, 400/3.14159, 400/3.14159, 400/3.14159)

    xml += point_light_xml(100, 100, 100, -5, 2, 1)
    xml += ambient_light_xml(0.02, 0.02, 0.02)

    # Chão (2 triângulos)
    floor_tris = [
        ((-6,-0.8,-2),(6,-0.8,-2),(6,-0.8,8)),
        ((-6,-0.8,-2),(6,-0.8, 8),(-6,-0.8,8)),
    ]
    fname = "floor.obj"
    write_multi_triangle_obj(os.path.join(out_dir, fname), floor_tris)
    objs[fname] = True
    xml += diffuse_bsdf("mat_floor", 0.07, 0.07, 0.07)
    xml += mesh_xml(fname, "mat_floor")

    xml += XML_FOOTER
    return xml, objs


def build_oren_nayar_scene(out_dir):
    """OrenNayarLambertVsON — 3 esferas sigma=0/0.5/0.9."""
    cam = CAMERA
    xml  = XML_HEADER.format(
        fov=cam["fov"],
        ex=cam["eye"][0], ey=cam["eye"][1], ez=cam["eye"][2],
        ax=cam["at"][0],  ay=cam["at"][1],  az=cam["at"][2],
        ux=cam["up"][0],  uy=cam["up"][1],  uz=cam["up"][2],
        spp=cam["spp"], w=cam["width"], h=cam["height"],
    )

    kd = (0.7, 0.5, 0.3)
    xml += oren_nayar_bsdf("mat_lamb", *kd, 0.0)
    xml += oren_nayar_bsdf("mat_on_mid", *kd, 0.5)
    xml += oren_nayar_bsdf("mat_on_rug", *kd, 0.9)

    xml += sphere_xml(-2.5, 0, 3, 1.0, "mat_lamb")
    xml += sphere_xml( 0.0, 0, 3, 1.0, "mat_on_mid")
    xml += sphere_xml( 2.5, 0, 3, 1.0, "mat_on_rug")

    xml += point_light_xml(260, 260, 260, 0, 0.5, -4.5)
    xml += ambient_light_xml(0.04, 0.04, 0.04)

    xml += XML_FOOTER
    return xml, {}


def build_ward_scene(out_dir):
    """WardTestStandart — 4 esferas: iso liso/rugoso + aniso."""
    cam = CAMERA
    xml  = XML_HEADER.format(
        fov=cam["fov"],
        ex=cam["eye"][0], ey=cam["eye"][1], ez=cam["eye"][2],
        ax=cam["at"][0],  ay=cam["at"][1],  az=cam["at"][2],
        ux=cam["up"][0],  uy=cam["up"][1],  uz=cam["up"][2],
        spp=cam["spp"], w=cam["width"], h=cam["height"],
    )

    kd = (0.06, 0.06, 0.06)
    ks = (0.90, 0.90, 0.90)
    xml += ward_bsdf("mat_iso_liso",  *kd, *ks, 0.10, 0.10)
    xml += ward_bsdf("mat_iso_rug",   *kd, *ks, 0.40, 0.40)
    xml += ward_bsdf("mat_aniso",     *kd, *ks, 0.10, 0.40)
    xml += ward_bsdf("mat_aniso_f",   *kd, *ks, 0.05, 0.50)

    xml += sphere_xml(-3, 0.5, 3, 0.8, "mat_iso_liso")
    xml += sphere_xml(-1, 0.5, 3, 0.8, "mat_iso_rug")
    xml += sphere_xml( 1, 0.5, 3, 0.8, "mat_aniso")
    xml += sphere_xml( 3, 0.5, 3, 0.8, "mat_aniso_f")

    xml += point_light_xml(320, 320, 320, 0, 2.5, -2)
    xml += ambient_light_xml(0.03, 0.03, 0.03)

    objs = {}
    floor_tris = [
        ((-6,-0.3,-2),(6,-0.3,-2),(6,-0.3,8)),
        ((-6,-0.3,-2),(6,-0.3, 8),(-6,-0.3,8)),
    ]
    fname = "floor.obj"
    write_multi_triangle_obj(os.path.join(out_dir, fname), floor_tris)
    objs[fname] = True
    xml += diffuse_bsdf("mat_floor", 0.07, 0.07, 0.07)
    xml += mesh_xml(fname, "mat_floor")

    xml += XML_FOOTER
    return xml, objs


def build_ashikhmin_scene(out_dir):
    """AshikhminShirleyTestStandart — 4 esferas."""
    cam = CAMERA
    xml  = XML_HEADER.format(
        fov=cam["fov"],
        ex=cam["eye"][0], ey=cam["eye"][1], ez=cam["eye"][2],
        ax=cam["at"][0],  ay=cam["at"][1],  az=cam["at"][2],
        ux=cam["up"][0],  uy=cam["up"][1],  uz=cam["up"][2],
        spp=cam["spp"], w=cam["width"], h=cam["height"],
    )

    kd_d = (0.10, 0.20, 0.80)
    kd_m = (1.00, 0.71, 0.29)
    ks_d = (0.50, 0.50, 0.50)
    ks_m = (0.90, 0.75, 0.50)

    xml += ashikhmin_bsdf("mat_plas_s",  *kd_d, *ks_d,   10,   10)
    xml += ashikhmin_bsdf("mat_plas_r",  *kd_d, *ks_d,   80,   80)
    xml += ashikhmin_bsdf("mat_metal_a", *kd_m, *ks_m,  500,   10)
    xml += ashikhmin_bsdf("mat_metal_p", *kd_m, *ks_m, 1000, 1000)

    xml += sphere_xml(-3, 0, 3, 0.8, "mat_plas_s")
    xml += sphere_xml(-1, 0, 3, 0.8, "mat_plas_r")
    xml += sphere_xml( 1, 0, 3, 0.8, "mat_metal_a")
    xml += sphere_xml( 3, 0, 3, 0.8, "mat_metal_p")

    xml += point_light_xml(500, 500, 500, 0, 4, 0)
    xml += ambient_light_xml(0.02, 0.02, 0.02)

    objs = {}
    floor_tris = [
        ((-6,-0.9,-2),(6,-0.9,-2),(6,-0.9,8)),
        ((-6,-0.9,-2),(6,-0.9, 8),(-6,-0.9,8)),
    ]
    fname = "floor.obj"
    write_multi_triangle_obj(os.path.join(out_dir, fname), floor_tris)
    objs[fname] = True
    xml += diffuse_bsdf("mat_floor", 0.07, 0.07, 0.07)
    xml += mesh_xml(fname, "mat_floor")

    xml += XML_FOOTER
    return xml, objs


def build_disney_scene(out_dir):
    """DisneyPresetsScene — 5 esferas com o Principled BSDF do Mitsuba."""
    cam = CAMERA
    xml  = XML_HEADER.format(
        fov=cam["fov"],
        ex=cam["eye"][0], ey=cam["eye"][1], ez=cam["eye"][2],
        ax=cam["at"][0],  ay=cam["at"][1],  az=cam["at"][2],
        ux=cam["up"][0],  uy=cam["up"][1],  uz=cam["up"][2],
        spp=cam["spp"], w=cam["width"], h=cam["height"],
    )

    # Mitsuba 3 tem o plugin "principled" que mapeia directamente
    presets = [
        # (id, base_color, metallic, roughness, specular)
        ("mat_gold",     "1.00, 0.78, 0.34", 1.0, 0.40, 0.5),
        ("mat_plastic",  "0.10, 0.20, 0.80", 0.0, 0.35, 0.5),
        ("mat_fabric",   "0.60, 0.10, 0.20", 0.0, 0.85, 0.5),
        ("mat_carpaint", "0.70, 0.05, 0.05", 0.0, 0.40, 0.5),
        ("mat_wax",      "0.90, 0.75, 0.65", 0.0, 0.60, 0.5),
    ]

    for mat_id, bc, met, rug, spec in presets:
        xml += f"""    <bsdf type="principled" id="{mat_id}">
        <rgb name="base_color" value="{bc}"/>
        <float name="metallic"   value="{fmt(met)}"/>
        <float name="roughness"  value="{fmt(rug)}"/>
        <float name="specular"   value="{fmt(spec)}"/>
    </bsdf>
"""

    xs = [-3.4, -1.7, 0.0, 1.7, 3.4]
    ids = ["mat_gold","mat_plastic","mat_fabric","mat_carpaint","mat_wax"]
    for x, mid in zip(xs, ids):
        xml += sphere_xml(x, 0, 3, 0.8, mid)

    xml += point_light_xml(500, 500, 500, 0, 4, -1)
    xml += ambient_light_xml(0.15, 0.15, 0.15)

    objs = {}
    floor_tris = [
        ((-8,-0.85,-2),(8,-0.85,-2),(8,-0.85,9)),
        ((-8,-0.85,-2),(8,-0.85, 9),(-8,-0.85,9)),
    ]
    fname = "floor.obj"
    write_multi_triangle_obj(os.path.join(out_dir, fname), floor_tris)
    objs[fname] = True
    xml += diffuse_bsdf("mat_floor", 0.07, 0.07, 0.07)
    xml += mesh_xml(fname, "mat_floor")

    xml += XML_FOOTER
    return xml, objs


# ---------------------------------------------------------------------------
# Registo de cenas disponíveis
# ---------------------------------------------------------------------------

SCENES = {
    "phong_spheres":       (build_phong_sphere_scene,    "PhongSphereScene"),
    "cook_torrance":       (build_cook_torrance_sphere_scene, "CookTorranceSphereScene"),
    "oren_nayar":          (build_oren_nayar_scene,       "OrenNayarLambertVsON"),
    "ward":                (build_ward_scene,             "WardTestStandart"),
    "ashikhmin":           (build_ashikhmin_scene,        "AshikhminShirleyTestStandart"),
    "disney":              (build_disney_scene,           "DisneyPresetsScene"),
}


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Gera cenas Mitsuba 3 a partir do VI-RT")
    parser.add_argument("--scene", default="all",
                        help=f"Nome da cena ou 'all'. Disponíveis: {', '.join(SCENES)}")
    parser.add_argument("--output-dir", default="mitsuba_scenes",
                        help="Directório de saída para os XMLs e OBJs")
    args = parser.parse_args()

    scenes_to_gen = list(SCENES.keys()) if args.scene == "all" else [args.scene]

    for scene_name in scenes_to_gen:
        if scene_name not in SCENES:
            print(f"[ERRO] Cena desconhecida: {scene_name}")
            continue

        scene_dir = os.path.join(args.output_dir, scene_name)
        os.makedirs(scene_dir, exist_ok=True)

        builder, vi_rt_name = SCENES[scene_name]
        print(f"[INFO] Gerando cena '{scene_name}' (VI-RT: {vi_rt_name})...")

        xml_content, extra_files = builder(scene_dir)

        xml_path = os.path.join(scene_dir, "scene.xml")
        with open(xml_path, "w") as f:
            f.write(xml_content)

        print(f"  -> {xml_path}")
        for fname in extra_files:
            print(f"  -> {os.path.join(scene_dir, fname)}")

    print("\n[OK] Cenas geradas. Para renderizar:")
    print(f"  mitsuba {args.output_dir}/<cena>/scene.xml -o <cena>_mitsuba.exr")
    print("  ou usar o script render_all.sh")


if __name__ == "__main__":
    main()