#ifndef CookTorranceDBeckmann_hpp
#define CookTorranceDBeckmann_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Variante de estudo: substitui D GGX pelo D de Beckmann (1963).
// F (Schlick) e G (Smith-GGX direct) ficam iguais ao base.
//
// D_Beckmann = exp(-tan²θ / α²) / (π · α² · cos⁴θ)
//
// Beckmann tem cauda mais curta que GGX: o highlight tem bordas mais
// abruptas, sem o "halo" longo que GGX produz em roughness médio.
// É a NDF usada no paper original de Cook-Torrance (1982).

class CookTorranceDBeckmann : public BRDF {
public:
    float roughness;
    float metallic;

    CookTorranceDBeckmann() : roughness(0.5f), metallic(0.0f) {}

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
            float alpha2  = alpha * alpha;
            float NdotH2  = NdotH * NdotH;

            // D — Beckmann
            float tan2theta = (1.f - NdotH2) / std::max(NdotH2, 1e-6f);
            float D = expf(-tan2theta / std::max(alpha2, 1e-6f))
                    / (float(M_PI) * std::max(alpha2, 1e-6f)
                       * std::max(NdotH2 * NdotH2, 1e-8f));

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

#endif /* CookTorranceDBeckmann_hpp */
