//
//  BuildScenes.hpp
//  VI-RT-InitialVersion
//

#ifndef BuildScenes_hpp
#define BuildScenes_hpp

#include "scene.hpp"
#include "AmbientLight.hpp"
#include "PointLight.hpp"
#include "AreaLight.hpp"
#include "Sphere.hpp"
#include "triangle.hpp"
#include "BRDF.hpp"
#include "Phong.hpp"
#include "PhongTexture.hpp"
#include "CookTorrance.hpp"
#include "CookTorranceTexture.hpp"
#include "CookTorranceNoEC.hpp"
#include "CookTorranceFConst.hpp"
#include "CookTorranceFExp.hpp"
#include "CookTorranceFInv.hpp"
#include "CookTorranceKDMetal.hpp"
#include "CookTorranceKDLerp.hpp"
#include "CookTorranceGNone.hpp"
#include "CookTorranceGOne.hpp"
#include "CookTorranceGKelemen.hpp"
#include "OrenNayar.hpp"
#include "Ward.hpp"
#include "WardTexture.hpp"
#include "OrenNayarTexture.hpp"
#include "AshikhminShirleyTexture.hpp"
#include "AshikhminShirley.hpp"
#include "AshikhminShirleyFConst.hpp"
#include "AshikhminShirleyFInv.hpp"
#include "AshikhminShirleyNoNorm.hpp"
#include "AshikhminShirleyLambDiff.hpp"
#include "AshikhminShirleyNoDiff.hpp"

void SpheresScene (Scene& scene, int const N_spheres);
void SpheresTriScene (Scene& scene);
void SingleTriScene (Scene& scene);
void DeFocusTriScene (Scene& scene);
void CornellBox (Scene& scene);
void DiffuseCornellBox (Scene& scene);
void DLightChallenge (Scene& scene);
// Phong
void PhongSphereScene (Scene& scene);
void PhongCubeScene (Scene& scene);
void PhongJustOneThing (Scene& scene);
void PhongTextureScene (Scene& scene);
// Cook-Torrance
void CookTorranceSphereScene (Scene& scene);
void CookTorranceTextureScene (Scene& scene);
void CookTorranceShowcase (Scene& scene);
void CookTorranceTestStandart(Scene& scene);
void CookTorranceNoECTest2(Scene& scene);
void CookTorranceFConstTest(Scene& scene);
void CookTorranceFExpTest(Scene& scene);
void CookTorranceFInvTest(Scene& scene);
void CookTorranceKDMetalTest(Scene& scene);
void CookTorranceKDLerpTest(Scene& scene);
void CookTorranceGNoneTest(Scene& scene);
void CookTorranceGOneTest(Scene& scene);
void CookTorranceGKelemenTest(Scene& scene);
// Oren-Nayar
void OrenNayarScene (Scene& scene);
void OrenNayarJustOneThing (Scene& scene);
// Ward
void WardScene(Scene& scene);
void WardJustOneThing(Scene& scene);
void WardCubeScene(Scene& scene);
// Ashikhmin-Shirley
void AshikhminShirleyScene (Scene& scene);
void AshikhminShirleyAnisotropicScene (Scene& scene);
void AshikhminShirleyMaterialsScene (Scene& scene);
void AshikhminShirleyJustOneThing (Scene& scene);
void AshikhminShirleyTestStandart (Scene& scene);
void AshikhminShirleyFConstTest (Scene& scene);
void AshikhminShirleyFInvTest (Scene& scene);
void AshikhminShirleyNoNormTest (Scene& scene);
void AshikhminShirleyLambDiffTest (Scene& scene);
void AshikhminShirleyNoDiffTest (Scene& scene);
// Texturas — Semana 1, Aluno B
// 5 esferas: Phong | Cook-Torrance | Oren-Nayar | Ward | Ashikhmin-Shirley
void TextureAllBRDFsSpheres(Scene& scene);
// 5 cubos com UV: mesma ordem
void TextureAllBRDFsBoxes(Scene& scene);
// Grade 2×8: esfera + cubo, sem textura e com todas as BRDFs texturizadas
void TextureFullShowcase(Scene& scene);
// Validação de UV: esfera + cubo, Dog e UMinho
void TextureUVDebug(Scene& scene);

#endif /* BuildScenes_hpp */