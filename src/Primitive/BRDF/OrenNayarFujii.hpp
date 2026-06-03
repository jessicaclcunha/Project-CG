#ifndef OrenNayarFujii_hpp
#define OrenNayarFujii_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Alternativa CORRECTA: Oren-Nayar "qualitative" de Fujii (2013),
// "A tiny improvement of Oren-Nayar reflectance model".
// Reformula A e B para garantir conservação de energia (a versão clássica
// pode ganhar energia ligeiramente em sigma alto). A normalização 1/π já
// está embutida em A e B, por isso f = Kd·(A + B·...), sem /π extra.
//
//   A = 1 / (π + (π/2 − 2/3)·σ)
//   B = σ / (π + (π/2 − 2/3)·σ)
//   f = Kd · [A + B·max(0,cosΔφ)·sinα·tanβ]
//
// Resultado esperado: quase idêntico ao standard — paralelo ao F_SG no
// CookTorrance (a forma exacta da curva não altera muito o resultado).
class OrenNayarFujii : public BRDF {
public:
    float sigma;

    OrenNayarFujii() : sigma(0.5f) {}

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            float NdotL = std::max(0.f, N.dot(wi));
            float NdotV = std::max(0.f, N.dot(wo));
            if (NdotL <= 0.f || NdotV <= 0.f) return color;

            float denom = float(M_PI) + (float(M_PI) / 2.f - 2.f / 3.f) * sigma;
            float A = 1.f  / denom;
            float B = sigma / denom;

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
            color += Kd * on;   // A,B já incluem a normalização 1/π
        }

        return color;
    }
};

#endif /* OrenNayarFujii_hpp */
