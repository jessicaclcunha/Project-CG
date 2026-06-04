#ifndef AshikhminShirleyFSG_hpp
#define AshikhminShirleyFSG_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Ashikhmin-Shirley com Fresnel SPHERICAL GAUSSIAN (Karis 2013) — alternativa
// correcta. Substitui o (1−HdotWi)^5 de Schlick pela aproximação SG, mantendo
// F0 = Ks por canal (preserva a cor). Tudo o resto fica igual ao base.
//
//   Schlick:  F = Ks + (1−Ks)·(1−HdotWi)^5
//   SG Karis: F = Ks + (1−Ks)·2^((−5.55473·HdotWi − 6.98316)·HdotWi)
//
// Resultado quase idêntico ao base — valida Schlick como aproximação robusta
// (a forma exacta da curva de Fresnel tem pouco impacto visual).
class AshikhminShirleyFSG : public BRDF {
public:
    float nu, nv;
    Vector tangent;
    bool hasTangent;

    AshikhminShirleyFSG()
        : nu(100.f), nv(100.f),
          tangent(Vector(0.f, 0.f, 0.f)), hasTangent(false) {}

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        float NdotL = N.dot(wi);
        float NdotV = N.dot(wo);
        if (NdotL <= 0.f || NdotV <= 0.f) return color;

        Vector T, B;
        if (hasTangent) {
            float d = N.dot(tangent);
            T = tangent - N * d;
            float tlen = T.norm();
            if (tlen > 1e-6f) { T = T / tlen; B = N.cross(T); }
            else N.CoordinateSystem(&T, &B);
        } else {
            N.CoordinateSystem(&T, &B);
        }

        Vector H = wi + wo;
        float hlen = H.norm();
        if (hlen < 1e-6f) return color;
        H = H / hlen;

        float HdotN  = std::max(N.dot(H),  1e-4f);
        float HdotWi = std::max(wi.dot(H), 1e-4f);

        if (!Ks_brdf.isZero() && (type & GLOSSY_REF)) {
            float HdotT = H.dot(T);
            float HdotB = H.dot(B);
            float HperpSq = HdotT * HdotT + HdotB * HdotB;
            float exponent;
            if (HperpSq < 1e-6f) {
                exponent = (nu + nv) * 0.5f;
            } else {
                float cos2phi = (HdotT * HdotT) / HperpSq;
                exponent = nu * cos2phi + nv * (1.f - cos2phi);
            }

            float norm    = sqrtf((nu + 1.f) * (nv + 1.f)) / (8.f * float(M_PI));
            float powTerm = powf(std::max(HdotN, 0.f), exponent);
            float denom   = HdotWi * std::max(NdotL, NdotV);
            if (denom < 1e-8f) denom = 1e-8f;

            // Spherical Gaussian Schlick (Karis 2013): 2^x = exp(x·ln2)
            float f_sg = expf((-5.55473f * HdotWi - 6.98316f) * HdotWi * 0.6931472f);
            RGB F(Ks_brdf.R + (1.f - Ks_brdf.R) * f_sg,
                  Ks_brdf.G + (1.f - Ks_brdf.G) * f_sg,
                  Ks_brdf.B + (1.f - Ks_brdf.B) * f_sg);

            // Cap para suprimir fireflies no terminador (igual ao base)
            float spec_val = std::min(norm * powTerm / denom, 50.f);
            color += F * spec_val;
        }

        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            float Rs = Ks_brdf.Y();
            float t1 = powf(1.f - NdotL * 0.5f, 5.f);
            float t2 = powf(1.f - NdotV * 0.5f, 5.f);
            float diffFactor = (28.f / (23.f * float(M_PI)))
                               * (1.f - Rs) * (1.f - t1) * (1.f - t2);
            color += Kd * diffFactor;
        }

        return color;
    }
};

#endif /* AshikhminShirleyFSG_hpp */
