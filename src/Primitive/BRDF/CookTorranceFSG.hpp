#ifndef CookTorranceFSG_hpp
#define CookTorranceFSG_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Variante de estudo: substitui Schlick pela aproximação Spherical Gaussian
// de Karis (2013), usada na Unreal Engine.
// D (GGX) e G (Smith-GGX direct) ficam iguais ao base.
//
// Schlick:  F = F0 + (1-F0) * (1-VdotH)^5
// SG Karis: F = F0 + (1-F0) * 2^((-5.55473*VdotH - 6.98316)*VdotH)
//
// Resultado quase idêntico a Schlick — é esse o ponto pedagógico:
// a forma precisa da curva de Fresnel tem pouco impacto visual,
// o que valida Schlick como aproximação robusta.

class CookTorranceFSG : public BRDF {
public:
    float roughness;
    float metallic;

    CookTorranceFSG() : roughness(0.5f), metallic(0.0f) {}

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

                // Spherical Gaussian Schlick (Karis 2013)
                // 2^x = exp(x * ln2),  ln2 ≈ 0.6931472
                float f_sg = expf((-5.55473f * VdotH - 6.98316f) * VdotH * 0.6931472f);
                F = RGB(F0.R + (1.f - F0.R) * f_sg,
                        F0.G + (1.f - F0.G) * f_sg,
                        F0.B + (1.f - F0.B) * f_sg);
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

#endif /* CookTorranceFSG_hpp */
