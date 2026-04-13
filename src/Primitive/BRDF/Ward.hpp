#ifndef Ward_hpp
#define Ward_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Ward (1992) — Measuring and Modeling Anisotropic Reflection
//
// Fórmula original Ward (1992) com extensão Fresnel Schlick:
//
//   f_r = F(VdotH, Ks) * exp( -(HdotT²/αx² + HdotB²/αy²) / NdotH² )
//             / ( 4π · αx · αy · sqrt(NdotL · NdotV) )
//
//   Ks_brdf actua como F0 (reflectância especular a incidência normal).
//   A incidência normal: F = Ks_brdf.  À grazing: F → (1,1,1) (Fresnel correcto).
//
// Limitação conhecida: Ward (1992) não é estritamente energia-conservante —
// o denominador sqrt(NdotL·NdotV) cresce em ângulos rasantes. Um cap
// empírico previne fireflies para roughness muito baixo.

class Ward : public BRDF {
public:
    float alphaX;        // rugosidade tangencial  [0.01 .. 1.0]
    float alphaY;        // rugosidade bitangencial [0.01 .. 1.0]
    Vector tangent;      // direção de "brushing" na superfície
    bool hasTangent;     // true se tangent foi explicitamente definido

    Ward() : alphaX(0.3f), alphaY(0.3f),
             tangent(Vector(0.f, 0.f, 0.f)), hasTangent(false) {}

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        // --- Difuso Lambertiano ---
        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            color += Kd * (1.f / float(M_PI));
        }

        // --- Especular Ward anisotrópico ---
        if (!Ks_brdf.isZero() && (type & GLOSSY_REF)) {
            float NdotL = N.dot(wi);
            float NdotV = N.dot(wo);

            if (NdotL <= 0.f || NdotV <= 0.f) return color;

            // Half-vector normalizado
            Vector H = wi + wo;
            float hlen = H.norm();
            if (hlen < 1e-6f) return color;
            H = H / hlen;

            float NdotH = std::max(N.dot(H), 1e-4f);
            float NdotH2 = NdotH * NdotH;
            float VdotH = std::max(0.f, wo.dot(H));

            // Frame de anisotropia
            Vector T, B;
            if (hasTangent) {
                // Gram-Schmidt: ortogonaliza tangent relativamente a N
                float nDotTangent = N.dot(tangent);
                T = tangent - N * nDotTangent;
                float tlen = T.norm();
                if (tlen > 1e-6f) {
                    T = T / tlen;
                    B = N.cross(T);
                } else {
                    // Tangente paralela a N — fallback
                    N.CoordinateSystem(&T, &B);
                }
            } else {
                N.CoordinateSystem(&T, &B);
            }

            float HdotT = H.dot(T);
            float HdotB = H.dot(B);

            float ax2 = alphaX * alphaX;
            float ay2 = alphaY * alphaY;

            float exponent = (HdotT * HdotT / ax2 + HdotB * HdotB / ay2) / NdotH2;
            float spec = expf(-exponent);

            float denom = 4.f * float(M_PI) * alphaX * alphaY
                        * sqrtf(NdotL * NdotV);
            if (denom < 1e-8f) return color;

            // Fresnel Schlick: Ks_brdf é F0 (reflectância a incidência normal).
            // A incidência normal (VdotH→1): F ≈ Ks_brdf — comportamento inalterado.
            // A ângulos rasantes (VdotH→0): F → (1,1,1) — todo material reflecte.
            float f_schlick = powf(1.f - VdotH, 5.f);
            RGB F(Ks_brdf.R + (1.f - Ks_brdf.R) * f_schlick,
                  Ks_brdf.G + (1.f - Ks_brdf.G) * f_schlick,
                  Ks_brdf.B + (1.f - Ks_brdf.B) * f_schlick);

            // Cap anti-firefly: Ward (1992) pode violar conservação de energia
            // para roughness baixo (denominador small); limita o pico da BRDF.
            float val = std::min(spec / denom, 50.f);
            color += F * val;
        }

        return color;
    }
};

#endif /* Ward_hpp */