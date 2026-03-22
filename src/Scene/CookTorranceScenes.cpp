//
//  CookTorranceScenes.cpp
//  VI-RT
//
//  Cook-Torrance BRDF scenes.
//

#include "BuildScenes.hpp"
#include "SceneHelpers.hpp"

static int AddCookTorranceMat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks, float const roughness, float const metallic) {
    CookTorrance *brdf = new CookTorrance;
    brdf->Ka = Ka;
    brdf->Kd = Kd;
    brdf->Ks      = RGB(0., 0., 0.);
    brdf->Ks_brdf = Ks;
    brdf->Kt = RGB(0., 0., 0.);
    brdf->roughness = roughness;
    brdf->metallic  = metallic;
    return (scene.AddMaterial(brdf));
}


void CookTorranceSphereScene (Scene& scene) {
    // COBRE: metal condutor — F0 = Kd (albedo tintado), metallic=1.0
    // Em PBR, o albedo do metal IS the F0. Kd deixa de ter componente difusa.
    RGB const Kd(0.95f, 0.64f, 0.54f);  // albedo do cobre (= F0 antigo)
    RGB const Ks(1.0f, 1.0f, 1.0f);     // guard do directLighting — manter (1,1,1)
    RGB const Ka(0.02f, 0.02f, 0.02f);
    float const metallic = 1.0f;         // condutor: F0 derivado de Kd
    // float const roughness = 1.0f

    int const ct_rough  = AddCookTorranceMat(scene, Ka, Kd, Ks, 1.0f,  metallic);
    int const ct_mid    = AddCookTorranceMat(scene, Ka, Kd, Ks, 0.5f,  metallic);
    int const ct_shiny  = AddCookTorranceMat(scene, Ka, Kd, Ks, 0.2f,  metallic);
    int const ct_mirror = AddCookTorranceMat(scene, Ka, Kd, Ks, 0.05f, metallic);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, ct_rough);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, ct_mid);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, ct_shiny);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, ct_mirror);

    AmbientLight *ambient = new AmbientLight(RGB(0.05f, 0.05f, 0.05f));
    scene.lights.push_back(ambient);
    scene.numLights++;

    PointLight *p1 = new PointLight(RGB(200.f, 200.f, 200.f), Point(0.f, 2.f, -1.f));
    scene.lights.push_back(p1);
    scene.numLights++;
}

void CookTorranceCubeScene (Scene& scene) {
    // COBRE: metal condutor — F0 = Kd (albedo tintado), metallic=1.0
    RGB const Kd(0.95f, 0.64f, 0.54f);
    RGB const Ks(1.0f, 1.0f, 1.0f);
    RGB const Ka(0.02f, 0.02f, 0.02f);
    float const metallic = 1.0f;

    int const ct_rough  = AddCookTorranceMat(scene, Ka, Kd, Ks, 1.0f,  metallic);
    int const ct_mid    = AddCookTorranceMat(scene, Ka, Kd, Ks, 0.5f,  metallic);
    int const ct_shiny  = AddCookTorranceMat(scene, Ka, Kd, Ks, 0.2f,  metallic);
    int const ct_mirror = AddCookTorranceMat(scene, Ka, Kd, Ks, 0.05f, metallic);

    AddBox(scene, Point(-3.f, 0.f, 3.f), 0.8f, ct_rough);
    AddBox(scene, Point(-1.f, 0.f, 3.f), 0.8f, ct_mid);
    AddBox(scene, Point( 1.f, 0.f, 3.f), 0.8f, ct_shiny);
    AddBox(scene, Point( 3.f, 0.f, 3.f), 0.8f, ct_mirror);

    AmbientLight *ambient = new AmbientLight(RGB(0.05f, 0.05f, 0.5f));
    scene.lights.push_back(ambient);
    scene.numLights++;

    PointLight *p1 = new PointLight(RGB(300.f, 300.f, 300.f), Point(0.f, 2.f, 0.f));
    scene.lights.push_back(p1);
    scene.numLights++;
}

void CookTorranceJustOneThing (Scene& scene) {

    // PBR metallic workflow: Kd = albedo base
    // Metais (metallic=1.0): F0 = Kd (o albedo tintado é o F0 do condutor)
    // Dielétricos (metallic=0.0): F0 = 0.04 auto (calculado no BRDF)

    /* OURO: metal condutor */
    RGB const Kd(1.00f, 0.71f, 0.29f);  // albedo do ouro = F0
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Ka(0.02f, 0.02f, 0.02f);
    float const roughness = 0.5f;
    float const metallic  = 1.0f;

    /* PRATA: metal condutor
    RGB const Kd(0.95f, 0.93f, 0.88f);  // albedo da prata = F0
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Ka(0.05f, 0.05f, 0.05f);
    float const roughness = 0.15f;
    float const metallic  = 1.0f;
    */
    /* COBRE: metal condutor
    RGB const Kd(0.95f, 0.64f, 0.54f);  // albedo do cobre = F0
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Ka(0.03f, 0.01f, 0.01f);
    float const roughness = 0.2f;
    float const metallic  = 1.0f;
    */
    /* SAFIRA: dielétrico (F0=0.04 auto)
    RGB const Kd(0.01f, 0.02f, 0.35f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Ka(0.01f, 0.01f, 0.03f);
    float const roughness = 0.2f;
    float const metallic  = 0.0f;
    */
    /* PLÁSTICO: dielétrico (F0=0.04 auto)
    RGB const Kd(0.8f,  0.1f,  0.1f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Ka(0.05f, 0.01f, 0.01f);
    float const roughness = 0.3f;
    float const metallic  = 0.0f;
    */
    /* BORRACHA: dielétrico (F0=0.04 auto)
    RGB const Kd(0.02f, 0.02f, 0.02f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Ka(0.01f, 0.01f, 0.01f);
    float const roughness = 0.9f;
    float const metallic  = 0.0f;
    */

    int const app = AddCookTorranceMat(scene, Ka, Kd, Ks, roughness, metallic);

    AddSphere(scene, Point(0.f, 0.f, 3.f), 0.8f, app);

    AmbientLight *ambient = new AmbientLight(RGB(0.05f, 0.05f, 0.05f));
    scene.lights.push_back(ambient);
    scene.numLights++;

    PointLight *p1 = new PointLight(RGB(300.f, 300.f, 300.f), Point(-1.f,  2.f, 0.f));
    scene.lights.push_back(p1);
    scene.numLights++;
}

/*
static int AddCookTorranceTexMat (Scene& scene, std::string filename, RGB const Ka, RGB const Kd, RGB const Ks, float const roughness, float const metallic) {
    CookTorranceTexture *brdf = new CookTorranceTexture(filename);
    brdf->Ka = Ka;
    brdf->Kd = Kd;
    brdf->Ks      = RGB(0., 0., 0.);  // sem raios de espelho
    brdf->Ks_brdf = Ks;               // especular do BRDF
    brdf->Kt = RGB(0., 0., 0.);
    brdf->roughness = roughness;
    brdf->metallic  = metallic;
    return (scene.AddMaterial(brdf));
}

// 3 esferas: sem textura (referência), Dog.ppm, UMinho.ppm — especular Cook-Torrance
void CookTorranceTextureScene (Scene& scene) {
    int const plain = AddCookTorranceMat(scene,
        RGB(0.05f, 0.05f, 0.05f),
        RGB(0.6f, 0.2f, 0.2f),
        RGB(1.0f, 1.0f, 1.0f),
        0.3f,
        RGB(0.04f, 0.04f, 0.04f));

    int const tex_dog = AddCookTorranceTexMat(scene, "Dog.ppm",
        RGB(0.05f, 0.05f, 0.05f),
        RGB(1.0f, 1.0f, 1.0f),
        RGB(1.0f, 1.0f, 1.0f),
        0.3f,
        RGB(0.04f, 0.04f, 0.04f));

    int const tex_um = AddCookTorranceTexMat(scene, "UMinho.ppm",
        RGB(0.05f, 0.05f, 0.05f),
        RGB(1.0f, 1.0f, 1.0f),
        RGB(1.0f, 1.0f, 1.0f),
        0.3f,
        RGB(0.04f, 0.04f, 0.04f));

    AddSphere(scene, Point(-2.f, 0.f, 5.f), 0.8f, plain);
    AddSphere(scene, Point( 0.f, 0.f, 5.f), 0.8f, tex_dog);
    AddSphere(scene, Point( 2.f, 0.f, 5.f), 0.8f, tex_um);

    AmbientLight *ambient = new AmbientLight(RGB(0.1f, 0.1f, 0.1f));
    scene.lights.push_back(ambient);
    scene.numLights++;
    PointLight *p1 = new PointLight(RGB(80.f, 80.f, 80.f), Point(0.f, 3.f, 0.f));
    scene.lights.push_back(p1);
    scene.numLights++;
}
*/
