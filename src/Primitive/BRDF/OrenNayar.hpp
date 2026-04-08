#ifndef OrenNayar_hpp
#define OrenNayar_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

class OrenNayar : public BRDF {
public:
    float sigma;  // desvio padrão das inclinações das microfacetas (radianos)
                  // sigma=0 → Lambertiano puro; sigma→π/2 → muito rugoso

    OrenNayar() : sigma(0.5f) {}

    // Oren-Nayar (1994) — difuso rugoso com retroreflexão
    //
    //   f_r = (Kd/π) * [A + B * max(0, cos(φi−φr)) * sin(α) * tan(β)]
    //
    //   A = 1 − 0.5 * σ² / (σ² + 0.33)
    //   B = 0.45 * σ² / (σ² + 0.09)
    //   α = max(θi, θr),  β = min(θi, θr)
    //
    //   Quando σ=0: A=1, B=0 → f_r = Kd/π  (Lambertiano)
    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            float NdotL = std::max(0.f, N.dot(wi));
            float NdotV = std::max(0.f, N.dot(wo));

            // Coeficientes A e B — dependem só de sigma
            float s2 = sigma * sigma;
            float A = 1.f - 0.5f * s2 / (s2 + 0.33f);
            float B = 0.45f * s2 / (s2 + 0.09f);

            // Ângulos com a normal (θi, θr)
            float thetaI = acosf(std::min(1.f, NdotL));
            float thetaR = acosf(std::min(1.f, NdotV));
            float alpha   = std::max(thetaI, thetaR);
            float beta    = std::min(thetaI, thetaR);

            // Diferença azimutal: projectar wi e wo no plano tangente à normal
            float cosDeltaPhi = 0.f;
            Vector wi_perp = wi - N * NdotL;
            Vector wo_perp = wo - N * NdotV;
            float lenI = wi_perp.norm();
            float lenR = wo_perp.norm();
            if (lenI > 1e-6f && lenR > 1e-6f)
                cosDeltaPhi = wi_perp.dot(wo_perp) / (lenI * lenR);

            float on = A + B * std::max(0.f, cosDeltaPhi) * sinf(alpha) * tanf(beta);
            color += Kd * (on / float(M_PI));
        }

        return color;
    }
};

#endif /* OrenNayar_hpp */
