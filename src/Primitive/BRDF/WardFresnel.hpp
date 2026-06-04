#ifndef WardFresnel_hpp
#define WardFresnel_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Ward com FRESNEL de Schlick (alternativa correcta) — o Ward clássico omite o
// Fresnel; aqui Ks é modulado por F = Ks + (1−Ks)·(1−HdotL)^5, que CLAREIA o
// bordo/ângulos rasantes (rim brightening). Extensão física, como no
// Ashikhmin-Shirley / CookTorrance. Mesmo lóbulo e denominador do standard.

class WardFresnel : public BRDF {
public:
    float alphaX;
    float alphaY;
    Vector tangent;
    bool hasTangent;

    WardFresnel() : alphaX(0.3f), alphaY(0.3f),
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

            float denom = 4.f * float(M_PI) * alphaX * alphaY
                        * sqrtf(NdotL * NdotV);
            if (denom < 1e-8f) return color;

            float val = std::min(spec / denom, 50.f);

            // FRESNEL de Schlick sobre Ks (por canal): F = Ks + (1−Ks)(1−HdotL)^5
            float HdotL = std::max(H.dot(wi), 0.f);
            float f_schlick = powf(1.f - HdotL, 5.f);
            RGB F(Ks_brdf.R + (1.f - Ks_brdf.R) * f_schlick,
                  Ks_brdf.G + (1.f - Ks_brdf.G) * f_schlick,
                  Ks_brdf.B + (1.f - Ks_brdf.B) * f_schlick);

            color += F * val;
        }

        return color;
    }
};

#endif /* WardFresnel_hpp */
