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
    //PhongSphereScene(scene);
    //PhongCubeScene (scene);
    //PhongJustOneThing (scene);
    //CookTorranceSphereScene(scene);
    CookTorranceCubeScene(scene);
    //CookTorranceJustOneThing (scene);
    

    //  === Default View Point (frontal) ===
    const Point Eye = {0, 0.5, -5}, At = {0, 0, 3};
    const Vector Up = {0, 1, 0};

    // === Up View Point (topo) ===
    //const Point Eye = {0, 10, 3}, At = {0, 0, 3};
    //const Vector Up = {0, 0, 1};

    // === Lateral View Point (lado direito) ===
    //const Point Eye = {10, 0.5, 3}, At = {0, 0, 3};
    //const Vector Up = {0, 1, 0};

    const float deFocusRad = 0*3.14f/180.f;
    const float FocusDist = 1.f;
    const float fovH = 60.f;
    const float fovHrad = fovH*3.14f/180.f;    // to radians
    //Perspective *cam = new Perspective(Eye, At, Up, W, H, fovHrad);
    Perspective *cam = new Perspective(Eye, At, Up, W, H, fovHrad, deFocusRad, FocusDist);

    /* Shader */
    shd = new DistributedShader(&scene, RGB(0.,0.,0.2));
    int const spp=128;

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


