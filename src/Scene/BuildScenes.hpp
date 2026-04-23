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
#include "OrenNayar.hpp"
#include "Ward.hpp"
#include "AshikhminShirley.hpp"

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
void CookTorranceCubeScene (Scene& scene);
void CookTorranceTextureScene (Scene& scene);
void CookTorranceJustOneThing (Scene& scene);
void OrenNayarScene (Scene& scene);
void OrenNayarJustOneThing (Scene& scene);
void WardScene(Scene& scene);
void WardJustOneThing(Scene& scene);
void WardCubeScene(Scene& scene);
void AshikhminShirleyScene(Scene& scene);
void AshikhminShirleyAnisotropicScene(Scene& scene);
void AshikhminShirleyMaterialsScene(Scene& scene);
void AshikhminShirleyJustOneThing(Scene& scene);
void AshikhminShirleyCubeScene(Scene& scene);

#endif /* BuildScenes_hpp */
