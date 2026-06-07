#ifndef PhongTexture_hpp
#define PhongTexture_hpp

#include "Phong.hpp"
#include "Texture.hpp"
#include <cmath>
#include <algorithm>

class PhongTexture : public Phong {
private:
    Texture* tex;
public:
    PhongTexture(std::string filename) {
        tex = Texture::get(filename);
        textured = true;
    }

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        if (type & DIFFUSE_REF) {
            RGB Kd_tex = Kd * tex->sampleUV(curTexCoord.u, curTexCoord.v);
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