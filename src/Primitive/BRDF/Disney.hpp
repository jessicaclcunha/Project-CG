#ifndef Disney_hpp
#define Disney_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

// Disney "Principled" BRDF (Brent Burley, 2012 — "Physically-Based Shading at
// Disney"). Um único modelo com parâmetros intuitivos [0,1] que imita quase
// qualquer material. Implementação fiel ao Disney BRDF explorer (disney.brdf).
//
// f() devolve o BRDF puro f_r; o shader multiplica por NdotL·luz·1/dist²
// (ver directLighting.cpp). Especular/clearcoat/sheen vão todos aqui — por isso
// o material usa Ks=Kt=0 (sem raios de espelho do shader).
//
// Nota: o pipeline já é linear + gamma global (ImagePPM), por isso baseColor é
// tratado como linear (sem mon2lin / conversão sRGB por-material).
//
// Normalização (crítica): smithG_GGX devolve 1/(NdotV+√(...)) = G1/(2·NdotV),
// logo Gs·Fs·Ds já inclui o 1/(4·NdotL·NdotV) do microfacet — não acrescentar.

namespace disney_detail {
    inline float sqr(float x) { return x * x; }
    inline float clamp01(float x) { return std::max(0.f, std::min(1.f, x)); }
    inline float lerp(float a, float b, float t) { return a + t * (b - a); }
    inline RGB   lerp(RGB a, RGB b, float t) {
        return RGB(a.R + t * (b.R - a.R),
                   a.G + t * (b.G - a.G),
                   a.B + t * (b.B - a.B));
    }
    inline float SchlickFresnel(float u) {
        float m = clamp01(1.f - u);
        float m2 = m * m;
        return m2 * m2 * m;   // (1-u)^5
    }
    // GTR1 — NDF do clearcoat (Berry)
    inline float GTR1(float NdotH, float a) {
        if (a >= 1.f) return 1.f / float(M_PI);
        float a2 = a * a;
        float t = 1.f + (a2 - 1.f) * NdotH * NdotH;
        return (a2 - 1.f) / (float(M_PI) * logf(a2) * t);
    }
    // GTR2 anisotrópico — NDF especular (GGX)
    inline float GTR2_aniso(float NdotH, float HdotX, float HdotY, float ax, float ay) {
        float d = sqr(HdotX / ax) + sqr(HdotY / ay) + NdotH * NdotH;
        return 1.f / (float(M_PI) * ax * ay * d * d);
    }
    // Smith G (isotrópico, clearcoat). Devolve G1/(2·NdotV).
    inline float smithG_GGX(float NdotV, float alphaG) {
        float a = alphaG * alphaG;
        float b = NdotV * NdotV;
        return 1.f / (NdotV + sqrtf(a + b - a * b));
    }
    // Smith G anisotrópico. Devolve G1/(2·NdotV).
    inline float smithG_GGX_aniso(float NdotV, float VdotX, float VdotY, float ax, float ay) {
        return 1.f / (NdotV + sqrtf(sqr(VdotX * ax) + sqr(VdotY * ay) + sqr(NdotV)));
    }
}

class Disney : public BRDF {
public:
    RGB   baseColor;
    float metallic;
    float subsurface;
    float specular;        // controla o F0 dieléctrico (0.08·specular)
    float roughness;
    float specularTint;
    float anisotropic;
    float sheen;
    float sheenTint;
    float clearcoat;
    float clearcoatGloss;
    Vector tangent;        // direção do grão (anisotropia)
    bool   hasTangent;

    Disney()
        : baseColor(0.8f, 0.8f, 0.8f),
          metallic(0.f), subsurface(0.f), specular(0.5f), roughness(0.5f),
          specularTint(0.f), anisotropic(0.f), sheen(0.f), sheenTint(0.5f),
          clearcoat(0.f), clearcoatGloss(1.f),
          tangent(Vector(0.f, 0.f, 0.f)), hasTangent(false) {}

    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        using namespace disney_detail;

        float NdotL = N.dot(wi);
        float NdotV = N.dot(wo);
        if (NdotL <= 0.f || NdotV <= 0.f) return RGB(0.f, 0.f, 0.f);

        Vector H = wi + wo;
        float hlen = H.norm();
        if (hlen < 1e-6f) return RGB(0.f, 0.f, 0.f);
        H = H / hlen;

        float NdotH = N.dot(H);
        float LdotH = wi.dot(H);

        // Frame de anisotropia (X = tangente, Y = bitangente)
        Vector X, Y;
        if (hasTangent) {
            float d = N.dot(tangent);
            X = tangent - N * d;
            float xlen = X.norm();
            if (xlen > 1e-6f) { X = X / xlen; Y = N.cross(X); }
            else N.CoordinateSystem(&X, &Y);
        } else {
            N.CoordinateSystem(&X, &Y);
        }

        // Cores derivadas
        float lum = baseColor.R * 0.3f + baseColor.G * 0.6f + baseColor.B * 0.1f;
        RGB Ctint = (lum > 0.f) ? (baseColor / lum) : RGB(1.f, 1.f, 1.f);
        RGB Cspec0 = lerp(lerp(RGB(1.f,1.f,1.f), Ctint, specularTint) * (specular * 0.08f),
                          baseColor, metallic);
        RGB Csheen = lerp(RGB(1.f, 1.f, 1.f), Ctint, sheenTint);

        // --- Difuso (Burley) + subsurface ---
        float FL = SchlickFresnel(NdotL), FV = SchlickFresnel(NdotV);
        float Fd90 = 0.5f + 2.f * LdotH * LdotH * roughness;
        float Fd = lerp(1.f, Fd90, FL) * lerp(1.f, Fd90, FV);

        // Subsurface (aproximação Hanrahan-Krueger)
        float Fss90 = LdotH * LdotH * roughness;
        float Fss = lerp(1.f, Fss90, FL) * lerp(1.f, Fss90, FV);
        float ss = 1.25f * (Fss * (1.f / (NdotL + NdotV) - 0.5f) + 0.5f);

        // --- Sheen ---
        float FH = SchlickFresnel(LdotH);
        RGB Fsheen = Csheen * (FH * sheen);

        RGB diffuse = (baseColor * (float(1.0 / M_PI) * lerp(Fd, ss, subsurface)) + Fsheen)
                      * (1.f - metallic);

        // --- Especular GGX anisotrópico ---
        float aspect = sqrtf(1.f - anisotropic * 0.9f);
        float ax = std::max(0.001f, sqr(roughness) / aspect);
        float ay = std::max(0.001f, sqr(roughness) * aspect);
        float Ds = GTR2_aniso(NdotH, H.dot(X), H.dot(Y), ax, ay);
        RGB Fs = lerp(Cspec0, RGB(1.f, 1.f, 1.f), FH);
        float Gs = smithG_GGX_aniso(NdotL, wi.dot(X), wi.dot(Y), ax, ay)
                 * smithG_GGX_aniso(NdotV, wo.dot(X), wo.dot(Y), ax, ay);
        RGB specularTerm = Fs * (Gs * Ds);

        // --- Clearcoat (IOR 1.5 → F0 0.04) ---
        float Dr = GTR1(NdotH, lerp(0.1f, 0.001f, clearcoatGloss));
        float Fr = lerp(0.04f, 1.f, FH);
        float Gr = smithG_GGX(NdotL, 0.25f) * smithG_GGX(NdotV, 0.25f);
        float clearcoatTerm = 0.25f * clearcoat * Gr * Fr * Dr;

        RGB result = diffuse + specularTerm + RGB(clearcoatTerm, clearcoatTerm, clearcoatTerm);
        return result;
    }
};

#endif /* Disney_hpp */
