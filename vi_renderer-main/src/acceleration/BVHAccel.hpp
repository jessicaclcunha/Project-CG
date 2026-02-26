#ifndef BVHACCEL_HPP
#define BVHACCEL_HPP

#include "primitive.hpp"
#include "BB_extensions.hpp"
#include "ray.hpp"
#include "intersection.hpp"
#include <vector>
#include <cstdint>

enum class SplitMethod { 
    SAH,           // Surface Area Heuristic (padrão)
    Middle,        // Split no meio
    EqualCounts    // Partições iguais
};

// Info da primitiva durante construção (PBR book)
struct BVHPrimitiveInfo {
    size_t primitiveNumber;
    BB bounds;
    Point centroid;
    
    BVHPrimitiveInfo(size_t primitiveNumber, const BB& bounds)
        : primitiveNumber(primitiveNumber), bounds(bounds),
          centroid(Centroid(bounds)) {}
};

// Node da construção (temporário, PBR book)
struct BVHBuildNode {
    BB bounds;
    BVHBuildNode* children[2];
    int splitAxis, firstPrimOffset, nPrimitives;
    
    void InitLeaf(int first, int n, const BB& b) {
        firstPrimOffset = first;
        nPrimitives = n;
        bounds = b;
        children[0] = children[1] = nullptr;
    }
    
    void InitInterior(int axis, BVHBuildNode* c0, BVHBuildNode* c1) {
        children[0] = c0;
        children[1] = c1;
        bounds = Union(c0->bounds, c1->bounds);
        splitAxis = axis;
        nPrimitives = 0;
    }
};

// Node final linear
struct LinearBVHNode {
    BB bounds;
    union {
        int primitivesOffset;    // leaf
        int secondChildOffset;   // interior 
    };
    uint16_t nPrimitives;  // 0 = interior node
    uint8_t axis;          // split axis
    uint8_t pad[1];        // alinhamento
    
    // Construtor padrão
    LinearBVHNode() : nPrimitives(0), axis(0) {
        pad[0] = 0;
        primitivesOffset = 0;
    }
};

class BVHAccel {
private:
    int maxPrimsInNode;
    SplitMethod splitMethod;
    std::vector<Primitive*> primitives;
    LinearBVHNode* nodes;
    BRDF** materials;
    
    // Métodos de construção (PBR book)
    BVHBuildNode* recursiveBuild(
        std::vector<BVHPrimitiveInfo>& primitiveInfo,
        int start, int end, int* totalNodes,
        std::vector<Primitive*>& orderedPrims);
    
    int flattenBVHTree(BVHBuildNode* node, int* offset);
    
public:
    BVHAccel(std::vector<Primitive*>& p,
             BRDF** mats, 
             int maxPrimsInNode = 4,
             SplitMethod splitMethod = SplitMethod::SAH);
    ~BVHAccel();
    
    bool Intersect(const Ray& ray, Intersection* isect) const;
    bool IntersectP(const Ray& ray) const; 
    
    bool IntersectP(const Ray& ray, float maxDist) const;
    
    void printStats() const;
};

#endif