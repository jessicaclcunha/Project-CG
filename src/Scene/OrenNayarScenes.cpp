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

// DEMO de contraste Lambert vs Oren-Nayar — o efeito "lua cheia".
// LUZ DE FAROL: point light praticamente na posição da câmara (0,0.5,-5) →
// luz ≈ vista → retroreflexão máxima. SEM fill (qualquer luz lateral levanta
// o bordo e mata o efeito). 3 esferas grandes:
//   σ=0   (Lambert): centro claro, bordo escuro → BOLA 3D
//   σ=0.5 (ON médio): bordo começa a levantar
//   σ=0.9 (ON rugoso): bordo tão claro como o centro → DISCO PLANO ("lua")
void OrenNayarLambertVsON (Scene& scene) {
    RGB const Kd(0.7f, 0.5f, 0.3f);   // argila
    RGB const Ka(0.02f, 0.01f, 0.01f);

    int const lamb = AddOrenNayarMat(scene, Ka, Kd, 0.0f);  // Lambert
    int const mid  = AddOrenNayarMat(scene, Ka, Kd, 0.5f);  // ON médio
    int const on   = AddOrenNayarMat(scene, Ka, Kd, 0.9f);  // ON rugoso

    AddSphere(scene, Point(-2.5f, 0.f, 3.f), 1.0f, lamb);
    AddSphere(scene, Point( 0.0f, 0.f, 3.f), 1.0f, mid);
    AddSphere(scene, Point( 2.5f, 0.f, 3.f), 1.0f, on);

    // Farol: na direcção da câmara (z muito negativo, x=y alinhados com Eye)
    PointLight *head = new PointLight(RGB(260.f,260.f,260.f), Point(0.f, 0.5f, -4.5f));
    scene.lights.push_back(head); scene.numLights++;

    // Ambient mínimo — SEM fill (fill mataria a retroreflexão)
    AmbientLight *ambient = new AmbientLight(RGB(0.04f, 0.04f, 0.04f));
    scene.lights.push_back(ambient); scene.numLights++;
}

// =========================================================================
// ESTUDO Oren-Nayar — variantes erradas + alternativas correctas
// Metodologia idêntica ao CookTorrance: 4 esferas, sigma 0.3/0.5/0.7/0.9,
// mesmo Kd e iluminação fixa (addONTestLighting). Comparar cada variante
// com OrenNayarTestStandart via RMSE/compare.sh.
// =========================================================================

// Iluminação partilhada por todas as cenas do estudo.
// LUZ ANGULAR (~30° da câmara, lado da câmara z<0, deslocada para cima-esquerda).
// Escolhida para medir TODAS as variantes: por estar do lado da câmara há
// back-scatter no limbo (revela NoB, C2 do Full); por estar em ângulo cria um
// terminador no lado oposto → forward-scatter (revela NoClamp, C3). Um farol
// puro mediria a NoB mas anularia a NoClamp; um lateral puro o inverso.
// SEM fill (a luz angular já dá zona clara e escura).
static void addONTestLighting (Scene& scene) {
    // Key angular: cima-esquerda, do lado da câmara (z<0) → back + forward scatter
    PointLight *key = new PointLight(RGB(220.f, 220.f, 220.f), Point(-2.f, 2.f, -2.f));
    scene.lights.push_back(key); scene.numLights++;

    // Ambient mínimo — evita preto absoluto
    AmbientLight *ambient = new AmbientLight(RGB(0.03f, 0.03f, 0.03f));
    scene.lights.push_back(ambient); scene.numLights++;

    // Chão escuro
    int floor_mat = AddDiffuseMat(scene, RGB(0.07f, 0.07f, 0.07f));
    AddTriangle(scene, Point(-6.f,-0.8f,-2.f), Point(6.f,-0.8f,-2.f), Point(6.f,-0.8f,8.f), floor_mat);
    AddTriangle(scene, Point(-6.f,-0.8f,-2.f), Point(6.f,-0.8f,8.f), Point(-6.f,-0.8f,8.f), floor_mat);
}

// ------------------------------------------------------------------------
// Helpers de material — um por variante (padrão de AddOrenNayarMat)
// ------------------------------------------------------------------------

static int AddNoBMat (Scene& scene, RGB const Ka, RGB const Kd, float const sigma) {
    OrenNayarNoB *brdf = new OrenNayarNoB;
    brdf->Ka = Ka; brdf->Kd = Kd;
    brdf->Ks = RGB(0.f,0.f,0.f); brdf->Ks_brdf = RGB(0.f,0.f,0.f);
    brdf->Kt = RGB(0.f,0.f,0.f); brdf->sigma = sigma;
    return scene.AddMaterial(brdf);
}

static int AddAFixoMat (Scene& scene, RGB const Ka, RGB const Kd, float const sigma) {
    OrenNayarAFixo *brdf = new OrenNayarAFixo;
    brdf->Ka = Ka; brdf->Kd = Kd;
    brdf->Ks = RGB(0.f,0.f,0.f); brdf->Ks_brdf = RGB(0.f,0.f,0.f);
    brdf->Kt = RGB(0.f,0.f,0.f); brdf->sigma = sigma;
    return scene.AddMaterial(brdf);
}

static int AddNoClampMat (Scene& scene, RGB const Ka, RGB const Kd, float const sigma) {
    OrenNayarNoClamp *brdf = new OrenNayarNoClamp;
    brdf->Ka = Ka; brdf->Kd = Kd;
    brdf->Ks = RGB(0.f,0.f,0.f); brdf->Ks_brdf = RGB(0.f,0.f,0.f);
    brdf->Kt = RGB(0.f,0.f,0.f); brdf->sigma = sigma;
    return scene.AddMaterial(brdf);
}

static int AddFujiiMat (Scene& scene, RGB const Ka, RGB const Kd, float const sigma) {
    OrenNayarFujii *brdf = new OrenNayarFujii;
    brdf->Ka = Ka; brdf->Kd = Kd;
    brdf->Ks = RGB(0.f,0.f,0.f); brdf->Ks_brdf = RGB(0.f,0.f,0.f);
    brdf->Kt = RGB(0.f,0.f,0.f); brdf->sigma = sigma;
    return scene.AddMaterial(brdf);
}

static int AddFullMat (Scene& scene, RGB const Ka, RGB const Kd, float const sigma) {
    OrenNayarFull *brdf = new OrenNayarFull;
    brdf->Ka = Ka; brdf->Kd = Kd;
    brdf->Ks = RGB(0.f,0.f,0.f); brdf->Ks_brdf = RGB(0.f,0.f,0.f);
    brdf->Kt = RGB(0.f,0.f,0.f); brdf->sigma = sigma;
    return scene.AddMaterial(brdf);
}

static int AddFullInterMat (Scene& scene, RGB const Ka, RGB const Kd, float const sigma) {
    OrenNayarFullInter *brdf = new OrenNayarFullInter;
    brdf->Ka = Ka; brdf->Kd = Kd;
    brdf->Ks = RGB(0.f,0.f,0.f); brdf->Ks_brdf = RGB(0.f,0.f,0.f);
    brdf->Kt = RGB(0.f,0.f,0.f); brdf->sigma = sigma;
    return scene.AddMaterial(brdf);
}

// ------------------------------------------------------------------------
// Cenas do estudo — 4 esferas, mesmo Kd, sigma crescente
// ------------------------------------------------------------------------

// Cores partilhadas por todas as cenas do estudo
#define ON_STUDY_KA RGB(0.02f, 0.01f, 0.01f)
#define ON_STUDY_KD RGB(0.7f, 0.5f, 0.3f)   // argila — bom contraste

void OrenNayarTestStandart (Scene& scene) {
    RGB const Ka = ON_STUDY_KA, Kd = ON_STUDY_KD;
    int m1 = AddOrenNayarMat(scene, Ka, Kd, 0.3f);
    int m2 = AddOrenNayarMat(scene, Ka, Kd, 0.5f);
    int m3 = AddOrenNayarMat(scene, Ka, Kd, 0.7f);
    int m4 = AddOrenNayarMat(scene, Ka, Kd, 0.9f);
    AddSphere(scene, Point(-3,0,3), 0.8f, m1);
    AddSphere(scene, Point(-1,0,3), 0.8f, m2);
    AddSphere(scene, Point( 1,0,3), 0.8f, m3);
    AddSphere(scene, Point( 3,0,3), 0.8f, m4);
    addONTestLighting(scene);
}

// Lambert: sigma=0 em todas → Kd/π puro. Diff vs standard = tudo o que o ON acrescenta.
void OrenNayarLambertTest (Scene& scene) {
    RGB const Ka = ON_STUDY_KA, Kd = ON_STUDY_KD;
    int m = AddOrenNayarMat(scene, Ka, Kd, 0.0f);
    AddSphere(scene, Point(-3,0,3), 0.8f, m);
    AddSphere(scene, Point(-1,0,3), 0.8f, m);
    AddSphere(scene, Point( 1,0,3), 0.8f, m);
    AddSphere(scene, Point( 3,0,3), 0.8f, m);
    addONTestLighting(scene);
}

void OrenNayarNoBTest (Scene& scene) {
    RGB const Ka = ON_STUDY_KA, Kd = ON_STUDY_KD;
    int m1 = AddNoBMat(scene, Ka, Kd, 0.3f);
    int m2 = AddNoBMat(scene, Ka, Kd, 0.5f);
    int m3 = AddNoBMat(scene, Ka, Kd, 0.7f);
    int m4 = AddNoBMat(scene, Ka, Kd, 0.9f);
    AddSphere(scene, Point(-3,0,3), 0.8f, m1);
    AddSphere(scene, Point(-1,0,3), 0.8f, m2);
    AddSphere(scene, Point( 1,0,3), 0.8f, m3);
    AddSphere(scene, Point( 3,0,3), 0.8f, m4);
    addONTestLighting(scene);
}

void OrenNayarAFixoTest (Scene& scene) {
    RGB const Ka = ON_STUDY_KA, Kd = ON_STUDY_KD;
    int m1 = AddAFixoMat(scene, Ka, Kd, 0.3f);
    int m2 = AddAFixoMat(scene, Ka, Kd, 0.5f);
    int m3 = AddAFixoMat(scene, Ka, Kd, 0.7f);
    int m4 = AddAFixoMat(scene, Ka, Kd, 0.9f);
    AddSphere(scene, Point(-3,0,3), 0.8f, m1);
    AddSphere(scene, Point(-1,0,3), 0.8f, m2);
    AddSphere(scene, Point( 1,0,3), 0.8f, m3);
    AddSphere(scene, Point( 3,0,3), 0.8f, m4);
    addONTestLighting(scene);
}

void OrenNayarNoClampTest (Scene& scene) {
    RGB const Ka = ON_STUDY_KA, Kd = ON_STUDY_KD;
    int m1 = AddNoClampMat(scene, Ka, Kd, 0.3f);
    int m2 = AddNoClampMat(scene, Ka, Kd, 0.5f);
    int m3 = AddNoClampMat(scene, Ka, Kd, 0.7f);
    int m4 = AddNoClampMat(scene, Ka, Kd, 0.9f);
    AddSphere(scene, Point(-3,0,3), 0.8f, m1);
    AddSphere(scene, Point(-1,0,3), 0.8f, m2);
    AddSphere(scene, Point( 1,0,3), 0.8f, m3);
    AddSphere(scene, Point( 3,0,3), 0.8f, m4);
    addONTestLighting(scene);
}

void OrenNayarFujiiTest (Scene& scene) {
    RGB const Ka = ON_STUDY_KA, Kd = ON_STUDY_KD;
    int m1 = AddFujiiMat(scene, Ka, Kd, 0.3f);
    int m2 = AddFujiiMat(scene, Ka, Kd, 0.5f);
    int m3 = AddFujiiMat(scene, Ka, Kd, 0.7f);
    int m4 = AddFujiiMat(scene, Ka, Kd, 0.9f);
    AddSphere(scene, Point(-3,0,3), 0.8f, m1);
    AddSphere(scene, Point(-1,0,3), 0.8f, m2);
    AddSphere(scene, Point( 1,0,3), 0.8f, m3);
    AddSphere(scene, Point( 3,0,3), 0.8f, m4);
    addONTestLighting(scene);
}

void OrenNayarFullTest (Scene& scene) {
    RGB const Ka = ON_STUDY_KA, Kd = ON_STUDY_KD;
    int m1 = AddFullMat(scene, Ka, Kd, 0.3f);
    int m2 = AddFullMat(scene, Ka, Kd, 0.5f);
    int m3 = AddFullMat(scene, Ka, Kd, 0.7f);
    int m4 = AddFullMat(scene, Ka, Kd, 0.9f);
    AddSphere(scene, Point(-3,0,3), 0.8f, m1);
    AddSphere(scene, Point(-1,0,3), 0.8f, m2);
    AddSphere(scene, Point( 1,0,3), 0.8f, m3);
    AddSphere(scene, Point( 3,0,3), 0.8f, m4);
    addONTestLighting(scene);
}

void OrenNayarFullInterTest (Scene& scene) {
    RGB const Ka = ON_STUDY_KA, Kd = ON_STUDY_KD;
    int m1 = AddFullInterMat(scene, Ka, Kd, 0.3f);
    int m2 = AddFullInterMat(scene, Ka, Kd, 0.5f);
    int m3 = AddFullInterMat(scene, Ka, Kd, 0.7f);
    int m4 = AddFullInterMat(scene, Ka, Kd, 0.9f);
    AddSphere(scene, Point(-3,0,3), 0.8f, m1);
    AddSphere(scene, Point(-1,0,3), 0.8f, m2);
    AddSphere(scene, Point( 1,0,3), 0.8f, m3);
    AddSphere(scene, Point( 3,0,3), 0.8f, m4);
    addONTestLighting(scene);
}

void OrenNayarTextureStandart (Scene& scene) {
    RGB const Ka = ON_STUDY_KA;
    RGB const Kd = ON_STUDY_KD;
 
    // --- Linha de cima: materiais originais ---
    int m1 = AddOrenNayarMat(scene, Ka, Kd, 0.3f);
    int m2 = AddOrenNayarMat(scene, Ka, Kd, 0.5f);
    int m3 = AddOrenNayarMat(scene, Ka, Kd, 0.7f);
    int m4 = AddOrenNayarMat(scene, Ka, Kd, 0.9f);
 
    // --- Linha de baixo: mesmos sigma com Dog.ppm ---
    auto AddOrenTex = [&](float sigma) -> int {
        OrenNayarTexture *brdf = new OrenNayarTexture("Dog.ppm");
        brdf->Ka = Ka;
        brdf->Kd = RGB(1.f, 1.f, 1.f);
        brdf->Ks = RGB(0.f, 0.f, 0.f);
        brdf->Ks_brdf = RGB(0.f, 0.f, 0.f);
        brdf->Kt = RGB(0.f, 0.f, 0.f);
        brdf->sigma = sigma;
        return scene.AddMaterial(brdf);
    };
 
    int t1 = AddOrenTex(0.3f);
    int t2 = AddOrenTex(0.5f);
    int t3 = AddOrenTex(0.7f);
    int t4 = AddOrenTex(0.9f);
 
    float const xs[4] = { -3.f, -1.f, 1.f, 3.f };
    float const z = 3.f;
    float const radius = 0.8f;
    int plain[4] = { m1, m2, m3, m4 };
    int tex  [4] = { t1, t2, t3, t4 };
 
    for (int i = 0; i < 4; i++) {
        AddSphere(scene, Point(xs[i], 2.0f, z), radius, plain[i]);
        AddSphere(scene, Point(xs[i], 0.2f, z), radius, tex  [i]);
    }
 
    // Iluminação igual à OrenNayarTestStandart
    addONTestLighting(scene);
}