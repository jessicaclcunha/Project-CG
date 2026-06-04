# Tempos de renderização

Tempo CPU em segundos reportado pelo `main.cpp` no final de cada `make run`
(linha `Rendering time = X.XXX secs`). Resolução 640×640, SPP=128, jitter on.

---

## CookTorrance

| Estudo | Tempo (s) |
|---|---|
| Standard (CookTorranceTestStandart) | 18.934 secs |
| NoEC (CookTorranceNoECTest2) | 18.291 secs |
| F constante (CookTorranceFConstTest) | 20.371 secs |
| F expoente (CookTorranceFExpTest) | 19.118 secs |
| F invertido (CookTorranceFInvTest) | 19.317 secs |
| kD metallic (CookTorranceKDMetalTest) | 19.111 secs |
| kD lerp (CookTorranceKDLerpTest) | 18.951 secs |
| G = 1 (CookTorranceGNoneTest) | 19.257 secs |
| G = G1L (CookTorranceGOneTest) | 20.127 secs |
| G Kelemen (CookTorranceGKelemenTest) | 20.177 secs |
| D Beckmann (CookTorranceDBeckmannTest) | 18.755 secs |
| D Blinn-Phong (CookTorranceDBlinnPhongTest) | 18.448 secs |
| F exacto (CookTorranceFExactTest) | 19.340 secs |
| F SG (CookTorranceFSGTest) | 21.221 secs |
| G CT 1982 (CookTorranceGCT1982Test) | 19.586 secs |
| G Smith IBL (CookTorranceGSmithIBLTest) | 20.255 secs |

---

## Oren-Nayar

| Estudo | Tempo (s) |
|---|---|
| Standard (OrenNayarTestStandart) | 7.812 secs |
| Lambert (OrenNayarLambertTest) | 7.798 secs |
| NoB — sem retroreflexão (OrenNayarNoBTest) | 7.839 secs |
| AFixo — sem escurecimento (OrenNayarAFixoTest) | 8.048 secs |
| NoClamp — azimute negativo (OrenNayarNoClampTest) | 7.962 secs |
| Fujii — energy-preserving (OrenNayarFujiiTest) | 7.815 secs |
| Full — C1/C2/C3 (OrenNayarFullTest) | 7.980 secs |
| FullInter — Full + L2 (OrenNayarFullInterTest) | 7.853 secs |

---

## Ward

> Nota: estes renders foram feitos com SPP=218 (valor actual de `main.cpp`), não 128.

| Estudo | Tempo (s) |
|---|---|
| Standard (WardTestStandart) | 7.965 secs |
| IsoForced — força αx=αy (WardIsoForcedTest) | 8.011 secs |
| NoNorm — sem 1/(4π·αx·αy) (WardNoNormTest) | 7.477 secs |
| NoDiff — especular puro (WardNoDiffTest) | 7.920 secs |
| NoGeom — sem √(NdotL·NdotV) (WardNoGeomTest) | 7.570 secs |
| Dür 2006 — denom linear (WardDurTest) | 7.462 secs |
| GMD 2010 — denom (NdotH)^4 (WardGMDTest) | 7.452 secs |
| Fresnel — Schlick no Ks (WardFresnelTest) | 7.619 secs |

---

## Ashikhmin-Shirley

> Nota: renders com SPP=218 (valor actual de `main.cpp`), não 128.

| Estudo | Tempo (s) |
|---|---|
| Standard (AshikhminShirleyTestStandart) | 9.757 secs |
| FConst — Fresnel constante (AshikhminShirleyFConstTest) | 8.397 secs |
| FInv — Fresnel invertido (AshikhminShirleyFInvTest) | 8.912 secs |
| NoNorm — sem √((nu+1)(nv+1)) (AshikhminShirleyNoNormTest) | 8.605 secs |
| LambDiff — difuso Lambert (AshikhminShirleyLambDiffTest) | 8.426 secs |
| NoDiff — especular puro (AshikhminShirleyNoDiffTest) | 8.695 secs |
| FExact — Fresnel dieléctrico exacto (AshikhminShirleyFExactTest) | 8.640 secs |
| FSG — Fresnel spherical-gaussian (AshikhminShirleyFSGTest) | 8.279 secs |

---

