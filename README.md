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

### 2. Comparação RMSE

```bash
cd RMSE && make          # compila rmse_exec
```

Comparação direta de duas imagens:

```bash
./RMSE/rmse_exec <variante.ppm> <referencia.ppm> <threshold> <diff_saida.ppm>
```

Imprime RMSE por canal, MAE, PSNR e SSIM, e escreve a imagem-diferença. O script
[RMSE/compare.sh](RMSE/compare.sh) `<titulo>` automatiza a comparação do render
atual (`build/apps/result/reference.ppm`) contra uma referência fixa (`standard.ppm`).

Fluxo típico de um caso de estudo:

```bash
make run                                              # render da BRDF standard
cp build/apps/result/reference.ppm build/apps/result/standard.ppm
# trocar para a variante em src/main.cpp e make run de novo
./RMSE/compare.sh <titulo>                            # compara variante vs standard
```

> Este RMSE (por canal + Y, sobre os PPM) mede **quão diferentes são duas BRDFs nossas**.
> É uma métrica **distinta** do RMSE da comparação com o Mitsuba (RGB global sobre PNG,
> que mede correção face a um renderer de referência) — ver
> [src/mitsuba/README.md](src/mitsuba/README.md). Os dois valores não são comparáveis entre si.

### 3. Comparação com Mitsuba 3

Requisitos: `mitsuba` (Python), `numpy`, `Pillow` e (opcional, para SSIM) `scikit-image`.

```bash
make mitsuba                      # compila o binário exportador (build/apps/mitsuba_render)
bash src/mitsuba/render_all.sh    # exporta cenas, renderiza nos 2 motores e compara
```

Resultados (métricas e imagens lado-a-lado) em `src/mitsuba/output/comparison/`;
visualização interativa em [src/mitsuba/comparison_viewer.html](src/mitsuba/comparison_viewer.html).
