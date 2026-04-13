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
    //
    //   Implementação: forma algébrica sem acosf/sinf/tanf.
    //   cos(α) = min(NdotL, NdotV)  →  sin(α) = sqrt(1 − cos²α)
    //   cos(β) = max(NdotL, NdotV)  →  tan(β) = sqrt(1 − cos²β) / cosβ
    //   Guard: cosβ ≤ ε → tanβ = 0 (evita divisão por zero em ângulos rasantes)
    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            float NdotL = std::max(0.f, N.dot(wi));
            float NdotV = std::max(0.f, N.dot(wo));

            // Coeficientes A e B — dependem só de sigma
            float s2 = sigma * sigma;
            float A = 1.f - 0.5f * s2 / (s2 + 0.33f);
            float B = 0.45f * s2 / (s2 + 0.09f);

            // sin(α) e tan(β) via identidades trigonométricas directas:
            //   α = max(θi,θr) → cosα = min(NdotL, NdotV)
            //   β = min(θi,θr) → cosβ = max(NdotL, NdotV)
            float cosAlpha = std::min(NdotL, NdotV);
            float cosBeta  = std::max(NdotL, NdotV);
            float sinAlpha = sqrtf(std::max(0.f, 1.f - cosAlpha * cosAlpha));
            float sinBeta  = sqrtf(std::max(0.f, 1.f - cosBeta  * cosBeta));
            float tanBeta  = (cosBeta > 1e-6f) ? sinBeta / cosBeta : 0.f;

            // Diferença azimutal: projectar wi e wo no plano tangente à normal
            float cosDeltaPhi = 0.f;
            Vector wi_perp = wi - N * NdotL;
            Vector wo_perp = wo - N * NdotV;
            float lenI = wi_perp.norm();
            float lenR = wo_perp.norm();
            if (lenI > 1e-6f && lenR > 1e-6f)
                cosDeltaPhi = wi_perp.dot(wo_perp) / (lenI * lenR);

            float on = A + B * std::max(0.f, cosDeltaPhi) * sinAlpha * tanBeta;
            color += Kd * (on / float(M_PI));
        }

        return color;
    }
};

#endif /* OrenNayar_hpp */
