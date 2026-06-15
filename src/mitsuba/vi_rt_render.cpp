//
//  vi_rt_render.cpp
//  VI-RT — driver de renderização do MÓDULO MITSUBA (separado da main interactiva)
//
//  Renderiza, em lote, as 6 cenas "standard" usadas na comparação VI-RT vs Mitsuba 3
//  e grava cada uma como <outdir>/<cena>.ppm. Estas cenas correspondem exactamente
//  às geradas por generate_mitsuba_scenes.py (mesma geometria/luz/câmara).
//
//  Uso:  ./mitsuba_render [outdir]      (outdir por omissão: "result")
//
//  Binário próprio (alvo `make mitsuba`) → a main.cpp do projecto fica 100%
//  interactiva e independente deste módulo.
//

#include <sys/stat.h>
#include <iostream>
#include <string>
#include <time.h>
#include "scene.hpp"
#include "Perspective.hpp"
#include "StandardRenderer.hpp"
#include "ImagePPM.hpp"
#include "DistributedShader.hpp"
#include "BuildScenes.hpp"
#include "MitsubaExporter.hpp"

int main(int argc, const char* argv[]) {
    const std::string outdir    = (argc > 1) ? argv[1] : "result";   // PPMs do VI-RT
    const std::string scenesdir = (argc > 2) ? argv[2] : "scenes";   // XML/OBJ p/ Mitsuba

    // As 6 cenas standard da comparação (têm de espelhar generate_mitsuba_scenes.py)
    const std::string scenes_to_render[] = {
        "phong_spheres",
        "cook_torrance",
        "oren_nayar",
        "ward",
        "ashikhmin",
        "disney"
    };

    mkdir(outdir.c_str(), 0777);
    mkdir(scenesdir.c_str(), 0777);
    std::cout << " VI-RT (modulo Mitsuba): render -> " << outdir << "/ , XML Mitsuba -> " << scenesdir << "/" << std::endl;

    for (const std::string& scene_name : scenes_to_render) {
        Scene scene;                       // nova a cada iteração (limpa a geometria)
        const int W = 640, H = 640;
        ImagePPM *img = new ImagePPM(W, H);

        if      (scene_name == "phong_spheres") PhongSphereScene(scene);
        else if (scene_name == "cook_torrance") CookTorranceSphereScene(scene);
        else if (scene_name == "oren_nayar")    OrenNayarLambertVsON(scene);
        else if (scene_name == "ward")          WardTestStandart(scene);
        else if (scene_name == "ashikhmin")     AshikhminShirleyTestStandart(scene);
        else if (scene_name == "disney")        DisneyPresetsScene(scene);

        std::cout << "\n-> A renderizar: " << scene_name << "..." << std::endl;

        // Câmara — IDÊNTICA à do generate_mitsuba_scenes.py (CAMERA)
        const Point Eye = {0, 0.5, -5}, At = {0, 0, 3};
        const Vector Up = {0, 1, 0};
        const float fovHrad = 60.f * 3.14f / 180.f;
        const float deFocusRad = 0.f, FocusDist = 1.f;
        Perspective *cam = new Perspective(Eye, At, Up, W, H, fovHrad, deFocusRad, FocusDist);

        Shader *shd = new DistributedShader(&scene, RGB(0.05, 0.05, 0.1));
        const int spp = 64;
        const bool jitter = true;
        StandardRenderer myRender(cam, &scene, img, shd, spp, jitter);

        clock_t start = clock();
        myRender.Render();
        double secs = ((double)(clock() - start)) / CLOCKS_PER_SEC;

        const std::string filename = outdir + "/" + scene_name + ".ppm";
        img->Save(filename);
        fprintf(stdout, "   tempo = %.3lf s  ->  %s\n", secs, filename.c_str());

        // Exporta o XML do Mitsuba a partir da MESMA Scene (geometria/luz/câmara idênticas)
        MiCamera mcam;
        mcam.eye = Eye; mcam.at = At; mcam.up = Up;
        mcam.fov = 60.f; mcam.W = W; mcam.H = H; mcam.spp = 128;  // spp Mitsuba: referência limpa
        ExportMitsubaScene(scene, mcam, scenesdir + "/" + scene_name);

        delete cam; delete shd; delete img;
    }

    std::cout << "\n Todas as cenas de referencia renderizadas." << std::endl;
    return 0;
}
