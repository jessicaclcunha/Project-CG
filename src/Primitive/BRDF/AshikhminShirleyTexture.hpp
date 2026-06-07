#ifndef AshikhminShirleyTexture_hpp
#define AshikhminShirleyTexture_hpp

#include "AshikhminShirley.hpp"
#include "ImagePPM.hpp"
#include <cmath>
#include <algorithm>

class AshikhminShirleyTexture : public AshikhminShirley {
private:
    ImagePPM texture;
    float tex_W, tex_H;

public:
    explicit AshikhminShirleyTexture(const std::string& filename) {
        texture.Load(filename);
        textured = true;
        tex_W = static_cast<float>(texture.W);
        tex_H = static_cast<float>(texture.H);
    }

    RGB GetKd(const Vec2& uv) {
        float u = std::max(0.f, std::min(uv.u, 0.9999f));
        // inversão de V: PPM tem y=0 no topo, UV tem v=0 no fundo
        float v = std::max(0.f, std::min(1.f - uv.v, 0.9999f));
        int x = std::max(0, std::min((int)std::floor(u * tex_W), (int)tex_W - 1));
        int y = std::max(0, std::min((int)std::floor(v * tex_H), (int)tex_H - 1));
        return Kd * texture.get(x, y);
    }

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        float NdotL = N.dot(wi);
        float NdotV = N.dot(wo);
        if (NdotL <= 0.f || NdotV <= 0.f) return color;

        // Frame de anisotropia (igual ao base)
        Vector T, B;
        if (hasTangent) {
            float d = N.dot(tangent);
            T = tangent - N * d;
            float tlen = T.norm();
            if (tlen > 1e-6f) { T = T / tlen; B = N.cross(T); }
            else { N.CoordinateSystem(&T, &B); }
        } else {
            N.CoordinateSystem(&T, &B);
        }

        // --- Especular Ashikhmin-Shirley (idêntico ao base) ---
        if (!Ks_brdf.isZero() && (type & GLOSSY_REF)) {
            Vector H = wi + wo;
            float hlen = H.norm();
            if (hlen > 1e-6f) {
                H = H / hlen;
                float HdotN  = std::max(N.dot(H),  1e-4f);
                float HdotWi = std::max(wi.dot(H), 1e-4f);
                float HdotT  = H.dot(T);
                float HdotB  = H.dot(B);

                float HperpSq = HdotT * HdotT + HdotB * HdotB;
                float exponent;
                if (HperpSq < 1e-6f) {
                    exponent = (nu + nv) * 0.5f;
                } else {
                    float cos2phi = (HdotT * HdotT) / HperpSq;
                    exponent = nu * cos2phi + nv * (1.f - cos2phi);
                }
                exponent = std::min(exponent, 8000.f);

                float norm    = sqrtf((nu + 2.f) * (nv + 2.f)) / (8.f * float(M_PI));
                float powTerm = powf(HdotN, exponent);
                float denom   = HdotWi * std::max(NdotL, NdotV);
                if (denom >= 1e-8f) {
                    float fc = powf(1.f - HdotWi, 5.f);
                    RGB F(Ks_brdf.R + (1.f - Ks_brdf.R) * fc,
                          Ks_brdf.G + (1.f - Ks_brdf.G) * fc,
                          Ks_brdf.B + (1.f - Ks_brdf.B) * fc);
                    float spec_val = std::min(norm * powTerm / denom, 50.f);
                    color += F * spec_val;
                }
            }
        }

        // --- Difuso Ashikhmin-Shirley com textura ---
        // f_d = (28/23π) * Kd_tex * (1-Rs) * [1-(1-N·L/2)^5] * [1-(1-N·V/2)^5]
        if (type & DIFFUSE_REF) {
            RGB Kd_tex = GetKd(curTexCoord);
            float Rs = Ks_brdf.Y();
            float f1 = 1.f - powf(1.f - NdotL * 0.5f, 5.f);
            float f2 = 1.f - powf(1.f - NdotV * 0.5f, 5.f);
            color += Kd_tex * ((28.f / (23.f * float(M_PI))) * (1.f - Rs) * f1 * f2);
        }

        return color;
    }
};

#endif /* AshikhminShirleyTexture_hpp */