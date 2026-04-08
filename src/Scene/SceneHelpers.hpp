//
//  SceneHelpers.hpp
//  VI-RT
//
//  Shared helper functions for scene construction.
//  Included by each per-BRDF scene file (PhongScenes.cpp, CookTorranceScenes.cpp, etc.)
//

#ifndef SceneHelpers_hpp
#define SceneHelpers_hpp

#include "scene.hpp"
#include "BRDF.hpp"
#include "DiffuseTexture.hpp"
#include "Sphere.hpp"
#include "triangle.hpp"

static inline int AddDiffuseMat (Scene& scene, RGB const color) {
    BRDF *brdf = new BRDF;

    brdf->Ka = color;
    brdf->Kd = color;
    brdf->Ks = RGB(0., 0., 0.);
    brdf->Kt = RGB(0., 0., 0.);

    return (scene.AddMaterial(brdf));
}

static inline int AddTextMat (Scene& scene, std::string filename, RGB const Ka, RGB const Kd, RGB const Ks, RGB const Kt, float const eta=1.f) {
    DiffuseTexture *brdf = new DiffuseTexture(filename);

    brdf->Ka = Ka;
    brdf->Kd = Kd;
    brdf->Ks = Ks;
    brdf->Kt = Kt;
    brdf->eta = eta;
    brdf->textured = true;

    return (scene.AddMaterial(brdf));
}


static inline int AddMat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks, RGB const Kt, float const eta=1.f) {
    BRDF *brdf = new BRDF;

    brdf->Ka = Ka;
    brdf->Kd = Kd;
    brdf->Ks = Ks;
    brdf->Kt = Kt;
    brdf->eta = eta;

    return (scene.AddMaterial(brdf));
}

static inline void AddSphere (Scene& scene, Point const C,
                             float const radius, int const mat_ndx) {
    Sphere *sphere = new Sphere(C, radius);
    Primitive *prim = new Primitive;
    prim->g = sphere;
    prim->material_ndx = mat_ndx;
    scene.AddPrimitive(prim);
}

static inline void AddTriangle (Scene& scene,
                         Point const v1, Point const v2, Point const v3,
                         int const mat_ndx) {

    Triangle *tri = new Triangle(v1, v2, v3);
    Primitive *prim = new Primitive;
    prim->g = tri;
    prim->material_ndx = mat_ndx;
    scene.AddPrimitive(prim);
}

static inline void AddTriangleUV (Scene& scene,
                         Point const v1, Point const v2, Point const v3,
                           Vec2 const uv1, Vec2 const uv2, Vec2 const uv3,
                         int const mat_ndx) {

    Triangle *tri = new Triangle(v1, v2, v3);
    tri->set_uv(uv1, uv2, uv3);
    Primitive *prim = new Primitive;
    prim->g = tri;
    prim->material_ndx = mat_ndx;
    scene.AddPrimitive(prim);
}

static inline void AddBox(Scene& scene,
                   Point const center, float const half,
                   int const mat_ndx)
{
    // 8 vértices
    float x0 = center.X - half, x1 = center.X + half;
    float y0 = center.Y - half, y1 = center.Y + half;
    float z0 = center.Z - half, z1 = center.Z + half;

    // Face frontal  (z = z0, normal -Z)
    AddTriangle(scene, Point(x0,y0,z0), Point(x1,y0,z0), Point(x1,y1,z0), mat_ndx);
    AddTriangle(scene, Point(x0,y0,z0), Point(x1,y1,z0), Point(x0,y1,z0), mat_ndx);
    // Face traseira (z = z1, normal +Z)
    AddTriangle(scene, Point(x1,y0,z1), Point(x0,y0,z1), Point(x0,y1,z1), mat_ndx);
    AddTriangle(scene, Point(x1,y0,z1), Point(x0,y1,z1), Point(x1,y1,z1), mat_ndx);
    // Face esquerda (x = x0, normal -X)
    AddTriangle(scene, Point(x0,y0,z1), Point(x0,y0,z0), Point(x0,y1,z0), mat_ndx);
    AddTriangle(scene, Point(x0,y0,z1), Point(x0,y1,z0), Point(x0,y1,z1), mat_ndx);
    // Face direita  (x = x1, normal +X)
    AddTriangle(scene, Point(x1,y0,z0), Point(x1,y0,z1), Point(x1,y1,z1), mat_ndx);
    AddTriangle(scene, Point(x1,y0,z0), Point(x1,y1,z1), Point(x1,y1,z0), mat_ndx);
    // Face inferior (y = y0, normal -Y)
    AddTriangle(scene, Point(x0,y0,z1), Point(x1,y0,z1), Point(x1,y0,z0), mat_ndx);
    AddTriangle(scene, Point(x0,y0,z1), Point(x1,y0,z0), Point(x0,y0,z0), mat_ndx);
    // Face superior (y = y1, normal +Y)
    AddTriangle(scene, Point(x0,y1,z0), Point(x1,y1,z0), Point(x1,y1,z1), mat_ndx);
    AddTriangle(scene, Point(x0,y1,z0), Point(x1,y1,z1), Point(x0,y1,z1), mat_ndx);
}


static void AddBoxUV(Scene& scene,
                     Point const center, float const half,
                     int const mat_ndx)
{
    float x0 = center.X - half, x1 = center.X + half;
    float y0 = center.Y - half, y1 = center.Y + half;
    float z0 = center.Z - half, z1 = center.Z + half;
 
    // Face frontal (z = z0, normal -Z)
    // u: x0→x1 = 0→1 ; v: y0→y1 = 0→1
    AddTriangleUV(scene,
        Point(x0,y0,z0), Point(x1,y0,z0), Point(x1,y1,z0),
        Vec2(0,0), Vec2(1,0), Vec2(1,1), mat_ndx);
    AddTriangleUV(scene,
        Point(x0,y0,z0), Point(x1,y1,z0), Point(x0,y1,z0),
        Vec2(0,0), Vec2(1,1), Vec2(0,1), mat_ndx);
 
    // Face traseira (z = z1, normal +Z)
    // u: x1→x0 = 0→1 (espelhado para consistência de orientação)
    AddTriangleUV(scene,
        Point(x1,y0,z1), Point(x0,y0,z1), Point(x0,y1,z1),
        Vec2(0,0), Vec2(1,0), Vec2(1,1), mat_ndx);
    AddTriangleUV(scene,
        Point(x1,y0,z1), Point(x0,y1,z1), Point(x1,y1,z1),
        Vec2(0,0), Vec2(1,1), Vec2(0,1), mat_ndx);
 
    // Face esquerda (x = x0, normal -X)
    // u: z1→z0 = 0→1 ; v: y0→y1 = 0→1
    AddTriangleUV(scene,
        Point(x0,y0,z1), Point(x0,y0,z0), Point(x0,y1,z0),
        Vec2(0,0), Vec2(1,0), Vec2(1,1), mat_ndx);
    AddTriangleUV(scene,
        Point(x0,y0,z1), Point(x0,y1,z0), Point(x0,y1,z1),
        Vec2(0,0), Vec2(1,1), Vec2(0,1), mat_ndx);
 
    // Face direita (x = x1, normal +X)
    // u: z0→z1 = 0→1 ; v: y0→y1 = 0→1
    AddTriangleUV(scene,
        Point(x1,y0,z0), Point(x1,y0,z1), Point(x1,y1,z1),
        Vec2(0,0), Vec2(1,0), Vec2(1,1), mat_ndx);
    AddTriangleUV(scene,
        Point(x1,y0,z0), Point(x1,y1,z1), Point(x1,y1,z0),
        Vec2(0,0), Vec2(1,1), Vec2(0,1), mat_ndx);
 
    // Face inferior (y = y0, normal -Y)
    // u: x0→x1 = 0→1 ; v: z1→z0 = 0→1
    AddTriangleUV(scene,
        Point(x0,y0,z1), Point(x1,y0,z1), Point(x1,y0,z0),
        Vec2(0,0), Vec2(1,0), Vec2(1,1), mat_ndx);
    AddTriangleUV(scene,
        Point(x0,y0,z1), Point(x1,y0,z0), Point(x0,y0,z0),
        Vec2(0,0), Vec2(1,1), Vec2(0,1), mat_ndx);
 
    // Face superior (y = y1, normal +Y)
    // u: x0→x1 = 0→1 ; v: z0→z1 = 0→1
    AddTriangleUV(scene,
        Point(x0,y1,z0), Point(x1,y1,z0), Point(x1,y1,z1),
        Vec2(0,0), Vec2(1,0), Vec2(1,1), mat_ndx);
    AddTriangleUV(scene,
        Point(x0,y1,z0), Point(x1,y1,z1), Point(x0,y1,z1),
        Vec2(0,0), Vec2(1,1), Vec2(0,1), mat_ndx);
}

static inline void AddBoxMultiMat(Scene& scene,
                                   Point const center, float const half,
                                   int const mat_front, int const mat_back,
                                   int const mat_left,  int const mat_right,
                                   int const mat_bottom, int const mat_top)
{
    float x0 = center.X - half, x1 = center.X + half;
    float y0 = center.Y - half, y1 = center.Y + half;
    float z0 = center.Z - half, z1 = center.Z + half;

    // Face frontal  (z = z0, normal -Z)
    AddTriangle(scene, Point(x0,y0,z0), Point(x1,y0,z0), Point(x1,y1,z0), mat_front);
    AddTriangle(scene, Point(x0,y0,z0), Point(x1,y1,z0), Point(x0,y1,z0), mat_front);
    // Face traseira (z = z1, normal +Z)
    AddTriangle(scene, Point(x1,y0,z1), Point(x0,y0,z1), Point(x0,y1,z1), mat_back);
    AddTriangle(scene, Point(x1,y0,z1), Point(x0,y1,z1), Point(x1,y1,z1), mat_back);
    // Face esquerda (x = x0, normal -X)
    AddTriangle(scene, Point(x0,y0,z1), Point(x0,y0,z0), Point(x0,y1,z0), mat_left);
    AddTriangle(scene, Point(x0,y0,z1), Point(x0,y1,z0), Point(x0,y1,z1), mat_left);
    // Face direita  (x = x1, normal +X)
    AddTriangle(scene, Point(x1,y0,z0), Point(x1,y0,z1), Point(x1,y1,z1), mat_right);
    AddTriangle(scene, Point(x1,y0,z0), Point(x1,y1,z1), Point(x1,y1,z0), mat_right);
    // Face inferior (y = y0, normal -Y)
    AddTriangle(scene, Point(x0,y0,z1), Point(x1,y0,z1), Point(x1,y0,z0), mat_bottom);
    AddTriangle(scene, Point(x0,y0,z1), Point(x1,y0,z0), Point(x0,y0,z0), mat_bottom);
    // Face superior (y = y1, normal +Y)
    AddTriangle(scene, Point(x0,y1,z0), Point(x1,y1,z0), Point(x1,y1,z1), mat_top);
    AddTriangle(scene, Point(x0,y1,z0), Point(x1,y1,z1), Point(x0,y1,z1), mat_top);
}

#endif /* SceneHelpers_hpp */
