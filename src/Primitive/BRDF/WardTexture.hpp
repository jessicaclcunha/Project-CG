#ifndef WardTexture_hpp
#define WardTexture_hpp

#include "Ward.hpp"
#include "ImagePPM.hpp"
#include <cmath>
#include <algorithm>

class WardTexture : public Ward {
private:
    ImagePPM texture;
    float tex_W, tex_H;

public:
    explicit WardTexture(const std::string& filename) {
        texture.Load(filename);
        textured = true;
        tex_W = static_cast<float>(texture.W);
        tex_H = static_cast<float>(texture.H);
    }

    RGB GetKd(const Vec2& uv) {
        float u = std::max(0.f, std::min(uv.u, 0.9999f));
        float v = std::max(0.f, std::min(1.f - uv.v, 0.9999f));
        int x = std::max(0, std::min((int)std::floor(u * tex_W), (int)tex_W - 1));
        int y = std::max(0, std::min((int)std::floor(v * tex_H), (int)tex_H - 1));
        return Kd * texture.get(x, y);
    }

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        if (type & DIFFUSE_REF) {
            RGB Kd_tex = GetKd(curTexCoord);
            color += Kd_tex * (1.f / float(M_PI));
        }

        if (!Ks_brdf.isZero() && (type & GLOSSY_REF)) {
            float NdotL = N.dot(wi);
            float NdotV = N.dot(wo);
            if (NdotL <= 0.f || NdotV <= 0.f) return color;

            Vector H = wi + wo;
            float hlen = H.norm();
            if (hlen < 1e-6f) return color;
            H = H / hlen;

            float NdotH = std::max(N.dot(H), 1e-4f);
            float NdotH2 = NdotH * NdotH;

            Vector T, B;
            if (hasTangent) {
                float ndt = N.dot(tangent);
                T = tangent - N * ndt;
                float tlen = T.norm();
                if (tlen > 1e-6f) { T = T / tlen; B = N.cross(T); }
                else { N.CoordinateSystem(&T, &B); }
            } else {
                N.CoordinateSystem(&T, &B);
            }

            float HdotT = H.dot(T), HdotB = H.dot(B);
            float ax2 = alphaX * alphaX, ay2 = alphaY * alphaY;
            float exponent = (HdotT*HdotT/ax2 + HdotB*HdotB/ay2) / NdotH2;
            float spec = expf(-exponent);
            float denom = 4.f * float(M_PI) * alphaX * alphaY * sqrtf(NdotL * NdotV);
            if (denom >= 1e-8f)
                color += Ks_brdf * std::min(spec / denom, 50.f);
        }

        return color;
    }
};

#endif /* WardTexture_hpp */