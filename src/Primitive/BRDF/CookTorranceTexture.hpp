#ifndef CookTorranceTexture_hpp
#define CookTorranceTexture_hpp

#include "CookTorrance.hpp"
#include "ImagePPM.hpp"
#include <cmath>
#include <algorithm>

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

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        float u = std::max(0.f, std::min(curTexCoord.u, 0.9999f));
        float v = std::max(0.f, std::min(1.f - curTexCoord.v, 0.9999f));
        int tx = (int)std::floor(u * tex_W);
        int ty = (int)std::floor(v * tex_H);
        tx = std::max(0, std::min(tx, (int)tex_W - 1));
        ty = std::max(0, std::min(ty, (int)tex_H - 1));
        RGB Kd_tex = Kd * texture.get(tx, ty);

        RGB F0(0.04f * (1.f - metallic) + Kd_tex.R * metallic,
               0.04f * (1.f - metallic) + Kd_tex.G * metallic,
               0.04f * (1.f - metallic) + Kd_tex.B * metallic);

        float NdotL = std::max(0.f, N.dot(wi));
        float NdotV = std::max(0.f, N.dot(wo));

        RGB F = F0;
        float NdotH = 0.f, VdotH = 0.f, alpha = roughness * roughness;
        bool valid_H = false;

        if (NdotL > 0.f && NdotV > 0.f) {
            Vector H = wi + wo;
            float hlen = H.norm();
            if (hlen > 1e-6f) {
                H = H / hlen;
                NdotH = std::max(0.f, N.dot(H));
                VdotH = std::max(0.f, wo.dot(H));
                float f_schlick = powf(1.f - VdotH, 5.f);
                F = RGB(F0.R + (1.f - F0.R) * f_schlick,
                        F0.G + (1.f - F0.G) * f_schlick,
                        F0.B + (1.f - F0.B) * f_schlick);
                valid_H = true;
            }
        }

        if (type & DIFFUSE_REF) {
            RGB kD((1.f - F.R) * (1.f - metallic),
                   (1.f - F.G) * (1.f - metallic),
                   (1.f - F.B) * (1.f - metallic));
            color += Kd_tex * kD * (1.f / float(M_PI));
        }

        if (valid_H && (type & GLOSSY_REF)) {
            float alpha2 = alpha * alpha;
            float denom  = NdotH * NdotH * (alpha2 - 1.f) + 1.f;
            float D = alpha2 / (float(M_PI) * denom * denom);
            float k   = (roughness + 1.f) * (roughness + 1.f) / 8.f;
            float G1L = NdotL / (NdotL * (1.f - k) + k);
            float G1V = NdotV / (NdotV * (1.f - k) + k);
            float G   = G1L * G1V;
            color += F * (D * G / (4.f * NdotL * NdotV));
        }

        return color;
    }
};

#endif /* CookTorranceTexture_hpp */