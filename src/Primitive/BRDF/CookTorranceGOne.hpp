#ifndef CookTorranceGOne_hpp
#define CookTorranceGOne_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Cook-Torrance com G simplificado: apenas G1L (masking do lado da luz).
// Standard usa G = G1L * G1V — corrige auto-sombra em ambas as direccoes.
// Aqui so se corrige a direccao da luz (G1L), ignorando o masking do observador (G1V).
// Resultado: ligeiramente mais brilhante que o standart (G1V >= 1 seria necessario
// para compensar, mas como G1V <= 1, remover G1V amplifica o especular).
class CookTorranceGOne : public BRDF {
public:
    float roughness;
    float metallic;

    CookTorranceGOne() : roughness(0.5f), metallic(0.0f) {}

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        RGB F0(0.04f * (1.f - metallic) + Kd.R * metallic,
               0.04f * (1.f - metallic) + Kd.G * metallic,
               0.04f * (1.f - metallic) + Kd.B * metallic);

        float NdotL = std::max(0.f, N.dot(wi));
        float NdotV = std::max(0.f, N.dot(wo));

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

        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            RGB kD((1.f - F.R) * (1.f - metallic),
                   (1.f - F.G) * (1.f - metallic),
                   (1.f - F.B) * (1.f - metallic));
            color += Kd * kD * (1.f / float(M_PI));
        }

        if (valid_H && (type & GLOSSY_REF)) {
            float alpha2 = alpha * alpha;
            float denom  = NdotH * NdotH * (alpha2 - 1.f) + 1.f;
            float D = alpha2 / (float(M_PI) * denom * denom);
            float k   = (roughness + 1.f) * (roughness + 1.f) / 8.f;
            // G = G1L apenas — masking so do lado da luz
            float G1L = NdotL / (NdotL * (1.f - k) + k);
            color += F * (D * G1L / (4.f * NdotL * NdotV));
        }

        return color;
    }
};

#endif /* CookTorranceGOne_hpp */
