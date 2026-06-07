//
//  DistributedShader.cpp
//  VI-RT-LPS
//
//

#include "DistributedShader.hpp"
#include "BRDF.hpp"
#include "ray.hpp"
#include "Shader_Utils.hpp"

RGB DistributedShader::specularReflection (Intersection isect, BRDF *f, int depth) {
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

RGB DistributedShader::specularTransmission (Intersection isect, BRDF *f, int depth) {
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

RGB DistributedShader::shade(bool intersected, Intersection isect, int depth) {
    RGB color(0.,0.,0.);
    if (!intersected) return (background);
    if (isect.isLight) return isect.Le;

    BRDF *f = isect.f;

    #define MAX_DEPTH 3
    if (!f->Ks.isZero() && depth < MAX_DEPTH) {
        color += specularReflection (isect, f, depth+1);
    }
    if (!f->Kt.isZero() && depth < MAX_DEPTH) {
        color += specularTransmission (isect, f, depth+1);
    }

    // directLighting() de directLighting.cpp já propaga curTexCoord
    // antes de cada chamada a f->f() — correcto para texturas
    color += directLighting(scene, isect, f, rng, U_dist, ALL_LIGHTS);
    return color;
};