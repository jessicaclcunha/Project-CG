#include "BuildScenes.hpp"
#include "SceneHelpers.hpp"
#include "CookTorranceNoEC.hpp"

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

// Key AreaLight pequena (y=5) + PointLight fill lateral + ambient + chão escuro.
// x0/x1, z0/z1 definem os limites da key; keyPower a sua potência.
static void setCTLighting (Scene& scene,
                            float x0, float x1,
                            float z0, float z1,
                            float keyPower,
                            float floorY = -0.8f) {
    Vector nDown(0.f, -1.f, 0.f);
    AreaLight *key0 = new AreaLight(RGB(keyPower,keyPower,keyPower),
        Point(x0,5.f,z0), Point(x1,5.f,z0), Point(x1,5.f,z1), nDown);
    AreaLight *key1 = new AreaLight(RGB(keyPower,keyPower,keyPower),
        Point(x0,5.f,z0), Point(x1,5.f,z1), Point(x0,5.f,z1), nDown);
    scene.lights.push_back(key0); scene.numLights++;
    scene.lights.push_back(key1); scene.numLights++;

    PointLight *fill = new PointLight(RGB(100.f,100.f,100.f), Point(-5.f,2.f,1.f));
    scene.lights.push_back(fill); scene.numLights++;

    AmbientLight *ambient = new AmbientLight(RGB(0.02f,0.02f,0.02f));
    scene.lights.push_back(ambient); scene.numLights++;

    int floor_mat = AddDiffuseMat(scene, RGB(0.07f,0.07f,0.07f));
    AddTriangle(scene, Point(-6.f,floorY,-2.f), Point(6.f,floorY,-2.f), Point(6.f,floorY,8.f), floor_mat);
    AddTriangle(scene, Point(-6.f,floorY,-2.f), Point(6.f,floorY,8.f), Point(-6.f,floorY,8.f), floor_mat);
}


void CookTorranceSphereScene (Scene& scene) {
    // 4 metais condutores a roughness fixo (0.3): isola o efeito do Kd/F0
    // cromáticos (gold, copper) vs neutros (silver, iron)
    RGB const Ks(1.0f, 1.0f, 1.0f);
    RGB const Ka(0.02f, 0.02f, 0.02f);
    float const roughness = 0.3f;

    int const gold   = AddCookTorranceMat(scene, Ka, RGB(1.00f, 0.71f, 0.29f), Ks, roughness, 1.0f);
    int const copper = AddCookTorranceMat(scene, Ka, RGB(0.95f, 0.64f, 0.54f), Ks, roughness, 1.0f);
    int const silver = AddCookTorranceMat(scene, Ka, RGB(0.95f, 0.93f, 0.88f), Ks, roughness, 1.0f);
    int const iron   = AddCookTorranceMat(scene, Ka, RGB(0.56f, 0.57f, 0.58f), Ks, roughness, 1.0f);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, gold);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, copper);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, silver);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, iron);

    setCTLighting(scene, -1.5f, 1.5f, 2.f, 4.f, 400.f);
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
    // Grade 3x4: linhas = metallic (1.0 / 0.5 / 0.0), colunas = roughness
    // roughness: 0.05  0.2   0.5   0.9
    // linha 0 (topo):  ouro,  metallic=1.0
    // linha 1 (meio):  ouro Kd, metallic=0.5 — isola o efeito do metallic
    // linha 2 (baixo): plastico azul, metallic=0.0

    RGB const Ks(1.0f, 1.0f, 1.0f);
    RGB const Ka(0.02f, 0.02f, 0.02f);

    RGB const Kd_metal(1.00f, 0.71f, 0.29f);   // albedo do ouro = F0
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);   // plastico azul

    float const roughness[4] = { 0.05f, 0.2f, 0.5f, 0.9f };

    // criar materiais
    int mat_metal[4], mat_semi[4], mat_diel[4];
    for (int i = 0; i < 4; i++) {
        mat_metal[i] = AddCookTorranceMat(scene, Ka, Kd_metal, Ks, roughness[i], 1.0f);
        mat_semi [i] = AddCookTorranceMat(scene, Ka, Kd_metal, Ks, roughness[i], 0.5f);
        mat_diel [i] = AddCookTorranceMat(scene, Ka, Kd_diel,  Ks, roughness[i], 0.0f);
    }

    // posicoes X das 4 colunas, centradas em 0
    float const xs[4] = { -3.f, -1.f, 1.f, 3.f };
    float const radius = 0.7f;
    float const z = 3.f;

    for (int i = 0; i < 4; i++) {
        AddSphere(scene, Point(xs[i],  2.2f, z), radius, mat_metal[i]);  // linha de cima
        AddSphere(scene, Point(xs[i],  0.0f, z), radius, mat_semi [i]);  // linha do meio
        AddSphere(scene, Point(xs[i], -2.2f, z), radius, mat_diel [i]);  // linha de baixo
    }

    // floorY=-3.1: base da esfera de baixo = -2.2 - 0.7 = -2.9; floor a -3.1 esta abaixo
    setCTLighting(scene, -2.f, 2.f, 0.5f, 5.5f, 500.f, -3.1f);
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

// -------------------------------------------------------------------------
// Cena de teste: conservação de energia (EC)
// -------------------------------------------------------------------------

static int AddCookTorranceNoECMat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks,
                                    float const roughness, float const metallic) {
    CookTorranceNoEC *brdf = new CookTorranceNoEC;
    brdf->Ka        = Ka;
    brdf->Kd        = Kd;
    brdf->Ks        = RGB(0.f, 0.f, 0.f);
    brdf->Ks_brdf   = Ks;
    brdf->Kt        = RGB(0.f, 0.f, 0.f);
    brdf->roughness = roughness;
    brdf->metallic  = metallic;
    return (scene.AddMaterial(brdf));
}

// -------------------------------------------------------------------------
// Cena melhorada: iluminação direccional para maximizar legibilidade do EC.
//
// Problema da cena 1: 8 AreaLights enormes → iluminação omnidireccional
// → especular lavado, zonas claras e escuras indiferenciadas.
//
// Solução: 1 AreaLight pequena (key, 3×2 u²) + 1 PointLight lateral (fill).
// Resultado: highlight nítido vs zona escura claramente separados —
// o metal sem difuso fica visivelmente escuro fora do highlight;
// o dielétrico liso mostra o Fresnel grazing no bordo iluminado.
// -------------------------------------------------------------------------

static void addECTestLighting2 (Scene& scene) {
    // Key light: AreaLight 3×2 u² centrada por cima das 4 esferas.
    // Intensidade ≈ 400/6 ≈ 67 u⁻² vs ≈ 2 u⁻² da cena 1 → highlight 30× mais nítido.
    Vector nDown(0.f, -1.f, 0.f);
    AreaLight *key0 = new AreaLight(RGB(400.f,400.f,400.f),
        Point(-1.5f,5.f,2.f), Point(1.5f,5.f,2.f), Point(1.5f,5.f,4.f), nDown);
    AreaLight *key1 = new AreaLight(RGB(400.f,400.f,400.f),
        Point(-1.5f,5.f,2.f), Point(1.5f,5.f,4.f), Point(-1.5f,5.f,4.f), nDown);
    scene.lights.push_back(key0); scene.numLights++;
    scene.lights.push_back(key1); scene.numLights++;

    // Fill light: PointLight lateral esquerdo — ilumina a face lateral das esferas
    // e torna o Fresnel grazing-angle visível sem apagar as sombras da key.
    PointLight *fill = new PointLight(RGB(100.f,100.f,100.f), Point(-5.f, 2.f, 1.f));
    scene.lights.push_back(fill); scene.numLights++;

    // Ambient mínimo — evita preto absoluto nas zonas sem luz directa.
    AmbientLight *ambient = new AmbientLight(RGB(0.02f, 0.02f, 0.02f));
    scene.lights.push_back(ambient); scene.numLights++;

    // Chão escuro (sem paredes — fundo dado pelo shader RGB 0.05,0.05,0.1).
    int floor_mat = AddDiffuseMat(scene, RGB(0.07f, 0.07f, 0.07f));
    AddTriangle(scene, Point(-6.f,-0.8f,-2.f), Point(6.f,-0.8f,-2.f), Point(6.f,-0.8f,8.f), floor_mat);
    AddTriangle(scene, Point(-6.f,-0.8f,-2.f), Point(6.f,-0.8f,8.f), Point(-6.f,-0.8f,8.f), floor_mat);
}

void CookTorranceTestStandart (Scene& scene) {
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);

    int m1 = AddCookTorranceMat(scene, Ka, Kd_diel,  Ks, 0.1f, 0.0f);
    int m2 = AddCookTorranceMat(scene, Ka, Kd_diel,  Ks, 0.6f, 0.0f);
    int m3 = AddCookTorranceMat(scene, Ka, Kd_metal, Ks, 0.3f, 1.0f);
    int m4 = AddCookTorranceMat(scene, Ka, Kd_diel,  Ks, 0.7f, 0.5f);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, m1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, m2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, m3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, m4);

    addECTestLighting2(scene);
}

// -------------------------------------------------------------------------
// Cenas de teste: variantes do Fresnel
// -------------------------------------------------------------------------

static int AddFConstMat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks,
                          float const roughness, float const metallic) {
    CookTorranceFConst *brdf = new CookTorranceFConst;
    brdf->Ka = Ka; brdf->Kd = Kd; brdf->Ks = RGB(0.f,0.f,0.f);
    brdf->Ks_brdf = Ks; brdf->Kt = RGB(0.f,0.f,0.f);
    brdf->roughness = roughness; brdf->metallic = metallic;
    return scene.AddMaterial(brdf);
}

static int AddFExpMat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks,
                        float const roughness, float const metallic) {
    CookTorranceFExp *brdf = new CookTorranceFExp;
    brdf->Ka = Ka; brdf->Kd = Kd; brdf->Ks = RGB(0.f,0.f,0.f);
    brdf->Ks_brdf = Ks; brdf->Kt = RGB(0.f,0.f,0.f);
    brdf->roughness = roughness; brdf->metallic = metallic;
    return scene.AddMaterial(brdf);
}

static int AddFInvMat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks,
                        float const roughness, float const metallic) {
    CookTorranceFInv *brdf = new CookTorranceFInv;
    brdf->Ka = Ka; brdf->Kd = Kd; brdf->Ks = RGB(0.f,0.f,0.f);
    brdf->Ks_brdf = Ks; brdf->Kt = RGB(0.f,0.f,0.f);
    brdf->roughness = roughness; brdf->metallic = metallic;
    return scene.AddMaterial(brdf);
}

void CookTorranceFConstTest (Scene& scene) {
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);

    int m1 = AddFConstMat(scene, Ka, Kd_diel,  Ks, 0.1f, 0.0f);
    int m2 = AddFConstMat(scene, Ka, Kd_diel,  Ks, 0.6f, 0.0f);
    int m3 = AddFConstMat(scene, Ka, Kd_metal, Ks, 0.3f, 1.0f);
    int m4 = AddFConstMat(scene, Ka, Kd_diel,  Ks, 0.7f, 0.5f);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, m1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, m2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, m3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, m4);

    addECTestLighting2(scene);
}

void CookTorranceFExpTest (Scene& scene) {
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);

    int m1 = AddFExpMat(scene, Ka, Kd_diel,  Ks, 0.1f, 0.0f);
    int m2 = AddFExpMat(scene, Ka, Kd_diel,  Ks, 0.6f, 0.0f);
    int m3 = AddFExpMat(scene, Ka, Kd_metal, Ks, 0.3f, 1.0f);
    int m4 = AddFExpMat(scene, Ka, Kd_diel,  Ks, 0.7f, 0.5f);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, m1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, m2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, m3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, m4);

    addECTestLighting2(scene);
}

void CookTorranceFInvTest (Scene& scene) {
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);

    int m1 = AddFInvMat(scene, Ka, Kd_diel,  Ks, 0.1f, 0.0f);
    int m2 = AddFInvMat(scene, Ka, Kd_diel,  Ks, 0.6f, 0.0f);
    int m3 = AddFInvMat(scene, Ka, Kd_metal, Ks, 0.3f, 1.0f);
    int m4 = AddFInvMat(scene, Ka, Kd_diel,  Ks, 0.7f, 0.5f);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, m1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, m2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, m3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, m4);

    addECTestLighting2(scene);
}

void CookTorranceNoECTest2 (Scene& scene) {
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);

    int m1 = AddCookTorranceNoECMat(scene, Ka, Kd_diel,  Ks, 0.1f, 0.0f);
    int m2 = AddCookTorranceNoECMat(scene, Ka, Kd_diel,  Ks, 0.6f, 0.0f);
    int m3 = AddCookTorranceNoECMat(scene, Ka, Kd_metal, Ks, 0.3f, 1.0f);
    int m4 = AddCookTorranceNoECMat(scene, Ka, Kd_diel,  Ks, 0.7f, 0.5f);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, m1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, m2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, m3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, m4);

    addECTestLighting2(scene);
}

// -------------------------------------------------------------------------
// Cenas de teste: mistura difuso/especular nao fisica (kD alternativo)
// -------------------------------------------------------------------------

static int AddKDMetalMat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks,
                           float const roughness, float const metallic) {
    CookTorranceKDMetal *brdf = new CookTorranceKDMetal;
    brdf->Ka = Ka; brdf->Kd = Kd; brdf->Ks = RGB(0.f,0.f,0.f);
    brdf->Ks_brdf = Ks; brdf->Kt = RGB(0.f,0.f,0.f);
    brdf->roughness = roughness; brdf->metallic = metallic;
    return scene.AddMaterial(brdf);
}

static int AddKDLerpMat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks,
                          float const roughness, float const metallic) {
    CookTorranceKDLerp *brdf = new CookTorranceKDLerp;
    brdf->Ka = Ka; brdf->Kd = Kd; brdf->Ks = RGB(0.f,0.f,0.f);
    brdf->Ks_brdf = Ks; brdf->Kt = RGB(0.f,0.f,0.f);
    brdf->roughness = roughness; brdf->metallic = metallic;
    return scene.AddMaterial(brdf);
}

void CookTorranceKDMetalTest (Scene& scene) {
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);

    int m1 = AddKDMetalMat(scene, Ka, Kd_diel,  Ks, 0.1f, 0.0f);
    int m2 = AddKDMetalMat(scene, Ka, Kd_diel,  Ks, 0.6f, 0.0f);
    int m3 = AddKDMetalMat(scene, Ka, Kd_metal, Ks, 0.3f, 1.0f);
    int m4 = AddKDMetalMat(scene, Ka, Kd_diel,  Ks, 0.7f, 0.5f);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, m1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, m2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, m3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, m4);

    addECTestLighting2(scene);
}

void CookTorranceKDLerpTest (Scene& scene) {
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);

    int m1 = AddKDLerpMat(scene, Ka, Kd_diel,  Ks, 0.1f, 0.0f);
    int m2 = AddKDLerpMat(scene, Ka, Kd_diel,  Ks, 0.6f, 0.0f);
    int m3 = AddKDLerpMat(scene, Ka, Kd_metal, Ks, 0.3f, 1.0f);
    int m4 = AddKDLerpMat(scene, Ka, Kd_diel,  Ks, 0.7f, 0.5f);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, m1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, m2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, m3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, m4);

    addECTestLighting2(scene);
}

// -------------------------------------------------------------------------
// Cenas de teste: modificadores do termo G (shadowing-masking)
// -------------------------------------------------------------------------

static int AddGNoneMat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks,
                         float const roughness, float const metallic) {
    CookTorranceGNone *brdf = new CookTorranceGNone;
    brdf->Ka = Ka; brdf->Kd = Kd; brdf->Ks = RGB(0.f,0.f,0.f);
    brdf->Ks_brdf = Ks; brdf->Kt = RGB(0.f,0.f,0.f);
    brdf->roughness = roughness; brdf->metallic = metallic;
    return scene.AddMaterial(brdf);
}

static int AddGOneMat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks,
                        float const roughness, float const metallic) {
    CookTorranceGOne *brdf = new CookTorranceGOne;
    brdf->Ka = Ka; brdf->Kd = Kd; brdf->Ks = RGB(0.f,0.f,0.f);
    brdf->Ks_brdf = Ks; brdf->Kt = RGB(0.f,0.f,0.f);
    brdf->roughness = roughness; brdf->metallic = metallic;
    return scene.AddMaterial(brdf);
}

static int AddGKelemenMat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks,
                            float const roughness, float const metallic) {
    CookTorranceGKelemen *brdf = new CookTorranceGKelemen;
    brdf->Ka = Ka; brdf->Kd = Kd; brdf->Ks = RGB(0.f,0.f,0.f);
    brdf->Ks_brdf = Ks; brdf->Kt = RGB(0.f,0.f,0.f);
    brdf->roughness = roughness; brdf->metallic = metallic;
    return scene.AddMaterial(brdf);
}

void CookTorranceGNoneTest (Scene& scene) {
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);

    int m1 = AddGNoneMat(scene, Ka, Kd_diel,  Ks, 0.1f, 0.0f);
    int m2 = AddGNoneMat(scene, Ka, Kd_diel,  Ks, 0.6f, 0.0f);
    int m3 = AddGNoneMat(scene, Ka, Kd_metal, Ks, 0.3f, 1.0f);
    int m4 = AddGNoneMat(scene, Ka, Kd_diel,  Ks, 0.7f, 0.5f);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, m1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, m2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, m3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, m4);

    addECTestLighting2(scene);
}

void CookTorranceGOneTest (Scene& scene) {
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);

    int m1 = AddGOneMat(scene, Ka, Kd_diel,  Ks, 0.1f, 0.0f);
    int m2 = AddGOneMat(scene, Ka, Kd_diel,  Ks, 0.6f, 0.0f);
    int m3 = AddGOneMat(scene, Ka, Kd_metal, Ks, 0.3f, 1.0f);
    int m4 = AddGOneMat(scene, Ka, Kd_diel,  Ks, 0.7f, 0.5f);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, m1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, m2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, m3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, m4);

    addECTestLighting2(scene);
}

void CookTorranceGKelemenTest (Scene& scene) {
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);

    int m1 = AddGKelemenMat(scene, Ka, Kd_diel,  Ks, 0.1f, 0.0f);
    int m2 = AddGKelemenMat(scene, Ka, Kd_diel,  Ks, 0.6f, 0.0f);
    int m3 = AddGKelemenMat(scene, Ka, Kd_metal, Ks, 0.3f, 1.0f);
    int m4 = AddGKelemenMat(scene, Ka, Kd_diel,  Ks, 0.7f, 0.5f);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, m1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, m2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, m3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, m4);

    addECTestLighting2(scene);
}

// -------------------------------------------------------------------------
// Estudo: alternativas correctas de D (Normal Distribution Function)
// -------------------------------------------------------------------------

static int AddDBeckmannMat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks,
                             float const roughness, float const metallic) {
    CookTorranceDBeckmann *brdf = new CookTorranceDBeckmann;
    brdf->Ka = Ka; brdf->Kd = Kd; brdf->Ks = RGB(0.f,0.f,0.f);
    brdf->Ks_brdf = Ks; brdf->Kt = RGB(0.f,0.f,0.f);
    brdf->roughness = roughness; brdf->metallic = metallic;
    return scene.AddMaterial(brdf);
}

static int AddDBlinnPhongMat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks,
                               float const roughness, float const metallic) {
    CookTorranceDBlinnPhong *brdf = new CookTorranceDBlinnPhong;
    brdf->Ka = Ka; brdf->Kd = Kd; brdf->Ks = RGB(0.f,0.f,0.f);
    brdf->Ks_brdf = Ks; brdf->Kt = RGB(0.f,0.f,0.f);
    brdf->roughness = roughness; brdf->metallic = metallic;
    return scene.AddMaterial(brdf);
}

void CookTorranceDBeckmannTest (Scene& scene) {
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);

    int m1 = AddDBeckmannMat(scene, Ka, Kd_diel,  Ks, 0.1f, 0.0f);
    int m2 = AddDBeckmannMat(scene, Ka, Kd_diel,  Ks, 0.6f, 0.0f);
    int m3 = AddDBeckmannMat(scene, Ka, Kd_metal, Ks, 0.3f, 1.0f);
    int m4 = AddDBeckmannMat(scene, Ka, Kd_diel,  Ks, 0.7f, 0.5f);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, m1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, m2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, m3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, m4);

    addECTestLighting2(scene);
}

void CookTorranceDBlinnPhongTest (Scene& scene) {
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);

    int m1 = AddDBlinnPhongMat(scene, Ka, Kd_diel,  Ks, 0.1f, 0.0f);
    int m2 = AddDBlinnPhongMat(scene, Ka, Kd_diel,  Ks, 0.6f, 0.0f);
    int m3 = AddDBlinnPhongMat(scene, Ka, Kd_metal, Ks, 0.3f, 1.0f);
    int m4 = AddDBlinnPhongMat(scene, Ka, Kd_diel,  Ks, 0.7f, 0.5f);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, m1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, m2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, m3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, m4);

    addECTestLighting2(scene);
}

// -------------------------------------------------------------------------
// Estudo: alternativas correctas de F (Fresnel)
// -------------------------------------------------------------------------

static int AddFExactMat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks,
                          float const roughness, float const eta_val) {
    CookTorranceFExact *brdf = new CookTorranceFExact;
    brdf->Ka = Ka; brdf->Kd = Kd; brdf->Ks = RGB(0.f,0.f,0.f);
    brdf->Ks_brdf = Ks; brdf->Kt = RGB(0.f,0.f,0.f);
    brdf->roughness = roughness; brdf->metallic = 0.f;
    brdf->eta = eta_val;
    return scene.AddMaterial(brdf);
}

static int AddFSGMat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks,
                       float const roughness, float const metallic) {
    CookTorranceFSG *brdf = new CookTorranceFSG;
    brdf->Ka = Ka; brdf->Kd = Kd; brdf->Ks = RGB(0.f,0.f,0.f);
    brdf->Ks_brdf = Ks; brdf->Kt = RGB(0.f,0.f,0.f);
    brdf->roughness = roughness; brdf->metallic = metallic;
    return scene.AddMaterial(brdf);
}

void CookTorranceFExactTest (Scene& scene) {
    // 4 dieléctricos (metallic=0), roughness fixo=0.3, IOR variado
    // Isola o efeito do índice de refracção no Fresnel exacto
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Kd(0.10f, 0.20f, 0.80f);

    int m1 = AddFExactMat(scene, Ka, Kd, Ks, 0.3f, 1.33f);  // água
    int m2 = AddFExactMat(scene, Ka, Kd, Ks, 0.3f, 1.50f);  // vidro
    int m3 = AddFExactMat(scene, Ka, Kd, Ks, 0.3f, 1.77f);  // safira
    int m4 = AddFExactMat(scene, Ka, Kd, Ks, 0.3f, 2.50f);  // IOR alto

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, m1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, m2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, m3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, m4);

    addECTestLighting2(scene);
}

void CookTorranceFSGTest (Scene& scene) {
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);

    int m1 = AddFSGMat(scene, Ka, Kd_diel,  Ks, 0.1f, 0.0f);
    int m2 = AddFSGMat(scene, Ka, Kd_diel,  Ks, 0.6f, 0.0f);
    int m3 = AddFSGMat(scene, Ka, Kd_metal, Ks, 0.3f, 1.0f);
    int m4 = AddFSGMat(scene, Ka, Kd_diel,  Ks, 0.7f, 0.5f);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, m1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, m2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, m3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, m4);

    addECTestLighting2(scene);
}

// -------------------------------------------------------------------------
// Estudo: alternativas correctas de G (Geometry / Shadowing-Masking)
// -------------------------------------------------------------------------

static int AddGCT1982Mat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks,
                           float const roughness, float const metallic) {
    CookTorranceGCT1982 *brdf = new CookTorranceGCT1982;
    brdf->Ka = Ka; brdf->Kd = Kd; brdf->Ks = RGB(0.f,0.f,0.f);
    brdf->Ks_brdf = Ks; brdf->Kt = RGB(0.f,0.f,0.f);
    brdf->roughness = roughness; brdf->metallic = metallic;
    return scene.AddMaterial(brdf);
}

static int AddGSmithIBLMat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks,
                              float const roughness, float const metallic) {
    CookTorranceGSmithIBL *brdf = new CookTorranceGSmithIBL;
    brdf->Ka = Ka; brdf->Kd = Kd; brdf->Ks = RGB(0.f,0.f,0.f);
    brdf->Ks_brdf = Ks; brdf->Kt = RGB(0.f,0.f,0.f);
    brdf->roughness = roughness; brdf->metallic = metallic;
    return scene.AddMaterial(brdf);
}

void CookTorranceGCT1982Test (Scene& scene) {
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);

    int m1 = AddGCT1982Mat(scene, Ka, Kd_diel,  Ks, 0.1f, 0.0f);
    int m2 = AddGCT1982Mat(scene, Ka, Kd_diel,  Ks, 0.6f, 0.0f);
    int m3 = AddGCT1982Mat(scene, Ka, Kd_metal, Ks, 0.3f, 1.0f);
    int m4 = AddGCT1982Mat(scene, Ka, Kd_diel,  Ks, 0.7f, 0.5f);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, m1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, m2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, m3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, m4);

    addECTestLighting2(scene);
}

void CookTorranceGSmithIBLTest (Scene& scene) {
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);

    int m1 = AddGSmithIBLMat(scene, Ka, Kd_diel,  Ks, 0.1f, 0.0f);
    int m2 = AddGSmithIBLMat(scene, Ka, Kd_diel,  Ks, 0.6f, 0.0f);
    int m3 = AddGSmithIBLMat(scene, Ka, Kd_metal, Ks, 0.3f, 1.0f);
    int m4 = AddGSmithIBLMat(scene, Ka, Kd_diel,  Ks, 0.7f, 0.5f);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, m1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, m2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, m3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, m4);

    addECTestLighting2(scene);
}


void CookTorranceTextureStandart (Scene& scene) {
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Ks(1.0f,  1.0f,  1.0f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);
 
    // --- Linha de cima: materiais originais (cor plana) ---
    int m1 = AddCookTorranceMat(scene, Ka, Kd_diel,  Ks, 0.1f, 0.0f);
    int m2 = AddCookTorranceMat(scene, Ka, Kd_metal, Ks, 0.3f, 1.0f);
    int m3 = AddCookTorranceMat(scene, Ka, Kd_diel,  Ks, 0.6f, 0.0f);
    int m4 = AddCookTorranceMat(scene, Ka, Kd_diel,  Ks, 0.7f, 0.5f);
 
    // --- Linha de baixo: mesmos materiais com Dog.ppm ---
    // Kd=1 para a textura não ser atenuada
    // roughness e metallic IDÊNTICOS aos originais, exceto t2 (metallic 1.0->0.5)
    // para evitar esfera preta com point light única
    int t1 = AddCookTorranceTexMat(scene, "Dog.ppm", Ka, RGB(1.f,1.f,1.f), Ks, 0.1f, 0.0f);
    int t2 = AddCookTorranceTexMat(scene, "Dog.ppm", Ka, RGB(1.f,1.f,1.f), Ks, 0.3f, 0.5f);
    int t3 = AddCookTorranceTexMat(scene, "Dog.ppm", Ka, RGB(1.f,1.f,1.f), Ks, 0.6f, 0.0f);
    int t4 = AddCookTorranceTexMat(scene, "Dog.ppm", Ka, RGB(1.f,1.f,1.f), Ks, 0.7f, 0.5f);
 
    float const xs[4] = { -3.f, -1.f, 1.f, 3.f };
    float const radius = 0.8f;
    float const z = 3.f;
    int plain[4] = { m1, m2, m3, m4 };
    int tex  [4] = { t1, t2, t3, t4 };
 
    for (int i = 0; i < 4; i++) {
        AddSphere(scene, Point(xs[i],  2.2f, z), radius, plain[i]); // linha de cima
        AddSphere(scene, Point(xs[i],  0.2f, z), radius, tex  [i]); // linha de baixo
    }
 
    // Iluminação igual à CookTorranceTestStandart
    addECTestLighting2(scene);
}