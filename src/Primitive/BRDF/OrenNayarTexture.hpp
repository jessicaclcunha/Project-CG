#ifndef OrenNayarTexture_hpp
#define OrenNayarTexture_hpp

#include "OrenNayar.hpp"
#include "Texture.hpp"
#include <cmath>
#include <algorithm>

class OrenNayarTexture : public OrenNayar {
private:
    Texture* tex;

public:
    explicit OrenNayarTexture(const std::string& filename) {
        tex = Texture::get(filename);
        textured = true;
    }

    RGB GetKd(const Vec2& uv) {
        return Kd * tex->sampleUV(uv.u, uv.v);
    }

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        if (type & DIFFUSE_REF) {
            RGB Kd_tex = GetKd(curTexCoord);

            float NdotL = std::max(0.f, N.dot(wi));
            float NdotV = std::max(0.f, N.dot(wo));
            float s2 = sigma * sigma;
            float A = 1.f - 0.5f * s2 / (s2 + 0.33f);
            float B = 0.45f * s2 / (s2 + 0.09f);

            float thetaI = acosf(std::min(1.f, NdotL));
            float thetaR = acosf(std::min(1.f, NdotV));
            float alpha  = std::max(thetaI, thetaR);
            float beta   = std::min(thetaI, thetaR);

            float cosDeltaPhi = 0.f;
            Vector wi_perp = wi - N * NdotL;
            Vector wo_perp = wo - N * NdotV;
            float lenI = wi_perp.norm(), lenR = wo_perp.norm();
            if (lenI > 1e-6f && lenR > 1e-6f)
                cosDeltaPhi = wi_perp.dot(wo_perp) / (lenI * lenR);

            float on = A + B * std::max(0.f, cosDeltaPhi) * sinf(alpha) * tanf(beta);
            color += Kd_tex * (on / float(M_PI));
        }

        return color;
    }
};

#endif /* OrenNayarTexture_hpp */