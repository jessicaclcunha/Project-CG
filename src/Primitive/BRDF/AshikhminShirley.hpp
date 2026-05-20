#ifndef AshikhminShirley_hpp
#define AshikhminShirley_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Ashikhmin-Shirley (2000) — An Anisotropic Phong BRDF Model
//
// f_s = sqrt((nu+2)(nv+2)) / (8π)
//     * (N·H)^(nu*cos²φ + nv*sin²φ) / ((H·wi) * max(N·L, N·V))
//     * F_Schlick(H·wi, Ks_brdf)
//
// f_d = (28*Kd)/(23π) * (1 - Rs) * [1-(1-N·L/2)^5] * [1-(1-N·V/2)^5]
//
// Notas de implementação:
//   - Normalização: (nu+2)(nv+2) conforme convenção consistente com as variantes
//   - Denominador: HdotWi * max(NdotL, NdotV) conforme paper original
//   - Difuso inclui (1-Rs) para conservação de energia
//   - Cap no especular (50.0) para suprimir fireflies no terminador

class AshikhminShirley : public BRDF {
public:
    float nu;        // expoente ao longo da tangente  (alto = lóbulo afiado)
    float nv;        // expoente ao longo da bitangente (diferente = anisotrópico)
    Vector tangent;
    bool hasTangent;

    AshikhminShirley() : nu(100.f), nv(10.f),
                         tangent(Vector(1.f, 0.f, 0.f)), hasTangent(false) {}

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        float NdotL = N.dot(wi);
        float NdotV = N.dot(wo);
        if (NdotL <= 0.f || NdotV <= 0.f) return color;

        // Frame de anisotropia
        Vector T, B;
        if (hasTangent) {
            float d = N.dot(tangent);
            T = tangent - N * d;
            float tlen = T.norm();
            if (tlen > 1e-6f) { T = T / tlen; B = N.cross(T); }
            else { N.CoordinateSystem(&T, &B); }
        } else {
            N.CoordinateSystem(&T, &B);
        }

        // --- Especular ---
        if (!Ks_brdf.isZero() && (type & GLOSSY_REF)) {
            Vector H = wi + wo;
            float hlen = H.norm();
            if (hlen > 1e-6f) {
                H = H / hlen;

                float HdotN  = std::max(N.dot(H),  1e-4f);
                float HdotWi = std::max(wi.dot(H), 1e-4f);
                float HdotT  = H.dot(T);
                float HdotB  = H.dot(B);

                // Expoente anisotrópico: nu*cos²φ + nv*sin²φ
                // projectado no plano tangente de H
                float HperpSq = HdotT * HdotT + HdotB * HdotB;
                float exponent;
                if (HperpSq < 1e-6f) {
                    exponent = (nu + nv) * 0.5f;
                } else {
                    float cos2phi = (HdotT * HdotT) / HperpSq;
                    exponent = nu * cos2phi + nv * (1.f - cos2phi);
                }
                exponent = std::min(exponent, 8000.f);

                // Normalização: sqrt((nu+2)(nv+2)) / (8π)
                float norm    = sqrtf((nu + 2.f) * (nv + 2.f)) / (8.f * float(M_PI));
                float powTerm = powf(HdotN, exponent);

                // Denominador conforme paper: HdotWi * max(NdotL, NdotV)
                float denom = HdotWi * std::max(NdotL, NdotV);
                if (denom >= 1e-8f) {
                    // Fresnel de Schlick
                    float fc = powf(1.f - HdotWi, 5.f);
                    RGB F(Ks_brdf.R + (1.f - Ks_brdf.R) * fc,
                          Ks_brdf.G + (1.f - Ks_brdf.G) * fc,
                          Ks_brdf.B + (1.f - Ks_brdf.B) * fc);

                    // Cap para suprimir fireflies no terminador
                    float spec_val = std::min(norm * powTerm / denom, 50.f);
                    color += F * spec_val;
                }
            }
        }

        // --- Difuso com conservação de energia ---
        // f_d = (28/23π) * Kd * (1-Rs) * [1-(1-NdotL/2)^5] * [1-(1-NdotV/2)^5]
        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            float Rs = Ks_brdf.Y();  // luminância do especular
            float f1 = 1.f - powf(1.f - NdotL * 0.5f, 5.f);
            float f2 = 1.f - powf(1.f - NdotV * 0.5f, 5.f);
            color += Kd * ((28.f / (23.f * float(M_PI))) * (1.f - Rs) * f1 * f2);
        }

        return color;
    }
};

#endif /* AshikhminShirley_hpp */