//
//  Scene.hpp
//  VI-RT
//
//  Created by Luis Paulo Santos on 30/01/2023.
//

#ifndef Scene_hpp
#define Scene_hpp

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "BVHAccel.hpp"
#include "primitive.hpp"
#include "light.hpp"
#include "ray.hpp"
#include "intersection.hpp"
#include "BRDF.hpp"

class AreaLight;

class Scene {
    std::vector <Primitive *> prims;
    std::vector <BRDF *> BRDFs;
    BVHAccel* bvh;
    bool useBVH;
    std::vector <Primitive *> lightPrims; 
    BVHAccel* lightBVH;                    
    bool useLightBVH;
    std::map<Geometry*, AreaLight*> geometryToLight; 
public:
    std::vector <Light *> lights;
    int numPrimitives, numLights, numBRDFs;

    Scene() : numPrimitives(0), numLights(0), numBRDFs(0), 
              bvh(nullptr), useBVH(false), 
              lightBVH(nullptr), useLightBVH(false) {}
    ~Scene();
    void BuildBVH();
    void BuildLightBVH();
    bool SetLights (void) { return true; };
    bool trace (Ray r, Intersection *isect);
    bool visibility (Ray s, const float maxL);
    int AddMaterial (BRDF *mat) {
        BRDFs.push_back (mat);
        numBRDFs++;
        return (numBRDFs-1);  // the material (BRDF) index is required to the primitive
    }
    void AddPrimitive (Primitive *prim) {
        // add primitive to scene
        prims.push_back(prim);
        numPrimitives++;
    }
    void printSummary(void) {
        std::cout << "#primitives = " << numPrimitives << " ; ";
        std::cout << "#lights = " << numLights << " ; ";
        std::cout << "#materials = " << numBRDFs << " ;" << std::endl;
    }
};

#endif /* Scene_hpp */
