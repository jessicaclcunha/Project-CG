#ifndef CookTorranceGCT1982_hpp
#define CookTorranceGCT1982_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Variante de estudo: substitui G Smith-GGX pelo G original do paper
// Cook-Torrance (1982). D (GGX) e F (Schlick) ficam iguais ao base.
//
// G_CT1982 = min(1,  2·NdotH·NdotV/VdotH,  2·NdotH·NdotL/VdotH)
//
// Formula geométrica derivada de Torrance-Sparrow (1967).
// Penaliza menos que Smith em roughness alto — resultado ligeiramente
// mais brilhante em superfícies rugosas, e sem a suavidade do Smith.

class CookTorranceGCT1982 : public BRDF {
public:
    float roughness;
    float metallic;

    CookTorranceGCT1982() : roughness(0.5f), metallic(0.0f) {}

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        RGB F0(0.04f * (1.f - metallic) + Kd.R * metallic,
               0.04f * (1.f - metallic) + Kd.G * metallic,
               0.04f * (1.f - metallic) + Kd.B * metallic);

        float NdotL = std::max(0.f, N.dot(wi));
        float NdotV = std::max(0.f, N.dot(wo));

        RGB F = F0;
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
                float f_schlick = powf(1.f - VdotH, 5.f);
                F = RGB(F0.R + (1.f - F0.R) * f_schlick,
                        F0.G + (1.f - F0.G) * f_schlick,
                        F0.B + (1.f - F0.B) * f_schlick);
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

            // G — Cook-Torrance 1982 (Torrance-Sparrow)
            float vdh_safe = std::max(VdotH, 1e-6f);
            float G = std::min({1.f,
                                2.f * NdotH * NdotV / vdh_safe,
                                2.f * NdotH * NdotL / vdh_safe});

            color += F * (D * G / (4.f * NdotL * NdotV));
        }

        return color;
    }
};

#endif /* CookTorranceGCT1982_hpp */
