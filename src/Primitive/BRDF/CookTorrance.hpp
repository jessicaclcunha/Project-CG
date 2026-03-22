#ifndef CookTorrance_hpp
#define CookTorrance_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

class CookTorrance : public BRDF {
public:
    float roughness;  // [0,1]: 0=espelho perfeito, 1=completamente difuso
    float metallic;   // [0,1]: 0=dielétrico (F0=0.04), 1=metal (F0=Kd)

    CookTorrance() : roughness(0.5f), metallic(0.0f) {}

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        // --- PBR: F0 derivado de metallic + albedo ---
        // Dielétrico: F0=0.04; Metal: F0=Kd; Intermédiário: lerp
        RGB F0(0.04f * (1.f - metallic) + Kd.R * metallic,
               0.04f * (1.f - metallic) + Kd.G * metallic,
               0.04f * (1.f - metallic) + Kd.B * metallic);

        float NdotV = std::max(0.f, N.dot(wo));

        // --- Difuso: acoplado ao Fresnel para conservação de energia ---
        // Metais não têm difuso (fator 1-metallic); o Fresnel determina
        // quanto vai para especular, o resto fica disponível para difuso.
        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            float f_diff = powf(1.f - NdotV, 5.f);
            RGB F_diff(F0.R + (1.f - F0.R) * f_diff,
                       F0.G + (1.f - F0.G) * f_diff,
                       F0.B + (1.f - F0.B) * f_diff);
            RGB kD((1.f - F_diff.R) * (1.f - metallic),
                   (1.f - F_diff.G) * (1.f - metallic),
                   (1.f - F_diff.B) * (1.f - metallic));
            color += Kd * kD * (1.f / float(M_PI));
        }

        // --- Especular: Cook-Torrance GGX/Smith/Schlick ---
        if (!Ks_brdf.isZero() && (type & GLOSSY_REF)) {
            float NdotL = std::max(0.f, N.dot(wi));

            if (NdotL > 0.f && NdotV > 0.f) {
                // Half-vector
                Vector H = wi + wo;
                float hlen = H.norm();
                if (hlen > 1e-6f) {
                    H = H / hlen;

                    float NdotH = std::max(0.f, N.dot(H));
                    float VdotH = std::max(0.f, wo.dot(H));

                    float alpha  = roughness * roughness;  // remapping: alpha = r^2
                    float alpha2 = alpha * alpha;

                    // D — GGX (Trowbridge-Reitz)
                    float denom = NdotH * NdotH * (alpha2 - 1.f) + 1.f;
                    float D = alpha2 / (float(M_PI) * denom * denom);

                    // F — Fresnel Schlick a VdotH
                    float f_schlick = powf(1.f - VdotH, 5.f);
                    RGB F(F0.R + (1.f - F0.R) * f_schlick,
                          F0.G + (1.f - F0.G) * f_schlick,
                          F0.B + (1.f - F0.B) * f_schlick);

                    // G — Smith-GGX para iluminação direta: k = (r+1)^2 / 8
                    float k   = (roughness + 1.f) * (roughness + 1.f) / 8.f;
                    float G1L = NdotL / (NdotL * (1.f - k) + k);
                    float G1V = NdotV / (NdotV * (1.f - k) + k);
                    float G   = G1L * G1V;

                    // Cook-Torrance especular: D*F*G / (4 * NdotL * NdotV)
                    RGB spec = F * (D * G / (4.f * NdotL * NdotV));
                    color += spec;
                }
            }
        }

        return color;
    }
};

#endif /* CookTorrance_hpp */
