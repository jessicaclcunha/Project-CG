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
    // COBRE: metal condutor — F0 derivado de Kd (albedo), metallic=1.0
    RGB const Kd(0.95f, 0.64f, 0.54f);  // albedo do cobre = F0
    RGB const Ks(1.0f, 1.0f, 1.0f);
    RGB const Ka(0.02f, 0.02f, 0.02f);
    float const metallic = 1.0f;

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

    // --- Face A: metal ---

    /* OURO: metal condutor */
    RGB   const KdA(1.00f, 0.71f, 0.29f);  // albedo do ouro = F0
    float const roughA  = 0.7f;
    float const metalA  = 1.0f;

    /* PRATA: metal condutor
    RGB   const KdA(0.95f, 0.93f, 0.88f);
    float const roughA  = 0.15f;
    float const metalA  = 1.0f;
    */
    /* COBRE: metal condutor
    RGB   const KdA(0.95f, 0.64f, 0.54f);
    float const roughA  = 0.2f;
    float const metalA  = 1.0f;
    */

    // --- Face B: dielétrico ---

    /* PLÁSTICO VERMELHO: dielétrico */
    RGB   const KdB(0.8f,  0.1f,  0.1f);
    float const roughB  = 0.6f;
    float const metalB  = 0.0f;

    /* CERÂMICA AZUL: dielétrico
    RGB   const KdB(0.1f,  0.2f,  0.6f);
    float const roughB  = 0.4f;
    float const metalB  = 0.0f;
    */
    /* BORRACHA: dielétrico
    RGB   const KdB(0.02f, 0.02f, 0.02f);
    float const roughB  = 0.9f;
    float const metalB  = 0.0f;
    */
    /* SAFIRA: dielétrico
    RGB   const KdB(0.01f, 0.02f, 0.35f);
    float const roughB  = 0.2f;
    float const metalB  = 0.0f;
    */

    int const matA = AddCookTorranceMat(scene, Ka, KdA, Ks, roughA, metalA);
    int const matB = AddCookTorranceMat(scene, Ka, KdB, Ks, roughB, metalB);

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

    Vector lightN(0.f, -1.f, 0.f);
    AreaLight *a1 = new AreaLight(RGB(80.f, 80.f, 80.f),
        Point(-2.f, 3.f, 1.f), Point(2.f, 3.f, 1.f), Point(2.f, 3.f, 5.f), lightN);
    scene.lights.push_back(a1);
    scene.numLights++;
    AreaLight *a2 = new AreaLight(RGB(80.f, 80.f, 80.f),
        Point(-2.f, 3.f, 1.f), Point(2.f, 3.f, 5.f), Point(-2.f, 3.f, 5.f), lightN);
    scene.lights.push_back(a2);
    scene.numLights++;

    PointLight *fill = new PointLight(RGB(30.f, 30.f, 30.f), Point(-3.f, 1.f, 0.f));
    scene.lights.push_back(fill);
    scene.numLights++;
}

void CookTorranceJustOneThing (Scene& scene) {

    /* OURO: metal condutor */
    RGB const Kd(1.00f, 0.71f, 0.29f);  // albedo do ouro = F0
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Ka(0.02f, 0.02f, 0.02f);
    float const roughness = 0.5f;
    float const metallic  = 1.0f;

    /* PRATA: metal condutor
    RGB const Kd(0.95f, 0.93f, 0.88f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Ka(0.05f, 0.05f, 0.05f);
    float const roughness = 0.15f;
    float const metallic  = 1.0f;
    */
    /* COBRE: metal condutor
    RGB const Kd(0.95f, 0.64f, 0.54f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Ka(0.03f, 0.01f, 0.01f);
    float const roughness = 0.2f;
    float const metallic  = 1.0f;
    */
    /* SAFIRA: dielétrico
    RGB const Kd(0.01f, 0.02f, 0.35f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Ka(0.01f, 0.01f, 0.03f);
    float const roughness = 0.2f;
    float const metallic  = 0.0f;
    */
    /* PLÁSTICO: dielétrico
    RGB const Kd(0.8f,  0.1f,  0.1f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Ka(0.05f, 0.01f, 0.01f);
    float const roughness = 0.3f;
    float const metallic  = 0.0f;
    */
    /* BORRACHA: dielétrico
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
    brdf->Ks        = RGB(0., 0., 0.);
    brdf->Ks_brdf   = Ks;
    brdf->Kt        = RGB(0., 0., 0.);
    brdf->roughness = roughness;
    brdf->metallic  = metallic;
    return (scene.AddMaterial(brdf));
}

void CookTorranceShowcase (Scene& scene) {
    // Grade 2x4: linhas = metallic vs dielétrico, colunas = roughness
    // roughness: 0.05  0.2   0.5   0.9
    // linha 0 (topo):  ouro,  metallic=1.0
    // linha 1 (baixo): plástico azul, metallic=0.0

    RGB const Ks(1.0f, 1.0f, 1.0f);
    RGB const Ka(0.02f, 0.02f, 0.02f);

    RGB const Kd_metal(1.00f, 0.71f, 0.29f);   // albedo do ouro = F0
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);   // plástico azul

    float const roughness[4] = { 0.05f, 0.2f, 0.5f, 0.9f };

    // criar materiais
    int mat_metal[4], mat_diel[4];
    for (int i = 0; i < 4; i++) {
        mat_metal[i] = AddCookTorranceMat(scene, Ka, Kd_metal, Ks, roughness[i], 1.0f);
        mat_diel [i] = AddCookTorranceMat(scene, Ka, Kd_diel,  Ks, roughness[i], 0.0f);
    }

    // posições X das 4 colunas, centradas em 0
    float const xs[4] = { -3.f, -1.f, 1.f, 3.f };
    float const radius = 0.7f;
    float const z = 3.f;

    for (int i = 0; i < 4; i++) {
        AddSphere(scene, Point(xs[i],  1.1f, z), radius, mat_metal[i]);  // linha de cima
        AddSphere(scene, Point(xs[i], -1.1f, z), radius, mat_diel [i]);  // linha de baixo
    }

    AmbientLight *ambient = new AmbientLight(RGB(0.04f, 0.04f, 0.04f));
    scene.lights.push_back(ambient);
    scene.numLights++;

    PointLight *p1 = new PointLight(RGB(300.f, 300.f, 300.f), Point(-1.f, 5.f, 0.f));
    scene.lights.push_back(p1);
    scene.numLights++;
}

void CookTorranceTextureScene (Scene& scene) {
    RGB const Ks(1.0f, 1.0f, 1.0f);
    float const roughness = 0.3f;
    float const metallic  = 0.0f;  // dielétrico

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

    AddSphere(scene, Point(-2.f, 0.8f, 5.f), 0.7f, plain_ct);
    AddSphere(scene, Point( 0.f, 0.8f, 5.f), 0.7f, tex_dog_ct);
    AddSphere(scene, Point( 2.f, 0.8f, 5.f), 0.7f, tex_um_ct);

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
