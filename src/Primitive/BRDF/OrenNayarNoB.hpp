#ifndef OrenNayarNoB_hpp
#define OrenNayarNoB_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Variante de estudo (ERRADA): Oren-Nayar SEM o termo B (retroreflexão).
// Mantém apenas o escurecimento A. Mostra o que o termo B contribui:
// sem ele, o limbo/terminador volta a ter o falloff Lambertiano, perdendo
// o aspecto "plano" característico das superfícies rugosas.
//
//   f = (Kd/π) * A          (B removido)
//   A = 1 − 0.5σ²/(σ²+0.33)
class OrenNayarNoB : public BRDF {
public:
    float sigma;

    OrenNayarNoB() : sigma(0.5f) {}

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            float NdotL = std::max(0.f, N.dot(wi));
            float NdotV = std::max(0.f, N.dot(wo));
            if (NdotL <= 0.f || NdotV <= 0.f) return color;

            float s2 = sigma * sigma;
            float A = 1.f - 0.5f * s2 / (s2 + 0.33f);

            color += Kd * (A / float(M_PI));
        }

        return color;
    }
};

#endif /* OrenNayarNoB_hpp */
