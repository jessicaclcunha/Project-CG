//
//  AshikhminShirleyScenes.cpp
//  VI-RT
//
//  Ashikhmin-Shirley BRDF scenes.
//  Câmara sugerida: Eye=(0, 0.5, -5), At=(0, 0, 3), Up=(0,1,0), FOV=60°
//

#include "BuildScenes.hpp"
#include "SceneHelpers.hpp"
#include "AshikhminShirley.hpp"

// -------------------------------------------------------------------------
// Helper: cria material Ashikhmin-Shirley
//   Ka    — ambiente
//   Kd    — difuso base (albedo)
//   Ks    — reflectância especular F0 (Schlick)
//   nu,nv — expoentes de Phong nas direções T e B
//   tangent / hasTangent — direção de "brushing" opcional
// -------------------------------------------------------------------------
static int AddASMat(Scene& scene,
                    RGB const Ka, RGB const Kd, RGB const Ks,
                    float const nu, float const nv,
                    Vector const tangent   = Vector(0.f, 0.f, 0.f),
                    bool  const hasTangent = false)
{
    AshikhminShirley *brdf = new AshikhminShirley;
    brdf->Ka         = Ka;
    brdf->Kd         = Kd;
    brdf->Ks         = RGB(0.f, 0.f, 0.f);   // sem raios de espelho mirror
    brdf->Ks_brdf    = Ks;                    // especular Ashikhmin-Shirley
    brdf->Kt         = RGB(0.f, 0.f, 0.f);
    brdf->nu         = std::max(1.f, nu);
    brdf->nv         = std::max(1.f, nv);
    brdf->tangent    = tangent;
    brdf->hasTangent = hasTangent;
    return (scene.AddMaterial(brdf));
}

// Versão com tangente explícita (hasTangent=true automático)
static int AddASMatT(Scene& scene,
                     RGB const Ka, RGB const Kd, RGB const Ks,
                     float const nu, float const nv,
                     Vector const tangent)
{
    return AddASMat(scene, Ka, Kd, Ks, nu, nv, tangent, true);
}

// -------------------------------------------------------------------------
// Luzes comuns às cenas pequenas
// -------------------------------------------------------------------------
static void AddDefaultLights(Scene& scene) {
    AmbientLight *ambient = new AmbientLight(RGB(0.05f, 0.05f, 0.05f));
    scene.lights.push_back(ambient);
    scene.numLights++;

    PointLight *p1 = new PointLight(RGB(300.f, 300.f, 300.f),
                                    Point(-1.f, 2.f, 0.f));
    scene.lights.push_back(p1);
    scene.numLights++;
}

// =========================================================================
// AshikhminShirleyScene
//   4 esferas com nu crescente, nv fixo → controlo da forma do lóbulo
//   Da esquerda para a direita: nu=1 (difuso), 10, 100, 1000 (espelho quase)
// =========================================================================
void AshikhminShirleyScene(Scene& scene)
{
    RGB const Ka(0.05f, 0.04f, 0.03f);
    RGB const Kd(0.20f, 0.16f, 0.10f);
    RGB const Ks(0.90f, 0.75f, 0.50f);  // ouro

    // nu=nv → isotrópico em todos (variação de brilho)
    int const s1 = AddASMat(scene, Ka, Kd, Ks,    1.f,    1.f);
    int const s2 = AddASMat(scene, Ka, Kd, Ks,   10.f,   10.f);
    int const s3 = AddASMat(scene, Ka, Kd, Ks,  100.f,  100.f);
    int const s4 = AddASMat(scene, Ka, Kd, Ks, 1000.f, 1000.f);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, s1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, s2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, s3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, s4);

    AddDefaultLights(scene);
}

// =========================================================================
// AshikhminShirleyAnisotropicScene
//   4 esferas com nu fixo e nv variável → anisotropia progressiva
//   nu fixo (lóbulo estreito em T), nv cresce (lóbulo alarga em B)
// =========================================================================
void AshikhminShirleyAnisotropicScene(Scene& scene)
{
    RGB const Ka(0.05f, 0.04f, 0.03f);
    RGB const Kd(0.15f, 0.12f, 0.08f);
    RGB const Ks(0.95f, 0.80f, 0.60f);  // ouro brilhante

    Vector const T(1.f, 0.f, 0.f);  // brushing horizontal

    int const s1 = AddASMatT(scene, Ka, Kd, Ks,  500.f,   500.f, T);   // isotrópico
    int const s2 = AddASMatT(scene, Ka, Kd, Ks,  500.f,   100.f, T);   // ligeiramente anisotrópico
    int const s3 = AddASMatT(scene, Ka, Kd, Ks,  500.f,    20.f, T);   // visivelmente anisotrópico
    int const s4 = AddASMatT(scene, Ka, Kd, Ks,  500.f,     5.f, T);   // muito anisotrópico

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, s1);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, s2);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, s3);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, s4);

    AddDefaultLights(scene);
}

// =========================================================================
// AshikhminShirleyMaterialsScene
//   4 esferas com presets de materiais físicos representativos:
//     1. Tecido (veludo) — nu=nv baixo, Kd dominante
//     2. Plástico — nu=nv médio, Ks cinzento
//     3. Metal escovado — nu>>nv, Ks dourado
//     4. Metal polido — nu=nv alto, Ks branco
// =========================================================================
void AshikhminShirleyMaterialsScene(Scene& scene)
{
    RGB const Ka(0.03f, 0.03f, 0.03f);

    // 1. Tecido/veludo: forte difuso, especular fraco e largo
    int const velvet = AddASMat(scene, Ka,
        RGB(0.35f, 0.05f, 0.10f),   // Kd vermelho escuro
        RGB(0.05f, 0.05f, 0.05f),   // Ks quase nulo
        2.f, 2.f);

    // 2. Plástico: Kd colorido, Ks cinzento médio, isotrópico
    int const plastic = AddASMat(scene, Ka,
        RGB(0.10f, 0.30f, 0.70f),   // Kd azul
        RGB(0.50f, 0.50f, 0.50f),   // Ks cinzento
        80.f, 80.f);

    // 3. Metal escovado horizontal: Kd mínimo, Ks dourado, nu>>nv
    int const brushed = AddASMatT(scene, Ka,
        RGB(0.05f, 0.04f, 0.02f),   // Kd quase nulo (metal)
        RGB(1.00f, 0.71f, 0.29f),   // Ks ouro
        500.f, 10.f,
        Vector(1.f, 0.f, 0.f));

    // 4. Metal polido: nu=nv muito alto, Ks prata
    int const polished = AddASMat(scene, Ka,
        RGB(0.04f, 0.04f, 0.04f),   // Kd quase nulo
        RGB(0.95f, 0.93f, 0.88f),   // Ks prata
        2000.f, 2000.f);

    AddSphere(scene, Point(-3.f, 0.f, 3.f), 0.8f, velvet);
    AddSphere(scene, Point(-1.f, 0.f, 3.f), 0.8f, plastic);
    AddSphere(scene, Point( 1.f, 0.f, 3.f), 0.8f, brushed);
    AddSphere(scene, Point( 3.f, 0.f, 3.f), 0.8f, polished);

    AddDefaultLights(scene);
}

// =========================================================================
// AshikhminShirleyJustOneThing
//   Esfera única — presets comentados para experimentação fácil
// =========================================================================
void AshikhminShirleyJustOneThing(Scene& scene)
{
    RGB const Ka(0.04f, 0.04f, 0.04f);

    /* METAL ESCOVADO DOURADO — anisotrópico forte */
    RGB const Kd(0.05f, 0.04f, 0.02f);
    RGB const Ks(1.00f, 0.71f, 0.29f);
    float const nu = 500.f, nv = 10.f;
    Vector const T(1.f, 0.f, 0.f);
    int const mat = AddASMatT(scene, Ka, Kd, Ks, nu, nv, T);

    /* PRATA POLIDA — isotrópica, muito brilhante
    RGB const Kd(0.04f, 0.04f, 0.04f);
    RGB const Ks(0.95f, 0.93f, 0.88f);
    int const mat = AddASMat(scene, Ka, Kd, Ks, 2000.f, 2000.f);
    */

    /* PLÁSTICO VERMELHO
    RGB const Kd(0.70f, 0.10f, 0.10f);
    RGB const Ks(0.50f, 0.50f, 0.50f);
    int const mat = AddASMat(scene, Ka, Kd, Ks, 80.f, 80.f);
    */

    /* VELUDO AZUL — grande retroreflexão no bordo
    RGB const Kd(0.05f, 0.10f, 0.45f);
    RGB const Ks(0.05f, 0.05f, 0.05f);
    int const mat = AddASMat(scene, Ka, Kd, Ks, 3.f, 3.f);
    */

    /* CERÂMICA — dielétrico com highlight branco médio
    RGB const Kd(0.15f, 0.20f, 0.55f);
    RGB const Ks(0.60f, 0.60f, 0.60f);
    int const mat = AddASMat(scene, Ka, Kd, Ks, 150.f, 150.f);
    */

    AddSphere(scene, Point(0.f, 0.f, 3.f), 0.8f, mat);

    AddDefaultLights(scene);
}

// =========================================================================
// AshikhminShirleyCubeScene
//   Cubo com faces de materiais diferentes (multi-material)
//   Frente: plástico; Topo + laterais: metal escovado
// =========================================================================
void AshikhminShirleyCubeScene(Scene& scene)
{
    RGB const Ka(0.03f, 0.03f, 0.03f);

    // Frente: plástico azul
    int const matFront = AddASMat(scene, Ka,
        RGB(0.10f, 0.20f, 0.60f),
        RGB(0.55f, 0.55f, 0.55f),
        60.f, 60.f);

    // Metal escovado dourado (tangente horizontal)
    int const matMetal = AddASMatT(scene, Ka,
        RGB(0.05f, 0.04f, 0.02f),
        RGB(1.00f, 0.71f, 0.29f),
        400.f, 15.f,
        Vector(1.f, 0.f, 0.f));

    AddBoxMultiMat(scene, Point(0.f, -0.7f, 3.f), 0.8f,
                   matFront, matMetal,
                   matMetal, matMetal,
                   matFront, matMetal);

    // Chão cinzento
    int const ground = AddDiffuseMat(scene, RGB(0.35f, 0.35f, 0.35f));
    AddTriangle(scene, Point(-6.f,-1.5f,-6.f), Point(6.f,-1.5f,-6.f), Point(6.f,-1.5f,10.f), ground);
    AddTriangle(scene, Point(-6.f,-1.5f,-6.f), Point(6.f,-1.5f,10.f), Point(-6.f,-1.5f,10.f), ground);

    AmbientLight *ambient = new AmbientLight(RGB(0.05f, 0.05f, 0.05f));
    scene.lights.push_back(ambient);
    scene.numLights++;

    // Painel de luz rectangular — highlight largo para revelar anisotropia
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