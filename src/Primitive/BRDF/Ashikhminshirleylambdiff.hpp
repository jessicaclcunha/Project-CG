#ifndef AshikhminShirleyLambDiff_hpp
#define AshikhminShirleyLambDiff_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Ashikhmin-Shirley com difuso LAMBERTIANO simples: Kd/pi.
// Diferença face ao original: os termos de conservação de energia
//   (28/(23π)) * (1-Rs) * (1-(1-N·wi/2)^5) * (1-(1-N·wo/2)^5)
// são substituídos por Kd/π — sem acoplamento ao especular.
// Efeito visual: difuso mais brilhante que o correto (Rs não subtrai
// energia ao difuso), especialmente notável em materiais com Ks alto.
// Viola conservação de energia: difuso + especular podem somar > 1.
class AshikhminShirleyLambDiff : public BRDF {
public:
    float nu, nv;
    Vector tangent;
    bool hasTangent;

    AshikhminShirleyLambDiff()
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

        // especular: idêntico ao original
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

            float f_schlick = powf(1.f - HdotWi, 5.f);
            RGB F(Ks_brdf.R + (1.f - Ks_brdf.R) * f_schlick,
                  Ks_brdf.G + (1.f - Ks_brdf.G) * f_schlick,
                  Ks_brdf.B + (1.f - Ks_brdf.B) * f_schlick);

            // Cap para suprimir fireflies no terminador (igual ao base)
            float spec_val = std::min(norm * powTerm / denom, 50.f);
            color += F * spec_val;
        }

        // difuso Lambertiano simples — sem acoplamento ao especular
        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            color += Kd * (1.f / float(M_PI));
        }

        return color;
    }
};

#endif /* AshikhminShirleyLambDiff_hpp */