#ifndef ColorConvert_hpp
#define ColorConvert_hpp

#include <cmath>

// sRGB -> linear RGB (IEC 61966-2-1)
static inline float srgb_to_linear(float c) {
    if (c <= 0.04045f) return c / 12.92f;
    return powf((c + 0.055f) / 1.055f, 2.4f);
}

// linear RGB (D65) -> XYZ
static inline void linrgb_to_xyz(float r, float g, float b,
                                 float& X, float& Y, float& Z) {
    X = 0.4124564f * r + 0.3575761f * g + 0.1804375f * b;
    Y = 0.2126729f * r + 0.7151522f * g + 0.0721750f * b;
    Z = 0.0193339f * r + 0.1191920f * g + 0.9503041f * b;
}

// XYZ -> Lab (D65 white point)
static inline void xyz_to_lab(float X, float Y, float Z,
                              float& L, float& a, float& b) {
    const float Xn = 0.95047f, Yn = 1.00000f, Zn = 1.08883f;
    const float delta = 6.f / 29.f;
    const float delta3 = delta * delta * delta;
    auto f = [&](float t) {
        if (t > delta3) return powf(t, 1.f / 3.f);
        return t / (3.f * delta * delta) + 4.f / 29.f;
    };
    float fx = f(X / Xn);
    float fy = f(Y / Yn);
    float fz = f(Z / Zn);
    L = 116.f * fy - 16.f;
    a = 500.f * (fx - fy);
    b = 200.f * (fy - fz);
}

// Convenience: sRGB in [0,1] -> Lab
static inline void srgb_to_lab(float R, float G, float B,
                               float& L, float& a, float& b) {
    float rL = srgb_to_linear(R);
    float gL = srgb_to_linear(G);
    float bL = srgb_to_linear(B);
    float X, Y, Z;
    linrgb_to_xyz(rL, gL, bL, X, Y, Z);
    xyz_to_lab(X, Y, Z, L, a, b);
}

// CIE76 Delta-E between two Lab triplets
static inline float deltaE76(float L1, float a1, float b1,
                             float L2, float a2, float b2) {
    float dL = L1 - L2;
    float da = a1 - a2;
    float db = b1 - b2;
    return sqrtf(dL * dL + da * da + db * db);
}

#endif /* ColorConvert_hpp */
