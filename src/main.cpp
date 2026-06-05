//
//  main.cpp
//  VI-RT
//
//  Created by Luis Paulo Santos on 30/01/2023.
//

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
    ImagePPM *img;    // Image
    Shader *shd;      // Shader
    clock_t start, end;
    double cpu_time_used;
    
    // Image resolution
    const int W= 640;
    const int H= 640;

    img = new ImagePPM(W,H);
    
    /* Scenes*/
    //DLightChallenge(scene);

    // -- PHONG --
    //PhongSphereScene(scene);
    //PhongCubeScene(scene);
    //PhongJustOneThing(scene);
    //PhongTextureScene(scene);      // <- textura original Phong (esfera + cubo)

    // -- COOK-TORRANCE --
    //CookTorranceSphereScene(scene);
    //CookTorranceShowcase(scene);
    //CookTorranceTextureScene(scene); // <- textura original CT (esfera + cubo)
    //CookTorranceTestStandart(scene);
    //CookTorranceNoECTest2(scene);
    //CookTorranceFConstTest(scene);
    //CookTorranceFExpTest(scene);
    //CookTorranceFInvTest(scene);
    //CookTorranceKDMetalTest(scene);
    //CookTorranceKDLerpTest(scene);
    //CookTorranceGNoneTest(scene);
    //CookTorranceGOneTest(scene);
    //CookTorranceGKelemenTest(scene);

    // -- WARD --
    //WardScene(scene);
    //WardJustOneThing(scene);
    //WardCubeScene(scene);

    // -- OREN-NAYAR --
    //OrenNayarScene(scene);
    //OrenNayarJustOneThing(scene);

    // -- ASHIKHMIN-SHIRLEY -- cenas base --
    //AshikhminShirleyScene(scene);
    //AshikhminShirleyAnisotropicScene(scene);
    //AshikhminShirleyMaterialsScene(scene);
    //AshikhminShirleyJustOneThing(scene);

    // -- ASHIKHMIN-SHIRLEY -- exploração de variantes --
    //AshikhminShirleyTestStandart(scene);
    //AshikhminShirleyFConstTest(scene);
    //AshikhminShirleyFInvTest(scene);
    //AshikhminShirleyNoNormTest(scene);
    //AshikhminShirleyLambDiffTest(scene);
    //AshikhminShirleyNoDiffTest(scene);

    //TextureAllBRDFsSpheres(scene); //4 esferas: Phong / Cook-Torrance / Oren-Nayar / Ward todos com Dog.ppm
    //TextureAllBRDFsBoxes(scene); //4 cubos com UV mapeamento per-face todos com UMinho.ppm para Phong, Cook-Torrance, Ward, Oren-Nayar
    //                           NOTA: usar FOV=80° ou Eye=(0,0.5,-8) para ver tudo
    TextureUVDebug(scene); // validação do UV mapping: esfera (UV esférico) + cubo (UV per-face) com Dog e UMinho

    // =========================================================================
    // Câmara
    // =========================================================================

    // === Default (frontal) — para a maioria das cenas ===
    const Point Eye = {0, 0.5, -5}, At = {0, 0, 3};
    const Vector Up = {0, 1, 0};

    // === Recuado — para TextureFullShowcase (7 colunas) ===
    //const Point Eye = {0, 0.5, -8}, At = {0, 0, 3};
    //const Vector Up = {0, 1, 0};

    // === Up View Point ===
    //const Point Eye = {0, 10, 3}, At = {0, 0, 3};
    //const Vector Up = {0, 0, 1};

    // === Lateral View Point (lado direito, diagonal) ===
    //const Point Eye = {8, 0.5, -2}, At = {0, 0, 3};
    //const Vector Up = {0, 1, 0};

    // === Diagonal View Point (frontal + topo) ===
    // const Point Eye = {3, 3, -3}, At = {0, 0, 3};
    // const Vector Up = {0, 1, 0};

    const float deFocusRad = 0*3.14f/180.f;
    const float FocusDist = 1.f;
    const float fovH = 60.f;
    const float fovHrad = fovH*3.14f/180.f;    // to radians
    //Perspective *cam = new Perspective(Eye, At, Up, W, H, fovHrad);
    Perspective *cam = new Perspective(Eye, At, Up, W, H, fovHrad, deFocusRad, FocusDist);

    /* Shader */
    shd = new DistributedShader(&scene, RGB(0.05,0.05,0.1));
    int const spp=218;

    //shd = new WhittedShader(&scene, RGB(0.,0.,0.2));
    //int const spp=1;


    
    bool const jitter=true;
    StandardRenderer myRender (cam, &scene, img, shd, spp, jitter);
    // render
    start = clock();
    myRender.Render();
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    // save the image
    mkdir("result", 0777);
    img->Save("result/reference.ppm");
    
    fprintf (stdout, "Rendering time = %.3lf secs\n\n", cpu_time_used);
    
    std::cout << "That's all, folks!" << std::endl;
    return 0;
}