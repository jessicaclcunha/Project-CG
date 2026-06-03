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

