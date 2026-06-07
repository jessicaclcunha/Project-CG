#ifndef PhongTexture_hpp
#define PhongTexture_hpp

#include "Phong.hpp"
#include "ImagePPM.hpp"
#include <cmath>
#include <algorithm>

class PhongTexture : public Phong {
private:
    ImagePPM texture;
    float tex_W, tex_H;
public:
    PhongTexture(std::string filename) {
        texture.Load(filename);
        textured = true;
        tex_W = float(texture.W);
        tex_H = float(texture.H);
    }

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        if (type & DIFFUSE_REF) {
            float u = std::max(0.f, std::min(curTexCoord.u, 0.9999f));
            float v = std::max(0.f, std::min(1.f - curTexCoord.v, 0.9999f));
            int x = (int)std::floor(u * tex_W);
            int y = (int)std::floor(v * tex_H);
            x = std::max(0, std::min(x, (int)tex_W - 1));
            y = std::max(0, std::min(y, (int)tex_H - 1));
            RGB Kd_tex = Kd * texture.get(x, y);
            color += Kd_tex * (1.f / M_PI);
        }

        if (!Ks_brdf.isZero() && (type & GLOSSY_REF)) {
            float cosWiN = std::max(0.f, N.dot(wi));
            Vector R = 2.f * cosWiN * N - wi;
            float cosRWo = std::max(0.f, R.dot(wo));
            float spec = ((ns + 2.f) / (2.f * M_PI)) * powf(cosRWo, ns);
            color += Ks_brdf * spec;
        }

        return color;
    }
};

#endif /* PhongTexture_hpp */