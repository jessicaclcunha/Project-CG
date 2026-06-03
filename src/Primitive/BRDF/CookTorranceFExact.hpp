#ifndef CookTorranceFExact_hpp
#define CookTorranceFExact_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Variante de estudo: substitui a aproximação de Schlick pelas equações
// exactas de Fresnel para dieléctricos, usando this->eta (IOR).
// D (GGX) e G (Smith-GGX direct) ficam iguais ao base.
//
// Fórmula exacta (Fresnel dieléctrico):
//   cosT = sqrt(1 - (1-cosI²)/η²)      [TIR se sinT² ≥ 1 → F=1]
//   Rs = ((cosI - η·cosT) / (cosI + η·cosT))²
//   Rp = ((η·cosI - cosT) / (η·cosI + cosT))²
//   F  = (Rs + Rp) / 2                 [escalar: só correcto para metallic=0]
//
// Destina-se a materiais dieléctricos (metallic=0). Varia eta para ver
// como o índice de refracção afecta o Fresnel sem usar a aproximação.

class CookTorranceFExact : public BRDF {
public:
    float roughness;
    float metallic;

    CookTorranceFExact() : roughness(0.5f), metallic(0.0f) { eta = 1.5f; }

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        float NdotL = std::max(0.f, N.dot(wi));
        float NdotV = std::max(0.f, N.dot(wo));

        RGB F(0.04f, 0.04f, 0.04f);  // fallback: F0 dieléctrico padrão
        float NdotH = 0.f, VdotH = 0.f;
        float alpha = roughness * roughness;
        bool valid_H = false;

        if (NdotL > 0.f && NdotV > 0.f) {
            Vector H = wi + wo;
            float hlen = H.norm();
            if (hlen > 1e-6f) {
                H = H / hlen;
                NdotH = std::max(0.f, N.dot(H));
                VdotH = std::max(0.f, wo.dot(H));

                // Fresnel exacto para dieléctrico
                float cosI  = VdotH;
                float sinT2 = (1.f - cosI * cosI) / std::max(eta * eta, 1e-6f);
                float f_val;
                if (sinT2 >= 1.f) {
                    f_val = 1.f;  // reflexão total interna
                } else {
                    float cosT = sqrtf(1.f - sinT2);
                    float denom_s = cosI + eta * cosT;
                    float denom_p = eta * cosI + cosT;
                    float Rs = (cosI - eta * cosT) / std::max(denom_s, 1e-6f);
                    float Rp = (eta * cosI - cosT) / std::max(denom_p, 1e-6f);
                    f_val = std::min(1.f, std::max(0.f, 0.5f * (Rs * Rs + Rp * Rp)));
                }
                F = RGB(f_val, f_val, f_val);
                valid_H = true;
            }
        }

        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            RGB kD((1.f - F.R) * (1.f - metallic),
                   (1.f - F.G) * (1.f - metallic),
                   (1.f - F.B) * (1.f - metallic));
            color += Kd * kD * (1.f / float(M_PI));
        }

        if (valid_H && (type & GLOSSY_REF)) {
            float alpha2 = alpha * alpha;

            // D — GGX (igual ao base)
            float denom = NdotH * NdotH * (alpha2 - 1.f) + 1.f;
            float D = alpha2 / (float(M_PI) * denom * denom);

            // G — Smith-GGX direct (igual ao base)
            float k   = (roughness + 1.f) * (roughness + 1.f) / 8.f;
            float G1L = NdotL / (NdotL * (1.f - k) + k);
            float G1V = NdotV / (NdotV * (1.f - k) + k);
            float G   = G1L * G1V;

            color += F * (D * G / (4.f * NdotL * NdotV));
        }

        return color;
    }
};

#endif /* CookTorranceFExact_hpp */
