#ifndef BB_EXTENSIONS_HPP
#define BB_EXTENSIONS_HPP

#include "BB.hpp"
#include <algorithm>

// Funções auxiliares para BVH
// ANTES ESTAVA A USAR: vec2point, contudo estava a dar erro, agr uso os atributos diretamente
inline float SurfaceArea(const BB& bb) {
    float dx = bb.max.X - bb.min.X;
    float dy = bb.max.Y - bb.min.Y;
    float dz = bb.max.Z - bb.min.Z;
    return 2.0f * (dx * dy + dx * dz + dy * dz);
}

inline int MaximumExtent(const BB& bb) {
    float dx = bb.max.X - bb.min.X;
    float dy = bb.max.Y - bb.min.Y;
    float dz = bb.max.Z - bb.min.Z;
    
    if (dx > dy && dx > dz) return 0;
    else if (dy > dz) return 1;
    else return 2;
}

inline Point Centroid(const BB& bb) {
    return Point((bb.min.X + bb.max.X) * 0.5f, 
                (bb.min.Y + bb.max.Y) * 0.5f, 
                (bb.min.Z + bb.max.Z) * 0.5f);
}

inline BB Union(const BB& b1, const BB& b2) {
    BB result;
    result.min = Point(std::min(b1.min.X, b2.min.X),
                      std::min(b1.min.Y, b2.min.Y), 
                      std::min(b1.min.Z, b2.min.Z));
    result.max = Point(std::max(b1.max.X, b2.max.X),
                      std::max(b1.max.Y, b2.max.Y),
                      std::max(b1.max.Z, b2.max.Z));
    return result;
}

inline BB Union(const BB& b, const Point& p) {
    BB result;
    result.min = Point(std::min(b.min.X, p.X),
                      std::min(b.min.Y, p.Y), 
                      std::min(b.min.Z, p.Z));
    result.max = Point(std::max(b.max.X, p.X),
                      std::max(b.max.Y, p.Y),
                      std::max(b.max.Z, p.Z));
    return result;
}

inline bool IntersectP(const BB& bb, const Ray& ray, const Vector& invDir, 
                      const int dirIsNeg[3]) {
    const Point bounds[2] = {bb.min, bb.max};
    
    // X slab
    float tMin = (bounds[dirIsNeg[0]].X - ray.o.X) * invDir.X;
    float tMax = (bounds[1-dirIsNeg[0]].X - ray.o.X) * invDir.X;
    
    // Y slab
    float tyMin = (bounds[dirIsNeg[1]].Y - ray.o.Y) * invDir.Y;
    float tyMax = (bounds[1-dirIsNeg[1]].Y - ray.o.Y) * invDir.Y;
    
    if (tMin > tyMax || tyMin > tMax) return false;
    
    tMin = std::max(tMin, tyMin);
    tMax = std::min(tMax, tyMax);
    
    // Z slab
    float tzMin = (bounds[dirIsNeg[2]].Z - ray.o.Z) * invDir.Z;
    float tzMax = (bounds[1-dirIsNeg[2]].Z - ray.o.Z) * invDir.Z;
    
    if (tMin > tzMax || tzMin > tMax) return false;
    
    tMin = std::max(tMin, tzMin);
    tMax = std::min(tMax, tzMax);
    
    return (tMin < 1e30f) && (tMax > 1e-6f); 
}

#endif