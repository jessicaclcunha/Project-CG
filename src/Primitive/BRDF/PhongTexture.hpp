#ifndef PhongTexture_hpp
#define PhongTexture_hpp

#include "Phong.hpp"
#include "ImagePPM.hpp"
#include <cmath>

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

    // Kd vem da imagem; especular igual ao Phong normal
    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        int x = (int)floor(curTexCoord.u * tex_W);
        int y = (int)floor(curTexCoord.v * tex_H);
        RGB Kd_tex = Kd * texture.get(x, y);

        // Conservação de energia: Kd_tex + Ks_brdf <= 1 por canal.
        float maxSum = std::max({Kd_tex.R + Ks_brdf.R,
                                 Kd_tex.G + Ks_brdf.G,
                                 Kd_tex.B + Ks_brdf.B});
        float norm = (maxSum > 1.f) ? 1.f / maxSum : 1.f;
        RGB eff_Kd = Kd_tex  * norm;
        RGB eff_Ks = Ks_brdf * norm;

        if (type & DIFFUSE_REF) {
            color += eff_Kd * (1.f / M_PI);
        }

        if (!eff_Ks.isZero() && (type & GLOSSY_REF)) {
            float cosWiN = std::max(0.f, N.dot(wi));
            Vector R = 2.f * cosWiN * N - wi;
            float cosRWo = std::max(0.f, R.dot(wo));
            float spec = ((ns + 2.f) / (2.f * M_PI)) * powf(cosRWo, ns);
            color += eff_Ks * spec;
        }

        return color;
    }
};

#endif /* PhongTexture_hpp */
