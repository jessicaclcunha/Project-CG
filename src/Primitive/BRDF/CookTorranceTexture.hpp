#ifndef CookTorranceTexture_hpp
#define CookTorranceTexture_hpp

#include "CookTorrance.hpp"
#include "ImagePPM.hpp"
#include <cmath>

class CookTorranceTexture : public CookTorrance {
private:
    ImagePPM texture;
    float tex_W, tex_H;
public:
    CookTorranceTexture(std::string filename) {
        texture.Load(filename);
        textured = true;
        tex_W = float(texture.W);
        tex_H = float(texture.H);
    }

    // Kd vem da imagem; especular igual ao CookTorrance normal
    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        // Samplear textura (albedo)
        int tx = (int)floor(curTexCoord.u * tex_W);
        int ty = (int)floor(curTexCoord.v * tex_H);
        RGB Kd_tex = Kd * texture.get(tx, ty);

        // PBR: F0 derivado de metallic + albedo textured
        RGB F0(0.04f * (1.f - metallic) + Kd_tex.R * metallic,
               0.04f * (1.f - metallic) + Kd_tex.G * metallic,
               0.04f * (1.f - metallic) + Kd_tex.B * metallic);

        float NdotV = std::max(0.f, N.dot(wo));

        // Difuso: acoplado ao Fresnel
        if (type & DIFFUSE_REF) {
            float f_diff = powf(1.f - NdotV, 5.f);
            RGB F_diff(F0.R + (1.f - F0.R) * f_diff,
                       F0.G + (1.f - F0.G) * f_diff,
                       F0.B + (1.f - F0.B) * f_diff);
            RGB kD((1.f - F_diff.R) * (1.f - metallic),
                   (1.f - F_diff.G) * (1.f - metallic),
                   (1.f - F_diff.B) * (1.f - metallic));
            color += Kd_tex * kD * (1.f / float(M_PI));
        }

        if (!Ks_brdf.isZero() && (type & GLOSSY_REF)) {
            float NdotL = std::max(0.f, N.dot(wi));

            if (NdotL > 0.f && NdotV > 0.f) {
                Vector H = wi + wo;
                float hlen = H.norm();
                if (hlen > 1e-6f) {
                    H = H / hlen;

                    float NdotH = std::max(0.f, N.dot(H));
                    float VdotH = std::max(0.f, wo.dot(H));

                    float alpha  = roughness * roughness;
                    float alpha2 = alpha * alpha;
                    float denom  = NdotH * NdotH * (alpha2 - 1.f) + 1.f;
                    float D = alpha2 / (float(M_PI) * denom * denom);

                    float f_schlick = powf(1.f - VdotH, 5.f);
                    RGB F(F0.R + (1.f - F0.R) * f_schlick,
                          F0.G + (1.f - F0.G) * f_schlick,
                          F0.B + (1.f - F0.B) * f_schlick);

                    // G: k para iluminação direta
                    float k   = (roughness + 1.f) * (roughness + 1.f) / 8.f;
                    float G1L = NdotL / (NdotL * (1.f - k) + k);
                    float G1V = NdotV / (NdotV * (1.f - k) + k);
                    float G   = G1L * G1V;

                    RGB spec = F * (D * G / (4.f * NdotL * NdotV));
                    color += spec;
                }
            }
        }

        return color;
    }
};

#endif /* CookTorranceTexture_hpp */
