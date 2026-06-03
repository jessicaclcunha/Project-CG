#ifndef OrenNayarFull_hpp
#define OrenNayarFull_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Alternativa CORRECTA: Oren-Nayar COMPLETO (1994), termo de single-scatter.
// O OrenNayar.hpp do projecto é a versão simplificada (só C1 + parte de C2).
// Esta inclui os 3 coeficientes do paper original:
//   C1 = 1 − 0.5·σ²/(σ²+0.33)
//   C2 = 0.45·σ²/(σ²+0.09) · { sinα               se cosΔφ ≥ 0
//                             sinα − (2β/π)³      caso contrário }
//   C3 = 0.125·σ²/(σ²+0.09) · (4αβ/π²)²
//   f  = (Kd/π)·[ C1 + cosΔφ·C2·tanβ + (1−|cosΔφ|)·C3·tan((α+β)/2) ]
//
// C2 corrige o back-scatter em sigma alto; C3 adiciona a contribuição
// inter-microfaceta no forward-scatter. Mais preciso que a versão simplificada.
class OrenNayarFull : public BRDF {
public:
    float sigma;

    OrenNayarFull() : sigma(0.5f) {}

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            float NdotL = std::max(0.f, N.dot(wi));
            float NdotV = std::max(0.f, N.dot(wo));
            if (NdotL <= 0.f || NdotV <= 0.f) return color;

            float s2  = sigma * sigma;
            float t09 = s2 / (s2 + 0.09f);

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

            float C1 = 1.f - 0.5f * s2 / (s2 + 0.33f);

            float twoBetaPi = 2.f * beta / float(M_PI);
            float C2 = 0.45f * t09;
            if (cosDeltaPhi >= 0.f)
                C2 *= sinf(alpha);
            else
                C2 *= (sinf(alpha) - twoBetaPi * twoBetaPi * twoBetaPi);

            float fourABpi2 = 4.f * alpha * beta / (float(M_PI) * float(M_PI));
            float C3 = 0.125f * t09 * fourABpi2 * fourABpi2;

            float term = C1
                       + cosDeltaPhi * C2 * tanf(beta)
                       + (1.f - std::fabs(cosDeltaPhi)) * C3 * tanf((alpha + beta) / 2.f);

            color += Kd * (term / float(M_PI));
        }

        return color;
    }
};

#endif /* OrenNayarFull_hpp */
