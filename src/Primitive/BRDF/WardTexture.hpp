#ifndef WardTexture_hpp
#define WardTexture_hpp

#include "Ward.hpp"
#include "Texture.hpp"
#include <cmath>
#include <algorithm>

class WardTexture : public Ward {
private:
    Texture* tex;

public:
    explicit WardTexture(const std::string& filename) {
        tex = Texture::get(filename);
        textured = true;
    }

    RGB GetKd(const Vec2& uv) {
        return Kd * tex->sampleUV(uv.u, uv.v);
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