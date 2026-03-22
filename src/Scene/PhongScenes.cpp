//
//  PhongScenes.cpp
//  VI-RT
//
//  Phong BRDF scenes.
//

#include "BuildScenes.hpp"
#include "SceneHelpers.hpp"

static int AddPhongMat (Scene& scene, RGB const Ka, RGB const Kd, RGB const Ks, float const ns) {
    Phong *brdf = new Phong;
    brdf->Ka = Ka;
    brdf->Kd = Kd;
    brdf->Ks     = RGB(0., 0., 0.);
    brdf->Ks_brdf = Ks;
    brdf->Kt = RGB(0., 0., 0.);
    brdf->ns = ns;
    return (scene.AddMaterial(brdf));
}

static int AddPhongTexMat (Scene& scene, std::string filename, RGB const Ka, RGB const Kd, RGB const Ks, float const ns) {
    PhongTexture *brdf = new PhongTexture(filename);
    brdf->Ka = Ka;
    brdf->Kd = Kd;
    brdf->Ks     = RGB(0., 0., 0.);
    brdf->Ks_brdf = Ks;
    brdf->Kt = RGB(0., 0., 0.);
    brdf->ns = ns;
    return (scene.AddMaterial(brdf));
}

void PhongSphereScene (Scene& scene) {
    int const diff_mat = AddPhongMat(scene,
        RGB(0.05f, 0.05f, 0.05f),
        RGB(0.6f,  0.2f,  0.2f),
        RGB(0.0f,  0.0f,  0.0f),
        1.f);

    // ns = 5
    int const phong_rough = AddPhongMat(scene,
        RGB(0.05f, 0.05f, 0.05f),
        RGB(0.2f,  0.4f,  0.7f),   // azul
        RGB(0.8f,  0.8f,  0.8f),
        5.f);

    // ns = 50
    int const phong_mid = AddPhongMat(scene,
        RGB(0.05f, 0.05f, 0.05f),
        RGB(0.2f,  0.6f,  0.2f),   // verde
        RGB(0.8f,  0.8f,  0.8f),
        50.f);

    // ns = 500
    int const phong_shiny = AddPhongMat(scene,
        RGB(0.05f, 0.05f, 0.05f),
        RGB(0.6f,  0.4f,  0.1f),   // laranja
        RGB(0.9f,  0.9f,  0.9f),
        500.f);

    AddSphere(scene, Point(-3.f, 0.f, 5.f), 0.8f, diff_mat);
    AddSphere(scene, Point(-1.f, 0.f, 5.f), 0.8f, phong_rough);
    AddSphere(scene, Point( 1.f, 0.f, 5.f), 0.8f, phong_mid);
    AddSphere(scene, Point( 3.f, 0.f, 5.f), 0.8f, phong_shiny);

    // ajustar aqui caso vejamos que esteja mt escuro, claro e coisas assim
    AmbientLight *ambient = new AmbientLight(RGB(0.05f, 0.05f, 0.05f));
    scene.lights.push_back(ambient);
    scene.numLights++;
    //PointLight *p1 = new PointLight(RGB(1.f, 1.f, 1.f), Point(0.f, 3.f, 0.f)); -- fica mt escuro
    PointLight *p1 = new PointLight(RGB(300.f, 300.f, 300.f), Point(-1.f, 2.f, 0.f));
    scene.lights.push_back(p1);
    scene.numLights++;
}

void PhongCubeScene (Scene& scene) {
    int const diff_mat = AddPhongMat(scene,
        RGB(0.05f, 0.05f, 0.05f),
        RGB(0.6f,  0.2f,  0.2f),
        RGB(0.0f,  0.0f,  0.0f),
        1.f);

    // ns = 5
    int const phong_rough = AddPhongMat(scene,
        RGB(0.05f, 0.05f, 0.05f),
        RGB(0.2f,  0.4f,  0.7f),   // azul
        RGB(0.8f,  0.8f,  0.8f),
        5.f);

    // ns = 50
    int const phong_mid = AddPhongMat(scene,
        RGB(0.05f, 0.05f, 0.05f),
        RGB(0.2f,  0.6f,  0.2f),   // verde
        RGB(0.8f,  0.8f,  0.8f),
        50.f);

    // ns = 500
    int const phong_shiny = AddPhongMat(scene,
        RGB(0.05f, 0.05f, 0.05f),
        RGB(0.6f,  0.4f,  0.1f),   // laranja
        RGB(0.9f,  0.9f,  0.9f),
        500.f);

    AddBox(scene, Point(-3.f, 0.f, 5.f), 0.8f, diff_mat);
    AddBox(scene, Point(-1.f, 0.f, 5.f), 0.8f, phong_rough);
    AddBox(scene, Point( 1.f, 0.f, 5.f), 0.8f, phong_mid);
    AddBox(scene, Point( 3.f, 0.f, 5.f), 0.8f, phong_shiny);

    AmbientLight *ambient = new AmbientLight(RGB(0.05f, 0.05f, 0.05f));
    scene.lights.push_back(ambient);
    scene.numLights++;
    PointLight *p1 = new PointLight(RGB(300.f, 300.f, 300.f), Point(-1.f, 2.f, 0.f));
    scene.lights.push_back(p1);
    scene.numLights++;
}

void PhongJustOneThing (Scene& scene) {

    /* PLÁSTICO VERMELHO 
    RGB const Ka(0.05f, 0.05f, 0.05f);
    RGB const Kd(0.6f,  0.2f,  0.2f);
    RGB const Ks(0.8f,  0.8f,  0.8f);
    float const ns = 50.f;
    */
    /* METAL POLIDO
    RGB const Ka(0.05f, 0.05f, 0.05f);
    RGB const Kd(0.3f,  0.3f,  0.3f);
    RGB const Ks(0.9f,  0.9f,  0.9f);
    float const ns = 500.f;
    */
    /* BORRACHA
    RGB const Ka(0.02f, 0.02f, 0.02f);
    RGB const Kd(0.4f,  0.4f,  0.1f);
    RGB const Ks(0.1f,  0.1f,  0.1f);
    float const ns = 5.f;
    */
    /* CERÂMICA AZUL*/
    RGB const Ka(0.05f, 0.05f, 0.08f);
    RGB const Kd(0.1f,  0.2f,  0.6f);
    RGB const Ks(0.7f,  0.7f,  0.7f);
    float const ns = 100.f;
    

    int const app = AddPhongMat(scene, Ka, Kd, Ks, ns);

    AddSphere(scene, Point(0.f, 0.f, 3.f), 0.8f, app);

    AmbientLight *ambient = new AmbientLight(RGB(0.05f, 0.05f, 0.05f));
    scene.lights.push_back(ambient);
    scene.numLights++;

    PointLight *p1 = new PointLight(RGB(600.f, 600.f, 600.f), Point(0.f, 2.f, -1.f));
    scene.lights.push_back(p1);
    scene.numLights++;
}

/*
// 3 esferas: sem textura (referência), Dog.ppm, UMinho.ppm — especular Phong
void PhongTextureScene (Scene& scene) {
    int const plain = AddPhongMat(scene,
        RGB(0.05f, 0.05f, 0.05f),
        RGB(0.6f, 0.2f, 0.2f),
        RGB(0.8f, 0.8f, 0.8f),
        50.f);

    int const tex_dog = AddPhongTexMat(scene, "Dog.ppm",
        RGB(0.05f, 0.05f, 0.05f),
        RGB(1.0f, 1.0f, 1.0f),
        RGB(0.8f, 0.8f, 0.8f),
        50.f);

    int const tex_um = AddPhongTexMat(scene, "UMinho.ppm",
        RGB(0.05f, 0.05f, 0.05f),
        RGB(1.0f, 1.0f, 1.0f),
        RGB(0.8f, 0.8f, 0.8f),
        50.f);

    AddSphere(scene, Point(-2.f, 0.f, 5.f), 0.8f, plain);
    AddSphere(scene, Point( 0.f, 0.f, 5.f), 0.8f, tex_dog);
    AddSphere(scene, Point( 2.f, 0.f, 5.f), 0.8f, tex_um);

    AmbientLight *ambient = new AmbientLight(RGB(0.1f, 0.1f, 0.1f));
    scene.lights.push_back(ambient);
    scene.numLights++;
    PointLight *p1 = new PointLight(RGB(80.f, 80.f, 80.f), Point(0.f, 3.f, 0.f));
    scene.lights.push_back(p1);
    scene.numLights++;
}
*/
