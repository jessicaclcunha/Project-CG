#ifndef Phong_hpp
#define Phong_hpp

#include "BRDF.hpp"
#include <cmath>
#include <algorithm>

class Phong : public BRDF {
public:
    float ns;  // expoente de brilho

    Phong() : ns(1.f) {}

    // Phong normalizado:
    //   difuso:  Kd / pi
    //   especular: Ks * (ns+2)/(2*pi) * max(0, R·wo)^ns
    //            - odne R = reflect(wi, N) = 2*(N·wi)*N - wi
    RGB f(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        RGB color(0.f, 0.f, 0.f);

        // Conservação de energia: Kd + Ks_brdf <= 1 por canal.
        // Escala proporcional pelo canal com maior soma, preservando ratio e matiz.
        float maxSum = std::max({Kd.R + Ks_brdf.R,
                                 Kd.G + Ks_brdf.G,
                                 Kd.B + Ks_brdf.B});
        float norm = (maxSum > 1.f) ? 1.f / maxSum : 1.f;
        RGB eff_Kd = Kd      * norm;
        RGB eff_Ks = Ks_brdf * norm;

        if (!eff_Kd.isZero() && (type & DIFFUSE_REF)) {
            color += eff_Kd * (1.f / M_PI);
        }

        if (!eff_Ks.isZero() && (type & GLOSSY_REF)) {
            float cosWiN = std::max(0.f, N.dot(wi));
            Vector R = 2.f * cosWiN * N - wi;
            float cosRWo = std::max(0.f, R.dot(wo));
            float spec = ((ns + 2.f) / (2.f * M_PI)) * powf(cosRWo, ns);
            color += eff_Ks * spec;
        }

        return color;
    }

    /* Stubs para o futuro, caso necessário

    // Cosine-weighted hemisphere pdf (used for diffuse sampling)
    float pdf(Vector wi, Vector wo, Vector N, const BRDF_TYPES type = BRDF_ALL) override {
        float cosTheta = std::max(0.f, N.dot(wo));
        return cosTheta / M_PI;
    }

    // Sample_f: cosine-weighted hemisphere sampling around N
    // Returns the BRDF value and sets *wo to the sampled direction
    RGB Sample_f(Vector wi, Vector N, float *prob, Vector *wo, const BRDF_TYPES type = BRDF_ALL) override {
        // Placeholder — used by path tracer; implement importance sampling if needed
        return RGB();
    }
        */
};


#endif /* Phong_hpp */
