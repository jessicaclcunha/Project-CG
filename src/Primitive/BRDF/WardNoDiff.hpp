#ifndef WardNoDiff_hpp
#define WardNoDiff_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Ward SEM DIFUSO (variante errada) — remove o termo Kd/π. Especular puro.
// Isola a contribuição difusa: como Kd é baixo no estudo (0.06), o impacto é
// pequeno mas mensurável. Paralelo ao AshikhminNoDiff.

class WardNoDiff : public BRDF {
public:
    float alphaX;
    float alphaY;
    Vector tangent;
    bool hasTangent;

    WardNoDiff() : alphaX(0.3f), alphaY(0.3f),
             tangent(Vector(0.f, 0.f, 0.f)), hasTangent(false) {}

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        // SEM DIFUSO: o termo Kd/π é omitido propositadamente.

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
            color += Ks_brdf * val;
        }

        return color;
    }
};

#endif /* WardNoDiff_hpp */
