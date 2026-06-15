# Módulo Mitsuba — validação das BRDFs

Compara os renders do **VI-RT** com os do **Mitsuba 3** (renderer de referência),
para validar as BRDFs. A mesma `Scene` C++ que o VI-RT renderiza é também
**exportada** para XML do Mitsuba → geometria, luz e câmara são **idênticas por
construção** (sem drift). A única aproximação é o mapeamento de cada BRDF para o
BSDF mais próximo do Mitsuba (ver limites abaixo).

## Fonte vs Gerado

**Código (versionado, é o que editas):**
| Ficheiro | Papel |
|---|---|
| `vi_rt_render.cpp` | binário do módulo: renderiza as 6 cenas **e** exporta os XML |
| `MitsubaExporter.{hpp,cpp}` | converte uma `Scene` VI-RT → `scene.xml` (+ `.obj`) |
| `render_all.sh` | orquestrador do pipeline completo |
| `compare_renders.py` | métricas (RMSE/PSNR/SSIM) + imagens de comparação |
| `comparison_viewer.html` | visualizador lado a lado |

**Gerado (em `output/`):**
| Pasta | Conteúdo | Git |
|---|---|---|
| `output/scenes/` | XML + OBJ (escritos pelo C++) | ignorado |
| `output/vi_rt/` | `.ppm` + `_vi_rt.png` | ignorado |
| `output/mitsuba/` | `.exr` + `_mitsuba.png` | ignorado |
| `output/comparison/` | `metrics.csv`, `report.json`, `*_comparison/_diff.png` | **versionado** |

> Regra de ouro: **`output/` é 100% regenerável**. Para recomeçar do zero:
> `rm -rf output && ./render_all.sh`.

## Como correr

```bash
cd src/mitsuba
./render_all.sh          # faz tudo: make mitsuba → render+XML → Mitsuba → PNG → comparação
```

Passos internos (todos para `output/`):
1. `make mitsuba` (na raiz) + correr o binário → `output/vi_rt/` (renders) e `output/scenes/` (XML).
2. Mitsuba renderiza os XML → `output/mitsuba/*.exr`.
3. EXR e PPM → PNG com o **mesmo** tone map (Reinhard + γ2.2), para a comparação ser justa.
4. `compare_renders.py` → `output/comparison/`.

Só compilar o binário (sem pipeline): `make mitsuba` na raiz.

## Limites da validação (importante)

A geometria/luz/câmara são exactas; o que varia é o **modelo de BRDF** do outro lado:

| BRDF VI-RT | BSDF Mitsuba | Validação |
|---|---|---|
| Difuso, CookTorrance, Disney | `diffuse` / `roughconductor`·`roughplastic` / `principled` | ✅ significativa |
| Ward, Ashikhmin-Shirley, Phong | aproximação (Mitsuba não tem estes modelos) | ⚠️ qualitativa |

Para Ward/AS/Phong a diferença **não** é necessariamente um bug — são modelos
distintos. O integrador (Mitsuba `direct` ≈ DistributedShader) e o tone map são
alinhados ao máximo, mas nunca serão bit-a-bit iguais (são renderers diferentes).

## Dependências
`mitsuba` (Python), `numpy`, `Pillow`; `scikit-image` para o SSIM.
