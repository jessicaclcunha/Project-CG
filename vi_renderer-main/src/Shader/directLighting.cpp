//
//  directLighting.cpp
//  VI-RT-3rdVersion
//
//  Created by Luis Paulo Santos on 05/03/2025.
//

#include "directLighting.hpp"
#include "AmbientLight.hpp"
#include "PointLight.hpp"
#include "AreaLight.hpp"

static RGB direct_AmbientLight (AmbientLight * l, BRDF  *  f);
static RGB direct_PointLight (PointLight  *  l, Scene *scene, Intersection isect, BRDF  *  f);
static RGB direct_AreaLight (AreaLight * l, Scene *scene, Intersection isect, BRDF* f, float *r);
static Light* powerWeightedLightSelection(Scene* scene, float rnd, float selected_pdf);

RGB directLighting (Scene *scene, Intersection isect, BRDF *f, std::mt19937& rng, std::uniform_real_distribution<float>U_dist, DIRECT_SAMPLE_MODE mode) {
    RGB color (0.,0.,0.);
    
#define XX 725
#define YY 540
    // Loop over scene's light sources
    for (Light* l : scene->lights) {

        /*if (mode==UNIFORM_ONE) {
            int l_ndx = U_dist(rng)*scene->numLights;
            if (isect.pix_x==XX && isect.pix_y==YY) {
                fprintf (stderr, "numLights=%d, l_ndx=%d, ", scene->numLights, l_ndx);
            }
            if (l_ndx >= scene->numLights) l_ndx=scene->numLights-1;
            if (isect.pix_x==XX && isect.pix_y==YY) {
                fprintf (stderr, "l_ndx_corrected=%d \n", l_ndx);
            }
            l = scene->lights[l_ndx];
        }
        */
        if (mode==UNIFORM_ONE) {
            float rnd = U_dist(rng);
            float light_pdf;
            Light* selected_light = powerWeightedLightSelection(scene, rnd, light_pdf);
            if (selected_light && light_pdf > 0.0f) {
                RGB contrib(0., 0., 0.);
                
                if (selected_light->type == AMBIENT_LIGHT) {
                    contrib = direct_AmbientLight((AmbientLight*)selected_light, f);
                } else if (selected_light->type == POINT_LIGHT) {
                    contrib = direct_PointLight((PointLight*)selected_light, scene, isect, f);
                } else if (selected_light->type == AREA_LIGHT) {
                    float r[2] = {U_dist(rng), U_dist(rng)};
                    contrib = direct_AreaLight((AreaLight*)selected_light, scene, isect, f, r);
                }
                
                // Importância da amostra: contribuição dividida pelo PDF
                color += contrib / light_pdf;
            }
        } 
        if (l->type == AMBIENT_LIGHT) {  // is it an ambient light ?
            color += direct_AmbientLight ((AmbientLight *)l, f);
            continue;
        }
        if (l->type == POINT_LIGHT) {  // is it a point light ?
            color += direct_PointLight ((PointLight *)l, scene, isect, f);
            continue;
        } // is POINT_LIGHT
        if (l->type == AREA_LIGHT) {  // is it a area light ?
            float r[2];
            RGB color_temp(0.,0.,0.);
            r[0] = U_dist(rng);
            r[1] = U_dist(rng);
            color_temp = direct_AreaLight ((AreaLight *)l, scene, isect, f, r);
            color += color_temp;
            if (isect.pix_x==XX && isect.pix_y==YY) {
                fprintf (stderr, "ARea light contributes with (%f,%f,%f) \n", color.R, color.G, color.B);
            }
        } // is AREA_LIGHT
    }  // loop over all light sources

    if (isect.pix_x==XX && isect.pix_y==YY) {
        fprintf (stderr, "Direct contributes with (%f,%f,%f) \n", color.R, color.G, color.B);
    }

    return color;
}



// Nova implementação - Power Weighted Light Sampling
static Light* powerWeightedLightSelection(Scene* scene, float rnd, float selected_pdf) {
    if (scene->lights.empty()) return nullptr;

    float total_power = 0.0f;
    std::vector<float> light_powers(scene->lights.size());

    for (size_t i = 0; i < scene->lights.size(); i++) {
        Light* l = scene->lights[i];
        float power = 0.0f;

        if (l->type == AREA_LIGHT) {
            AreaLight* al = (AreaLight*)l;
            // Para área lights, potência = intensidade * área
            power = (al->power.R + al->power.G + al->power.B) * al->gem->area();
        } else if (l->type == POINT_LIGHT) {
            PointLight* pl = (PointLight*)l;
            // Para point lights, tratamos como tendo "área" 1 (ou poderia ser baseado na intensidade)
            power = (pl->color.R + pl->color.G + pl->color.B);
        } else if (l->type == AMBIENT_LIGHT) {
            // Ambient lights não participam nesta seleção
            continue;
        }

        light_powers[i] = power;
        total_power += power;
    }

    // Calcular PDFs e construir CDF
    std::vector<float> pdfs(scene->lights.size(), 0.0f);
    std::vector<float> cdf(scene->lights.size(), 0.0f);
    float cdf_sum = 0.0f;

    for (size_t i = 0; i < scene->lights.size(); i++) {
        if (light_powers[i] > 0.0f) {
            pdfs[i] = light_powers[i] / total_power;
            cdf_sum += pdfs[i];
            cdf[i] = cdf_sum;
        }
    }

    // Normalizar CDF (devido a possíveis luzes não consideradas como ambient)
    if (cdf_sum > 0.0f) {
        for (size_t i = 0; i < scene->lights.size(); i++) {
            cdf[i] /= cdf_sum;
        }
    }

    // Selecionar luz baseada no CDF
    for (size_t i = 0; i < scene->lights.size(); i++) {
        if (rnd <= cdf[i] && light_powers[i] > 0.0f) {
            selected_pdf = pdfs[i];
            return scene->lights[i];
        }
    }

    // Fallback (não deveria acontecer)
    selected_pdf = 1.0f / scene->lights.size();
    return scene->lights[0];
}






static RGB direct_AmbientLight (AmbientLight * l, BRDF * f) {
    RGB color (0., 0., 0.);
    if (!f->Ka.isZero()) {
        RGB Ka = f->Ka;
        color += Ka * l->L();
    }
    return (color);
}

static RGB direct_PointLight (PointLight* l, Scene *scene, Intersection isect, BRDF * f) {
    RGB color (0., 0., 0.);
    RGB Kd;
    
    if (f->textured) {
        DiffuseTexture * df = (DiffuseTexture *)f;
        Kd = df->GetKd(isect.TexCoord);
    }
    else {
        Kd = f->Kd;
    }

    if (!Kd.isZero()) {
        Point Lpos;
        RGB L = l->Sample_L(NULL, &Lpos);
        Vector Ldir=isect.p.vec2point(Lpos);
        float Ldistance = Ldir.norm();
        Ldir.normalize();
        float cosL = Ldir.dot(isect.sn);
        if (cosL>0) {
            
            Ray shadow = Ray(isect.p, Ldir, SHADOW);
            shadow.pix_x = isect.pix_x;
            shadow.pix_y = isect.pix_y;
            
            shadow.adjustOrigin(isect.gn);
            
            if (scene->visibility(shadow, Ldistance-EPSILON)) {
                color += L * Kd * cosL;
                if (Ldistance>0.f) color /= (Ldistance*Ldistance);
            }
        }
    } // Kd is zero

    
    return (color);
}

static RGB direct_AreaLight (AreaLight* l, Scene *scene, Intersection isect, BRDF* f, float *r) {
    RGB color (0., 0., 0.);
    RGB Kd;
    float pdf, cosL, cosLN_l, Ldistance;
    RGB L;
    Point Lpos;
    
    if (f->textured) {
        DiffuseTexture * df = (DiffuseTexture *)f;
        Kd = df->GetKd(isect.TexCoord);
    }
    else {
        Kd = f->Kd;
    }

    pdf = 0.;
    if (!Kd.isZero()) {
        Point Lpos;
        
        L = l->Sample_L(r, &Lpos, pdf);
        // the pdf computed above is just 1/Area
        Vector Ldir=isect.p.vec2point(Lpos);
        Ldistance = Ldir.norm();
        Ldir.normalize();
        cosL = Ldir.dot(isect.sn);
        cosLN_l = -1.f * Ldir.dot(l->gem->normal);
        if (cosL>1.e-4 && cosLN_l>1.e-4) {
            
            Ray shadow = Ray(isect.p, Ldir, SHADOW);
            shadow.pix_x = isect.pix_x;
            shadow.pix_y = isect.pix_y;
            
            shadow.adjustOrigin(isect.gn);
            
            if (scene->visibility(shadow, Ldistance-EPSILON)) {
                color = L * Kd * cosL;
                if (pdf >0.) color /= pdf;
                if (Ldistance>0.f) color /= (Ldistance*Ldistance);
                color *= cosLN_l;
            }
        }
    } // Kd is zero
    
    return (color);
}
