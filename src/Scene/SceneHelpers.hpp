//
//  SceneHelpers.hpp
//  VI-RT

#ifndef SceneHelpers_hpp
#define SceneHelpers_hpp

#include "scene.hpp"
#include "BRDF.hpp"
#include "Ward.hpp"
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
    brdf->Ka = Ka; brdf->Kd = Kd; brdf->Ks = Ks; brdf->Kt = Kt; brdf->eta = eta;
    brdf->textured = true;
    return (scene.AddMaterial(brdf));
}

static inline int AddMat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks, RGB const Kt, float const eta=1.f) {
    BRDF *brdf = new BRDF;
    brdf->Ka = Ka; brdf->Kd = Kd; brdf->Ks = Ks; brdf->Kt = Kt; brdf->eta = eta;
    return (scene.AddMaterial(brdf));
}

static inline void AddSphere (Scene& scene, Point const C, float const radius, int const mat_ndx) {
    Sphere *sphere = new Sphere(C, radius);
    Primitive *prim = new Primitive;
    prim->g = sphere; prim->material_ndx = mat_ndx;
    scene.AddPrimitive(prim);
}

static inline void AddTriangle (Scene& scene,
                         Point const v1, Point const v2, Point const v3, int const mat_ndx) {
    Triangle *tri = new Triangle(v1, v2, v3);
    Primitive *prim = new Primitive;
    prim->g = tri; prim->material_ndx = mat_ndx;
    scene.AddPrimitive(prim);
}

static inline void AddTriangleUV (Scene& scene,
                         Point const v1, Point const v2, Point const v3,
                         Vec2 const uv1, Vec2 const uv2, Vec2 const uv3, int const mat_ndx) {
    Triangle *tri = new Triangle(v1, v2, v3);
    tri->set_uv(uv1, uv2, uv3);
    Primitive *prim = new Primitive;
    prim->g = tri; prim->material_ndx = mat_ndx;
    scene.AddPrimitive(prim);
}

static inline void AddBox(Scene& scene, Point const center, float const half, int const mat_ndx) {
    float x0=center.X-half, x1=center.X+half;
    float y0=center.Y-half, y1=center.Y+half;
    float z0=center.Z-half, z1=center.Z+half;
    AddTriangle(scene, Point(x0,y0,z0), Point(x1,y0,z0), Point(x1,y1,z0), mat_ndx);
    AddTriangle(scene, Point(x0,y0,z0), Point(x1,y1,z0), Point(x0,y1,z0), mat_ndx);
    AddTriangle(scene, Point(x1,y0,z1), Point(x0,y0,z1), Point(x0,y1,z1), mat_ndx);
    AddTriangle(scene, Point(x1,y0,z1), Point(x0,y1,z1), Point(x1,y1,z1), mat_ndx);
    AddTriangle(scene, Point(x0,y0,z1), Point(x0,y0,z0), Point(x0,y1,z0), mat_ndx);
    AddTriangle(scene, Point(x0,y0,z1), Point(x0,y1,z0), Point(x0,y1,z1), mat_ndx);
    AddTriangle(scene, Point(x1,y0,z0), Point(x1,y0,z1), Point(x1,y1,z1), mat_ndx);
    AddTriangle(scene, Point(x1,y0,z0), Point(x1,y1,z1), Point(x1,y1,z0), mat_ndx);
    AddTriangle(scene, Point(x0,y0,z1), Point(x1,y0,z1), Point(x1,y0,z0), mat_ndx);
    AddTriangle(scene, Point(x0,y0,z1), Point(x1,y0,z0), Point(x0,y0,z0), mat_ndx);
    AddTriangle(scene, Point(x0,y1,z0), Point(x1,y1,z0), Point(x1,y1,z1), mat_ndx);
    AddTriangle(scene, Point(x0,y1,z0), Point(x1,y1,z1), Point(x0,y1,z1), mat_ndx);
}

// ---------------------------------------------------------------------------
// AddBoxUV
//
// Os GetKd de todos os BRDFs texturizados fazem  v_final = 1 - v
// (para compensar que PPM tem y=0 no topo mas a esfera tem v=0 em baixo).
//
// Para que os cubos apareçam correctos após essa inversão, os UVs têm de ser
// definidos com v já "pré-invertido":
//
//   TL = (0, 1)   BL = (0, 0)
//   TR = (1, 1)   BR = (1, 0)
//
// Depois do GetKd fazer 1-v:
//   TL(0,1) → v_final=0 → topo da textura PPM aparece no topo visual   ✓
//   BL(0,0) → v_final=1 → fundo da textura PPM aparece na base visual   ✓
// ---------------------------------------------------------------------------
static void AddBoxUV(Scene& scene, Point const center, float const half, int const mat_ndx)
{
    float x0=center.X-half, x1=center.X+half;
    float y0=center.Y-half, y1=center.Y+half;
    float z0=center.Z-half, z1=center.Z+half;

    // UVs pré-invertidos em V para compensar o 1-v do GetKd
    Vec2 TL(0.f, 1.f);   // topo-esquerda  visual → v=1 → após 1-v → v_final=0 (topo PPM)
    Vec2 TR(1.f, 1.f);   // topo-direita   visual → v=1 → após 1-v → v_final=0
    Vec2 BL(0.f, 0.f);   // base-esquerda  visual → v=0 → após 1-v → v_final=1 (fundo PPM)
    Vec2 BR(1.f, 0.f);   // base-direita   visual → v=0 → após 1-v → v_final=1

    // FRENTE (z=z0, normal -Z)
    // Vista de -Z: X cresce para a direita, Y para cima
    // (x0,y0)=BL  (x1,y0)=BR  (x1,y1)=TR  (x0,y1)=TL
    AddTriangleUV(scene, Point(x0,y0,z0), Point(x1,y0,z0), Point(x1,y1,z0), BL, BR, TR, mat_ndx);
    AddTriangleUV(scene, Point(x0,y0,z0), Point(x1,y1,z0), Point(x0,y1,z0), BL, TR, TL, mat_ndx);

    // TRÁS (z=z1, normal +Z)
    // Vista de +Z: X decresce para a direita (espelhado), Y para cima
    // (x1,y0)=BL  (x0,y0)=BR  (x0,y1)=TR  (x1,y1)=TL
    AddTriangleUV(scene, Point(x1,y0,z1), Point(x0,y0,z1), Point(x0,y1,z1), BL, BR, TR, mat_ndx);
    AddTriangleUV(scene, Point(x1,y0,z1), Point(x0,y1,z1), Point(x1,y1,z1), BL, TR, TL, mat_ndx);

    // ESQUERDA (x=x0, normal -X)
    // Vista de -X: Z decresce para a direita, Y para cima
    // (x0,y0,z1)=BL  (x0,y0,z0)=BR  (x0,y1,z0)=TR  (x0,y1,z1)=TL
    AddTriangleUV(scene, Point(x0,y0,z1), Point(x0,y0,z0), Point(x0,y1,z0), BL, BR, TR, mat_ndx);
    AddTriangleUV(scene, Point(x0,y0,z1), Point(x0,y1,z0), Point(x0,y1,z1), BL, TR, TL, mat_ndx);

    // DIREITA (x=x1, normal +X)
    // Vista de +X: Z cresce para a direita, Y para cima
    // (x1,y0,z0)=BL  (x1,y0,z1)=BR  (x1,y1,z1)=TR  (x1,y1,z0)=TL
    AddTriangleUV(scene, Point(x1,y0,z0), Point(x1,y0,z1), Point(x1,y1,z1), BL, BR, TR, mat_ndx);
    AddTriangleUV(scene, Point(x1,y0,z0), Point(x1,y1,z1), Point(x1,y1,z0), BL, TR, TL, mat_ndx);

    // BAIXO (y=y0, normal -Y)
    // Vista de baixo: X cresce para a direita, Z cresce afasta
    // (x0,y0,z0)=BL  (x1,y0,z0)=BR  (x1,y0,z1)=TR  (x0,y0,z1)=TL
    AddTriangleUV(scene, Point(x0,y0,z0), Point(x1,y0,z0), Point(x1,y0,z1), BL, BR, TR, mat_ndx);
    AddTriangleUV(scene, Point(x0,y0,z0), Point(x1,y0,z1), Point(x0,y0,z1), BL, TR, TL, mat_ndx);

    // CIMA (y=y1, normal +Y)
    // Vista de cima: X cresce para a direita, Z decresce aproxima
    // (x0,y1,z1)=BL  (x1,y1,z1)=BR  (x1,y1,z0)=TR  (x0,y1,z0)=TL
    AddTriangleUV(scene, Point(x0,y1,z1), Point(x1,y1,z1), Point(x1,y1,z0), BL, BR, TR, mat_ndx);
    AddTriangleUV(scene, Point(x0,y1,z1), Point(x1,y1,z0), Point(x0,y1,z0), BL, TR, TL, mat_ndx);
}

static inline void AddBoxMultiMat(Scene& scene, Point const center, float const half,
                                   int const mat_front, int const mat_back,
                                   int const mat_left,  int const mat_right,
                                   int const mat_bottom, int const mat_top)
{
    float x0=center.X-half, x1=center.X+half;
    float y0=center.Y-half, y1=center.Y+half;
    float z0=center.Z-half, z1=center.Z+half;
    AddTriangle(scene, Point(x0,y0,z0), Point(x1,y0,z0), Point(x1,y1,z0), mat_front);
    AddTriangle(scene, Point(x0,y0,z0), Point(x1,y1,z0), Point(x0,y1,z0), mat_front);
    AddTriangle(scene, Point(x1,y0,z1), Point(x0,y0,z1), Point(x0,y1,z1), mat_back);
    AddTriangle(scene, Point(x1,y0,z1), Point(x0,y1,z1), Point(x1,y1,z1), mat_back);
    AddTriangle(scene, Point(x0,y0,z1), Point(x0,y0,z0), Point(x0,y1,z0), mat_left);
    AddTriangle(scene, Point(x0,y0,z1), Point(x0,y1,z0), Point(x0,y1,z1), mat_left);
    AddTriangle(scene, Point(x1,y0,z0), Point(x1,y0,z1), Point(x1,y1,z1), mat_right);
    AddTriangle(scene, Point(x1,y0,z0), Point(x1,y1,z1), Point(x1,y1,z0), mat_right);
    AddTriangle(scene, Point(x0,y0,z1), Point(x1,y0,z1), Point(x1,y0,z0), mat_bottom);
    AddTriangle(scene, Point(x0,y0,z1), Point(x1,y0,z0), Point(x0,y0,z0), mat_bottom);
    AddTriangle(scene, Point(x0,y1,z0), Point(x1,y1,z0), Point(x1,y1,z1), mat_top);
    AddTriangle(scene, Point(x0,y1,z0), Point(x1,y1,z1), Point(x0,y1,z1), mat_top);
}

static int AddWardMat(Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks,
                      float const alphaX, float const alphaY,
                      Vector const tangent = Vector(0.f,0.f,0.f), bool const hasTangent = false)
{
    Ward *brdf = new Ward;
    brdf->Ka=Ka; brdf->Kd=Kd; brdf->Ks=RGB(0.f,0.f,0.f); brdf->Ks_brdf=Ks;
    brdf->Kt=RGB(0.f,0.f,0.f);
    brdf->alphaX=std::max(0.01f,alphaX); brdf->alphaY=std::max(0.01f,alphaY);
    brdf->tangent=tangent; brdf->hasTangent=hasTangent;
    return (scene.AddMaterial(brdf));
}

static int AddWardMatT(Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks,
                       float const alphaX, float const alphaY, Vector const tangent)
{
    return AddWardMat(scene, Ka, Kd, Ks, alphaX, alphaY, tangent, true);
}

#endif /* SceneHelpers_hpp */