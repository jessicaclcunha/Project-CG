#ifndef AshikhminShirley_hpp
#define AshikhminShirley_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Ashikhmin-Shirley (2000) — An Anisotropic Phong BRDF Model
//
// f_s = sqrt((nu+1)(nv+1)) / (8π)
//     * (N·H)^(nu*cos²φ + nv*sin²φ) / ((H·L) * max(N·L, N·V))
//     * F_Schlick(H·L, Ks)
//
// f_d = (28*Kd)/(23π) * (1 - Ks) * [1-(1-N·L/2)^5] * [1-(1-N·V/2)^5]
//
// Requer tangente (T) e bitangente (B) para anisotropia real.

class AshikhminShirley : public BRDF {
public:
    float nu;  // expoente especular ao longo da tangente  (alto = mais afiado)
    float nv;  // expoente especular ao longo da bitangente (diferente = anisotrópico)
    Vector tangent;
    bool hasTangent;

    AshikhminShirley() : nu(100.f), nv(10.f),
                         tangent(Vector(1.f, 0.f, 0.f)), hasTangent(false) {}

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
            else { N.CoordinateSystem(&T, &B); }
        } else {
            N.CoordinateSystem(&T, &B);
        }

        if (!Ks_brdf.isZero() && (type & GLOSSY_REF)) {
            Vector H = wi + wo;
            float hlen = H.norm();
            if (hlen > 1e-6f) {
                H = H / hlen;

                float NdotH = std::max(N.dot(H), 0.f);
                float HdotL = std::max(H.dot(wi), 1e-4f);
                float HdotT = H.dot(T);
                float HdotB = H.dot(B);
                float sinSqH = std::max(1.f - NdotH * NdotH, 0.f);

                float exponent = (sinSqH > 1e-5f)
                    ? (nu * HdotT * HdotT + nv * HdotB * HdotB) / sinSqH
                    : 0.5f * (nu + nv);
                exponent = std::min(exponent, 8000.f);

                float norm  = sqrtf((nu + 1.f) * (nv + 1.f)) / (8.f * float(M_PI));
                float D     = norm * powf(NdotH, exponent);

                // CORRECÇÃO: denominador inclui NdotL para cancelar o cosL externo
                // evita o pico no terminador
                float denom = HdotL * (NdotL + NdotV - NdotL * NdotV);
                if (denom < 1e-8f) goto skip_spec;

                float fc = powf(1.f - HdotL, 5.f);
                RGB F(Ks_brdf.R + (1.f - Ks_brdf.R) * fc,
                    Ks_brdf.G + (1.f - Ks_brdf.G) * fc,
                    Ks_brdf.B + (1.f - Ks_brdf.B) * fc);

                // Cap para evitar fireflies no terminador
                float spec_val = std::min(D / denom, 50.f);
                color += F * spec_val;
            }
        }
        skip_spec:;

        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            float f1 = 1.f - powf(1.f - NdotL * 0.5f, 5.f);
            float f2 = 1.f - powf(1.f - NdotV * 0.5f, 5.f);
            color += Kd * (28.f / (23.f * float(M_PI))) * f1 * f2;
        }

        return color;
    }
};

#endif /* AshikhminShirley_hpp */