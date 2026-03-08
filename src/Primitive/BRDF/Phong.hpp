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
        

        // o if só serve para otimização, porque vão sempre fazer os dois
        if (!Kd.isZero() && (type & DIFFUSE_REF)) {
            color += Kd * (1.f / M_PI);
        }

        if (!Ks.isZero() && (type & GLOSSY_REF)) {
            float cosWiN = std::max(0.f, N.dot(wi));
            Vector R = 2.f * cosWiN * N - wi;
            float cosRWo = std::max(0.f, R.dot(wo));
            float spec = ((ns + 2.f) / (2.f * M_PI)) * powf(cosRWo, ns);
            color += Ks * spec;
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
