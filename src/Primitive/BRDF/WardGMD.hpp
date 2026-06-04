#ifndef WardGMD_hpp
#define WardGMD_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Ward de Geisler-Moroder & Dür (2010) (alternativa correcta) — modelo de
// albedo limitado (bounded). Substitui o √(NdotL·NdotV) do original pelo termo
// (NdotH)^4. É a forma moderna recomendada do Ward. O numerador exp(...) é
// idêntico ao standard; só muda o termo geométrico do denominador.
//
// Nota: o Ward.hpp do projecto está rotulado como GMD 2010, mas usa de facto o
// denominador √(NdotL·NdotV) — ou seja, é o Ward ORIGINAL 1992. Esta classe
// implementa a verdadeira GMD.

class WardGMD : public BRDF {
public:
    float alphaX;
    float alphaY;
    Vector tangent;
    bool hasTangent;

    WardGMD() : alphaX(0.3f), alphaY(0.3f),
             tangent(Vector(0.f, 0.f, 0.f)), hasTangent(false) {}

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            color += Kd * (1.f / float(M_PI));
        }

        if (!Ks_brdf.isZero() && (type & GLOSSY_REF)) {
            float NdotL = N.dot(wi);
            float NdotV = N.dot(wo);

            if (NdotL <= 0.f || NdotV <= 0.f) return color;

            Vector H = wi + wo;
            float hlen = H.norm();
            if (hlen < 1e-6f) return color;
            H = H / hlen;

            float NdotH = std::max(N.dot(H), 1e-4f);
            float NdotH2 = NdotH * NdotH;

            Vector T, B;
            if (hasTangent) {
                float nDotTangent = N.dot(tangent);
                T = tangent - N * nDotTangent;
                float tlen = T.norm();
                if (tlen > 1e-6f) {
                    T = T / tlen;
                    B = N.cross(T);
                } else {
                    N.CoordinateSystem(&T, &B);
                }
            } else {
                N.CoordinateSystem(&T, &B);
            }

            float HdotT = H.dot(T);
            float HdotB = H.dot(B);

            float ax2 = alphaX * alphaX;
            float ay2 = alphaY * alphaY;

            float exponent = (HdotT * HdotT / ax2 + HdotB * HdotB / ay2) / NdotH2;
            float spec = expf(-exponent);

            // GMD 2010: termo geométrico (NdotH)^4 (bounded albedo)
            float NdotH4 = NdotH2 * NdotH2;
            float denom = 4.f * float(M_PI) * alphaX * alphaY * NdotH4;
            if (denom < 1e-8f) return color;

            float val = std::min(spec / denom, 50.f);
            color += Ks_brdf * val;
        }

        return color;
    }
};

#endif /* WardGMD_hpp */
