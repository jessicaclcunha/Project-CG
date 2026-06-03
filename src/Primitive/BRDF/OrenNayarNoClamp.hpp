#ifndef OrenNayarNoClamp_hpp
#define OrenNayarNoClamp_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Variante de estudo (ERRADA): Oren-Nayar sem o clamp max(0,·) no termo
// azimutal. O original usa max(0, cos(φi−φr)) porque só o back-scatter
// (cosΔφ ≥ 0) contribui com retroreflexão. Ao permitir cosΔφ negativo, o
// forward-scatter passa a SUBTRAIR energia, escurecendo incorrectamente as
// zonas onde luz e câmara estão em lados azimutais opostos.
//
//   f = (Kd/π) * [A + B·cosΔφ·sinα·tanβ]   (cosΔφ pode ser < 0)
class OrenNayarNoClamp : public BRDF {
public:
    float sigma;

    OrenNayarNoClamp() : sigma(0.5f) {}

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            float NdotL = std::max(0.f, N.dot(wi));
            float NdotV = std::max(0.f, N.dot(wo));
            if (NdotL <= 0.f || NdotV <= 0.f) return color;

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
            float lenI = wi_perp.norm();
            float lenR = wo_perp.norm();
            if (lenI > 1e-6f && lenR > 1e-6f)
                cosDeltaPhi = wi_perp.dot(wo_perp) / (lenI * lenR);

            // <-- clamp removido: cosDeltaPhi entra com sinal
            float on = A + B * cosDeltaPhi * sinf(alpha) * tanf(beta);
            color += Kd * (on / float(M_PI));
        }

        return color;
    }
};

#endif /* OrenNayarNoClamp_hpp */
