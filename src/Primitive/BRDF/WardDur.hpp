#ifndef WardDur_hpp
#define WardDur_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Ward com normalização de Dür (2006) (alternativa correcta) — substitui o
// √(NdotL·NdotV) original pelo termo LINEAR (NdotL·NdotV). O Ward 1992
// sobre-ilumina a ângulos rasantes; a forma linear atenua mais, sendo uma
// correcção simples e documentada. Mesmo lóbulo, só muda o denominador.

class WardDur : public BRDF {
public:
    float alphaX;
    float alphaY;
    Vector tangent;
    bool hasTangent;

    WardDur() : alphaX(0.3f), alphaY(0.3f),
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

            // DÜR 2006: termo geométrico LINEAR (NdotL·NdotV) em vez de √(...)
            float denom = 4.f * float(M_PI) * alphaX * alphaY
                        * (NdotL * NdotV);
            if (denom < 1e-8f) return color;

            float val = std::min(spec / denom, 50.f);
            color += Ks_brdf * val;
        }

        return color;
    }
};

#endif /* WardDur_hpp */
