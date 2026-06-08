//
//  AshikhminShirleyScenes.cpp
//  VI-RT
//
//  Ashikhmin-Shirley BRDF scenes — exploração de variantes.
//  Câmara sugerida: Eye=(0, 0.5, -5), At=(0, 0, 3), Up=(0,1,0), FOV=60°
//

#include "BuildScenes.hpp"
#include "SceneHelpers.hpp"
#include "AshikhminShirley.hpp"
#include "AshikhminShirleyFConst.hpp"
#include "AshikhminShirleyFInv.hpp"
#include "AshikhminShirleyNoNorm.hpp"
#include "AshikhminShirleyLambDiff.hpp"
#include "AshikhminShirleyNoDiff.hpp"
#include "AshikhminShirleyFExact.hpp"
#include "AshikhminShirleyFSG.hpp"

// -------------------------------------------------------------------------
// Iluminação padrão para cenas de teste de variantes
// EXPERIÊNCIA point light (estilo Ward): key pontual concentrada → highlight
// nítido, sem a fragmentação do painel da area light. Sem fill (evita 2º
// highlight que confundiria o streak anisotrópico). Para reverter, voltar à
// AreaLight + fill (rewind do git).
// -------------------------------------------------------------------------
static void addASTestLighting(Scene& scene, float floorY = -0.9f) {
    // Key pontual frontal-superior (entre câmara e esferas) → highlight na face visível
    PointLight *key = new PointLight(RGB(500.f, 500.f, 500.f), Point(0.f, 4.f, 0.f));
    scene.lights.push_back(key); scene.numLights++;

    AmbientLight *ambient = new AmbientLight(RGB(0.02f, 0.02f, 0.02f));
    scene.lights.push_back(ambient); scene.numLights++;

    int floor_mat = AddDiffuseMat(scene, RGB(0.07f, 0.07f, 0.07f));
    AddTriangle(scene, Point(-6.f, floorY, -2.f), Point(6.f, floorY, -2.f), Point(6.f, floorY, 8.f), floor_mat);
    AddTriangle(scene, Point(-6.f, floorY, -2.f), Point(6.f, floorY, 8.f), Point(-6.f, floorY, 8.f), floor_mat);
}

// -------------------------------------------------------------------------
// Helpers para criar cada variante
// -------------------------------------------------------------------------
static int AddASMat(Scene& scene,
                    RGB const Ka, RGB const Kd, RGB const Ks,
                    float nu, float nv,
                    Vector tangent = Vector(0.f,0.f,0.f), bool hasTangent = false)
{
    AshikhminShirley *brdf = new AshikhminShirley;
    brdf->Ka = Ka; brdf->Kd = Kd;
    brdf->Ks = RGB(0.f,0.f,0.f); brdf->Ks_brdf = Ks; brdf->Kt = RGB(0.f,0.f,0.f);
    brdf->nu = std::max(1.f, nu); brdf->nv = std::max(1.f, nv);
    brdf->tangent = tangent; brdf->hasTangent = hasTangent;
    return scene.AddMaterial(brdf);
}

// Helper genérico — instancia qualquer BRDF tipo-AS (mesmos campos).
// Usado pelo estudo de variantes (o standard e as 5 variantes partilham
// exactamente os mesmos materiais via buildASStudyScene<T>).
template <typename T>
static int AddASVariantMat(Scene& scene,
                           RGB const Ka, RGB const Kd, RGB const Ks,
                           float nu, float nv,
                           Vector const tangent, bool const hasTangent)
{
    T *brdf = new T;
    brdf->Ka = Ka; brdf->Kd = Kd;
    brdf->Ks = RGB(0.f,0.f,0.f); brdf->Ks_brdf = Ks; brdf->Kt = RGB(0.f,0.f,0.f);
    brdf->nu = std::max(1.f, nu); brdf->nv = std::max(1.f, nv);
    brdf->tangent = tangent; brdf->hasTangent = hasTangent;
    return scene.AddMaterial(brdf);
}

// =========================================================================
// AshikhminShirleyScene (mantida — isotrópica, variação de brilho)
// =========================================================================
void AshikhminShirleyScene(Scene& scene)
{
    RGB const Ka(0.05f, 0.04f, 0.03f);
    RGB const Kd(0.20f, 0.16f, 0.10f);
    RGB const Ks(0.90f, 0.75f, 0.50f);

    int const s1 = AddASMat(scene, Ka, Kd, Ks,    1.f,    1.f);
    int const s2 = AddASMat(scene, Ka, Kd, Ks,   10.f,   10.f);
    int const s3 = AddASMat(scene, Ka, Kd, Ks,  100.f,  100.f);
    int const s4 = AddASMat(scene, Ka, Kd, Ks, 1000.f, 1000.f);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, s1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, s2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, s3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, s4);

    addASTestLighting(scene);
}

// =========================================================================
// AshikhminShirleyAnisotropicScene (mantida — nu fixo, nv variável)
// =========================================================================
void AshikhminShirleyAnisotropicScene(Scene& scene)
{
    RGB const Ka(0.05f, 0.04f, 0.03f);
    RGB const Kd(0.15f, 0.12f, 0.08f);
    RGB const Ks(0.95f, 0.80f, 0.60f);
    Vector const T(1.f, 0.f, 0.f);

    int const s1 = AddASMat(scene, Ka, Kd, Ks,  500.f,  500.f, T, true);
    int const s2 = AddASMat(scene, Ka, Kd, Ks,  500.f,  100.f, T, true);
    int const s3 = AddASMat(scene, Ka, Kd, Ks,  500.f,   20.f, T, true);
    int const s4 = AddASMat(scene, Ka, Kd, Ks,  500.f,    5.f, T, true);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, s1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, s2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, s3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, s4);

    addASTestLighting(scene);
}

// =========================================================================
// AshikhminShirleyMaterialsScene (mantida — presets físicos)
// =========================================================================
void AshikhminShirleyMaterialsScene(Scene& scene)
{
    RGB const Ka(0.03f, 0.03f, 0.03f);

    int const velvet = AddASMat(scene, Ka,
        RGB(0.35f, 0.05f, 0.10f), RGB(0.05f, 0.05f, 0.05f), 2.f, 2.f);

    int const plastic = AddASMat(scene, Ka,
        RGB(0.10f, 0.30f, 0.70f), RGB(0.50f, 0.50f, 0.50f), 80.f, 80.f);

    int const brushed = AddASMat(scene, Ka,
        RGB(0.05f, 0.04f, 0.02f), RGB(1.00f, 0.71f, 0.29f),
        500.f, 10.f, Vector(1.f, 0.f, 0.f), true);

    int const polished = AddASMat(scene, Ka,
        RGB(0.04f, 0.04f, 0.04f), RGB(0.95f, 0.93f, 0.88f), 2000.f, 2000.f);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, velvet);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, plastic);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, brushed);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, polished);

    addASTestLighting(scene);
}

// =========================================================================
// CENAS DE TESTE DE VARIANTES — seguem o padrão CookTorrance
//
// Cada cena coloca 4 esferas com o mesmo material base mas variando
// um único parâmetro (nu/nv), usando a versão alterada da BRDF.
// A comparação directa com AshikhminShirleyTestStandart revela
// o impacto de cada modificação.
//
// Materiais de referência (mesmos 4 das cenas CookTorrance):
//   m1: plástico suave,  nu=nv=10
//   m2: plástico rugoso, nu=nv=80
//   m3: metal escovado,  nu=500, nv=10, tangente horizontal
//   m4: metal polido,    nu=nv=1000
// =========================================================================

static void addAS4Spheres(Scene& scene, int m1, int m2, int m3, int m4) {
    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, m1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, m2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, m3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, m4);
}

// Builder partilhado — os 4 materiais do estudo com a BRDF T.
// O standard e as 5 variantes chamam isto, garantindo materiais IDÊNTICOS
// (só muda a classe BRDF) → o RMSE mede apenas a diferença do modelo.
//   m1: plástico azul,  nu=nv=10,  Ks branco
//   m2: plástico azul,  nu=nv=80,  Ks branco
//   m3: metal escovado, nu=500 nv=10, tangente horizontal, Ks dourado (aniso)
//   m4: metal polido,   nu=nv=1000, Ks dourado
template <typename BRDFType>
static void buildASStudyScene(Scene& scene)
{
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);  // plástico azul
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);  // ouro — difuso vivo p/ corpo visível
                                              // (Kd=0 seria "metal físico" mas, sem IBL,
                                              //  daria esfera preta só com o highlight)
    RGB const Ks_diel (0.50f, 0.50f, 0.50f);  // especular dielétrico → reflexo BRANCO
    RGB const Ks_metal(0.90f, 0.75f, 0.50f);  // especular metálico → reflexo dourado
    Vector const T0(0.f, 0.f, 0.f);
    Vector const T(1.f, 0.f, 0.f);

    int m1 = AddASVariantMat<BRDFType>(scene, Ka, Kd_diel,  Ks_diel,    10.f,   10.f, T0, false);
    int m2 = AddASVariantMat<BRDFType>(scene, Ka, Kd_diel,  Ks_diel,    80.f,   80.f, T0, false);
    int m3 = AddASVariantMat<BRDFType>(scene, Ka, Kd_metal, Ks_metal,  500.f,   10.f, T,  true);
    int m4 = AddASVariantMat<BRDFType>(scene, Ka, Kd_metal, Ks_metal, 1000.f, 1000.f, T0, false);

    addAS4Spheres(scene, m1, m2, m3, m4);
    addASTestLighting(scene);
}

// Referência do estudo
void AshikhminShirleyTestStandart(Scene& scene) { buildASStudyScene<AshikhminShirley>(scene); }

// Variantes (mesma cena, só muda a BRDF)
void AshikhminShirleyFConstTest  (Scene& scene) { buildASStudyScene<AshikhminShirleyFConst>(scene); }   // Fresnel constante
void AshikhminShirleyFInvTest    (Scene& scene) { buildASStudyScene<AshikhminShirleyFInv>(scene); }     // Fresnel invertido
void AshikhminShirleyNoNormTest  (Scene& scene) { buildASStudyScene<AshikhminShirleyNoNorm>(scene); }   // sem normalização √((nu+1)(nv+1))
void AshikhminShirleyLambDiffTest(Scene& scene) { buildASStudyScene<AshikhminShirleyLambDiff>(scene); } // difuso Lambertiano (sem CE)
void AshikhminShirleyNoDiffTest  (Scene& scene) { buildASStudyScene<AshikhminShirleyNoDiff>(scene); }   // especular puro (sem difuso)

// Alternativas correctas (formulações de Fresnel legítimas)
void AshikhminShirleyFExactTest  (Scene& scene) { buildASStudyScene<AshikhminShirleyFExact>(scene); }   // Fresnel dieléctrico exacto
void AshikhminShirleyFSGTest     (Scene& scene) { buildASStudyScene<AshikhminShirleyFSG>(scene); }      // Fresnel spherical-gaussian


void AshikhminShirleyTextureStandart (Scene& scene) {
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);
    RGB const Ks_diel (0.50f, 0.50f, 0.50f);
    RGB const Ks_metal(0.90f, 0.75f, 0.50f);
    Vector const T(1.f, 0.f, 0.f);
    Vector const T0(0.f, 0.f, 0.f);
 
    // --- Linha de cima: materiais originais ---
    int m1 = AddASMat(scene, Ka, Kd_diel,  Ks_diel,    10.f,   10.f, T0, false);
    int m2 = AddASMat(scene, Ka, Kd_diel,  Ks_diel,    80.f,   80.f, T0, false);
    int m3 = AddASMat(scene, Ka, Kd_metal, Ks_metal,  500.f,   10.f, T,  true);
    int m4 = AddASMat(scene, Ka, Kd_metal, Ks_metal, 1000.f, 1000.f, T0, false);
 
    // --- Linha de baixo: mesmos nu/nv/tangente com Dog.ppm ---
    auto AddASTex = [&](RGB Ks, float nu, float nv, Vector t, bool hasT) -> int {
        AshikhminShirleyTexture *brdf = new AshikhminShirleyTexture("Dog.ppm");
        brdf->Ka = Ka;
        brdf->Kd = RGB(1.f, 1.f, 1.f);
        brdf->Ks = RGB(0.f, 0.f, 0.f);
        brdf->Ks_brdf = Ks;
        brdf->Kt = RGB(0.f, 0.f, 0.f);
        brdf->nu = std::max(1.f, nu);
        brdf->nv = std::max(1.f, nv);
        brdf->tangent = t;
        brdf->hasTangent = hasT;
        return scene.AddMaterial(brdf);
    };
 
    int t1 = AddASTex(Ks_diel,     10.f,   10.f, T0, false);
    int t2 = AddASTex(Ks_diel,     80.f,   80.f, T0, false);
    int t3 = AddASTex(Ks_metal,   500.f,   10.f, T,  true);
    int t4 = AddASTex(Ks_metal,  1000.f, 1000.f, T0, false);
 
    float const xs[4] = { -3.f, -1.f, 1.f, 3.f };
    float const z = 3.f;
    float const radius = 0.8f;
    int plain[4] = { m1, m2, m3, m4 };
    int tex  [4] = { t1, t2, t3, t4 };
 
    for (int i = 0; i < 4; i++) {
        AddSphere(scene, Point(xs[i], 2.0f, z), radius, plain[i]);
        AddSphere(scene, Point(xs[i], 0.2f, z), radius, tex  [i]);
    }
 
    // Iluminação igual à AshikhminShirleyTestStandart
    addASTestLighting(scene);
}