//
//  DiffuseTexture.hpp
//  VI-RT-V4-PathTracing
//
//  Created by Luis Paulo Santos on 19/03/2025.
//

#ifndef DiffuseTexture_hpp
#define DiffuseTexture_hpp

#include "BRDF.hpp"
#include "Texture.hpp"
#include "Triangle.hpp"
#include <cmath>
#include <algorithm>

class DiffuseTexture: public BRDF {
private:
    Texture* tex;
public:
    DiffuseTexture(std::string filename) {
        tex = Texture::get(filename);
        textured = true;
    }

    RGB GetKd (Vec2 TexCoord) {
        return Kd * tex->sampleUV(TexCoord.u, TexCoord.v);
    }
    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB Kd_tex = GetKd(curTexCoord);
        return Kd_tex * (1.f / M_PI);
    }
};

#endif /* DiffuseTexture_hpp */