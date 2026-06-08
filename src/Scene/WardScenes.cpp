//
//  WardScenes.cpp
//  VI-RT
//
// Câmara: Eye=(0, 0.5, -5), At=(0, 0, 3), Up=(0,1,0), FOV=60°

#include "BuildScenes.hpp"
#include "SceneHelpers.hpp"
#include "Ward.hpp"
#include "WardIsoForced.hpp"
#include "WardNoNorm.hpp"
#include "WardNoDiff.hpp"
#include "WardNoGeom.hpp"
#include "WardDur.hpp"
#include "WardGMD.hpp"
#include "WardFresnel.hpp"

// =========================================================================
// ESTUDO Ward — setup de cena
// Point light concentrada → highlight nítido (a forma esticada da anisotropia
// lê-se melhor). Sem fill (diluiria o highlight). Ver docs/Ward_Study_Setup.md.
// =========================================================================

// Material/cor partilhado pelo estudo: metal escovado cinzento escuro
#define WARD_STUDY_KA RGB(0.03f, 0.03f, 0.03f)
#define WARD_STUDY_KD RGB(0.06f, 0.06f, 0.06f)
#define WARD_STUDY_KS RGB(0.90f, 0.90f, 0.90f)

static void addWardTestLighting (Scene& scene) {
    // Key pontual frontal-superior → highlight nítido na face visível
    PointLight *key = new PointLight(RGB(320.f, 320.f, 320.f), Point(0.f, 2.5f, -2.f));
    scene.lights.push_back(key); scene.numLights++;

    // Ambient mínimo
    AmbientLight *ambient = new AmbientLight(RGB(0.03f, 0.03f, 0.03f));
    scene.lights.push_back(ambient); scene.numLights++;

    // Chão escuro (base das esferas em y=0.5−0.8=−0.3)
    int floor_mat = AddDiffuseMat(scene, RGB(0.07f, 0.07f, 0.07f));
    AddTriangle(scene, Point(-6.f,-0.3f,-2.f), Point(6.f,-0.3f,-2.f), Point(6.f,-0.3f,8.f), floor_mat);
    AddTriangle(scene, Point(-6.f,-0.3f,-2.f), Point(6.f,-0.3f,8.f), Point(-6.f,-0.3f,8.f), floor_mat);
}

// Standard: 4 esferas cobrindo overall roughness (iso) e razão de anisotropia.
void WardTestStandart (Scene& scene) {
    RGB const Ka = WARD_STUDY_KA, Kd = WARD_STUDY_KD, Ks = WARD_STUDY_KS;

    int m1 = AddWardMat (scene, Ka, Kd, Ks, 0.10f, 0.10f);                   // iso liso
    int m2 = AddWardMat (scene, Ka, Kd, Ks, 0.40f, 0.40f);                   // iso rugoso
    int m3 = AddWardMatT(scene, Ka, Kd, Ks, 0.10f, 0.40f, Vector(1.f,0.f,0.f)); // aniso
    int m4 = AddWardMatT(scene, Ka, Kd, Ks, 0.05f, 0.50f, Vector(1.f,0.f,0.f)); // aniso forte

    AddSphere(scene, Point(-3.f, 0.5f, 3.f), 0.8f, m1);
    AddSphere(scene, Point(-1.f, 0.5f, 3.f), 0.8f, m2);
    AddSphere(scene, Point( 1.f, 0.5f, 3.f), 0.8f, m3);
    AddSphere(scene, Point( 3.f, 0.5f, 3.f), 0.8f, m4);

    addWardTestLighting(scene);
}

// =========================================================================
// ESTUDO Ward — variantes (erradas + alternativas correctas)
// Cada cena replica EXACTAMENTE os 4 materiais do standard (mesmos αx/αy/
// tangente, mesmo Kd/Ks, mesma iluminação), trocando apenas a classe BRDF.
// Templates garantem que não há deriva de parâmetros face ao standard.
// Comparar com WardTestStandart via RMSE/compare.sh. Ver docs/Ward_Study_Setup.md.
// =========================================================================

// Helper genérico de material — instancia qualquer BRDF tipo-Ward (mesmos campos)
template <typename T>
static int AddWardVariantMat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks,
                              float const alphaX, float const alphaY,
                              Vector const tangent, bool const hasTangent) {
    T *brdf = new T;
    brdf->Ka         = Ka;
    brdf->Kd         = Kd;
    brdf->Ks         = RGB(0.f, 0.f, 0.f);
    brdf->Ks_brdf    = Ks;
    brdf->Kt         = RGB(0.f, 0.f, 0.f);
    brdf->alphaX     = std::max(0.01f, alphaX);
    brdf->alphaY     = std::max(0.01f, alphaY);
    brdf->tangent    = tangent;
    brdf->hasTangent = hasTangent;
    return scene.AddMaterial(brdf);
}

// Builder genérico — os 4 materiais do standard com a BRDF T
template <typename T>
static void buildWardVariantScene (Scene& scene) {
    RGB const Ka = WARD_STUDY_KA, Kd = WARD_STUDY_KD, Ks = WARD_STUDY_KS;

    int m1 = AddWardVariantMat<T>(scene, Ka, Kd, Ks, 0.10f, 0.10f, Vector(0.f,0.f,0.f), false); // iso liso
    int m2 = AddWardVariantMat<T>(scene, Ka, Kd, Ks, 0.40f, 0.40f, Vector(0.f,0.f,0.f), false); // iso rugoso
    int m3 = AddWardVariantMat<T>(scene, Ka, Kd, Ks, 0.10f, 0.40f, Vector(1.f,0.f,0.f), true);  // aniso
    int m4 = AddWardVariantMat<T>(scene, Ka, Kd, Ks, 0.05f, 0.50f, Vector(1.f,0.f,0.f), true);  // aniso forte

    AddSphere(scene, Point(-3.f, 0.5f, 3.f), 0.8f, m1);
    AddSphere(scene, Point(-1.f, 0.5f, 3.f), 0.8f, m2);
    AddSphere(scene, Point( 1.f, 0.5f, 3.f), 0.8f, m3);
    AddSphere(scene, Point( 3.f, 0.5f, 3.f), 0.8f, m4);

    addWardTestLighting(scene);
}

// --- Variantes erradas ---
void WardIsoForcedTest (Scene& scene) { buildWardVariantScene<WardIsoForced>(scene); }
void WardNoNormTest    (Scene& scene) { buildWardVariantScene<WardNoNorm>(scene); }
void WardNoDiffTest    (Scene& scene) { buildWardVariantScene<WardNoDiff>(scene); }
void WardNoGeomTest    (Scene& scene) { buildWardVariantScene<WardNoGeom>(scene); }

// --- Alternativas correctas ---
void WardDurTest       (Scene& scene) { buildWardVariantScene<WardDur>(scene); }
void WardGMDTest       (Scene& scene) { buildWardVariantScene<WardGMD>(scene); }
void WardFresnelTest   (Scene& scene) { buildWardVariantScene<WardFresnel>(scene); }

void WardTextureStandart (Scene& scene) {
    RGB const Ka = WARD_STUDY_KA;
    RGB const Kd = WARD_STUDY_KD;
    RGB const Ks = WARD_STUDY_KS;
 
    // --- Linha de cima: materiais originais (cor plana) ---
    int m1 = AddWardMat(scene, Ka, Kd, Ks, 0.10f, 0.10f);
    int m2 = AddWardMat(scene, Ka, Kd, Ks, 0.40f, 0.40f);
    int m3 = AddWardMatT(scene, Ka, Kd, Ks, 0.10f, 0.40f, Vector(1.f,0.f,0.f));
    int m4 = AddWardMatT(scene, Ka, Kd, Ks, 0.05f, 0.50f, Vector(1.f,0.f,0.f));
 
    // --- Linha de baixo: mesmos materiais com Dog.ppm ---
    // Kd=1 para a textura não ser atenuada; alphaX/alphaY/tangente IDÊNTICOS
    auto AddWardTex = [&](float ax, float ay, Vector t, bool hasT) -> int {
        WardTexture *brdf = new WardTexture("Dog.ppm");
        brdf->Ka = Ka;
        brdf->Kd = RGB(1.f, 1.f, 1.f);
        brdf->Ks = RGB(0.f, 0.f, 0.f);
        brdf->Ks_brdf = Ks;
        brdf->Kt = RGB(0.f, 0.f, 0.f);
        brdf->alphaX = std::max(0.01f, ax);
        brdf->alphaY = std::max(0.01f, ay);
        brdf->tangent = t;
        brdf->hasTangent = hasT;
        return scene.AddMaterial(brdf);
    };
 
    int t1 = AddWardTex(0.10f, 0.10f, Vector(0.f,0.f,0.f), false);
    int t2 = AddWardTex(0.40f, 0.40f, Vector(0.f,0.f,0.f), false);
    int t3 = AddWardTex(0.10f, 0.40f, Vector(1.f,0.f,0.f), true);
    int t4 = AddWardTex(0.05f, 0.50f, Vector(1.f,0.f,0.f), true);
 
    float const xs[4] = { -3.f, -1.f, 1.f, 3.f };
    float const z = 3.f;
    float const radius = 0.8f;
    int plain[4] = { m1, m2, m3, m4 };
    int tex  [4] = { t1, t2, t3, t4 };
 
    for (int i = 0; i < 4; i++) {
        AddSphere(scene, Point(xs[i], 2.0f, z), radius, plain[i]);
        AddSphere(scene, Point(xs[i], 0.2f, z), radius, tex  [i]);
    }
 
    // Iluminação igual à WardTestStandart
    addWardTestLighting(scene);
}