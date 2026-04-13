//
//  WardScenes.cpp
//  VI-RT
//
// Câmara: Eye=(0, 0.5, -5), At=(0, 0, 3), Up=(0,1,0), FOV=60°

#include "BuildScenes.hpp"
#include "SceneHelpers.hpp"
#include "Ward.hpp"

static void AddLights(Scene& scene) {
    AmbientLight *ambient = new AmbientLight(RGB(0.06f, 0.05f, 0.04f));
    scene.lights.push_back(ambient);
    scene.numLights++;
    PointLight *fill = new PointLight(RGB(300.f, 300.f, 300.f), Point(0.f, 1.f, -3.f));
    scene.lights.push_back(fill);
    scene.numLights++;
}

void WardScene(Scene& scene)
{
    RGB const Ka(0.05f, 0.04f, 0.03f);
    RGB const Kd(0.20f, 0.16f, 0.10f);
    RGB const Ks(0.90f, 0.75f, 0.50f);

    int const s1 = AddWardMat (scene, Ka, Kd, Ks, 0.30f, 0.30f);
    int const s2 = AddWardMat (scene, Ka, Kd, Ks, 0.08f, 0.08f);
    int const s3 = AddWardMatT(scene, Ka, Kd, Ks, 0.15f, 0.40f, Vector(1.f,0.f,0.f));

    int const s4 = AddWardMatT(scene, Ka, Kd, Ks, 0.10f, 0.40f, Vector(1.f,0.f,0.f));

    AddSphere(scene, Point( 3.f, 0.5f, 3.f), 0.8f, s1);
    AddSphere(scene, Point( 1.f, 0.5f, 3.f), 0.8f, s2);
    AddSphere(scene, Point(-1.f, 0.5f, 3.f), 0.8f, s3);
    AddSphere(scene, Point(-3.f, 0.5f, 3.f), 0.8f, s4); 
    
    AddLights(scene);
}

// ---------------------------------------------------------------------------
// WardJustOneThing
// ---------------------------------------------------------------------------
void WardJustOneThing(Scene& scene)
{
    RGB const Ka(0.05f, 0.04f, 0.03f);
    RGB const Kd(0.20f, 0.16f, 0.10f);
    RGB const Ks(0.95f, 0.80f, 0.60f);

    /* METAL ESCOVADO DOURADO */
    int const mat = AddWardMatT(scene, Ka, Kd, Ks, 0.15f, 0.40f,
                                Vector(1.f, 0.f, 0.f));
    /* METAL FORTE
    int const mat = AddWardMatT(scene, Ka, Kd, Ks, 0.05f, 0.40f,
                                Vector(1.f, 0.f, 0.f));
    */
    /* PRATA
    int const mat = AddWardMatT(scene,
        RGB(0.04f,0.04f,0.04f), RGB(0.15f,0.15f,0.15f), RGB(0.95f,0.95f,0.95f),
        0.15f, 0.40f, Vector(1.f,0.f,0.f));
    */
    /* ISOTRÓPICO
    int const mat = AddWardMat(scene, Ka,
        RGB(0.50f,0.10f,0.10f), RGB(0.80f,0.80f,0.80f), 0.20f, 0.20f);
    */

    AddSphere(scene, Point(0.f, 0.5f, 3.f), 0.8f, mat);

    AddLights(scene);
}

// ---------------------------------------------------------------------------
// WardCubeScene
// ---------------------------------------------------------------------------
void WardCubeScene(Scene& scene)
{
    RGB const Ka(0.05f, 0.04f, 0.03f);

    int const matFront = AddWardMat(scene, Ka,
        RGB(0.35f, 0.05f, 0.05f),
        RGB(0.70f, 0.70f, 0.70f),
        0.25f, 0.25f);

    int const matMetal = AddWardMatT(scene, Ka,
        RGB(0.15f, 0.12f, 0.06f),
        RGB(0.95f, 0.80f, 0.50f),
        0.05f, 0.40f, Vector(1.f, 0.f, 0.f));

    AddBoxMultiMat(scene, Point(0.f, -0.7f, 3.f), 0.8f,
                   matFront, matMetal, matMetal, matMetal, matFront, matMetal);

    AddLights(scene);
}