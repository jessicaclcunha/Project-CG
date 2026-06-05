//
//  DiffuseTexture.hpp
//  VI-RT-V4-PathTracing
//
//  Created by Luis Paulo Santos on 19/03/2025.
//

#ifndef DiffuseTexture_hpp
#define DiffuseTexture_hpp

#include "BRDF.hpp"
#include "ImagePPM.hpp"
#include "Triangle.hpp"
#include <cmath>
#include <algorithm>

class DiffuseTexture: public BRDF {
private:
    ImagePPM texture;
    float tex_W, tex_H;
public:
    DiffuseTexture(std::string filename) {
        texture.Load(filename);
        textured = true;
        tex_W = float(texture.W);
        tex_H = float(texture.H);
    }

    RGB GetKd (Vec2 TexCoord) {
        float u = std::max(0.f, std::min(TexCoord.u, 0.9999f));
        float v = std::max(0.f, std::min(1.f - TexCoord.v, 0.9999f));
        int x = (int)std::floor(u * tex_W);
        int y = (int)std::floor(v * tex_H);
        x = std::max(0, std::min(x, (int)tex_W - 1));
        y = std::max(0, std::min(y, (int)tex_H - 1));
        return Kd * texture.get(x, y);
    }
    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB Kd_tex = GetKd(curTexCoord);
        return Kd_tex * (1.f / M_PI);
    }
};

#endif /* DiffuseTexture_hpp */