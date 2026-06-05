#include "BuildScenes.hpp"
#include "SceneHelpers.hpp"
#include "WardTexture.hpp"
#include "OrenNayarTexture.hpp"
#include "AshikhminShirleyTexture.hpp"

// -------------------------------------------------------------------------
// Iluminação padrão
// -------------------------------------------------------------------------
static void addTextureLighting(Scene& scene) {
    AmbientLight *ambient = new AmbientLight(RGB(0.08f, 0.08f, 0.08f));
    scene.lights.push_back(ambient); scene.numLights++;

    PointLight *p1 = new PointLight(RGB(300.f, 300.f, 300.f), Point(-1.f, 3.f, 0.f));
    scene.lights.push_back(p1); scene.numLights++;

    PointLight *p2 = new PointLight(RGB(150.f, 150.f, 150.f), Point(3.f, 2.f, -1.f));
    scene.lights.push_back(p2); scene.numLights++;
}

// -------------------------------------------------------------------------
// Helpers: criar material texturizado para cada BRDF
// -------------------------------------------------------------------------
static int MakePhongTex(Scene& scene, const std::string& file) {
    PhongTexture *brdf = new PhongTexture(file);
    brdf->Ka = RGB(0.05f, 0.05f, 0.05f);
    brdf->Kd = RGB(1.f, 1.f, 1.f);
    brdf->Ks = RGB(0.f, 0.f, 0.f);
    brdf->Ks_brdf = RGB(0.8f, 0.8f, 0.8f);
    brdf->Kt = RGB(0.f, 0.f, 0.f);
    brdf->ns = 50.f;
    return scene.AddMaterial(brdf);
}

static int MakeCookTex(Scene& scene, const std::string& file) {
    CookTorranceTexture *brdf = new CookTorranceTexture(file);
    brdf->Ka = RGB(0.05f, 0.05f, 0.05f);
    brdf->Kd = RGB(1.f, 1.f, 1.f);
    brdf->Ks = RGB(0.f, 0.f, 0.f);
    brdf->Ks_brdf = RGB(1.f, 1.f, 1.f);
    brdf->Kt = RGB(0.f, 0.f, 0.f);
    brdf->roughness = 0.3f;
    brdf->metallic  = 0.0f;
    return scene.AddMaterial(brdf);
}

static int MakeWardTex(Scene& scene, const std::string& file) {
    WardTexture *brdf = new WardTexture(file);
    brdf->Ka = RGB(0.05f, 0.04f, 0.03f);
    brdf->Kd = RGB(1.f, 1.f, 1.f);
    brdf->Ks = RGB(0.f, 0.f, 0.f);
    brdf->Ks_brdf = RGB(0.90f, 0.75f, 0.50f);
    brdf->Kt = RGB(0.f, 0.f, 0.f);
    brdf->alphaX = 0.15f; brdf->alphaY = 0.40f;
    brdf->hasTangent = true; brdf->tangent = Vector(1.f, 0.f, 0.f);
    return scene.AddMaterial(brdf);
}

static int MakeOrenTex(Scene& scene, const std::string& file) {
    OrenNayarTexture *brdf = new OrenNayarTexture(file);
    brdf->Ka = RGB(0.05f, 0.05f, 0.05f);
    brdf->Kd = RGB(1.f, 1.f, 1.f);
    brdf->Ks = RGB(0.f, 0.f, 0.f);
    brdf->Ks_brdf = RGB(0.f, 0.f, 0.f);
    brdf->Kt = RGB(0.f, 0.f, 0.f);
    brdf->sigma = 0.5f;
    return scene.AddMaterial(brdf);
}

static int MakeAshikTex(Scene& scene, const std::string& file) {
    AshikhminShirleyTexture *brdf = new AshikhminShirleyTexture(file);
    brdf->Ka = RGB(0.03f, 0.03f, 0.03f);
    brdf->Kd = RGB(1.f, 1.f, 1.f);
    brdf->Ks = RGB(0.f, 0.f, 0.f);
    brdf->Ks_brdf = RGB(0.90f, 0.75f, 0.50f);
    brdf->Kt = RGB(0.f, 0.f, 0.f);
    brdf->nu = 100.f; brdf->nv = 100.f;
    brdf->hasTangent = false;
    return scene.AddMaterial(brdf);
}

// =========================================================================
// Cena 1 — Esferas: 1 por BRDF, Dog.ppm
// Phong | Cook-Torrance | Oren-Nayar | Ward | Ashikhmin-Shirley
// =========================================================================
void TextureAllBRDFsSpheres(Scene& scene) {
    int const phong = MakePhongTex(scene, "Dog.ppm");
    int const cook  = MakeCookTex (scene, "Dog.ppm");
    int const oren  = MakeOrenTex (scene, "Dog.ppm");
    int const ward  = MakeWardTex (scene, "Dog.ppm");
    int const ashik = MakeAshikTex(scene, "Dog.ppm");

    AddSphere(scene, Point(-4.f, 0.f, 5.f), 0.7f, phong);
    AddSphere(scene, Point(-2.f, 0.f, 5.f), 0.7f, cook);
    AddSphere(scene, Point( 0.f, 0.f, 5.f), 0.7f, oren);
    AddSphere(scene, Point( 2.f, 0.f, 5.f), 0.7f, ward);
    AddSphere(scene, Point( 4.f, 0.f, 5.f), 0.7f, ashik);

    addTextureLighting(scene);
}

// =========================================================================
// Cena 2 — Cubos com UV: 1 por BRDF, UMinho.ppm
// =========================================================================
void TextureAllBRDFsBoxes(Scene& scene) {
    int const phong = MakePhongTex(scene, "UMinho.ppm");
    int const cook  = MakeCookTex (scene, "UMinho.ppm");
    int const oren  = MakeOrenTex (scene, "UMinho.ppm");
    int const ward  = MakeWardTex (scene, "UMinho.ppm");
    int const ashik = MakeAshikTex(scene, "UMinho.ppm");

    AddBoxUV(scene, Point(-4.f, 0.f, 5.f), 0.55f, phong);
    AddBoxUV(scene, Point(-2.f, 0.f, 5.f), 0.55f, cook);
    AddBoxUV(scene, Point( 0.f, 0.f, 5.f), 0.55f, oren);
    AddBoxUV(scene, Point( 2.f, 0.f, 5.f), 0.55f, ward);
    AddBoxUV(scene, Point( 4.f, 0.f, 5.f), 0.55f, ashik);

    addTextureLighting(scene);
}

// =========================================================================
// Cena 3 — Grade completa 2×8:
//   Linha de cima:  esferas
//   Linha de baixo: cubos
//
//   Col 0: Phong sem textura (referência)
//   Col 1: PhongTexture Dog
//   Col 2: PhongTexture UMinho
//   Col 3: CookTorranceTexture Dog
//   Col 4: WardTexture Dog
//   Col 5: OrenNayarTexture Dog
//   Col 6: AshikhminShirleyTexture Dog
//   Col 7: AshikhminShirleyTexture UMinho
//
//  NOTA: usar Eye=(0, 0.5, -8) ou FOV=80° para ver 8 colunas
// =========================================================================
void TextureFullShowcase(Scene& scene) {
    const float z    = 5.f;
    const float xs[] = { -5.25f, -3.75f, -2.25f, -0.75f, 0.75f, 2.25f, 3.75f, 5.25f };
    const float yS   =  0.9f;
    const float yB   = -0.9f;
    const float rSph = 0.60f;
    const float hBox = 0.46f;

    // col 0: Phong sem textura (referência)
    Phong *plain = new Phong;
    plain->Ka = RGB(0.05f, 0.05f, 0.05f);
    plain->Kd = RGB(0.6f, 0.2f, 0.2f);
    plain->Ks = RGB(0.f, 0.f, 0.f);
    plain->Ks_brdf = RGB(0.8f, 0.8f, 0.8f);
    plain->Kt = RGB(0.f, 0.f, 0.f);
    plain->ns = 50.f;
    int const m0 = scene.AddMaterial(plain);

    int const m1 = MakePhongTex (scene, "Dog.ppm");
    int const m2 = MakePhongTex (scene, "UMinho.ppm");
    int const m3 = MakeCookTex  (scene, "Dog.ppm");
    int const m4 = MakeWardTex  (scene, "Dog.ppm");
    int const m5 = MakeOrenTex  (scene, "Dog.ppm");
    int const m6 = MakeAshikTex (scene, "Dog.ppm");
    int const m7 = MakeAshikTex (scene, "UMinho.ppm");

    int mats[] = { m0, m1, m2, m3, m4, m5, m6, m7 };

    for (int i = 0; i < 8; i++) {
        AddSphere(scene, Point(xs[i], yS, z), rSph, mats[i]);
        AddBoxUV (scene, Point(xs[i], yB, z), hBox, mats[i]);
    }

    addTextureLighting(scene);
}

// =========================================================================
// Cena 4 — UV Debug: validação do mapeamento UV
// =========================================================================
void TextureUVDebug(Scene& scene) {
    int const dog_sph  = MakePhongTex(scene, "Dog.ppm");
    int const um_sph   = MakePhongTex(scene, "UMinho.ppm");
    int const dog_box  = MakePhongTex(scene, "Dog.ppm");
    int const um_box   = MakePhongTex(scene, "UMinho.ppm");

    AddSphere(scene, Point(-1.5f,  0.7f, 5.f), 0.65f, dog_sph);
    AddSphere(scene, Point( 1.5f,  0.7f, 5.f), 0.65f, um_sph);
    AddBoxUV (scene, Point(-1.5f, -0.7f, 5.f), 0.50f, dog_box);
    AddBoxUV (scene, Point( 1.5f, -0.7f, 5.f), 0.50f, um_box);

    addTextureLighting(scene);
}