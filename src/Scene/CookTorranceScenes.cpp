//
//  CookTorranceScenes.cpp
//  VI-RT
//
//  Cook-Torrance BRDF scenes.
//

#include "BuildScenes.hpp"
#include "SceneHelpers.hpp"

static int AddCookTorranceMat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks, float const roughness, float const metallic, RGB const Ks_mirror = RGB(0.f, 0.f, 0.f)) {
    CookTorrance *brdf = new CookTorrance;
    brdf->Ka = Ka;
    brdf->Kd = Kd;
    brdf->Ks      = Ks_mirror;   // raios de reflexão mirror (shader)
    brdf->Ks_brdf = Ks;          // especular GGX (BRDF)
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
    // ====== MATERIAIS DAS FACES — descomentar/trocar à vontade ======
    RGB const Ks(1.0f, 1.0f, 1.0f);
    RGB const Ka(0.02f, 0.02f, 0.02f);

    // --- Face A: metal (sem Ks_mirror — especular vem do GGX via AreaLight) ---

    /* OURO: metal condutor */
    RGB   const KdA(1.00f, 0.71f, 0.29f);
    float const roughA = 0.3f;
    float const metalA = 1.0f;

    /* PRATA: metal condutor
    RGB   const KdA(0.95f, 0.93f, 0.88f);
    float const roughA = 0.15f;
    float const metalA = 1.0f;
    */
    /* COBRE: metal condutor
    RGB   const KdA(0.95f, 0.64f, 0.54f);
    float const roughA = 0.2f;
    float const metalA = 1.0f;
    */

    // --- Face B: dielétrico ---

    /* PLÁSTICO VERMELHO: dielétrico */
    RGB   const KdB(0.8f,  0.1f,  0.1f);
    float const roughB = 0.3f;
    float const metalB = 0.0f;

    /* CERÂMICA AZUL: dielétrico
    RGB   const KdB(0.1f,  0.2f,  0.6f);
    float const roughB = 0.4f;
    float const metalB = 0.0f;
    */
    /* BORRACHA: dielétrico
    RGB   const KdB(0.02f, 0.02f, 0.02f);
    float const roughB = 0.9f;
    float const metalB = 0.0f;
    */
    /* SAFIRA: dielétrico
    RGB   const KdB(0.01f, 0.02f, 0.35f);
    float const roughB = 0.2f;
    float const metalB = 0.0f;
    */

    int const matA = AddCookTorranceMat(scene, Ka, KdA, Ks, roughA, metalA);
    int const matB = AddCookTorranceMat(scene, Ka, KdB, Ks, roughB, metalB);

    // Da diagonal (3,3,-3) vê-se front, right, top
    // front=plástico (difuso claro), right=ouro, top=ouro (apanha AreaLight)
    //                          front, back, left,  right, bottom, top
    AddBoxMultiMat(scene, Point(0.f, 0.f, 3.f), 0.8f,
                   matB, matA, matA, matA, matB, matA);

    // Chão cinzento
    int const ground = AddDiffuseMat(scene, RGB(0.4f, 0.4f, 0.4f));
    AddTriangle(scene, Point(-5.f,-0.8f,-5.f), Point(5.f,-0.8f,-5.f), Point(5.f,-0.8f,10.f), ground);
    AddTriangle(scene, Point(-5.f,-0.8f,-5.f), Point(5.f,-0.8f,10.f), Point(-5.f,-0.8f,10.f), ground);

    AmbientLight *ambient = new AmbientLight(RGB(0.05f, 0.05f, 0.05f));
    scene.lights.push_back(ambient);
    scene.numLights++;

    // Painel de luz rectangular (AreaLight) — highlight largo para GGX
    Vector lightN(0.f, -1.f, 0.f);
    AreaLight *a1 = new AreaLight(RGB(80.f, 80.f, 80.f),
        Point(-2.f, 3.f, 1.f), Point(2.f, 3.f, 1.f), Point(2.f, 3.f, 5.f), lightN);
    scene.lights.push_back(a1);
    scene.numLights++;
    AreaLight *a2 = new AreaLight(RGB(80.f, 80.f, 80.f),
        Point(-2.f, 3.f, 1.f), Point(2.f, 3.f, 5.f), Point(-2.f, 3.f, 5.f), lightN);
    scene.lights.push_back(a2);
    scene.numLights++;

    // Fill light lateral — revela Fresnel rasante nas faces metálicas
    PointLight *fill = new PointLight(RGB(30.f, 30.f, 30.f), Point(-3.f, 1.f, 0.f));
    scene.lights.push_back(fill);
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

static int AddCookTorranceTexMat (Scene& scene, std::string filename,
                                   RGB const Ka, RGB const Kd, RGB const Ks,
                                   float const roughness, float const metallic) {
    CookTorranceTexture *brdf = new CookTorranceTexture(filename);
    brdf->Ka        = Ka;
    brdf->Kd        = Kd;
    brdf->Ks        = RGB(0., 0., 0.);  // sem raios de espelho
    brdf->Ks_brdf   = Ks;               // especular GGX (BRDF)
    brdf->Kt        = RGB(0., 0., 0.);
    brdf->roughness = roughness;
    brdf->metallic  = metallic;       
    return (scene.AddMaterial(brdf));
}

void CookTorranceTextureScene (Scene& scene) {
    // plástico vermelho dielétrico — metallic=0.0 → F0=0.04 calculado no BRDF
    RGB const Ks(1.0f, 1.0f, 1.0f);
    float const roughness = 0.3f;
    float const metallic  = 0.5f;  // dielétrico

    int const plain_ct = AddCookTorranceMat(scene,
        RGB(0.05f, 0.05f, 0.05f),
        RGB(0.6f, 0.2f, 0.2f),
        Ks, roughness, metallic);

    int const tex_dog_ct = AddCookTorranceTexMat(scene, "Dog.ppm",
        RGB(0.05f, 0.05f, 0.05f),
        RGB(1.0f, 1.0f, 1.0f),
        Ks, roughness, metallic);

    int const tex_um_ct = AddCookTorranceTexMat(scene, "UMinho.ppm",
        RGB(0.05f, 0.05f, 0.05f),
        RGB(1.0f, 1.0f, 1.0f),
        Ks, roughness, metallic);

    // linha de cima: esferas
    AddSphere(scene, Point(-2.f, 0.8f, 5.f), 0.7f, plain_ct);
    AddSphere(scene, Point( 0.f, 0.8f, 5.f), 0.7f, tex_dog_ct);
    AddSphere(scene, Point( 2.f, 0.8f, 5.f), 0.7f, tex_um_ct);

    // linha de baixo: cubos com UV mapping
    AddBoxUV(scene, Point(-2.f, -0.8f, 5.f), 0.55f, plain_ct);
    AddBoxUV(scene, Point( 0.f, -0.8f, 5.f), 0.55f, tex_dog_ct);
    AddBoxUV(scene, Point( 2.f, -0.8f, 5.f), 0.55f, tex_um_ct);

    AmbientLight *ambient = new AmbientLight(RGB(0.08f, 0.08f, 0.08f));
    scene.lights.push_back(ambient);
    scene.numLights++;

    PointLight *p1 = new PointLight(RGB(200.f, 200.f, 200.f),
                                     Point(-1.f, 3.f, 0.f));
    scene.lights.push_back(p1);
    scene.numLights++;
}