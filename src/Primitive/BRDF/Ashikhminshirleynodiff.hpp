#ifndef AshikhminShirleyNoDiff_hpp
#define AshikhminShirleyNoDiff_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Ashikhmin-Shirley SEM componente difusa: apenas especular.
// Permite isolar visualmente o lóbulo especular puro —
// útil para perceber o que o termo difuso acrescenta e
// para comparar com materiais condutores (metais puros,
// onde o difuso é fisicamente zero).
// Efeito visual: zonas fora do highlight ficam pretas;
// o lóbulo anisotrópico torna-se bem visível sem "base" de difuso.
class AshikhminShirleyNoDiff : public BRDF {
public:
    float nu, nv;
    Vector tangent;
    bool hasTangent;

    AshikhminShirleyNoDiff()
        : nu(100.f), nv(100.f),
          tangent(Vector(0.f, 0.f, 0.f)), hasTangent(false) {}

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        float NdotL = N.dot(wi);
        float NdotV = N.dot(wo);
        if (NdotL <= 0.f || NdotV <= 0.f) return color;

        Vector T, B;
        if (hasTangent) {
            float d = N.dot(tangent);
            T = tangent - N * d;
            float tlen = T.norm();
            if (tlen > 1e-6f) { T = T / tlen; B = N.cross(T); }
            else N.CoordinateSystem(&T, &B);
        } else {
            N.CoordinateSystem(&T, &B);
        }

        Vector H = wi + wo;
        float hlen = H.norm();
        if (hlen < 1e-6f) return color;
        H = H / hlen;

        float HdotN  = std::max(N.dot(H),  1e-4f);
        float HdotWi = std::max(wi.dot(H), 1e-4f);

        // apenas especular — difuso suprimido completamente
        if (!Ks_brdf.isZero() && (type & GLOSSY_REF)) {
            float HdotT = H.dot(T);
            float HdotB = H.dot(B);
            float HperpSq = HdotT * HdotT + HdotB * HdotB;
            float exponent;
            if (HperpSq < 1e-6f) {
                exponent = (nu + nv) * 0.5f;
            } else {
                float cos2phi = (HdotT * HdotT) / HperpSq;
                exponent = nu * cos2phi + nv * (1.f - cos2phi);
            }
            float norm    = sqrtf((nu + 2.f) * (nv + 2.f)) / (8.f * float(M_PI));
            float powTerm = powf(std::max(HdotN, 0.f), exponent);
            float denom   = HdotWi * std::max(NdotL, NdotV);
            if (denom < 1e-8f) denom = 1e-8f;

            float f_schlick = powf(1.f - HdotWi, 5.f);
            RGB F(Ks_brdf.R + (1.f - Ks_brdf.R) * f_schlick,
                  Ks_brdf.G + (1.f - Ks_brdf.G) * f_schlick,
                  Ks_brdf.B + (1.f - Ks_brdf.B) * f_schlick);

            color += F * (norm * powTerm / denom);
        }

        // difuso: zero — intencionalmente omitido

        return color;
    }
};

#endif /* AshikhminShirleyNoDiff_hpp */