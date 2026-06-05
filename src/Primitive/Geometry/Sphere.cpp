//
//  Sphere.cpp
//  VI-RT
//
//  Created by Luis Paulo Santos on 08/02/2025.
//

#include <stdio.h>
#include <cmath>
#include "Sphere.hpp"

bool Sphere::intersect(Ray r, Intersection *isect) {

    if (!bb.intersect(r)) {
        return false;
    }

    Vector oc = r.o.vec2point(C);
    float h = r.dir.dot(oc);
    float c = oc.normSQ() - radiusSq;
    float discriminant = h*h - c;
    if (discriminant < EPSILON) {
        return (false);
    }

    float t = h - std::sqrt(discriminant);

    if (t > EPSILON)
    {
        Point pHit = r.o + t * r.dir;

        // Normal geométrica: sempre aponta para fora da esfera (não afectada pelo Faceforward)
        Vector normal = C.vec2point(pHit);
        normal.normalize();

        Vector wo = -1.f * r.dir;
        Vector const for_normal = normal.Faceforward(wo);
        isect->p = pHit;
        isect->gn = for_normal;
        isect->sn = for_normal;
        isect->wo = wo;
        isect->depth = t;
        isect->FaceID = -1;
        isect->pix_x = r.pix_x;
        isect->pix_y = r.pix_y;
        isect->incident_eta = r.propagating_eta;

        float u = 0.5f + atan2f(-normal.X, -normal.Z) / (2.f * float(M_PI));
        float v = 0.5f + asinf(std::fmax(-1.f, std::fmin(1.f, normal.Y))) / float(M_PI);

        // clamp para evitar out-of-bounds nas BRDFs texturizadas
        isect->TexCoord.u = std::fmax(0.f, std::fmin(u, 0.9999f));
        isect->TexCoord.v = std::fmax(0.f, std::fmin(v, 0.9999f));

        return true;
    }
    else {
        return false;
    }
}