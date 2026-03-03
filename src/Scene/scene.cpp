//
//  Scene.cpp
//  VI-RT
//
//  Created by Luis Paulo Santos on 30/01/2023.
//

#include "scene.hpp"

#include "primitive.hpp"
#include "BRDF.hpp"
#include "AreaLight.hpp"

#include <iostream>
#include <set>
#include <vector>

Scene::~Scene() {
    if (bvh) delete bvh;
    if (lightBVH) delete lightBVH;
    for (auto prim : lightPrims) {
        delete prim;
    }
}

void Scene::BuildBVH() {
    if (prims.size() > 0) {
        bvh = new BVHAccel(prims, BRDFs.data(), 4, SplitMethod::SAH);
        useBVH = true;
        fprintf(stdout, "BVH acceleration enabled\n");
        fprintf(stdout, "Total primitives in scene: %zu\n", prims.size());
    }
}

void Scene::BuildLightBVH() {
    lightPrims.clear();
    geometryToLight.clear();
    
    int areaLightCount = 0;
    for (auto l : lights) {
        if (l->type == AREA_LIGHT) {
            AreaLight* al = (AreaLight*)l;
            geometryToLight[al->gem] = al;
            
            // Criar uma primitiva que aponta para a geometria da luz
            Primitive* lightPrim = new Primitive;
            lightPrim->g = al->gem;
            
            lightPrim->material_ndx = -1;

            lightPrims.push_back(lightPrim);
            areaLightCount++;
        }
    }
    
    if (areaLightCount > 0) {
        fprintf(stdout, "Building Light BVH with %d area lights...\n", areaLightCount);
        
        // As luzes não usam materiais, mas o BVH precisa de um ponteiro válido
        lightBVH = new BVHAccel(lightPrims, BRDFs.data(), 4, SplitMethod::SAH);
        useLightBVH = true;
        
        fprintf(stdout, "Light BVH acceleration enabled\n");
    } else {
        fprintf(stdout, "No area lights found, Light BVH not built\n");
    }
}

bool Scene::trace(Ray r, Intersection *isect) {
    Intersection curr_isect;
    bool intersection = false;    
    static int total_traces = 0;
    static int bvh_hits = 0;
    static int light_hits = 0;

    total_traces++;
    
    curr_isect.pix_x = isect->pix_x = r.pix_x;
    curr_isect.pix_y = isect->pix_y = r.pix_y;

    if (numPrimitives == 0) return false;
    
    // USA BVH PARA TRACE
    if (useBVH && bvh) {
        // BVH agora já atribui o material internamente
        intersection = bvh->Intersect(r, isect);
        if (intersection) bvh_hits++;
    }
    else {
        // FORÇA BRUTA apenas quando BVH não está disponível
        for (auto prim_itr = prims.begin(); prim_itr != prims.end(); prim_itr++) {
            if ((*prim_itr)->g->intersect(r, &curr_isect)) {
                if (!intersection) {
                    intersection = true;
                    *isect = curr_isect;
                    isect->f = BRDFs[(*prim_itr)->material_ndx];
                }
                else if (curr_isect.depth < isect->depth) {
                    *isect = curr_isect;
                    isect->f = BRDFs[(*prim_itr)->material_ndx];
                }
            }
        }
    }
    
    // LUZES
    isect->isLight = false;
    
    if (useLightBVH && lightBVH) {
        Intersection light_isect;
        light_isect.pix_x = r.pix_x;
        light_isect.pix_y = r.pix_y;
        
        if (lightBVH->Intersect(r, &light_isect)) {
            if (!intersection || light_isect.depth < isect->depth) {
                // Método mais eficiente: percorrer as primitivas das luzes e verificar qual geometria foi atingida
                for (size_t i = 0; i < lightPrims.size(); i++) {
                    // Testar se este triângulo específico foi atingido
                    // fazendo uma intersecção precisa
                    Intersection test_isect;
                    if (lightPrims[i]->g->intersect(r, &test_isect)) {
                        // Verificar se é a mesma intersecção (mesma profundidade)
                        if (std::abs(test_isect.depth - light_isect.depth) < EPSILON) {
                            // Encontrou! Buscar a luz correspondente
                            auto it = geometryToLight.find(lightPrims[i]->g);
                            if (it != geometryToLight.end()) {
                                intersection = true;
                                *isect = light_isect;
                                isect->isLight = true;
                                isect->Le = it->second->L();
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        // Fallback: força bruta para luzes quando Light BVH não está disponível
        for (auto l = lights.begin(); l != lights.end(); l++) {
            if ((*l)->type == AREA_LIGHT) {
                AreaLight *al = (AreaLight *)*l;
                if (al->gem->intersect(r, &curr_isect)) {
                    if (!intersection) {
                        intersection = true;
                        *isect = curr_isect;
                        isect->isLight = true;
                        isect->Le = al->L();
                    }
                    else if (curr_isect.depth < isect->depth) {
                        *isect = curr_isect;
                        isect->isLight = true;
                        isect->Le = al->L();
                    }
                }
            }
        }
    }
    
    isect->r_type = r.rtype;
    if (total_traces % 100000 == 0) {
        fprintf(stderr, "Traces: %d, BVH hits: %d (%.1f%%)\n", 
                total_traces, bvh_hits, 100.0f * bvh_hits / total_traces);
    }
    
    return intersection;
}

bool Scene::visibility(Ray s, const float maxL) {
    static int total_shadow_rays = 0;
    static int bvh_shadow_rays = 0;
    
    total_shadow_rays++;
    
    if (useBVH && bvh) {
        bvh_shadow_rays++;
        
        if (total_shadow_rays % 1000000 == 0) {
            fprintf(stderr, "Shadow rays: %d total, %d using BVH (%.1f%%)\n", 
                    total_shadow_rays, bvh_shadow_rays, 
                    100.0f * bvh_shadow_rays / total_shadow_rays);
        }
        
        return !bvh->IntersectP(s, maxL);
    }
    if (numPrimitives == 0) return true;
    
    Intersection curr_isect;
    for (auto prim : prims) {
        if (prim->g->intersect(s, &curr_isect)) {
            if (curr_isect.depth < maxL) {
                return false;  
            }
        }
    }
    return true; 
}