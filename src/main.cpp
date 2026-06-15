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

    //--PHONG--
    //PhongSphereScene(scene);
    //PhongCubeScene (scene);
    //PhongJustOneThing (scene);
    //PhongTextureScene(scene);

    // ----------------------------------------------- // -----------------------------------------------

    //--COOK-TORRANCE--
    //CookTorranceSphereScene(scene);
    //CookTorranceShowcase(scene);

    //--COOK-TORRANCE-- SCENE DE CONTROLO --
    //CookTorranceTestStandart(scene); // Standart

    //--COOK-TORRANCE-- EC --
    //CookTorranceNoECTest2(scene);   // sem EC

    //--COOK-TORRANCE -- alternativas de F "erradas" --
    //CookTorranceFConstTest(scene);  // Fresnel constante (F=F0)
    //CookTorranceFExpTest(scene);    // Fresnel expoente 20
    //CookTorranceFInvTest(scene);    // Fresnel invertido

    //--COOK-TORRANCE -- alternativas de kd "erradas" --
    //CookTorranceKDMetalTest(scene); // kD = metallic (papel invertido)
    //CookTorranceKDLerpTest(scene);  // kD fixo 0.5/0.5

    //--COOK-TORRANCE -- alternativas de G "erradas" --
    //CookTorranceGNoneTest(scene);   // G = 1 (sem shadowing-masking)
    //CookTorranceGOneTest(scene);    // G = G1L apenas
    //CookTorranceGKelemenTest(scene);// G Kelemen: F*D / (4*VdotH^2)

    //--COOK-TORRANCE -- alternativas correctas de D, F, G --
    //CookTorranceDBeckmannTest(scene);   // D Beckmann (original CT 1982): cauda mais curta que GGX
    //CookTorranceDBlinnPhongTest(scene); // D Blinn-Phong NDF: cauda mínima, highlight abrupto
    //CookTorranceFExactTest(scene);      // F exacto (Fresnel dieléctrico com IOR variado)
    //CookTorranceFSGTest(scene);         // F Spherical Gaussian (Karis/Unreal): quase igual a Schlick
    //CookTorranceGCT1982Test(scene);     // G original paper 1982: min(1, 2NdotH*NdotV/VdotH, ...)
    //CookTorranceGSmithIBLTest(scene);   // G Smith-GGX IBL: k=roughness²/2 (mais shadow-masking)

    // ----------------------------------------------- // -----------------------------------------------

    //--OREN-NAYAR -- cena base (demo) --
    //OrenNayarLambertVsON(scene);        // contraste Lambert vs ON — luz de farol

    //--OREN-NAYAR -- SCENE DE CONTROLO --
    //OrenNayarTestStandart(scene);       // Standart (ON simplificado)

    //--OREN-NAYAR -- variantes "erradas" --
    //OrenNayarLambertTest(scene);        // Lambert puro (sigma=0): tudo o que o ON acrescenta
    //OrenNayarNoBTest(scene);            // sem termo B (sem retroreflexão)
    //OrenNayarAFixoTest(scene);          // A=1 fixo (sem escurecimento)
    //OrenNayarNoClampTest(scene);        // sem clamp do azimute (cosΔφ negativo)

    //--OREN-NAYAR -- alternativas correctas --
    //OrenNayarFujiiTest(scene);          // Fujii energy-preserving (quase igual ao standard)
    //OrenNayarFullTest(scene);           // ON completo 1994 (C1/C2/C3)
    //OrenNayarFullInterTest(scene);      // Full + inter-reflexão L2 (Kd²)

    // ----------------------------------------------- // -----------------------------------------------

    //--WARD -- setup do estudo --
    //WardTestStandart(scene);           // standard (4 esferas: iso liso/rugoso + aniso/aniso forte)

    //--WARD -- variantes "erradas" --
    //WardIsoForcedTest(scene);          // força αx=αy (mata a anisotropia)
    //WardNoNormTest(scene);             // sem normalização 1/(4π·αx·αy)
    //WardNoDiffTest(scene);             // sem difuso Kd/π (especular puro)
    //WardNoGeomTest(scene);             // sem atenuação geométrica √(NdotL·NdotV)

    //--WARD -- alternativas correctas --
    //WardDurTest(scene);                // Dür 2006: denom linear (NdotL·NdotV)
    //WardGMDTest(scene);                // Geisler-Moroder & Dür 2010: denom (NdotH)^4
    //WardFresnelTest(scene);            // + Fresnel de Schlick no Ks

    // ----------------------------------------------- // -----------------------------------------------

    //--ASHIKHMIN-SHIRLEY -- cenas base --
    //AshikhminShirleyScene(scene);
    //AshikhminShirleyAnisotropicScene(scene);
    //AshikhminShirleyMaterialsScene(scene);

    //--WARD -- setup do estudo --
    //AshikhminShirleyTestStandart(scene);    // referência

    //--ASHIKHMIN-SHIRLEY -- exploração de variantes --
    //AshikhminShirleyFConstTest(scene);    // Fresnel constante
    //AshikhminShirleyFInvTest(scene);      // Fresnel invertido
    //AshikhminShirleyNoNormTest(scene);    // sem normalização do lóbulo
    //AshikhminShirleyLambDiffTest(scene);  // difuso Lambertiano (sem CE)
    //AshikhminShirleyNoDiffTest(scene);    // especular puro (sem difuso)

    //--ASHIKHMIN-SHIRLEY -- alternativas correctas --
    //AshikhminShirleyFExactTest(scene);    // Fresnel dieléctrico exacto
    //AshikhminShirleyFSGTest(scene);       // Fresnel spherical-gaussian

    // ----------------------------------------------- // -----------------------------------------------

    //--DISNEY -- demo de presets --
    //DisneyPresetsScene(scene);              // metal/plástico/tecido/verniz/cera

    //--DISNEY -- sweeps de parâmetro --
    //DisneyRoughnessSweep(scene);          // roughness 0→1 (metal)
    //DisneyMetallicSweep(scene);           // metallic 0→1
    //DisneyAnisoSweep(scene);              // anisotropic 0→1
    //DisneySheenSweep(scene);              // sheen 0→1
    //DisneySubsurfaceSweep(scene);         // subsurface 0→1
    //DisneyClearcoatSweep(scene);          // clearcoat 0→1

    // ----------------------------------------------- // -----------------------------------------------

    //--Texture --
    //TextureAllBRDFsSpheres(scene);    //4 esferas: Phong / Cook-Torrance / Oren-Nayar / Ward todos com Dog.ppm
    TextureAllBRDFsBoxes(scene);      //4 cubos com UV mapeamento per-face todos com UMinho.ppm para Phong, Cook-Torrance, Ward, Oren-Nayar
    //                                    NOTA: usar FOV=80° ou Eye=(0,0.5,-8) para ver tudo
    //TextureUVDebug(scene);              // validação do UV mapping: esfera (UV esférico) + cubo (UV per-face) com Dog e UMinho
    //CookTorranceTextureStandart(scene);
    //WardTextureStandart(scene);
    //OrenNayarTextureStandart(scene);
    //AshikhminShirleyTextureStandart(scene);
    // ----------------------------------------------- // -----------------------------------------------

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


