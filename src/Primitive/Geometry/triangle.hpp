//
//  triangle.hpp
//  VI-RT
//
//  Created by Luis Paulo Santos on 05/02/2023.
//

#ifndef triangle_hpp
#define triangle_hpp

#include "geometry.hpp"
#include "vector.hpp"
#include <math.h>

class Triangle: public Geometry {
    Vector computeBarycentrics (Point p);
    Vec2 interpolateTexture(Vector baryCoord);
public:
    bool BackFaceCulling;
    Point v1, v2, v3;
    Vec2 uv1, uv2, uv3;  // texture coordinates for each vertex
    Vector normal;
    Vector edge1, edge2, edge3;
    BB bb;

    bool intersect (Ray r, Intersection *isect);
    bool isInside(Point p);

    // Construtor com normal explícita
    Triangle(Point _v1, Point _v2, Point _v3, Vector _normal, bool backface=true)
        : v1(_v1), v2(_v2), v3(_v3), normal(_normal)
    {
        edge1 = v1.vec2point(v2);
        edge2 = v1.vec2point(v3);
        edge3 = v2.vec2point(v3);
        bb.min.set(v1.X, v1.Y, v1.Z);
        bb.max.set(v1.X, v1.Y, v1.Z);
        bb.update(v2);
        bb.update(v3);
        BackFaceCulling = backface;
        // UV planar por defeito: cobre a face sem distorção nula
        uv1 = Vec2(0.f, 0.f);
        uv2 = Vec2(1.f, 0.f);
        uv3 = Vec2(1.f, 1.f);
    }

    // Construtor sem normal (calculada automaticamente)
    Triangle(Point _v1, Point _v2, Point _v3, bool backface=false)
        : v1(_v1), v2(_v2), v3(_v3), BackFaceCulling(backface)
    {
        edge1 = v1.vec2point(v2);
        edge2 = v1.vec2point(v3);
        edge3 = v2.vec2point(v3);
        normal = edge1.cross(edge2);
        normal.normalize();
        bb.min.set(v1.X, v1.Y, v1.Z);
        bb.max.set(v1.X, v1.Y, v1.Z);
        bb.update(v2);
        bb.update(v3);
        // UV planar por defeito
        uv1 = Vec2(0.f, 0.f);
        uv2 = Vec2(1.f, 0.f);
        uv3 = Vec2(1.f, 1.f);
    }

    void set_uv (Vec2 _uv1, Vec2 _uv2, Vec2 _uv3) {
        uv1 = _uv1;
        uv2 = _uv2;
        uv3 = _uv3;
    }

    // Heron's formula
    float area () {
        const float len1 = edge1.norm();
        const float len2 = edge2.norm();
        const float len3 = edge3.norm();
        const float hp = (len1+len2+len3)/2.f;
        return sqrtf(hp*(hp-len1)*(hp-len2)*(hp-len3));
    }

    float points_area (Point v1, Point v2, Point v3) {
        Vector e1 = v1.vec2point(v2);
        Vector e2 = v2.vec2point(v3);
        Vector e3 = v3.vec2point(v1);
        const float len1 = e1.norm();
        const float len2 = e2.norm();
        const float len3 = e3.norm();
        const float hp = (len1+len2+len3)/2.f;
        return sqrtf(hp*(hp-len1)*(hp-len2)*(hp-len3));
    }
};

#endif /* triangle_hpp */