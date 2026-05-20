//
//  BuildScenes.hpp
//  VI-RT-InitialVersion
//
//  Created by Luis Paulo Santos on 11/02/2025.
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
void PhongSphereScene (Scene& scene);
void PhongCubeScene (Scene& scene);
void PhongJustOneThing (Scene& scene);
void PhongTextureScene (Scene& scene);
void CookTorranceSphereScene (Scene& scene);
void CookTorranceTextureScene (Scene& scene);
void CookTorranceShowcase (Scene& scene);
void OrenNayarScene (Scene& scene);
void OrenNayarJustOneThing (Scene& scene);
void WardScene(Scene& scene);
void WardJustOneThing(Scene& scene);
void WardCubeScene(Scene& scene);
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
// Ashikhmin-Shirley — cenas base
void AshikhminShirleyScene (Scene& scene);
void AshikhminShirleyAnisotropicScene (Scene& scene);
void AshikhminShirleyMaterialsScene (Scene& scene);
void AshikhminShirleyJustOneThing (Scene& scene);
// Ashikhmin-Shirley — cenas de exploração de variantes
void AshikhminShirleyTestStandart (Scene& scene);   // referência com os 4 materiais padrão
void AshikhminShirleyFConstTest (Scene& scene);     // Fresnel constante (F = F0)
void AshikhminShirleyFInvTest (Scene& scene);       // Fresnel invertido
void AshikhminShirleyNoNormTest (Scene& scene);     // sem fator sqrt((nu+2)(nv+2))
void AshikhminShirleyLambDiffTest (Scene& scene);   // difuso Lambertiano simples
void AshikhminShirleyNoDiffTest (Scene& scene);     // sem componente difusa

#endif /* BuildScenes_hpp */