#ifndef AshikhminShirleyFExact_hpp
#define AshikhminShirleyFExact_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Ashikhmin-Shirley com Fresnel DIELÉCTRICO EXACTO (alternativa correcta).
// Substitui a aproximação de Schlick pelas equações exactas de Fresnel para
// dieléctricos, usando this->eta (IOR). Tudo o resto (lóbulo, normalização,
// difuso) fica igual ao base.
//
//   cosI = HdotWi
//   sinT² = (1 − cosI²)/η²        [TIR se sinT² ≥ 1 → F = 1]
//   Rs = (cosI − η·cosT)/(cosI + η·cosT)
//   Rp = (η·cosI − cosT)/(η·cosI + cosT)
//   F  = ½(Rs² + Rp²)            [escalar]
//
// Nota: o Fresnel dieléctrico é escalar → correcto para os DIELÉCTRICOS
// (plásticos). Nos metais perde a cor do reflexo (dourado → esbranquiçado),
// pois metal precisa de Fresnel complexo/colorido — essa limitação é, em si,
// o ponto pedagógico a observar.
class AshikhminShirleyFExact : public BRDF {
public:
    float nu, nv;
    Vector tangent;
    bool hasTangent;

    AshikhminShirleyFExact()
        : nu(100.f), nv(100.f),
          tangent(Vector(0.f, 0.f, 0.f)), hasTangent(false) { eta = 1.5f; }

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

            // Fresnel dieléctrico exacto (escalar) — substitui Schlick
            float cosI  = HdotWi;
            float sinT2 = (1.f - cosI * cosI) / std::max(eta * eta, 1e-6f);
            float f_val;
            if (sinT2 >= 1.f) {
                f_val = 1.f;  // reflexão total interna
            } else {
                float cosT = sqrtf(1.f - sinT2);
                float Rs = (cosI - eta * cosT) / std::max(cosI + eta * cosT, 1e-6f);
                float Rp = (eta * cosI - cosT) / std::max(eta * cosI + cosT, 1e-6f);
                f_val = std::min(1.f, std::max(0.f, 0.5f * (Rs * Rs + Rp * Rp)));
            }
            RGB F(f_val, f_val, f_val);

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

#endif /* AshikhminShirleyFExact_hpp */
