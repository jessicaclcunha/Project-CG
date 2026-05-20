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

    //--PHONG--
    //PhongSphereScene(scene);
    //PhongCubeScene (scene);
    //PhongJustOneThing (scene);
    //PhongTextureScene(scene);

    //--COOK-TORRANCE--
    //CookTorranceSphereScene(scene);
    //CookTorranceShowcase(scene);
    //CookTorranceTestStandart(scene); // Standart: com EC
    //CookTorranceNoECTest2(scene);   // Teste:    sem EC
    //CookTorranceFConstTest(scene);  // Fresnel constante (F=F0)
    //CookTorranceFExpTest(scene);    // Fresnel expoente 20
    //CookTorranceFInvTest(scene);    // Fresnel invertido
    //CookTorranceKDMetalTest(scene); // kD = metallic (papel invertido)
    //CookTorranceKDLerpTest(scene);  // kD fixo 0.5/0.5
    //CookTorranceGNoneTest(scene);   // G = 1 (sem shadowing-masking)
    //CookTorranceGOneTest(scene);    // G = G1L apenas
    //CookTorranceGKelemenTest(scene);// G Kelemen: F*D / (4*VdotH^2)

    //--WARD--
    //WardScene(scene);
    //WardJustOneThing(scene);
    //WardCubeScene(scene);

    //--ASHIKHMIN-SHIRLEY -- cenas base --
    //AshikhminShirleyScene(scene);
    //AshikhminShirleyAnisotropicScene(scene);
    AshikhminShirleyMaterialsScene(scene);
    //AshikhminShirleyJustOneThing(scene);
 
    //--ASHIKHMIN-SHIRLEY -- exploração de variantes --
    //AshikhminShirleyTestStandart(scene);    // referência
    //AshikhminShirleyFConstTest(scene);    // Fresnel constante
    //AshikhminShirleyFInvTest(scene);      // Fresnel invertido
    //AshikhminShirleyNoNormTest(scene);    // sem normalização do lóbulo
    //AshikhminShirleyLambDiffTest(scene);  // difuso Lambertiano (sem CE)
    //AshikhminShirleyNoDiffTest(scene);    // especular puro (sem difuso)
    
    

    //  === Default View Point (frontal) ===
    const Point Eye = {0, 0.5, -5}, At = {0, 0, 3};
    const Vector Up = {0, 1, 0};

    // === Up View Point (topo) ===
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


