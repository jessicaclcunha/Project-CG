#ifndef Ward_hpp
#define Ward_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Ward (1992) — Measuring and Modeling Anisotropic Reflection
//
// Fórmula Ward normalizada (Geisler-Moroder & Dür 2010):
//
//   f_r = Ks * exp( -(HdotT²/αx² + HdotB²/αy²) / NdotH² )
//             / ( 4π · αx · αy · sqrt(NdotL · NdotV) )

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

            float val = std::min(spec / denom, 50.f);
            color += Ks_brdf * val;
        }

        return color;
    }
};

#endif /* Ward_hpp */