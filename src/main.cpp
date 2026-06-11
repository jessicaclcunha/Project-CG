//
//  main.cpp
//  VI-RT
//
//  Created by Luis Paulo Santos on 30/01/2023.
//

#include <sys/stat.h>
#include <iostream>
#include <string>
#include <time.h>
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

int main(int argc, const char * argv[]) {
    
    // Lista de todas as cenas principais a renderizar em lote
    std::string scenes_to_render[] = {
        "phong_spheres", 
        "cook_torrance", 
        "oren_nayar", 
        "ward", 
        "ashikhmin", 
        "disney"
    };

    // Cria a pasta de resultados caso não exista
    mkdir("result", 0777);

    std::cout << " VI-RT: A iniciar renderizacao... " << std::endl;

    // Ciclo que percorre e renderiza cada cena da lista
    for (const std::string& scene_name : scenes_to_render) {
        
        Scene scene;          // Criada na stack para limpar a geometria a cada iteracao
        ImagePPM *img;        // Image
        Shader *shd;          // Shader
        clock_t start, end;
        double cpu_time_used;
        
        // Image resolution
        const int W = 640;
        const int H = 640;
        img = new ImagePPM(W, H);

        // Seleção da cena atual do ciclo
        if (scene_name == "phong_spheres") {
            PhongSphereScene(scene);
        } 
        else if (scene_name == "cook_torrance") {
            CookTorranceSphereScene(scene);
        } 
        else if (scene_name == "oren_nayar") {
            OrenNayarLambertVsON(scene);
        } 
        else if (scene_name == "ward") {
            WardTestStandart(scene);
        } 
        else if (scene_name == "ashikhmin") {
            AshikhminShirleyTestStandart(scene);
        } 
        else if (scene_name == "disney") {
            DisneyPresetsScene(scene);
        }

        std::cout << "\n-> A renderizar: " << scene_name << "..." << std::endl;

        // === Default View Point (frontal — para cenas de esferas) ===
        const Point Eye = {0, 0.5, -5}, At = {0, 0, 3};
        const Vector Up = {0, 1, 0};
        const float fovH = 60.f;

        const float deFocusRad = 0 * 3.14f / 180.f;
        const float FocusDist = 1.f;
        const float fovHrad = fovH * 3.14f / 180.f;    // to radians
        Perspective *cam = new Perspective(Eye, At, Up, W, H, fovHrad, deFocusRad, FocusDist);

        /* Shader e SPP config */
        shd = new DistributedShader(&scene, RGB(0.05, 0.05, 0.1));
        int const spp = 64;
        
        bool const jitter = true;
        StandardRenderer myRender(cam, &scene, img, shd, spp, jitter);
        
        // Renderização e contagem de tempo
        start = clock();
        myRender.Render();
        end = clock();
        
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

        // Guardar a imagem automaticamente com o respetivo nome
        std::string filename = "result/" + scene_name + ".ppm";
        img->Save(filename.c_str());
        
        fprintf(stdout, "   Tempo de execucao = %.3lf secs\n", cpu_time_used);
        std::cout << "   Guardado em: " << filename << std::endl;
        
        // Limpeza de memória da iteração atual
        delete cam;
        delete shd;
        delete img;
    }

    std::cout << "\n=========================================" << std::endl;
    std::cout << " Todas as cenas foram renderizadas!" << std::endl;
    std::cout << "=========================================" << std::endl;

    /* ========================================================================================
       Se quiseres incluir alguma destas variantes no ciclo automático, basta adicionares
       o nome dela na lista 'scenes_to_render' lá em cima e criar o respetivo 'else if'.
    ========================================================================================
    
    //--PHONG--
    //PhongCubeScene (scene);
    //PhongJustOneThing (scene);
    //PhongTextureScene(scene);

    //--COOK-TORRANCE--
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

    //--WARD -- variantes "erradas" --
    //WardIsoForcedTest(scene);          // força αx=αy (mata a anisotropia)
    //WardNoNormTest(scene);             // sem normalização 1/(4π·αx·αy)
    //WardNoDiffTest(scene);             // sem difuso Kd/π (especular puro)
    //WardNoGeomTest(scene);             // sem atenuação geométrica √(NdotL·NdotV)

    //--WARD -- alternativas correctas --
    //WardDurTest(scene);                // Dür 2006: denom linear (NdotL·NdotV)
    //WardGMDTest(scene);                // Geisler-Moroder & Dür 2010: denom (NdotH)^4
    //WardFresnelTest(scene);            // + Fresnel de Schlick no Ks

    //--ASHIKHMIN-SHIRLEY -- cenas base --
    //AshikhminShirleyScene(scene);
    //AshikhminShirleyAnisotropicScene(scene);
    //AshikhminShirleyMaterialsScene(scene);

    //--ASHIKHMIN-SHIRLEY -- exploração de variantes --
    //AshikhminShirleyFConstTest(scene);    // Fresnel constante
    //AshikhminShirleyFInvTest(scene);      // Fresnel invertido
    //AshikhminShirleyNoNormTest(scene);    // sem normalização do lóbulo
    //AshikhminShirleyLambDiffTest(scene);  // difuso Lambertiano (sem CE)
    //AshikhminShirleyNoDiffTest(scene);    // especular puro (sem difuso)

    //--ASHIKHMIN-SHIRLEY -- alternativas correctas --
    //AshikhminShirleyFExactTest(scene);    // Fresnel dieléctrico exacto
    //AshikhminShirleyFSGTest(scene);       // Fresnel spherical-gaussian

    //--DISNEY -- sweeps de parâmetro --
    //DisneyRoughnessSweep(scene);          // roughness 0→1 (metal)
    //DisneyMetallicSweep(scene);           // metallic 0→1
    //DisneyAnisoSweep(scene);              // anisotropic 0→1
    //DisneySheenSweep(scene);              // sheen 0→1
    //DisneySubsurfaceSweep(scene);         // subsurface 0→1
    //DisneyClearcoatSweep(scene);          // clearcoat 0→1

    //--Texture --
    //TextureAllBRDFsSpheres(scene);
    //TextureAllBRDFsBoxes(scene);
    //TextureUVDebug(scene);
    //CookTorranceTextureStandart(scene);
    //WardTextureStandart(scene);
    //OrenNayarTextureStandart(scene);
    //AshikhminShirleyTextureStandart(scene);

    //BRDFShowcaseCornellBox(scene);
    
    // === Cornell Box View Point (Jensen et al.) ===
    // const Point Eye = {278, 273, -800}, At = {278, 273, 0};
    // const Vector Up = {0, 1, 0};
    // const float fovH = 39.3f;
    ========================================================================================
    */
    
    return 0;
}