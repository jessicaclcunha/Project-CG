//
//  DisneyScenes.cpp
//  VI-RT
//
//  Disney Principled BRDF — cena de presets + sweeps de parâmetro.
//  Câmara: Eye=(0, 0.5, -5), At=(0, 0, 3), Up=(0,1,0), FOV=60°
//

#include "BuildScenes.hpp"
#include "SceneHelpers.hpp"
#include "Disney.hpp"

// -------------------------------------------------------------------------
// Helper de material Disney — todos os parâmetros com defaults sensatos.
// Pipeline: difuso/especular/sheen/clearcoat vão todos no f(); sem raios de
// espelho (Ks=Kt=0). Kd/Ks_brdf não-nulos só para passar o guard de
// directLighting (não entram no cálculo Disney).
// -------------------------------------------------------------------------
static int AddDisneyMat(Scene& scene, RGB const baseColor,
                        float metallic = 0.f, float roughness = 0.5f,
                        float subsurface = 0.f, float specular = 0.5f,
                        float specularTint = 0.f, float anisotropic = 0.f,
                        float sheen = 0.f, float sheenTint = 0.5f,
                        float clearcoat = 0.f, float clearcoatGloss = 1.f,
                        Vector const tangent = Vector(0.f,0.f,0.f),
                        bool const hasTangent = false)
{
    Disney *brdf = new Disney;
    brdf->baseColor      = baseColor;
    brdf->metallic       = metallic;
    brdf->roughness      = roughness;
    brdf->subsurface     = subsurface;
    brdf->specular       = specular;
    brdf->specularTint   = specularTint;
    brdf->anisotropic    = anisotropic;
    brdf->sheen          = sheen;
    brdf->sheenTint      = sheenTint;
    brdf->clearcoat      = clearcoat;
    brdf->clearcoatGloss = clearcoatGloss;
    brdf->tangent        = tangent;
    brdf->hasTangent     = hasTangent;

    brdf->Kd      = baseColor;            // passa o guard de directLighting
    brdf->Ks_brdf = RGB(1.f, 1.f, 1.f);   // idem (não usado no cálculo Disney)
    brdf->Ks      = RGB(0.f, 0.f, 0.f);
    brdf->Kt      = RGB(0.f, 0.f, 0.f);
    brdf->Ka      = RGB(baseColor.R * 0.05f, baseColor.G * 0.05f, baseColor.B * 0.05f); // ambiente fraco (fill)
    return scene.AddMaterial(brdf);
}

// Iluminação partilhada: 1 point light frontal-superior (highlight nítido) +
// ambient mínimo + chão escuro. Iteramos visualmente após o 1º render.
static void addDisneyLighting(Scene& scene) {
    PointLight *key = new PointLight(RGB(500.f, 500.f, 500.f), Point(0.f, 4.f, -1.f));
    scene.lights.push_back(key); scene.numLights++;

    AmbientLight *amb = new AmbientLight(RGB(0.15f, 0.15f, 0.15f));
    scene.lights.push_back(amb); scene.numLights++;

    int floor_mat = AddDiffuseMat(scene, RGB(0.07f, 0.07f, 0.07f));
    AddTriangle(scene, Point(-8.f,-0.85f,-2.f), Point(8.f,-0.85f,-2.f), Point(8.f,-0.85f,9.f), floor_mat);
    AddTriangle(scene, Point(-8.f,-0.85f,-2.f), Point(8.f,-0.85f,9.f), Point(-8.f,-0.85f,9.f), floor_mat);
}

// Luz LATERAL/rasante — cria meia-esfera em sombra. Ambiente baixo para a sombra
// ficar escura (senão o ambiente esconde o efeito). Usada no subsurface sweep,
// onde o efeito é "encher" o terminador.
static void addDisneyLightingSide(Scene& scene) {
    PointLight *key = new PointLight(RGB(550.f, 550.f, 550.f), Point(5.f, 1.5f, 1.f));
    scene.lights.push_back(key); scene.numLights++;

    AmbientLight *amb = new AmbientLight(RGB(0.06f, 0.06f, 0.06f));
    scene.lights.push_back(amb); scene.numLights++;

    int floor_mat = AddDiffuseMat(scene, RGB(0.07f, 0.07f, 0.07f));
    AddTriangle(scene, Point(-8.f,-0.85f,-2.f), Point(8.f,-0.85f,-2.f), Point(8.f,-0.85f,9.f), floor_mat);
    AddTriangle(scene, Point(-8.f,-0.85f,-2.f), Point(8.f,-0.85f,9.f), Point(-8.f,-0.85f,9.f), floor_mat);
}

// Coloca 5 esferas em fila (x = -3.4 .. 3.4), y=0, z=3, r=0.8.
static void addDisneyRow(Scene& scene, int m0, int m1, int m2, int m3, int m4) {
    float const xs[5] = { -3.4f, -1.7f, 0.f, 1.7f, 3.4f };
    int const m[5] = { m0, m1, m2, m3, m4 };
    for (int i = 0; i < 5; ++i)
        AddSphere(scene, Point(xs[i], 0.f, 3.f), 0.8f, m[i]);
}

// =========================================================================
// CENA DE PRESETS — o mesmo BRDF a imitar 5 materiais distintos
// =========================================================================
void DisneyPresetsScene(Scene& scene) {
    // baseColor, metallic, roughness, subsurface, specular, specTint, aniso,
    //            sheen, sheenTint, clearcoat, ccGloss
    int metal     = AddDisneyMat(scene, RGB(1.00f, 0.78f, 0.34f), 1.f, 0.40f);                                   // ouro (rough alto p/ corpo visível sem IBL)
    int plastic   = AddDisneyMat(scene, RGB(0.10f, 0.20f, 0.80f), 0.f, 0.35f);                                   // plástico azul
    int fabric    = AddDisneyMat(scene, RGB(0.60f, 0.10f, 0.20f), 0.f, 0.85f, 0.f, 0.5f, 0.f, 0.f, 1.f, 0.5f);   // tecido (sheen=1)
    int carpaint  = AddDisneyMat(scene, RGB(0.70f, 0.05f, 0.05f), 0.f, 0.40f, 0.f, 0.5f, 0.f, 0.f, 0.f, 0.5f, 1.f, 1.f); // verniz (clearcoat=1)
    int wax       = AddDisneyMat(scene, RGB(0.90f, 0.75f, 0.65f), 0.f, 0.60f, 1.f);                              // cera/pele (subsurface=1)

    addDisneyRow(scene, metal, plastic, fabric, carpaint, wax);
    addDisneyLighting(scene);
}

// =========================================================================
// SWEEPS — cada cena varia 1 parâmetro de 0→1 (resto fixo)
// =========================================================================

// Roughness sobre metal (ouro): highlight pequeno/nítido → grande/difuso
void DisneyRoughnessSweep(Scene& scene) {
    RGB const c(1.00f, 0.78f, 0.34f);
    int m0 = AddDisneyMat(scene, c, 1.f, 0.02f);
    int m1 = AddDisneyMat(scene, c, 1.f, 0.25f);
    int m2 = AddDisneyMat(scene, c, 1.f, 0.50f);
    int m3 = AddDisneyMat(scene, c, 1.f, 0.75f);
    int m4 = AddDisneyMat(scene, c, 1.f, 1.00f);
    addDisneyRow(scene, m0, m1, m2, m3, m4);
    addDisneyLighting(scene);
}

// Metallic: dieléctrico (difuso + highlight branco) → metal (reflexo colorido, sem difuso)
void DisneyMetallicSweep(Scene& scene) {
    RGB const c(0.95f, 0.70f, 0.30f);
    int m0 = AddDisneyMat(scene, c, 0.00f, 0.25f);
    int m1 = AddDisneyMat(scene, c, 0.25f, 0.25f);
    int m2 = AddDisneyMat(scene, c, 0.50f, 0.25f);
    int m3 = AddDisneyMat(scene, c, 0.75f, 0.25f);
    int m4 = AddDisneyMat(scene, c, 1.00f, 0.25f);
    addDisneyRow(scene, m0, m1, m2, m3, m4);
    addDisneyLighting(scene);
}

// Anisotropic: highlight redondo → esticado (metal, tangente horizontal)
void DisneyAnisoSweep(Scene& scene) {
    RGB const c(0.95f, 0.95f, 0.95f);
    Vector const T(1.f, 0.f, 0.f);
    int m0 = AddDisneyMat(scene, c, 1.f, 0.35f, 0.f, 0.5f, 0.f, 0.00f, 0.f, 0.5f, 0.f, 1.f, T, true);
    int m1 = AddDisneyMat(scene, c, 1.f, 0.35f, 0.f, 0.5f, 0.f, 0.25f, 0.f, 0.5f, 0.f, 1.f, T, true);
    int m2 = AddDisneyMat(scene, c, 1.f, 0.35f, 0.f, 0.5f, 0.f, 0.50f, 0.f, 0.5f, 0.f, 1.f, T, true);
    int m3 = AddDisneyMat(scene, c, 1.f, 0.35f, 0.f, 0.5f, 0.f, 0.75f, 0.f, 0.5f, 0.f, 1.f, T, true);
    int m4 = AddDisneyMat(scene, c, 1.f, 0.35f, 0.f, 0.5f, 0.f, 1.00f, 0.f, 0.5f, 0.f, 1.f, T, true);
    addDisneyRow(scene, m0, m1, m2, m3, m4);
    addDisneyLighting(scene);
}

// Sheen: brilho rasante no contorno cresce. Base quase preta + sheenTint=0
// (rim branco) para o sheen ressaltar contra o corpo escuro.
void DisneySheenSweep(Scene& scene) {
    RGB const c(0.02f, 0.02f, 0.03f);
    int m0 = AddDisneyMat(scene, c, 0.f, 0.50f, 0.f, 0.5f, 0.f, 0.f, 0.00f, 0.0f);
    int m1 = AddDisneyMat(scene, c, 0.f, 0.50f, 0.f, 0.5f, 0.f, 0.f, 0.25f, 0.0f);
    int m2 = AddDisneyMat(scene, c, 0.f, 0.50f, 0.f, 0.5f, 0.f, 0.f, 0.50f, 0.0f);
    int m3 = AddDisneyMat(scene, c, 0.f, 0.50f, 0.f, 0.5f, 0.f, 0.f, 0.75f, 0.0f);
    int m4 = AddDisneyMat(scene, c, 0.f, 0.50f, 0.f, 0.5f, 0.f, 0.f, 1.00f, 0.0f);
    addDisneyRow(scene, m0, m1, m2, m3, m4);
    addDisneyLighting(scene);
}

// Subsurface: sombreado endurecido → suave/translúcido. Base pálida (cera) +
// roughness baixo para o terminador (sombra) ser nítido e ver-se a amolecer.
void DisneySubsurfaceSweep(Scene& scene) {
    RGB const c(0.95f, 0.85f, 0.80f);
    int m0 = AddDisneyMat(scene, c, 0.f, 0.35f, 0.00f);
    int m1 = AddDisneyMat(scene, c, 0.f, 0.35f, 0.25f);
    int m2 = AddDisneyMat(scene, c, 0.f, 0.35f, 0.50f);
    int m3 = AddDisneyMat(scene, c, 0.f, 0.35f, 0.75f);
    int m4 = AddDisneyMat(scene, c, 0.f, 0.35f, 1.00f);
    addDisneyRow(scene, m0, m1, m2, m3, m4);
    addDisneyLightingSide(scene);   // luz lateral → terminador grande p/ o subsurface encher
}

// Clearcoat: 2ª camada de verniz sobre base mate escura. clearcoatGloss=0.3
// (lóbulo do verniz mais largo, não pinpoint) para o 2º highlight ser visível.
void DisneyClearcoatSweep(Scene& scene) {
    RGB const c(0.50f, 0.04f, 0.04f);
    // base com specular=0 → o ÚNICO highlight é o do verniz (clearcoat)
    int m0 = AddDisneyMat(scene, c, 0.f, 0.70f, 0.f, 0.0f, 0.f, 0.f, 0.f, 0.5f, 0.00f, 0.30f);
    int m1 = AddDisneyMat(scene, c, 0.f, 0.70f, 0.f, 0.0f, 0.f, 0.f, 0.f, 0.5f, 0.25f, 0.30f);
    int m2 = AddDisneyMat(scene, c, 0.f, 0.70f, 0.f, 0.0f, 0.f, 0.f, 0.f, 0.5f, 0.50f, 0.30f);
    int m3 = AddDisneyMat(scene, c, 0.f, 0.70f, 0.f, 0.0f, 0.f, 0.f, 0.f, 0.5f, 0.75f, 0.30f);
    int m4 = AddDisneyMat(scene, c, 0.f, 0.70f, 0.f, 0.0f, 0.f, 0.f, 0.f, 0.5f, 1.00f, 0.30f);
    addDisneyRow(scene, m0, m1, m2, m3, m4);
    addDisneyLighting(scene);
}
