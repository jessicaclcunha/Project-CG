//
//  OrenNayarScenes.cpp
//  VI-RT
//
//  Oren-Nayar BRDF scenes.
//

#include "BuildScenes.hpp"
#include "SceneHelpers.hpp"

static int AddOrenNayarMat (Scene& scene, RGB const Ka, RGB const Kd, float const sigma) {
    OrenNayar *brdf = new OrenNayar;
    brdf->Ka    = Ka;
    brdf->Kd    = Kd;
    brdf->Ks    = RGB(0.f, 0.f, 0.f);
    brdf->Ks_brdf = RGB(0.f, 0.f, 0.f);  // sem especular — modelo puramente difuso
    brdf->Kt    = RGB(0.f, 0.f, 0.f);
    brdf->sigma = sigma;
    return (scene.AddMaterial(brdf));
}

// 4 esferas com mesmo Kd e sigma crescente: 0.0 (Lambertiano) → 0.9 (muito rugoso)
// Permite comparar visualmente o efeito do sigma na retroreflexão
void OrenNayarScene (Scene& scene) {
    RGB const Kd(0.7f, 0.5f, 0.3f);  // argila clara — bom para ver variação de rugosidade
    RGB const Ka(0.02f, 0.01f, 0.01f);

    int const on0  = AddOrenNayarMat(scene, Ka, Kd, 0.0f);  // Lambertiano puro (referência)
    int const on3  = AddOrenNayarMat(scene, Ka, Kd, 0.3f);
    int const on6  = AddOrenNayarMat(scene, Ka, Kd, 0.6f);
    int const on9  = AddOrenNayarMat(scene, Ka, Kd, 0.9f);  // muito rugoso

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, on0);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, on3);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, on6);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, on9);

    AmbientLight *ambient = new AmbientLight(RGB(0.05f, 0.05f, 0.05f));
    scene.lights.push_back(ambient);
    scene.numLights++;

    PointLight *p1 = new PointLight(RGB(300.f, 300.f, 300.f), Point(-1.f, 2.f, 0.f));
    scene.lights.push_back(p1);
    scene.numLights++;
}

// Esfera única — presets comentados para fácil experimentação
void OrenNayarJustOneThing (Scene& scene) {

    /* ARGILA: rugoso, tom quente */
    RGB const Kd(0.7f,  0.5f,  0.3f);
    RGB const Ka(0.02f, 0.01f, 0.01f);
    float const sigma = 0.6f;

    /* GESSO: rugoso, branco-acinzentado
    RGB const Kd(0.85f, 0.85f, 0.80f);
    RGB const Ka(0.02f, 0.02f, 0.02f);
    float const sigma = 0.5f;
    */
    /* VELUDO: muito rugoso, retroreflexão máxima
    RGB const Kd(0.1f,  0.05f, 0.2f);
    RGB const Ka(0.01f, 0.01f, 0.02f);
    float const sigma = 0.9f;
    */
    /* CALCÁRIO: ligeiramente rugoso
    RGB const Kd(0.75f, 0.70f, 0.60f);
    RGB const Ka(0.02f, 0.02f, 0.01f);
    float const sigma = 0.3f;
    */
    /* LAMBERTIANO: sigma=0 — idêntico a difuso puro (teste de sanidade)
    RGB const Kd(0.7f,  0.5f,  0.3f);
    RGB const Ka(0.02f, 0.01f, 0.01f);
    float const sigma = 0.0f;
    */

    int const mat = AddOrenNayarMat(scene, Ka, Kd, sigma);
    AddSphere(scene, Point(0.f, 0.f, 3.f), 0.8f, mat);

    AmbientLight *ambient = new AmbientLight(RGB(0.05f, 0.05f, 0.05f));
    scene.lights.push_back(ambient);
    scene.numLights++;

    PointLight *p1 = new PointLight(RGB(300.f, 300.f, 300.f), Point(-1.f, 2.f, 0.f));
    scene.lights.push_back(p1);
    scene.numLights++;
}
