//
//  BRDFShowcaseCornellBox.cpp
//  VI-RT
//
//  Cornell Box com 5 objetos no interior, cada um com uma BRDF diferente.
//  Permite comparação direta entre modelos na mesma cena e iluminação.
//
//  Layout interior (vista de cima, eixo Z a afastar da câmara):
//
//   [Parede esq. verde]     [Parede dir. vermelha]
//         |                         |
//         |  (A)  (B)  (C)  (D)     |
//         |             (E)         |
//   [Parede frontal — textura Dog.ppm]
//
//   (A) Esfera   — Phong (cerâmica azul, ns=100)
//   (B) Esfera   — Cook-Torrance (ouro metálico)
//   (C) Esfera   — Oren-Nayar (argila rugosa)
//   (D) Esfera   — Ward anisotrópico (metal escovado)
//   (E) Caixa    — Ashikhmin-Shirley (plástico polido)
//
//  Câmara: Eye=(278, 273, -800), At=(278, 273, 0), Up=(0,1,0), FOV≈39°
//  (coordenadas originais da Cornell Box de Jensen et al.)
//
//  Iluminação: área light no tecto (igual à DiffuseCornellBox) + sem ambient
//  para forçar iluminação global pura — recomendado spp >= 64.
//

#include "BuildScenes.hpp"
#include "SceneHelpers.hpp"

// -------------------------------------------------------------------------
// Helpers de material para cada BRDF
// -------------------------------------------------------------------------

static int MakePhongMat(Scene& scene)
{
    // Cerâmica azul (ns=100): difuso frio + especular branco afiado
    Phong *brdf = new Phong;
    brdf->Ka      = RGB(0.05f, 0.05f, 0.08f);
    brdf->Kd      = RGB(0.10f, 0.20f, 0.60f);
    brdf->Ks      = RGB(0.f,   0.f,   0.f);    // sem raios de espelho
    brdf->Ks_brdf = RGB(0.70f, 0.70f, 0.70f);
    brdf->Kt      = RGB(0.f,   0.f,   0.f);
    brdf->ns      = 100.f;
    return scene.AddMaterial(brdf);
}

static int MakeCookTorranceMat(Scene& scene)
{
    // Ouro metálico (metallic=1, roughness=0.3)
    CookTorrance *brdf = new CookTorrance;
    brdf->Ka        = RGB(0.02f, 0.01f, 0.00f);
    brdf->Kd        = RGB(1.00f, 0.71f, 0.29f);  // albedo dourado
    brdf->Ks        = RGB(0.f,   0.f,   0.f);
    brdf->Ks_brdf   = RGB(1.f,   1.f,   1.f);
    brdf->Kt        = RGB(0.f,   0.f,   0.f);
    brdf->roughness = 0.30f;
    brdf->metallic  = 1.00f;
    return scene.AddMaterial(brdf);
}

static int MakeOrenNayarMat(Scene& scene)
{
    // Argila rugosa (sigma=0.7): efeito "lua cheia" marcado
    OrenNayar *brdf = new OrenNayar;
    brdf->Ka      = RGB(0.02f, 0.01f, 0.01f);
    brdf->Kd      = RGB(0.70f, 0.45f, 0.25f);  // tom argila
    brdf->Ks      = RGB(0.f,   0.f,   0.f);
    brdf->Ks_brdf = RGB(0.f,   0.f,   0.f);    // puramente difuso
    brdf->Kt      = RGB(0.f,   0.f,   0.f);
    brdf->sigma   = 0.70f;
    return scene.AddMaterial(brdf);
}

static int MakeWardMat(Scene& scene)
{
    // Metal escovado anisotrópico (αx=0.05, αy=0.40, tangente horizontal)
    Ward *brdf = new Ward;
    brdf->Ka         = RGB(0.03f, 0.03f, 0.03f);
    brdf->Kd         = RGB(0.06f, 0.06f, 0.06f);   // quase preto
    brdf->Ks         = RGB(0.f,   0.f,   0.f);
    brdf->Ks_brdf    = RGB(0.90f, 0.90f, 0.90f);
    brdf->Kt         = RGB(0.f,   0.f,   0.f);
    brdf->alphaX     = 0.05f;
    brdf->alphaY     = 0.40f;
    brdf->tangent    = Vector(1.f, 0.f, 0.f);
    brdf->hasTangent = true;
    return scene.AddMaterial(brdf);
}

static int MakeAshikhminShirleyMat(Scene& scene)
{
    // Plástico polido (nu=nv=200): difuso escuro, especular branco afiado
    AshikhminShirley *brdf = new AshikhminShirley;
    brdf->Ka        = RGB(0.02f, 0.02f, 0.02f);
    brdf->Kd        = RGB(0.12f, 0.08f, 0.25f);   // violeta escuro
    brdf->Ks        = RGB(0.f,   0.f,   0.f);
    brdf->Ks_brdf   = RGB(0.80f, 0.80f, 0.80f);
    brdf->Kt        = RGB(0.f,   0.f,   0.f);
    brdf->nu        = 200.f;
    brdf->nv        = 200.f;
    brdf->hasTangent = false;
    return scene.AddMaterial(brdf);
}

// -------------------------------------------------------------------------
// Materiais das paredes (difusos simples — não disputam atenção aos objetos)
// -------------------------------------------------------------------------

static int MakeWallMat(Scene& scene, RGB const Ka, RGB const Kd)
{
    BRDF *brdf = new BRDF;
    brdf->Ka = Ka; brdf->Kd = Kd;
    brdf->Ks = RGB(0.f,0.f,0.f); brdf->Kt = RGB(0.f,0.f,0.f);
    return scene.AddMaterial(brdf);
}

// -------------------------------------------------------------------------
// Cena principal
// -------------------------------------------------------------------------

void BRDFShowcaseCornellBox(Scene& scene)
{
    // ----- Materiais das paredes -----
    int const white_mat  = MakeWallMat(scene, RGB(0.10f,0.10f,0.10f), RGB(0.60f,0.60f,0.60f));
    int const red_mat    = MakeWallMat(scene, RGB(0.10f,0.00f,0.00f), RGB(0.60f,0.00f,0.00f));
    int const green_mat  = MakeWallMat(scene, RGB(0.00f,0.10f,0.00f), RGB(0.00f,0.60f,0.00f));

    // Parede de fundo com textura Dog.ppm
    int const back_tex   = AddTextMat(scene, "Dog.ppm",
                               RGB(0.10f,0.10f,0.10f),
                               RGB(0.70f,0.70f,0.70f),
                               RGB(0.f,0.f,0.f),
                               RGB(0.f,0.f,0.f));

    // ----- Materiais dos objetos -----
    int const mat_phong  = MakePhongMat(scene);
    int const mat_ct     = MakeCookTorranceMat(scene);
    int const mat_on     = MakeOrenNayarMat(scene);
    int const mat_ward   = MakeWardMat(scene);
    int const mat_as     = MakeAshikhminShirleyMat(scene);

    // =========================================================
    // GEOMETRIA DA CORNELL BOX (coordenadas Jensen et al.)
    // =========================================================

    // Chão
    AddTriangle(scene, Point(552.8f,0.f,0.f),   Point(0.f,0.f,0.f),     Point(0.f,0.f,559.2f), white_mat);
    AddTriangle(scene, Point(549.6f,0.f,559.2f), Point(552.8f,0.f,0.f),  Point(0.f,0.f,559.2f), white_mat);

    // Tecto
    AddTriangle(scene, Point(556.f,548.8f,0.f),   Point(0.f,548.8f,0.f),     Point(0.f,548.8f,559.2f), white_mat);
    AddTriangle(scene, Point(556.f,548.8f,559.2f), Point(556.f,548.8f,0.f),  Point(0.f,548.8f,559.2f), white_mat);

    // Parede de fundo — textura
    AddTriangleUV(scene,
        Point(0.f,0.f,559.2f), Point(549.6f,0.f,559.2f), Point(556.f,548.8f,559.2f),
        Vec2(1.f,1.f), Vec2(0.f,1.f), Vec2(0.f,0.f), back_tex);
    AddTriangleUV(scene,
        Point(0.f,0.f,559.2f), Point(0.f,548.8f,559.2f), Point(556.f,548.8f,559.2f),
        Vec2(1.f,1.f), Vec2(1.f,0.f), Vec2(0.f,0.f), back_tex);

    // Parede esquerda (verde)
    AddTriangle(scene, Point(0.f,0.f,0.f),     Point(0.f,0.f,559.2f),   Point(0.f,548.8f,559.2f), green_mat);
    AddTriangle(scene, Point(0.f,0.f,0.f),     Point(0.f,548.8f,0.f),   Point(0.f,548.8f,559.2f), green_mat);

    // Parede direita (vermelha)
    AddTriangle(scene, Point(552.8f,0.f,0.f),   Point(549.6f,0.f,559.2f),   Point(549.6f,548.8f,559.2f), red_mat);
    AddTriangle(scene, Point(552.8f,0.f,0.f),   Point(552.8f,548.8f,0.f),   Point(549.6f,548.8f,559.2f), red_mat);

    // =========================================================
    // OBJETOS INTERIORES — 4 esferas + 1 bloco
    //
    //  Posições em X espacadas ao longo da largura da caixa (552u).
    //  Raio das esferas: 65u  →  base das esferas no chão (y=65)
    //  Bloco no centro-fundo
    //
    //  Vista de topo:
    //    z=200   (A)  (B)  (C)  (D)
    //    z=350              [E]
    // =========================================================

    const float r = 65.f;   // raio das esferas
    const float yS = r;     // centro Y das esferas (poisam no chão)

    // (A) Phong — cerâmica azul — esquerda
    AddSphere(scene, Point(100.f, yS, 200.f), r, mat_phong);

    // (B) Cook-Torrance — ouro — centro-esquerda
    AddSphere(scene, Point(220.f, yS, 180.f), r, mat_ct);

    // (C) Oren-Nayar — argila — centro
    AddSphere(scene, Point(340.f, yS, 190.f), r, mat_on);

    // (D) Ward — metal escovado — centro-direita
    AddSphere(scene, Point(460.f, yS, 200.f), r, mat_ward);

    // (E) Ashikhmin-Shirley — plástico violeta — caixa ao fundo
    //     Bloco 130x130x130 centrado em (278, 65, 380)
    AddBox(scene, Point(278.f, 65.f, 380.f), 65.f, mat_as);

    // =========================================================
    // ILUMINAÇÃO — área light no tecto (igual à DiffuseCornellBox)
    // 3 painéis para cobertura uniforme
    // =========================================================
    for (int lll = -1; lll < 2; lll++) {
        AreaLight *a1 = new AreaLight(
            RGB(8.f, 8.f, 8.f),
            Point(250.f + lll*150.f, 545.f, 250.f + lll*150.f),
            Point(300.f + lll*150.f, 545.f, 250.f + lll*150.f),
            Point(300.f + lll*150.f, 545.f, 300.f + lll*150.f),
            Vector(0.f, -1.f, 0.f));
        scene.lights.push_back(a1);
        scene.numLights++;

        AreaLight *a2 = new AreaLight(
            RGB(8.f, 8.f, 8.f),
            Point(250.f + lll*150.f, 545.f, 250.f + lll*150.f),
            Point(250.f + lll*150.f, 545.f, 300.f + lll*150.f),
            Point(300.f + lll*150.f, 545.f, 300.f + lll*150.f),
            Vector(0.f, -1.f, 0.f));
        scene.lights.push_back(a2);
        scene.numLights++;
    }
}