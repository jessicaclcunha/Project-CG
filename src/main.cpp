//
//  main.cpp
//  VI-RT
//
//  Created by Luis Paulo Santos on 30/01/2023.
//
//  Descomentar UMA cena de cada vez.

#include <sys/stat.h>
#include <iostream>
#include "scene.hpp"
#include "Perspective.hpp"
#include "DummyRenderer.hpp"
#include "StandardRenderer.hpp"
#include "ImagePPM.hpp"
#include "AmbientShader.hpp"
#include "WhittedShader.hpp"
#include "DistributedShader.hpp"
#include "PathTracingShader.hpp"
#include "AmbientLight.hpp"
#include "Sphere.hpp"
#include "BuildScenes.hpp"
#include <time.h>


int main(int argc, const char * argv[]) {
    Scene scene;
    ImagePPM *img;
    Shader *shd;
    clock_t start, end;
    double cpu_time_used;

    const int W = 640;
    const int H = 640;
    img = new ImagePPM(W, H);

    // ── PHONG ─────────────────────────────────────────────────────────────────

    // 4 esferas cinzentas, ns = 1/5/50/500 — highlight passa de mancha larga a ponto minúsculo.
    //PhongSphereScene(scene);

    // 4 cubos, ns = 1/5/50/500 — mesmo efeito de ns mas em geometria plana.
    //PhongCubeScene(scene);

    // Esfera única — cerâmica azul (trocar preset no ficheiro para outros materiais).
    //PhongJustOneThing(scene);

    // 3 esferas + 3 cubos — textura (Dog / UMinho) vs. material liso; especular não é texturizado.
    //PhongTextureScene(scene);


    // ── COOK-TORRANCE ─────────────────────────────────────────────────────────

    // 4 esferas cobre, roughness = 1.0/0.5/0.2/0.05 — lóbulo GGX de largo a espelho, com AreaLight.
    //CookTorranceSphereScene(scene);

    // Cubo ouro (metal) + plástico vermelho (dielétrico) — highlight tintado vs. branco, efeito Fresnel.
    //CookTorranceCubeScene(scene);

    // Esfera única — ouro (trocar preset no ficheiro para prata / cobre / safira / plástico).
    //CookTorranceJustOneThing(scene);

    // 3 esferas + 3 cubos — textura (Dog / UMinho) vs. material liso; especular não é texturizado.
    //CookTorranceTextureScene(scene);


    // ── OREN-NAYAR ────────────────────────────────────────────────────────────

    // 4 esferas argila, σ = 0/0.3/0.6/0.9 — σ alto → bordas tão brilhantes quanto o centro (efeito Lua).
    //OrenNayarScene(scene);

    // Esfera única — argila σ=0.6 (trocar preset no ficheiro para gesso / veludo / calcário).
    //OrenNayarJustOneThing(scene);


    // ── WARD ──────────────────────────────────────────────────────────────────

    // 4 esferas: 2 isotrópicas (highlight circular) + 2 anisotrópicas (highlight elíptico).
    //WardScene(scene);

    // Esfera única — escovado dourado (trocar preset no ficheiro para metal forte / prata / isotrópico).
    //WardJustOneThing(scene);

    // Cubo com face frontal isotrópica (vermelho) e restantes anisotrópicas (metal escovado).
    //WardCubeScene(scene); - isto nao funciona, porque o WARD funciona sobre angulos e como o cubo tem sempre 90º....o efeito desaparece


    // ── CÂMARA ────────────────────────────────────────────────────────────────
    // Usar para: todas as cenas de esferas (Phong, CookTorrance, OrenNayar, Ward)
    //const Point Eye = {0, 0.5, -5}, At = {0, 0, 3};
    //const Vector Up = {0, 1, 0};

    // Usar para: Ward — ver a orientação do highlight anisotrópico de cima (elipse horizontal vs. vertical)
    //const Point Eye = {0, 10, 3}, At = {0, 0, 3};
    //const Vector Up = {0, 0, 1};

    // Usar para: Cook-Torrance metálico — ver o efeito Fresnel a ângulos rasantes nas bordas da esfera
    //const Point Eye = {10, 0.5, 3}, At = {0, 0, 3};
    //const Vector Up = {0, 1, 0};

    // Usar para: cubos (PhongCube, CookTorranceCube, WardCube) — mostra frente + topo + lado direito
    //const Point Eye = {3, 3, -3}, At = {0, 0, 3};
    //const Vector Up = {0, 1, 0};

    const float deFocusRad = 0 * 3.14f / 180.f;
    const float FocusDist  = 1.f;
    const float fovH       = 60.f;
    const float fovHrad    = fovH * 3.14f / 180.f;
    Perspective *cam = new Perspective(Eye, At, Up, W, H, fovHrad, deFocusRad, FocusDist);

    // ── SHADER ────────────────────────────────────────────────────────────────

    shd = new DistributedShader(&scene, RGB(0.05, 0.05, 0.1));
    int const spp = 128;  // aumentar para 256/512 em renders finais

    //shd = new WhittedShader(&scene, RGB(0., 0., 0.2));  // iteração rápida (sem AreaLight soft shadows)
    //int const spp = 1;

    bool const jitter = true;
    StandardRenderer myRender(cam, &scene, img, shd, spp, jitter);

    start = clock();
    myRender.Render();
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    mkdir("result", 0777);
    img->Save("result/reference.ppm");

    fprintf(stdout, "Rendering time = %.3lf secs\n\n", cpu_time_used);
    std::cout << "That's all, folks!" << std::endl;
    return 0;
}