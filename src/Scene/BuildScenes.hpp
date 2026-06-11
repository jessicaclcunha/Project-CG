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
#include "CookTorrance.hpp"
#include "CookTorranceNoEC.hpp"
#include "CookTorranceFConst.hpp"
#include "CookTorranceFExp.hpp"
#include "CookTorranceFInv.hpp"
#include "CookTorranceKDMetal.hpp"
#include "CookTorranceKDLerp.hpp"
#include "CookTorranceGNone.hpp"
#include "CookTorranceGOne.hpp"
#include "CookTorranceGKelemen.hpp"
#include "CookTorranceDBeckmann.hpp"
#include "CookTorranceDBlinnPhong.hpp"
#include "CookTorranceFExact.hpp"
#include "CookTorranceFSG.hpp"
#include "CookTorranceGCT1982.hpp"
#include "CookTorranceGSmithIBL.hpp"
#include "OrenNayar.hpp"
#include "OrenNayarNoB.hpp"
#include "OrenNayarAFixo.hpp"
#include "OrenNayarNoClamp.hpp"
#include "OrenNayarFujii.hpp"
#include "OrenNayarFull.hpp"
#include "OrenNayarFullInter.hpp"
#include "Ward.hpp"
#include "WardIsoForced.hpp"
#include "WardNoNorm.hpp"
#include "WardNoDiff.hpp"
#include "WardNoGeom.hpp"
#include "WardDur.hpp"
#include "WardGMD.hpp"
#include "WardFresnel.hpp"
#include "AshikhminShirley.hpp"
#include "AshikhminShirleyFConst.hpp"
#include "AshikhminShirleyFInv.hpp"
#include "AshikhminShirleyNoNorm.hpp"
#include "AshikhminShirleyLambDiff.hpp"
#include "AshikhminShirleyNoDiff.hpp"
#include "AshikhminShirleyFExact.hpp"
#include "AshikhminShirleyFSG.hpp"
#include "Disney.hpp"
#include "PhongTexture.hpp"
#include "CookTorranceTexture.hpp"
#include "WardTexture.hpp"
#include "OrenNayarTexture.hpp"
#include "AshikhminShirleyTexture.hpp"

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
void CookTorranceSphereScene (Scene& scene);
void CookTorranceShowcase (Scene& scene);
void OrenNayarLambertVsON (Scene& scene);
// Oren-Nayar — estudo de variantes erradas + alternativas correctas
void OrenNayarTestStandart (Scene& scene);
void OrenNayarLambertTest (Scene& scene);
void OrenNayarNoBTest (Scene& scene);
void OrenNayarAFixoTest (Scene& scene);
void OrenNayarNoClampTest (Scene& scene);
void OrenNayarFujiiTest (Scene& scene);
void OrenNayarFullTest (Scene& scene);
void OrenNayarFullInterTest (Scene& scene);
// Ward — setup do estudo
void WardTestStandart(Scene& scene);
// Ward — variantes erradas
void WardIsoForcedTest(Scene& scene);
void WardNoNormTest(Scene& scene);
void WardNoDiffTest(Scene& scene);
void WardNoGeomTest(Scene& scene);
// Ward — alternativas correctas
void WardDurTest(Scene& scene);
void WardGMDTest(Scene& scene);
void WardFresnelTest(Scene& scene);
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
// CookTorrance — estudo de alternativas correctas de D, F e G
void CookTorranceDBeckmannTest(Scene& scene);
void CookTorranceDBlinnPhongTest(Scene& scene);
void CookTorranceFExactTest(Scene& scene);
void CookTorranceFSGTest(Scene& scene);
void CookTorranceGCT1982Test(Scene& scene);
void CookTorranceGSmithIBLTest(Scene& scene);
// Ashikhmin-Shirley — cenas base
void AshikhminShirleyScene (Scene& scene);
void AshikhminShirleyAnisotropicScene (Scene& scene);
void AshikhminShirleyMaterialsScene (Scene& scene);
// Ashikhmin-Shirley — cenas de exploração de variantes
void AshikhminShirleyTestStandart (Scene& scene);
void AshikhminShirleyFConstTest (Scene& scene);
void AshikhminShirleyFInvTest (Scene& scene);
void AshikhminShirleyNoNormTest (Scene& scene);
void AshikhminShirleyLambDiffTest (Scene& scene);
void AshikhminShirleyNoDiffTest (Scene& scene);
// Ashikhmin-Shirley — alternativas correctas (Fresnel)
void AshikhminShirleyFExactTest (Scene& scene);
void AshikhminShirleyFSGTest (Scene& scene);
// Disney Principled BRDF — demo de presets + sweeps de parâmetro
void DisneyPresetsScene (Scene& scene);
void DisneyRoughnessSweep (Scene& scene);
void DisneyMetallicSweep (Scene& scene);
void DisneyAnisoSweep (Scene& scene);
void DisneySheenSweep (Scene& scene);
void DisneySubsurfaceSweep (Scene& scene);
void DisneyClearcoatSweep (Scene& scene);
// Texturas — cenas de demonstração de texturas
void PhongTextureScene (Scene& scene);
void CookTorranceTextureScene (Scene& scene);
void TextureAllBRDFsSpheres(Scene& scene);
void TextureAllBRDFsBoxes(Scene& scene);
void TextureFullShowcase(Scene& scene);
void TextureUVDebug(Scene& scene);
void CookTorranceTextureStandart(Scene& scene);
void WardTextureStandart(Scene& scene);
void OrenNayarTextureStandart(Scene& scene);
void AshikhminShirleyTextureStandart(Scene& scene);
void BRDFShowcaseCornellBox(Scene& scene);

#endif /* BuildScenes_hpp */