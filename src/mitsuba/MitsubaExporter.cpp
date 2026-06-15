//
//  MitsubaExporter.cpp
//  VI-RT — módulo Mitsuba (opção 3: o C++ exporta o XML a partir da própria Scene)
//

#include "MitsubaExporter.hpp"
#include "scene.hpp"
#include "primitive.hpp"
#include "Sphere.hpp"
#include "triangle.hpp"
#include "light.hpp"
#include "PointLight.hpp"
#include "AreaLight.hpp"
#include "AmbientLight.hpp"
#include "BRDF.hpp"
#include "Phong.hpp"
#include "OrenNayar.hpp"
#include "CookTorrance.hpp"
#include "Ward.hpp"
#include "AshikhminShirley.hpp"
#include "Disney.hpp"

#include <cstdio>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <sys/stat.h>

// --- helpers de formatação ---
static std::string f1(float v) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.6g", v);
    return std::string(buf);
}
static std::string rgb(const RGB& c) {
    return f1(c.R) + ", " + f1(c.G) + ", " + f1(c.B);
}

// Phong/AS exponent → roughness GGX:  alpha ≈ sqrt(2/(n+2))
static float expToAlpha(float n) {
    float a = (n > 0.f) ? sqrtf(2.f / (n + 2.f)) : 1.f;
    if (a < 0.01f) a = 0.01f;
    if (a > 1.f)   a = 1.f;
    return a;
}

// ---------------------------------------------------------------------------
// Mapeamento BRDF (VI-RT) -> BSDF (Mitsuba). Esta é a ÚNICA parte aproximada:
// a geometria/luz/câmara são exactas (lidas da Scene); aqui escolhe-se o BSDF
// do Mitsuba mais próximo de cada modelo. Mirror dos mapeamentos do gerador
// Python antigo, agora a ler os campos reais via dynamic_cast.
// ---------------------------------------------------------------------------
static std::string emitBSDF(BRDF* b, const std::string& id) {
    std::ostringstream o;

    // Disney -> principled (≈ 1:1)
    if (Disney* d = dynamic_cast<Disney*>(b)) {
        o << "    <bsdf type=\"principled\" id=\"" << id << "\">\n"
          << "        <rgb name=\"base_color\" value=\"" << rgb(d->baseColor) << "\"/>\n"
          << "        <float name=\"roughness\" value=\"" << f1(d->roughness) << "\"/>\n"
          << "        <float name=\"metallic\" value=\"" << f1(d->metallic) << "\"/>\n"
          << "        <float name=\"specular\" value=\"" << f1(d->specular) << "\"/>\n"
          << "        <float name=\"spec_tint\" value=\"" << f1(d->specularTint) << "\"/>\n"
          << "        <float name=\"anisotropic\" value=\"" << f1(d->anisotropic) << "\"/>\n"
          << "        <float name=\"sheen\" value=\"" << f1(d->sheen) << "\"/>\n"
          << "        <float name=\"sheen_tint\" value=\"" << f1(d->sheenTint) << "\"/>\n"
          << "        <float name=\"clearcoat\" value=\"" << f1(d->clearcoat) << "\"/>\n"
          << "        <float name=\"clearcoat_gloss\" value=\"" << f1(d->clearcoatGloss) << "\"/>\n"
          << "        <float name=\"flatness\" value=\"" << f1(d->subsurface) << "\"/>\n"
          << "    </bsdf>\n";
        return o.str();
    }
    // CookTorrance -> roughconductor (metal) ou roughplastic (dieléctrico)
    if (CookTorrance* c = dynamic_cast<CookTorrance*>(b)) {
        float alpha = c->roughness > 0.01f ? c->roughness : 0.01f;
        if (c->metallic > 0.5f) {
            o << "    <bsdf type=\"roughconductor\" id=\"" << id << "\">\n"
              << "        <rgb name=\"specular_reflectance\" value=\"" << rgb(c->Kd) << "\"/>\n"
              << "        <float name=\"alpha\" value=\"" << f1(alpha) << "\"/>\n"
              << "        <string name=\"distribution\" value=\"ggx\"/>\n"
              << "        <string name=\"material\" value=\"Au\"/>\n"
              << "    </bsdf>\n";
        } else {
            o << "    <bsdf type=\"roughplastic\" id=\"" << id << "\">\n"
              << "        <rgb name=\"diffuse_reflectance\" value=\"" << rgb(c->Kd) << "\"/>\n"
              << "        <float name=\"alpha\" value=\"" << f1(alpha) << "\"/>\n"
              << "        <string name=\"distribution\" value=\"ggx\"/>\n"
              << "        <float name=\"int_ior\" value=\"1.5\"/>\n"
              << "    </bsdf>\n";
        }
        return o.str();
    }
    // Ward -> roughconductor anisotrópico (αx/αy → alpha_u/alpha_v)
    if (Ward* w = dynamic_cast<Ward*>(b)) {
        o << "    <bsdf type=\"roughconductor\" id=\"" << id << "\">\n"
          << "        <rgb name=\"specular_reflectance\" value=\"" << rgb(w->Ks_brdf) << "\"/>\n"
          << "        <float name=\"alpha_u\" value=\"" << f1(w->alphaX > 0.01f ? w->alphaX : 0.01f) << "\"/>\n"
          << "        <float name=\"alpha_v\" value=\"" << f1(w->alphaY > 0.01f ? w->alphaY : 0.01f) << "\"/>\n"
          << "        <string name=\"distribution\" value=\"ggx\"/>\n"
          << "    </bsdf>\n";
        return o.str();
    }
    // Ashikhmin-Shirley -> roughplastic (nu/nv → alpha médio)
    if (AshikhminShirley* a = dynamic_cast<AshikhminShirley*>(b)) {
        float alpha = 0.5f * (expToAlpha(a->nu) + expToAlpha(a->nv));
        o << "    <bsdf type=\"roughplastic\" id=\"" << id << "\">\n"
          << "        <rgb name=\"diffuse_reflectance\" value=\"" << rgb(a->Kd) << "\"/>\n"
          << "        <rgb name=\"specular_reflectance\" value=\"" << rgb(a->Ks_brdf) << "\"/>\n"
          << "        <float name=\"alpha\" value=\"" << f1(alpha) << "\"/>\n"
          << "        <string name=\"distribution\" value=\"ggx\"/>\n"
          << "    </bsdf>\n";
        return o.str();
    }
    // Phong -> roughplastic (ns → alpha)
    if (Phong* p = dynamic_cast<Phong*>(b)) {
        o << "    <bsdf type=\"roughplastic\" id=\"" << id << "\">\n"
          << "        <rgb name=\"diffuse_reflectance\" value=\"" << rgb(p->Kd) << "\"/>\n"
          << "        <rgb name=\"specular_reflectance\" value=\"" << rgb(p->Ks_brdf) << "\"/>\n"
          << "        <float name=\"alpha\" value=\"" << f1(expToAlpha(p->ns)) << "\"/>\n"
          << "        <string name=\"distribution\" value=\"ggx\"/>\n"
          << "    </bsdf>\n";
        return o.str();
    }
    // Oren-Nayar (e BRDF base difuso) -> diffuse
    // Mitsuba 3 não tem Oren-Nayar nativo → aproximação difusa.
    o << "    <bsdf type=\"diffuse\" id=\"" << id << "\">\n"
      << "        <rgb name=\"reflectance\" value=\"" << rgb(b->Kd) << "\"/>\n"
      << "    </bsdf>\n";
    return o.str();
}

// Escreve um .obj com a lista de triângulos (v + f, índices 1-based)
static void writeObj(const std::string& path,
                     const std::vector<std::vector<Point> >& tris) {
    std::ofstream f(path.c_str());
    int idx = 1;
    for (size_t t = 0; t < tris.size(); ++t) {
        for (int k = 0; k < 3; ++k)
            f << "v " << f1(tris[t][k].X) << " " << f1(tris[t][k].Y) << " " << f1(tris[t][k].Z) << "\n";
        f << "f " << idx << " " << idx+1 << " " << idx+2 << "\n";
        idx += 3;
    }
}

void ExportMitsubaScene(const Scene& scene, const MiCamera& cam, const std::string& dir) {
    mkdir(dir.c_str(), 0777);
    std::ofstream xml((dir + "/scene.xml").c_str());

    // --- cabeçalho: integrador direct (≈ DistributedShader) + sensor ---
    xml << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    xml << "<scene version=\"3.0.0\">\n\n";
    xml << "    <integrator type=\"direct\">\n";
    xml << "        <integer name=\"emitter_samples\" value=\"1\"/>\n";
    xml << "        <integer name=\"bsdf_samples\" value=\"1\"/>\n";
    xml << "    </integrator>\n\n";
    xml << "    <sensor type=\"perspective\">\n";
    xml << "        <float name=\"fov\" value=\"" << f1(cam.fov) << "\"/>\n";
    xml << "        <string name=\"fov_axis\" value=\"y\"/>\n";
    xml << "        <transform name=\"to_world\">\n";
    xml << "            <lookat origin=\"" << f1(cam.eye.X) << "," << f1(cam.eye.Y) << "," << f1(cam.eye.Z) << "\""
        << " target=\"" << f1(cam.at.X) << "," << f1(cam.at.Y) << "," << f1(cam.at.Z) << "\""
        << " up=\"" << f1(cam.up.X) << "," << f1(cam.up.Y) << "," << f1(cam.up.Z) << "\"/>\n";
    xml << "        </transform>\n";
    xml << "        <sampler type=\"independent\"><integer name=\"sample_count\" value=\"" << cam.spp << "\"/></sampler>\n";
    xml << "        <film type=\"hdrfilm\">\n";
    xml << "            <integer name=\"width\" value=\"" << cam.W << "\"/>\n";
    xml << "            <integer name=\"height\" value=\"" << cam.H << "\"/>\n";
    xml << "            <rfilter type=\"gaussian\"/>\n";
    xml << "        </film>\n";
    xml << "    </sensor>\n\n";

    // --- BSDFs (1 por material, definidos antes de serem referenciados) ---
    for (int i = 0; i < scene.getNumBRDFs(); ++i) {
        std::ostringstream id; id << "mat_" << i;
        xml << emitBSDF(scene.getBRDF(i), id.str());
    }
    xml << "\n";

    // --- luzes ---
    int areaIdx = 0;
    for (size_t li = 0; li < scene.lights.size(); ++li) {
        Light* l = scene.lights[li];
        if (l->type == POINT_LIGHT) {
            PointLight* pl = static_cast<PointLight*>(l);
            xml << "    <emitter type=\"point\">\n"
                << "        <rgb name=\"intensity\" value=\"" << rgb(pl->color) << "\"/>\n"
                << "        <point name=\"position\" x=\"" << f1(pl->pos.X) << "\" y=\"" << f1(pl->pos.Y) << "\" z=\"" << f1(pl->pos.Z) << "\"/>\n"
                << "    </emitter>\n";
        } else if (l->type == AMBIENT_LIGHT) {
            AmbientLight* al = static_cast<AmbientLight*>(l);
            xml << "    <emitter type=\"constant\">\n"
                << "        <rgb name=\"radiance\" value=\"" << rgb(al->L()) << "\"/>\n"
                << "    </emitter>\n";
        } else if (l->type == AREA_LIGHT) {
            AreaLight* ar = static_cast<AreaLight*>(l);
            std::ostringstream fn; fn << "arealight_" << areaIdx++ << ".obj";
            std::vector<std::vector<Point> > tri(1, std::vector<Point>(3));
            tri[0][0] = ar->gem->v1; tri[0][1] = ar->gem->v2; tri[0][2] = ar->gem->v3;
            writeObj(dir + "/" + fn.str(), tri);
            xml << "    <shape type=\"obj\">\n"
                << "        <string name=\"filename\" value=\"" << fn.str() << "\"/>\n"
                << "        <emitter type=\"area\"><rgb name=\"radiance\" value=\"" << rgb(ar->power) << "\"/></emitter>\n"
                << "    </shape>\n";
        }
    }
    xml << "\n";

    // --- geometria: esferas inline; triângulos agrupados por material em .obj ---
    std::map<int, std::vector<std::vector<Point> > > triGroups;
    const std::vector<Primitive*>& prims = scene.getPrimitives();
    for (size_t pi = 0; pi < prims.size(); ++pi) {
        Primitive* p = prims[pi];
        std::ostringstream id; id << "mat_" << p->material_ndx;

        if (Sphere* s = dynamic_cast<Sphere*>(p->g)) {
            xml << "    <shape type=\"sphere\">\n"
                << "        <point name=\"center\" x=\"" << f1(s->C.X) << "\" y=\"" << f1(s->C.Y) << "\" z=\"" << f1(s->C.Z) << "\"/>\n"
                << "        <float name=\"radius\" value=\"" << f1(s->radius) << "\"/>\n"
                << "        <ref id=\"" << id.str() << "\"/>\n"
                << "    </shape>\n";
        } else if (Triangle* t = dynamic_cast<Triangle*>(p->g)) {
            std::vector<Point> v(3);
            v[0] = t->v1; v[1] = t->v2; v[2] = t->v3;
            triGroups[p->material_ndx].push_back(v);
        }
    }
    for (std::map<int, std::vector<std::vector<Point> > >::iterator it = triGroups.begin();
         it != triGroups.end(); ++it) {
        std::ostringstream fn; fn << "mesh_" << it->first << ".obj";
        writeObj(dir + "/" + fn.str(), it->second);
        xml << "    <shape type=\"obj\">\n"
            << "        <string name=\"filename\" value=\"" << fn.str() << "\"/>\n"
            << "        <ref id=\"mat_" << it->first << "\"/>\n"
            << "    </shape>\n";
    }

    xml << "\n</scene>\n";
}
