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

        // F0 derivado de metallic: dielétrico=0.04, metal=Kd
        RGB F0(0.04f * (1.f - metallic) + Kd.R * metallic,
               0.04f * (1.f - metallic) + Kd.G * metallic,
               0.04f * (1.f - metallic) + Kd.B * metallic);

        float NdotL = std::max(0.f, N.dot(wi));
        float NdotV = std::max(0.f, N.dot(wo));

        // Fresnel a VdotH — partilhado entre difuso e especular
        // Fallback F=F0 se não for possível calcular H (NdotV=0)
        RGB F = F0;
        float NdotH = 0.f, VdotH = 0.f, alpha = roughness * roughness;
        bool valid_H = false;

        if (NdotL > 0.f && NdotV > 0.f) {
            Vector H = wi + wo;
            float hlen = H.norm();
            if (hlen > 1e-6f) {
                H = H / hlen;
                NdotH = std::max(0.f, N.dot(H));
                VdotH = std::max(0.f, wo.dot(H));
                float f_schlick = powf(1.f - VdotH, 5.f);
                F = RGB(F0.R + (1.f - F0.R) * f_schlick,
                        F0.G + (1.f - F0.G) * f_schlick,
                        F0.B + (1.f - F0.B) * f_schlick);
                valid_H = true;
            }
        }

        // --- Difuso: kD = (1-F) * (1-metallic) / pi ---
        // Fresnel determina quanto foi para especular; metallic anula difuso em metais
        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            RGB kD((1.f - F.R) * (1.f - metallic),
                   (1.f - F.G) * (1.f - metallic),
                   (1.f - F.B) * (1.f - metallic));
            color += Kd * kD * (1.f / float(M_PI));
        }

        // --- Especular: Cook-Torrance GGX/Smith/Schlick ---
        if (valid_H && (type & GLOSSY_REF)) {
            float alpha2 = alpha * alpha;

            // D — GGX (Trowbridge-Reitz)
            float denom = NdotH * NdotH * (alpha2 - 1.f) + 1.f;
            float D = alpha2 / (float(M_PI) * denom * denom);

            // G — Smith-GGX para iluminação direta: k = (r+1)²/8
            float k   = (roughness + 1.f) * (roughness + 1.f) / 8.f;
            float G1L = NdotL / (NdotL * (1.f - k) + k);
            float G1V = NdotV / (NdotV * (1.f - k) + k);
            float G   = G1L * G1V;

            color += F * (D * G / (4.f * NdotL * NdotV));
        }

        return color;
    }
};

#endif /* CookTorrance_hpp */
