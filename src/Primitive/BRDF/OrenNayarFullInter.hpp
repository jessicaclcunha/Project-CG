#ifndef OrenNayarFullInter_hpp
#define OrenNayarFullInter_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Alternativa CORRECTA: Oren-Nayar COMPLETO + inter-reflexão (1994).
// É o OrenNayarFull (single-scatter C1/C2/C3) somado ao termo L2 de
// inter-reflexão de 2ª ordem (luz que ressalta entre microfacetas):
//
//   f2 = 0.17·(Kd²/π)·(σ²/(σ²+0.13))·[1 − cosΔφ·(2β/π)²]   (Kd² per-canal)
//
// Como depende de Kd², superfícies escuras quase não mostram efeito; as
// claras ganham brilho extra. É o modelo mais completo do paper original.
class OrenNayarFullInter : public BRDF {
public:
    float sigma;

    OrenNayarFullInter() : sigma(0.5f) {}

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

            // --- Single-scatter (igual ao OrenNayarFull) ---
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

            // --- Inter-reflexão L2 (2ª ordem, depende de Kd²) ---
            float inter = 0.17f * (s2 / (s2 + 0.13f))
                        * (1.f - cosDeltaPhi * twoBetaPi * twoBetaPi);
            color += Kd * Kd * (inter / float(M_PI));
        }

        return color;
    }
};

#endif /* OrenNayarFullInter_hpp */
