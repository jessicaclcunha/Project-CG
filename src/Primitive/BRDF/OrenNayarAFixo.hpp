#ifndef OrenNayarAFixo_hpp
#define OrenNayarAFixo_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Variante de estudo (ERRADA): Oren-Nayar com A=1 fixo (sem escurecimento).
// Mantém o termo B (retroreflexão) mas remove o darkening A. Mostra o que o
// termo A faz: sem ele a superfície não escurece com sigma, ficando demasiado
// clara em sigma alto — soma B "grátis" por cima da energia total, aproximando
// uma violação de conservação de energia.
//
//   f = (Kd/π) * [1 + B·max(0,cosΔφ)·sinα·tanβ]
//   B = 0.45σ²/(σ²+0.09)
class OrenNayarAFixo : public BRDF {
public:
    float sigma;

    OrenNayarAFixo() : sigma(0.5f) {}

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            float NdotL = std::max(0.f, N.dot(wi));
            float NdotV = std::max(0.f, N.dot(wo));
            if (NdotL <= 0.f || NdotV <= 0.f) return color;

            float s2 = sigma * sigma;
            float A = 1.f;                              // <-- escurecimento removido
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

            float on = A + B * std::max(0.f, cosDeltaPhi) * sinf(alpha) * tanf(beta);
            color += Kd * (on / float(M_PI));
        }

        return color;
    }
};

#endif /* OrenNayarAFixo_hpp */
