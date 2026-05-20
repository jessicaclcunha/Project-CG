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

// -------------------------------------------------------------------------
// Iluminação padrão para cenas de teste de variantes
// Igual à usada em CookTorrance: 1 AreaLight key + 1 PointLight fill + ambient + chão
// -------------------------------------------------------------------------
static void addASTestLighting(Scene& scene, float floorY = -0.9f) {
    Vector nDown(0.f, -1.f, 0.f);
    AreaLight *key0 = new AreaLight(RGB(400.f, 400.f, 400.f),
        Point(-1.5f, 5.f, 2.f), Point(1.5f, 5.f, 2.f), Point(1.5f, 5.f, 4.f), nDown);
    AreaLight *key1 = new AreaLight(RGB(400.f, 400.f, 400.f),
        Point(-1.5f, 5.f, 2.f), Point(1.5f, 5.f, 4.f), Point(-1.5f, 5.f, 4.f), nDown);
    scene.lights.push_back(key0); scene.numLights++;
    scene.lights.push_back(key1); scene.numLights++;

    PointLight *fill = new PointLight(RGB(100.f, 100.f, 100.f), Point(-5.f, 2.f, 1.f));
    scene.lights.push_back(fill); scene.numLights++;

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

static int AddASFConstMat(Scene& scene,
                           RGB const Ka, RGB const Kd, RGB const Ks,
                           float nu, float nv,
                           Vector tangent = Vector(0.f,0.f,0.f), bool hasTangent = false)
{
    AshikhminShirleyFConst *brdf = new AshikhminShirleyFConst;
    brdf->Ka = Ka; brdf->Kd = Kd;
    brdf->Ks = RGB(0.f,0.f,0.f); brdf->Ks_brdf = Ks; brdf->Kt = RGB(0.f,0.f,0.f);
    brdf->nu = std::max(1.f, nu); brdf->nv = std::max(1.f, nv);
    brdf->tangent = tangent; brdf->hasTangent = hasTangent;
    return scene.AddMaterial(brdf);
}

static int AddASFInvMat(Scene& scene,
                         RGB const Ka, RGB const Kd, RGB const Ks,
                         float nu, float nv,
                         Vector tangent = Vector(0.f,0.f,0.f), bool hasTangent = false)
{
    AshikhminShirleyFInv *brdf = new AshikhminShirleyFInv;
    brdf->Ka = Ka; brdf->Kd = Kd;
    brdf->Ks = RGB(0.f,0.f,0.f); brdf->Ks_brdf = Ks; brdf->Kt = RGB(0.f,0.f,0.f);
    brdf->nu = std::max(1.f, nu); brdf->nv = std::max(1.f, nv);
    brdf->tangent = tangent; brdf->hasTangent = hasTangent;
    return scene.AddMaterial(brdf);
}

static int AddASNoNormMat(Scene& scene,
                           RGB const Ka, RGB const Kd, RGB const Ks,
                           float nu, float nv,
                           Vector tangent = Vector(0.f,0.f,0.f), bool hasTangent = false)
{
    AshikhminShirleyNoNorm *brdf = new AshikhminShirleyNoNorm;
    brdf->Ka = Ka; brdf->Kd = Kd;
    brdf->Ks = RGB(0.f,0.f,0.f); brdf->Ks_brdf = Ks; brdf->Kt = RGB(0.f,0.f,0.f);
    brdf->nu = std::max(1.f, nu); brdf->nv = std::max(1.f, nv);
    brdf->tangent = tangent; brdf->hasTangent = hasTangent;
    return scene.AddMaterial(brdf);
}

static int AddASLambDiffMat(Scene& scene,
                              RGB const Ka, RGB const Kd, RGB const Ks,
                              float nu, float nv,
                              Vector tangent = Vector(0.f,0.f,0.f), bool hasTangent = false)
{
    AshikhminShirleyLambDiff *brdf = new AshikhminShirleyLambDiff;
    brdf->Ka = Ka; brdf->Kd = Kd;
    brdf->Ks = RGB(0.f,0.f,0.f); brdf->Ks_brdf = Ks; brdf->Kt = RGB(0.f,0.f,0.f);
    brdf->nu = std::max(1.f, nu); brdf->nv = std::max(1.f, nv);
    brdf->tangent = tangent; brdf->hasTangent = hasTangent;
    return scene.AddMaterial(brdf);
}

static int AddASNoDiffMat(Scene& scene,
                           RGB const Ka, RGB const Kd, RGB const Ks,
                           float nu, float nv,
                           Vector tangent = Vector(0.f,0.f,0.f), bool hasTangent = false)
{
    AshikhminShirleyNoDiff *brdf = new AshikhminShirleyNoDiff;
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
// AshikhminShirleyJustOneThing (mantida — esfera única)
// =========================================================================
void AshikhminShirleyJustOneThing(Scene& scene)
{
    RGB const Ka(0.04f, 0.04f, 0.04f);
    RGB const Kd(0.05f, 0.04f, 0.02f);
    RGB const Ks(1.00f, 0.71f, 0.29f);
    int const mat = AddASMat(scene, Ka, Kd, Ks, 500.f, 10.f, Vector(1.f, 0.f, 0.f), true);

    AddSphere(scene, Point(0.f, 0.f, 3.f), 0.8f, mat);
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

// Referência: Ashikhmin-Shirley padrão com os 4 materiais de teste
void AshikhminShirleyTestStandart(Scene& scene)
{
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);  // plástico azul
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);  // ouro
    RGB const Ks(0.90f, 0.75f, 0.50f);        // especular dourado
    Vector const T(1.f, 0.f, 0.f);

    int m1 = AddASMat(scene, Ka, Kd_diel,  Ks,   10.f,   10.f);
    int m2 = AddASMat(scene, Ka, Kd_diel,  Ks,   80.f,   80.f);
    int m3 = AddASMat(scene, Ka, Kd_metal, Ks,  500.f,   10.f, T, true);
    int m4 = AddASMat(scene, Ka, Kd_metal, Ks, 1000.f, 1000.f);

    addAS4Spheres(scene, m1, m2, m3, m4);
    addASTestLighting(scene);
}

// Teste 1: Fresnel CONSTANTE (F = F0 sem variação angular)
// Expectativa: sem "rim light" dourado nas bordas — highlight mais plano,
//              bordo escuro em vez de claro em materiais dielétricos.
void AshikhminShirleyFConstTest(Scene& scene)
{
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);
    RGB const Ks(0.90f, 0.75f, 0.50f);
    Vector const T(1.f, 0.f, 0.f);

    int m1 = AddASFConstMat(scene, Ka, Kd_diel,  Ks,   10.f,   10.f);
    int m2 = AddASFConstMat(scene, Ka, Kd_diel,  Ks,   80.f,   80.f);
    int m3 = AddASFConstMat(scene, Ka, Kd_metal, Ks,  500.f,   10.f, T, true);
    int m4 = AddASFConstMat(scene, Ka, Kd_metal, Ks, 1000.f, 1000.f);

    addAS4Spheres(scene, m1, m2, m3, m4);
    addASTestLighting(scene);
}

// Teste 2: Fresnel INVERTIDO (F máximo ao centro, mínimo nas bordas)
// Expectativa: highlight brilhante ao centro, borda escura —
//              efeito visualmente invertido face ao comportamento físico.
void AshikhminShirleyFInvTest(Scene& scene)
{
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);
    RGB const Ks(0.90f, 0.75f, 0.50f);
    Vector const T(1.f, 0.f, 0.f);

    int m1 = AddASFInvMat(scene, Ka, Kd_diel,  Ks,   10.f,   10.f);
    int m2 = AddASFInvMat(scene, Ka, Kd_diel,  Ks,   80.f,   80.f);
    int m3 = AddASFInvMat(scene, Ka, Kd_metal, Ks,  500.f,   10.f, T, true);
    int m4 = AddASFInvMat(scene, Ka, Kd_metal, Ks, 1000.f, 1000.f);

    addAS4Spheres(scene, m1, m2, m3, m4);
    addASTestLighting(scene);
}

// Teste 3: SEM fator de normalização sqrt((nu+2)(nv+2))
// Expectativa: materiais com nu/nv altos ficam cada vez mais brilhantes
//              (energy not conserved) — curva inversa à correcta.
void AshikhminShirleyNoNormTest(Scene& scene)
{
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);
    RGB const Ks(0.90f, 0.75f, 0.50f);
    Vector const T(1.f, 0.f, 0.f);

    int m1 = AddASNoNormMat(scene, Ka, Kd_diel,  Ks,   10.f,   10.f);
    int m2 = AddASNoNormMat(scene, Ka, Kd_diel,  Ks,   80.f,   80.f);
    int m3 = AddASNoNormMat(scene, Ka, Kd_metal, Ks,  500.f,   10.f, T, true);
    int m4 = AddASNoNormMat(scene, Ka, Kd_metal, Ks, 1000.f, 1000.f);

    addAS4Spheres(scene, m1, m2, m3, m4);
    addASTestLighting(scene);
}

// Teste 4: difuso LAMBERTIANO simples (sem acoplamento ao especular)
// Expectativa: zonas fora do highlight são mais claras que o correcto
//              pois o difuso não é atenuado pelo Rs — violação de energia
//              visível em materiais com Ks alto (ouro/metal).
void AshikhminShirleyLambDiffTest(Scene& scene)
{
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);
    RGB const Ks(0.90f, 0.75f, 0.50f);
    Vector const T(1.f, 0.f, 0.f);

    int m1 = AddASLambDiffMat(scene, Ka, Kd_diel,  Ks,   10.f,   10.f);
    int m2 = AddASLambDiffMat(scene, Ka, Kd_diel,  Ks,   80.f,   80.f);
    int m3 = AddASLambDiffMat(scene, Ka, Kd_metal, Ks,  500.f,   10.f, T, true);
    int m4 = AddASLambDiffMat(scene, Ka, Kd_metal, Ks, 1000.f, 1000.f);

    addAS4Spheres(scene, m1, m2, m3, m4);
    addASTestLighting(scene);
}

// Teste 5: SEM componente difusa (especular puro)
// Expectativa: zonas fora do highlight ficam pretas — isola o lóbulo
//              especular; o padrão anisotrópico fica mais legível.
//              Aproxima-se de um condutor puro (metal sem difuso).
void AshikhminShirleyNoDiffTest(Scene& scene)
{
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Kd_diel (0.10f, 0.20f, 0.80f);
    RGB const Kd_metal(1.00f, 0.71f, 0.29f);
    RGB const Ks(0.90f, 0.75f, 0.50f);
    Vector const T(1.f, 0.f, 0.f);

    int m1 = AddASNoDiffMat(scene, Ka, Kd_diel,  Ks,   10.f,   10.f);
    int m2 = AddASNoDiffMat(scene, Ka, Kd_diel,  Ks,   80.f,   80.f);
    int m3 = AddASNoDiffMat(scene, Ka, Kd_metal, Ks,  500.f,   10.f, T, true);
    int m4 = AddASNoDiffMat(scene, Ka, Kd_metal, Ks, 1000.f, 1000.f);

    addAS4Spheres(scene, m1, m2, m3, m4);
    addASTestLighting(scene);
}