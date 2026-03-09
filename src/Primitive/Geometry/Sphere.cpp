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
    
    // from https://raytracing.github.io/books/RayTracingInOneWeekend.html#surfacenormalsandmultipleobjects/simplifyingtheray-sphereintersectioncode
    Vector oc = r.o.vec2point(C);
    //float a = r.dir.normSQ();
    //float a = 1.f;   // ray direction is normalized
    float h = r.dir.dot(oc);
    float c = oc.normSQ() - radiusSq;
    float discriminant = h*h - c;
    if (discriminant < EPSILON) {
        return (false);
    }
    
    // intersection distance along ray
    float t = h - std::sqrt(discriminant);
    
    if (t > EPSILON) // ray intersection
    {
        Point pHit = r.o + t* r.dir;
        Vector normal = C.vec2point(pHit);
        normal.normalize();
        
        // Fill Intersection data from triangle hit : pag 165
        Vector wo = -1.f * r.dir;
        // make sure the normal points to the same side of the surface as wo
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

        Vector normal_local = C.vec2point(pHit);
        normal_local.normalize();

        float u = 0.5f + atan2f(normal_local.Z, normal_local.X) / (2.0f * M_PI);        
        float v = 0.5f - asinf(normal_local.Y) / M_PI;

        u = fmodf(u * 2.0f, 1.0f);  // Repete textura 2x
        v = fmodf(v * 2.0f, 1.0f);

        v = fmodf(v + 0.6f, 1.0f);
    
        // Garantir que u e v estão em [0, 1] (por causa de erros de floating point)
        if (u < 0.0f) u = 0.0f;
        if (u > 1.0f) u = 1.0f;
        if (v < 0.0f) v = 0.0f;
        if (v > 1.0f) v = 1.0f;
        
        isect->TexCoord.u = u;
        isect->TexCoord.v = v;
        
        return true;
    }
    else  {// This means that there is a line intersection but not a ray intersection.
        return false;
    }
    
    return false;
}
 
