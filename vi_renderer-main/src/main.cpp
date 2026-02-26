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
#include "OrthographicCamera.hpp"
#include "FisheyeCamera.hpp" 
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

// ============================================
// TONE MAPPER INCLUDES 
// ============================================
#include "ToneMapper/ReinhardBasic.hpp"
#include "ToneMapper/ReinhardAdvanced.hpp"
#include "ToneMapper/ACES.hpp"
#include "ToneMapper/Exposure.hpp"
#include "ToneMapper/Linear.hpp"

int main(int argc, const char * argv[]) {
    Scene scene;
    ImagePPM *img;    
    Shader *shd;      
    clock_t start, end;
    double cpu_time_used;
    
    const int W = 640;
    const int H = 640;
    img = new ImagePPM(W, H);
    
    DLightChallenge(scene);
    //CornellBox(scene);
    scene.printSummary();
    //MassiveSphereScene(scene, 10000);

    //  === BVH PARA RAYS, MATERIALS, ... E BVH EXLCUSIVO DE LUZES  ===
    scene.BuildBVH(); // documentar para não funcionar
    scene.BuildLightBVH(); // documentar para não funcionar

    //  === Default View Point  ===
    const Point Eye = {280, 265, -500}, At = {280, 260, 0};  
    const Vector Up = {0, 1, 0};  

    // === Up View Point  ===
    //const Point Eye = {275, 500, 300}, At = {275, 100, 300};
    //const Vector Up = {0, 0, 1};   
    
    // === Lateral View Point  ===
    //const Point Eye = {500, 274, 300}, At = {200, 274, 300};
    //const Vector Up = {0, 1, 0};

    //  === TYPES OF CAMERAS  ===
    //#define USE_ORTHOGRAPHIC 
    //#define USE_FISHEYE

    // === How to test Fisheye camera ===
    //FisheyeTestScene(scene);
    //const Point Eye = {0, 200, 0}, At = {0, 200, 100}; 
    //const Vector Up = {0, 1, 0};
    
    Camera *cam;
    
    #ifdef USE_FISHEYE
        float fov_degrees = 180.0f;
        cam = new FisheyeCamera(Eye, At, Up, W, H, fov_degrees);
        fprintf(stdout, "Using FISHEYE camera (fov=%.1f°)\n", fov_degrees);
        
    #elif defined(USE_ORTHOGRAPHIC)
        float ortho_width = 800.0f;
        cam = new OrthographicCamera(Eye, At, Up, W, H, ortho_width);
        fprintf(stdout, "Using ORTHOGRAPHIC camera (width=%.1f)\n", ortho_width);
        
    #else
        const float fovH = 60.f;
        const float fovHrad = fovH*3.14f/180.f;
        const float deFocusRad = 0*3.14f/180.f;
        const float FocusDist = 1.;
        cam = new Perspective(Eye, At, Up, W, H, fovHrad, deFocusRad, FocusDist);
        fprintf(stdout, "Using PERSPECTIVE camera (fov=%.1f°)\n", fovH);
    #endif

    /*   Dummy */
    // create the shader
    //shd = new DummyShader(&scene, W, H);
    // declare the renderer
    //DummyRenderer myRender (cam, &scene, img, shd);

    /*   Standard */
    // create the shader
    //shd = new AmbientShader(&scene, RGB(0.1,0.1,0.8));
    //shd = new WhittedShader(&scene, RGB(0.1,0.1,0.8));
    //shd = new DistributedShader(&scene, RGB(0.1,0.1,0.8));
    shd = new PathTracing(&scene, RGB(0., 0., 0.2));

    int const spp = 16;
    bool const jitter = true;

    StandardRenderer myRender(cam, &scene, img, shd, spp, jitter);
    
    start = clock();
    
    myRender.Render();  

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    // ============================================
    // TONE MAPPING (Aplicar aos dados da imagem)
    // ============================================
    
    // Choose tone mapping algorithm, If none defined, uses default ImagePPM tone mapping
    //#define USE_REINHARD_BASIC_TONEMAP
    //#define USE_REINHARD_ADVANCED_TONEMAP  
    //#define USE_ACES_TONEMAP
    //#define USE_EXPOSURE_TONEMAP
    //#define USE_LINEAR_TONEMAP

    RGB max_rgb(0, 0, 0), min_rgb(999, 999, 999);
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            RGB pixel = img->get(x, y);
            max_rgb.R = std::max(max_rgb.R, pixel.R);
            max_rgb.G = std::max(max_rgb.G, pixel.G);
            max_rgb.B = std::max(max_rgb.B, pixel.B);
            min_rgb.R = std::min(min_rgb.R, pixel.R);
            min_rgb.G = std::min(min_rgb.G, pixel.G);
            min_rgb.B = std::min(min_rgb.B, pixel.B);
        }
    }
    fprintf(stdout, "HDR Range - Min: (%.3f, %.3f, %.3f) Max: (%.3f, %.3f, %.3f)\n", 
            min_rgb.R, min_rgb.G, min_rgb.B, max_rgb.R, max_rgb.G, max_rgb.B);
    
    #ifdef USE_ACES_TONEMAP
        fprintf(stdout, "=== APPLYING ACES TONE MAPPING ===\n");
        
        RGB *hdr_data = new RGB[W * H];
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                hdr_data[y * W + x] = img->get(x, y);
            }
        }

        RGB *ldr_data = new RGB[W * H];
        ACES toneMapper(1.5f, 2.2f); 
        toneMapper.ToneMap(W, H, hdr_data, ldr_data);

        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                img->set(x, y, ldr_data[y * W + x]);
            }
        }

        fprintf(stdout, "Applied ACES tone mapping (exposure=2.5)\n");
        delete[] hdr_data;
        delete[] ldr_data;

    #elif defined(USE_REINHARD_BASIC_TONEMAP)
        fprintf(stdout, "=== APPLYING REINHARD BASIC TONE MAPPING ===\n");
        
        RGB *hdr_data = new RGB[W * H];
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                hdr_data[y * W + x] = img->get(x, y);
            }
        }

        RGB *ldr_data = new RGB[W * H];
        ReinhardBasic toneMapper;
        toneMapper.ToneMap(W, H, hdr_data, ldr_data);

        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                img->set(x, y, ldr_data[y * W + x]);
            }
        }

        fprintf(stdout, "Applied REINHARD BASIC tone mapping\n");
        delete[] hdr_data;
        delete[] ldr_data;
        
    #elif defined(USE_REINHARD_ADVANCED_TONEMAP)
        #include "ToneMapper/ReinhardAdvanced.hpp"
        
        RGB *hdr_data = new RGB[W * H];
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                hdr_data[y * W + x] = img->get(x, y);
            }
        }
        
        RGB *ldr_data = new RGB[W * H];
        ReinhardAdvanced toneMapper(1.0f, 2.2f);
        toneMapper.ToneMap(W, H, hdr_data, ldr_data);
        
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                img->set(x, y, ldr_data[y * W + x]);
            }
        }
        
        fprintf(stdout, "Applied REINHARD ADVANCED tone mapping\n");
        
        delete[] hdr_data;
        delete[] ldr_data;
        
    #elif defined(USE_EXPOSURE_TONEMAP)
        fprintf(stdout, "=== APPLYING EXPOSURE TONE MAPPING ===\n");
        
        RGB *hdr_data = new RGB[W * H];
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                hdr_data[y * W + x] = img->get(x, y);
            }
        }

        RGB *ldr_data = new RGB[W * H];
        Exposure toneMapper(1.0f, 2.2f); 
        toneMapper.ToneMap(W, H, hdr_data, ldr_data);

        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                img->set(x, y, ldr_data[y * W + x]);
            }
        }

        fprintf(stdout, "Applied EXPOSURE tone mapping (exposure=0.3)\n");
        delete[] hdr_data;
        delete[] ldr_data;
        
    #elif defined(USE_LINEAR_TONEMAP)
        #include "ToneMapper/Linear.hpp"
        
        RGB *hdr_data = new RGB[W * H];
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                hdr_data[y * W + x] = img->get(x, y);
            }
        }
        
        RGB *ldr_data = new RGB[W * H];
        Linear toneMapper(1.0f, 2.2f);
        toneMapper.ToneMap(W, H, hdr_data, ldr_data);
        
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                img->set(x, y, ldr_data[y * W + x]);
            }
        }
        
        fprintf(stdout, "Applied LINEAR tone mapping\n");
        
        delete[] hdr_data;
        delete[] ldr_data;
        
    #else
        fprintf(stdout, "=== USING DEFAULT TONE MAPPING ===\n");
        fprintf(stdout, "Using default ImagePPM tone mapping (Reinhard)\n");
    #endif

    mkdir("result", 0777);
    img->Save("result/reference.ppm");
    
    fprintf(stdout, "Rendering time = %.3lf secs\n\n", cpu_time_used);
    
    std::cout << "That's all, folks!" << std::endl;
    return 0;
}