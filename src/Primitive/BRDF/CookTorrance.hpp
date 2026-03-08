#ifndef CookTorrance_hpp
#define CookTorrance_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

class CookTorrance : public BRDF {
public:
    float roughness;  // [0,1]: 0=espelho perfeito, 1=completamente difuso
    RGB   F0;         // reflectância a incidência normal (ex: metal=0.8, plástico=0.04)

    CookTorrance() : roughness(0.5f), F0(RGB(0.04f, 0.04f, 0.04f)) {}

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        // --- Difuso: Kd / pi ---
        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            color += Kd * (1.f / M_PI);
        }

        // --- Especular: Cook-Torrance GGX/Smith/Schlick ---
        if (!Ks.isZero() && (type & GLOSSY_REF)) {
            float NdotL = std::max(0.f, N.dot(wi));
            float NdotV = std::max(0.f, N.dot(wo));

            if (NdotL > 0.f && NdotV > 0.f) {
                // Half-vector
                Vector H = wi + wo;
                float hlen = H.norm();
                if (hlen > 1e-6f) {
                    H = H / hlen;  // normalizar

                    float NdotH = std::max(0.f, N.dot(H));
                    float VdotH = std::max(0.f, wo.dot(H));

                    float alpha = roughness * roughness;  // remapping: alpha = r^2

                    // D — GGX (Trowbridge-Reitz)
                    float alpha2 = alpha * alpha;
                    float denom  = NdotH * NdotH * (alpha2 - 1.f) + 1.f;
                    float D = alpha2 / (float(M_PI) * denom * denom);

                    // F — Fresnel Schlick: F = F0 + (1-F0) * (1 - VdotH)^5
                    // RGB não tem operator-, por isso expandimos manualmente
                    float f_schlick = powf(1.f - VdotH, 5.f);
                    RGB   F(F0.R + (1.f - F0.R) * f_schlick,
                            F0.G + (1.f - F0.G) * f_schlick,
                            F0.B + (1.f - F0.B) * f_schlick);

                    // G — Smith com GGX (Schlick-GGX aproximação): k = alpha/2
                    float k   = alpha / 2.f;
                    float G1L = NdotL / (NdotL * (1.f - k) + k);
                    float G1V = NdotV / (NdotV * (1.f - k) + k);
                    float G   = G1L * G1V;

                    // Cook-Torrance especular: D*F*G / (4 * NdotL * NdotV)
                    RGB spec = F * (D * G / (4.f * NdotL * NdotV));
                    color += Ks * spec;
                }
            }
        }

        return color;
    }
};

#endif /* CookTorrance_hpp */
