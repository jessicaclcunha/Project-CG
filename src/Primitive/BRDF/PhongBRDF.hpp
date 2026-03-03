//
//  PhongBRDF.hpp
//  VI-RT
//
//  Phong BRDF: diffuse (Lambertian) + glossy specular lobe
//
//  f(wi, wo) = Kd/pi  +  Ks * (ns+2)/(2*pi) * max(0, R·wo)^ns
//
//  where R = reflect(-wi, N) is the perfect-mirror direction for wi.
//  The (ns+2)/(2*pi) normalization factor keeps the lobe energy-conserving.
//
//  Convention (same as the rest of the renderer):
//    wi  = direction FROM the surface TO the light  (Ldir)
//    wo  = direction FROM the surface TO the camera (isect.wo)
//    N   = shading normal (isect.sn)
//

#ifndef PhongBRDF_hpp
#define PhongBRDF_hpp

#include "BRDF.hpp"
#include "vector.hpp"
#include "RGB.hpp"
#include <cmath>
#include <algorithm>

class PhongBRDF : public BRDF {
public:
    float ns;   // shininess exponent  (e.g. 10 = rough, 100 = shiny, 1000 = mirror-like)

    PhongBRDF() : ns(10.f) { textured = false; }
    explicit PhongBRDF(float shininess) : ns(shininess) { textured = false; }

    // -----------------------------------------------------------------------
    // f(wi, wo) – evaluate the BRDF for a pair of directions
    //   wi : incident direction (pointing AWAY from surface, toward light)
    //   wo : outgoing direction (pointing AWAY from surface, toward camera)
    //   N  is NOT passed here; the caller must pass wi/wo already in the
    //      frame where N is available, OR embed N usage inside directLighting.
    //
    //   For the specular lobe we need N.  We receive it implicitly: the
    //   reflect() helper needs N, so we expose a second variant that takes N.
    // -----------------------------------------------------------------------
    virtual RGB f(Vector wi, Vector wo, const BRDF_TYPES types = BRDF_ALL) override {
        // Without N we cannot compute the Phong lobe – return diffuse only.
        RGB color(0.f, 0.f, 0.f);
        if ((types & DIFFUSE_REF) && !Kd.isZero()) {
            color += Kd * (1.f / M_PI);
        }
        return color;
    }

    // -----------------------------------------------------------------------
    // Full evaluation including the specular lobe – requires the surface normal.
    // This is the function called by directLighting.
    // -----------------------------------------------------------------------
    RGB f(Vector wi, Vector wo, Vector N){
        RGB color(0.f, 0.f, 0.f);

        // --- Diffuse (Lambertian) term ---
        if (!Kd.isZero()) {
            color += Kd * (1.f / M_PI);
        }

        // --- Specular (Phong) term ---
        if (!Ks.isZero()) {
            // Perfect-mirror reflection of wi around N
            // R = 2*(N·wi)*N - wi
            float NdotWi = std::max(0.f, N.dot(wi));
            Vector R = N * (2.f * NdotWi) - wi;
            R.normalize();

            float RdotWo = std::max(0.f, R.dot(wo));
            if (RdotWo > 0.f) {
                float normFactor = (ns + 2.f) / (2.f * (float)M_PI);
                color += Ks * (normFactor * std::pow(RdotWo, ns));
            }
        }

        return color;
    }

    // pdf for importance sampling (used in path tracing – left simple for now)
    virtual float pdf(Vector wi, Vector wo, const BRDF_TYPES types = BRDF_ALL) override {
        // Cosine-weighted hemisphere over diffuse; uniform over specular lobe.
        // For now return cosine-weighted pdf (same as Lambertian).
        return 1.f / (float)M_PI;
    }
};

#endif /* PhongBRDF_hpp */