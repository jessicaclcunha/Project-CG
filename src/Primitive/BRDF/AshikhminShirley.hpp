#ifndef AshikhminShirley_hpp
#define AshikhminShirley_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Ashikhmin & Shirley (2000) — An Anisotropic Phong BRDF Model
// Journal of Graphics Tools, 5(2):25–32.
//
// BRDF anisotrópico com especular dependente de Fresnel e difuso que
// conserva energia: quanto mais especular, menos difuso.
//
// Especular (anisotropic Phong over half-vector H):
//
//   f_s(wi, wo) = sqrt((nu+2)(nv+2)) / (8π)
//               * (H·N)^( nu*cos²φ + nv*sin²φ )
//               / ( (H·wi) * max(N·wi, N·wo) )
//               * F(wi, H)
//
//   onde:
//     nu, nv  — expoentes de Phong nas direções tangente (T) e bitangente (B)
//     φ       — ângulo azimutal de H no espaço tangente
//     F(...)  — Fresnel Schlick em (wi · H)
//
// Difuso (conservação de energia):
//
//   f_d(wi, wo) = 28*Rd / (23π)
//               * (1 - Rs)
//               * (1 - (1 - N·wi/2)^5)
//               * (1 - (1 - N·wo/2)^5)
//
//   onde Rs é a reflectância especular (componente cinzenta de Ks_brdf).
//
// Notas de implementação:
//   - nu = nv → isotrópico (equivale a Phong clássico com o half-vector)
//   - nu >> nv → highlights estreitos na direção T (ex.: metal escovado horizontal)
//   - nu << nv → highlights estreitos na direção B (ex.: metal escovado vertical)
//   - Rs deve ser a luminância de Ks_brdf (Y())
//   - O difuso usa Kd como "Rd" (reflectância difusa base)

class AshikhminShirley : public BRDF {
public:
    float nu;           // expoente de Phong na direção tangente   [1 .. 10000]
    float nv;           // expoente de Phong na direção bitangente [1 .. 10000]
    Vector tangent;     // direção de "brushing" na superfície
    bool hasTangent;    // true se tangent foi definido explicitamente

    AshikhminShirley()
        : nu(100.f), nv(100.f),
          tangent(Vector(0.f, 0.f, 0.f)), hasTangent(false) {}

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        float NdotL = N.dot(wi);
        float NdotV = N.dot(wo);

        // Ambos os lados têm de ser positivos
        if (NdotL <= 0.f || NdotV <= 0.f) return color;

        // ---- Frame de anisotropia (T, B ortogonais a N) -----------------
        Vector T, B;
        if (hasTangent) {
            // Gram-Schmidt: ortogonaliza tangent relativamente a N
            float d = N.dot(tangent);
            T = tangent - N * d;
            float tlen = T.norm();
            if (tlen > 1e-6f) {
                T = T / tlen;
                B = N.cross(T);
            } else {
                N.CoordinateSystem(&T, &B);
            }
        } else {
            N.CoordinateSystem(&T, &B);
        }

        // ---- Half-vector H -----------------------------------------------
        Vector H = wi + wo;
        float hlen = H.norm();
        if (hlen < 1e-6f) return color;
        H = H / hlen;

        float HdotN  = std::max(N.dot(H),  1e-4f);
        float HdotWi = std::max(wi.dot(H), 1e-4f);  // usado no Fresnel e normalização

        // ---- Componente especular Ashikhmin-Shirley ----------------------
        if (!Ks_brdf.isZero() && (type & GLOSSY_REF)) {
            // Ângulo azimutal de H no plano tangente
            float HdotT = H.dot(T);
            float HdotB = H.dot(B);

            // Expoente anisotrópico combinado: nu*cos²φ + nv*sin²φ
            // cos²φ = (H·T)²/‖H_perp‖²,  mas HdotN² + HdotT² + HdotB² ≈ 1
            float HperpSq = HdotT * HdotT + HdotB * HdotB;
            float exponent;
            if (HperpSq < 1e-6f) {
                // H quase paralelo a N → isotrópico puro
                exponent = (nu + nv) * 0.5f;
            } else {
                float cos2phi = (HdotT * HdotT) / HperpSq;
                float sin2phi = 1.f - cos2phi;
                exponent = nu * cos2phi + nv * sin2phi;
            }

            // Normalização do lóbulo: sqrt((nu+2)(nv+2)) / (8π)
            float norm = sqrtf((nu + 2.f) * (nv + 2.f)) / (8.f * float(M_PI));

            // Potência do cosseno
            float powTerm = powf(std::max(HdotN, 0.f), exponent);

            // Denominador: (H·wi) * max(N·wi, N·wo)
            float denom = HdotWi * std::max(NdotL, NdotV);
            if (denom < 1e-8f) denom = 1e-8f;

            // Fresnel Schlick em (wi · H)
            float f_schlick = powf(1.f - HdotWi, 5.f);
            RGB F(Ks_brdf.R + (1.f - Ks_brdf.R) * f_schlick,
                  Ks_brdf.G + (1.f - Ks_brdf.G) * f_schlick,
                  Ks_brdf.B + (1.f - Ks_brdf.B) * f_schlick);

            RGB spec = F * (norm * powTerm / denom);
            color += spec;
        }

        // ---- Componente difusa (conservação de energia) ------------------
        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            // Rs: reflectância especular escalar (luminância de Ks_brdf)
            float Rs = Ks_brdf.Y();

            // Termos de Fresnel para wi e wo (ângulos com N)
            float t1 = powf(1.f - NdotL * 0.5f, 5.f);
            float t2 = powf(1.f - NdotV * 0.5f, 5.f);

            // f_d = 28*Kd/(23π) * (1-Rs) * (1-(1-N·wi/2)^5) * (1-(1-N·wo/2)^5)
            float diffFactor = (28.f / (23.f * float(M_PI)))
                               * (1.f - Rs)
                               * (1.f - t1)
                               * (1.f - t2);

            color += Kd * diffFactor;
        }

        return color;
    }
};

#endif /* AshikhminShirley_hpp */