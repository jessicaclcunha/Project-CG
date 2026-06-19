# VI-RT — Estudo de BRDFs

Ray tracer (base de Visualização e Iluminação, Mestrado em Informática) estendido
para implementar e comparar vários modelos de BRDF.

## O que implementámos

- **BRDFs:** Phong (Blinn-Phong normalizado), Cook-Torrance (GGX/Smith/Schlick, fluxo PBR metallic/roughness), Oren-Nayar, Ward (anisotrópico), Ashikhmin-Shirley (anisotrópico) e Disney (principled).
- **Casos de estudo por BRDF:** para cada modelo criámos variantes que (1) trocam uma função interna por outra igualmente correta (ex.: D Beckmann vs GGX, Fresnel exacto vs Schlick) e (2) alteram/removem algo dado como certo (ex.: sem conservação de energia, sem termo geométrico, Fresnel invertido) — para analisar o impacto de cada escolha.
- **Cenas de teste:** uma cena base por BRDF e cenas dedicadas a cada caso de estudo (varrimento do parâmetro principal em várias esferas/cubos).
- **Geometria e câmara:** esferas, triângulos e caixas (cubo triangulado com UV por face); câmara perspetiva com defocus.
- **Texturas:** mapeamento UV a partir de imagens PPM; a textura afeta **apenas** o difuso (Kd), o especular vem sempre dos parâmetros do material.
- **Ferramenta RMSE:** comparação quantitativa entre renders (RMSE por canal, MAE, PSNR e SSIM) para medir o quão diferentes são duas BRDFs.
- **Integração com Mitsuba 3:** exportador C++ que gera a cena Mitsuba a partir da *mesma* `Scene` do renderer, renderiza nos dois motores e compara lado-a-lado (RMSE/PSNR/SSIM + imagens de diferença).

## Como correr

### 1. Renderer principal

```bash
make          # compilar
make run      # compilar, renderizar e mostrar a imagem
make display  # apenas mostrar a última imagem renderizada
make clean    # limpar artefactos de build
```

A imagem é escrita em `build/apps/result/reference.ppm`.

**Escolher a cena:** abrir [src/main.cpp](src/main.cpp), na secção "CATÁLOGO DE CENAS"
descomentar **uma** chamada de cena (a ativa é a que está sem `//`). Resolução, SPP,
shader e câmara configuram-se mais abaixo nesse mesmo ficheiro.

> Correr sempre via `make run` (ou a partir de `build/apps/`): as texturas são
> carregadas por caminho relativo e os PPM são copiados para `build/apps/`.

### 2. Comparação RMSE (estudo das BRDFs)

```bash
cd RMSE && make          # compila rmse_exec
```

Comparação direta de duas imagens:

```bash
./RMSE/rmse_exec <variante.ppm> <referencia.ppm> <threshold> <diff_saida.ppm>
```

O script [RMSE/compare.sh](RMSE/compare.sh) `<titulo>` automatiza a comparação do render
atual (`build/apps/result/reference.ppm`) contra uma referência fixa (`standard.ppm`).
Fluxo típico de um caso de estudo:

```bash
make run                                              # render da BRDF standard
cp build/apps/result/reference.ppm build/apps/result/standard.ppm
# trocar para a variante em src/main.cpp e make run de novo
./RMSE/compare.sh <titulo>                            # compara variante vs standard
```

#### Sobre as métricas

**`rmse_exec` (C++) — usado no estudo das BRDFs.** Lê os dois **PPM** diretamente
(float, [0,1]) e imprime, para cada par de imagens:

- **RMSE por canal (R, G, B) e luminância Y** — `sqrt(mean(diferença²))`; menor = mais parecidas.
- **MAE** — erro absoluto médio `mean(|diferença|)`.
- **Max** — maior diferença num único píxel.
- **PSNR Y** (dB) — `10·log10(1/MSE_Y)`; maior = mais parecidas (∞ = idênticas).
- **SSIM Y** — similaridade estrutural em tiles 8×8; `1.0` = idêntico.
- **ΔE76** — diferença de cor perceptual (espaço CIE Lab).
- **Histograma de |ΔY|** — distribuição do erro de luminância.

**`compare_renders.py` (Python) — usado na comparação com o Mitsuba** (secção 3).
Calcula sobre os **PNG já tone-mapped** (8-bit, [0,1]), depois de redimensionar ambos
para o mesmo tamanho:

- **RMSE** = `sqrt(mean((vi_rt − mitsuba)²))` sobre **todos os píxeis × 3 canais (RGB)** → **um único número global**.
- **PSNR** a partir do MSE (max=1.0); **SSIM** via `scikit-image` (3 canais).

> ⚠️ **Os dois RMSE não são o mesmo nem são comparáveis entre si.** O do estudo
> (`rmse_exec`, por canal + Y, sobre PPM) mede **"quão diferentes são duas BRDFs
> nossas"**. O do Mitsuba (RGB global, sobre PNG) mede **"quão perto o VI-RT está de
> um renderer de referência"** (validação).

### 3. Comparação com Mitsuba 3

Renderiza as cenas no *nosso* motor (VI-RT) **e** no Mitsuba 3 (referência), a partir
da **mesma `Scene` C++** (geometria/luz/câmara idênticas por construção), e compara as
duas imagens para **validar** as BRDFs.

```bash
cd src/mitsuba
./render_all.sh          # faz tudo: make mitsuba → render+XML → Mitsuba → PNG → comparação
```

Passos internos (tudo vai para `src/mitsuba/output/`):

1. `make mitsuba` (na raiz) + correr o binário → `output/vi_rt/` (renders) e `output/scenes/` (XML).
2. Mitsuba renderiza os XML → `output/mitsuba/*.exr`.
3. EXR e PPM → PNG com o **mesmo** tone map (Reinhard + γ2.2), para a comparação ser justa.
4. `compare_renders.py` → `output/comparison/` (`metrics.csv`, `report.json`, imagens lado-a-lado).

Só compilar o binário (sem pipeline): `make mitsuba` na raiz.

Visualização interativa dos resultados — o viewer faz `fetch` do `report.json`, que o
browser bloqueia via `file://`, por isso serve-se por HTTP local:

```bash
cd src/mitsuba
python3 -m http.server 8000
```
Depois abre no browser: http://localhost:8000/comparison_viewer.html

> `output/` é 100% regenerável (`rm -rf src/mitsuba/output && ./render_all.sh`).
> Só `output/comparison/` é versionado.

#### Nuances do Mitsuba (limites da validação)

A geometria/luz/câmara são exactas; o que varia é o **modelo de BRDF** do outro lado:

| BRDF VI-RT | BSDF Mitsuba | Validação |
|---|---|---|
| Difuso, Cook-Torrance, Disney | `diffuse` / `roughconductor`·`roughplastic` / `principled` | ✅ significativa |
| Ward, Ashikhmin-Shirley, Phong | aproximação (Mitsuba não tem estes modelos) | ⚠️ qualitativa |

Para Ward/Ashikhmin-Shirley/Phong a diferença **não** é necessariamente um bug — são
modelos distintos. O integrador (Mitsuba `direct` ≈ `DistributedShader`) e o tone map
são alinhados ao máximo, mas nunca serão bit-a-bit iguais (são renderers diferentes).

**Dependências:** `mitsuba` (Python), `numpy`, `Pillow`; `scikit-image` para o SSIM.
