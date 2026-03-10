//
//  BRDF.hpp
//  VI-RT
//
//  Created by Luis Paulo Santos on 30/01/2023.
//

#ifndef BRDF_hpp
#define BRDF_hpp

#include "vector.hpp"
#include "RGB.hpp"

typedef enum {
    SPECULAR_REF=1,
    DIFFUSE_REF=2,
    SPECULAR_TRANS=4,
    GLOSSY_REF=8,
    BRDF_ALL= SPECULAR_REF | DIFFUSE_REF | SPECULAR_TRANS | GLOSSY_REF
} BRDF_TYPES;

class BRDF {
public:
    bool textured;
    float eta;
    RGB Ka, Kd, Ks, Kt;
    RGB Ks_brdf;  // especular glossy do BRDF — independente dos raios de espelho do shader

    Vec2 curTexCoord;  // set by caller before invoking f() for textured materials

    BRDF () {textured=false;}
    ~BRDF () {}
    // return the BRDF RGB value for (incident, scattering) directions and surface normal N
    virtual RGB f (Vector wi, Vector wo, Vector N, const BRDF_TYPES = BRDF_ALL) {return RGB();}
    // return an outgoing direction wo and brdf RGB value for a given wi, normal N and probability
    virtual RGB Sample_f (Vector wi, Vector N, float *prob, Vector *wo, const BRDF_TYPES = BRDF_ALL) {return RGB();}
    // return the probability of sampling wo given wi and surface normal N
    virtual float pdf(Vector wi, Vector wo, Vector N, const BRDF_TYPES = BRDF_ALL) {return 0.;}
};

#endif /* BRDF_hpp */
