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

class DiffuseTexture: public BRDF {
private:
    ImagePPM texture;
    float tex_W, tex_H;
public:
    DiffuseTexture(std::string filename) {
        texture.Load(filename);
        textured=true;
        tex_W = float (texture.W);
        tex_H = float (texture.H);
    }
    RGB GetKd (Vec2 TexCoord) {
        int x = (int)floor(TexCoord.u * tex_W);
        int y = (int)floor(TexCoord.v * tex_H);

        RGB color = Kd*texture.get(x, y);
        return color;
    }

    // Lambertian diffuse using the texture colour: Kd_tex / pi
    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB Kd_tex = GetKd(curTexCoord);
        return Kd_tex * (1.f / M_PI);
    }
};


#endif /* DiffuseTexture_hpp */
