//
//  WhittedShader.cpp
//  VI-RT-LPS
//
//  Created by Luis Paulo Santos on 14/03/2023.
//

#include "WhittedShader.hpp"
#include "BRDF.hpp"
#include "ray.hpp"
#include "AmbientLight.hpp"
#include "PointLight.hpp"

#include "Shader_Utils.hpp"

static RGB direct_AmbientLight (AmbientLight * l, BRDF * f) {
    RGB color (0., 0., 0.);
    if (!f->Ka.isZero()) {
        color += f->Ka * l->L();
    }
    return (color);
}

static RGB direct_PointLight (PointLight* l, Scene *scene, Intersection isect, BRDF * f) {
    RGB color (0., 0., 0.);

    if (!f->Kd.isZero() || !f->Ks_brdf.isZero()) {
        Point Lpos;
        RGB L = l->Sample_L(NULL, &Lpos);
        Vector Ldir = isect.p.vec2point(Lpos);
        float Ldistance = Ldir.norm();
        Ldir.normalize();
        float cosL = Ldir.dot(isect.sn);
        if (cosL > 0) {
            Ray shadow = Ray(isect.p, Ldir, SHADOW);
            shadow.pix_x = isect.pix_x;
            shadow.pix_y = isect.pix_y;
            shadow.adjustOrigin(isect.gn);

            if (scene->visibility(shadow, Ldistance - EPSILON)) {
                // propagar coordenadas UV para materiais texturizados
                f->curTexCoord = isect.TexCoord;
                RGB brdf = f->f(Ldir, isect.wo, isect.sn);
                color = L * brdf * cosL;
                if (Ldistance > 0.f) color /= (Ldistance * Ldistance);
            }
        }
    }
    return (color);
}

static RGB directLighting (Scene *scene, Intersection isect, BRDF *f) {
    RGB color (0.,0.,0.);
    for (auto l : scene->lights) {
        if (l->type == AMBIENT_LIGHT) {
            color += direct_AmbientLight ((AmbientLight *)l, f);
            continue;
        }
        if (l->type == POINT_LIGHT) {
            color += direct_PointLight ((PointLight *)l, scene, isect, f);
            continue;
        }
    }
    return color;
}

RGB WhittedShader::specularReflection (Intersection isect, BRDF *f, int depth) {
    RGB color(0.,0.,0.);
    Vector Rdir = reflect(isect.wo, isect.sn);
    Ray specular(isect.p, Rdir, SPEC_REFL);
    specular.pix_x = isect.pix_x;
    specular.pix_y = isect.pix_y;
    specular.FaceID = isect.FaceID;
    specular.adjustOrigin(isect.gn);
    specular.propagating_eta = isect.incident_eta;

    bool intersected;
    Intersection s_isect;
    intersected = scene->trace(specular, &s_isect);
    color = f->Ks * shade (intersected, s_isect, depth+1);
    return color;
}

RGB WhittedShader::specularTransmission (Intersection isect, BRDF *f, int depth) {
    RGB color(0., 0., 0.);
    float const IOR = isect.incident_eta / isect.f->eta;
    Vector const V = -1.*isect.wo;
    Vector const N = isect.sn;
    auto cos_theta = std::fmin(N.dot(-1.*V), 1.0);
    double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);
    bool const cannot_refract = (IOR*sin_theta>1.);
    Vector const dir = (cannot_refract ? reflect(V,N) : refract (V, N, IOR));
    Ray refraction(isect.p, dir, (cannot_refract ? SPEC_REFL : SPEC_TRANS));
    refraction.pix_x = isect.pix_x;
    refraction.pix_y = isect.pix_y;
    refraction.FaceID = isect.FaceID;
    refraction.adjustOrigin(-1. * isect.gn);
    refraction.propagating_eta = (cannot_refract ? isect.incident_eta : isect.f->eta);

    bool intersected;
    Intersection t_isect;
    intersected = scene->trace(refraction, &t_isect);
    color = f->Kt * shade (intersected, t_isect, depth+1);
    return color;
}

RGB WhittedShader::shade(bool intersected, Intersection isect, int depth) {
    RGB color(0.,0.,0.);
    if (!intersected) return (background);
    if (isect.isLight) return isect.Le;

    BRDF *f = isect.f;

    #define MAX_DEPTH 3
    if (!f->Ks.isZero() && depth < MAX_DEPTH) {
        color += specularReflection (isect, f, depth);
    }
    if (!f->Kt.isZero() && depth < MAX_DEPTH) {
        color += specularTransmission (isect, f, depth);
    }

    color += directLighting(scene, isect, f);
    return color;
};