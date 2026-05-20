#ifndef AshikhminShirleyFInv_hpp
#define AshikhminShirleyFInv_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Ashikhmin-Shirley com Fresnel INVERTIDO: F = F0 + (1-F0) * (H·wi)^5.
// Comportamento oposto ao físico: máxima reflexão no centro (incidência normal),
// mínima reflexão nas bordas (incidência rasante).
// Efeito visual: highlight brilhante ao centro, borda escura — completamente não físico.
class AshikhminShirleyFInv : public BRDF {
public:
    float nu, nv;
    Vector tangent;
    bool hasTangent;

    AshikhminShirleyFInv()
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

            // Fresnel invertido: HdotWi^5 em vez de (1-HdotWi)^5
            // -> brilho máximo ao centro, mínimo na borda
            float f_inv = powf(HdotWi, 5.f);
            RGB F(Ks_brdf.R + (1.f - Ks_brdf.R) * f_inv,
                  Ks_brdf.G + (1.f - Ks_brdf.G) * f_inv,
                  Ks_brdf.B + (1.f - Ks_brdf.B) * f_inv);

            color += F * (norm * powTerm / denom);
        }

        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            float Rs = Ks_brdf.Y();
            float t1 = powf(1.f - NdotL * 0.5f, 5.f);
            float t2 = powf(1.f - NdotV * 0.5f, 5.f);
            float diffFactor = (28.f / (23.f * float(M_PI)))
                               * (1.f - Rs) * (1.f - t1) * (1.f - t2);
            color += Kd * diffFactor;
        }

        return color;
    }
};

#endif /* AshikhminShirleyFInv_hpp */