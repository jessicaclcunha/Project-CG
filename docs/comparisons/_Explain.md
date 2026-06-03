# Como ler um relatório de comparação

Cada `<TÍTULO>.md` em `docs/comparisons/` compara o render actual (`build/apps/result/reference.ppm`) contra a referência fixa (`build/apps/result/standard.ppm`) e gera métricas + imagem-diferença.

Este guia explica cada secção do relatório e como combinar os sinais para diagnosticar o tipo de erro.

---

## Estrutura do relatório

1. **Cabeçalho** — variante, referência, data, resolução
2. **Métricas per-canal** — RMSE / MAE / Max em R, G, B, Y
3. **Métricas adicionais** — legacy, PSNR, SSIM, ΔE
4. **Estatísticas da referência** — Y min/max/mean
5. **Histograma de |ΔY|** — distribuição do erro de luminância
6. **Imagem-diferença** — visualização per-pixel

---

## 1. Cabeçalho

Só identificação. Útil para perceber qual variante vs qual referência.

---

## 2. Métricas per-canal (R, G, B, Y)

| Métrica | O que mede | Como ler |
|---|---|---|
| **RMSE** | √(média do erro²) | Penaliza outliers; sensível a picos |
| **MAE**  | Média do \|erro\|  | Robusta; reflecte o erro típico |
| **Max**  | Pior pixel       | Mostra a magnitude do caso extremo |

**Perguntas-chave:**

- **R, G, B iguais?** → problema de luminância (intensidade), não de cor.
- **Um canal sobressai?** → problema de cor:
  - R/G altos, B baixo → tons quentes/dourados afectados (típico em metálicos)
  - B alto, R/G baixos → tons frios/azuis afectados
- **RMSE >> MAE (rácio >5)?** → erro **localizado** (poucos píxeis estragam, resto idêntico).
- **RMSE ≈ MAE × 1.25?** → erro **uniformemente espalhado**.
- **Max muito acima de RMSE?** → existem outliers individuais; cruzar com a imagem-diferença.

---

## 3. Métricas adicionais

### PSNR Y (dB)

Razão sinal/ruído. Quanto maior, melhor.

| dB | Significado |
|---|---|
| > 40 | Imperceptível ao olho humano |
| 30 – 40 | Ligeira |
| 20 – 30 | Visível |
| < 20 | Gritante |

### SSIM Y ∈ [-1, 1]

Mede **similaridade estrutural** (formas, padrões, contornos). 1 = idêntico. Não cai por mudanças globais de intensidade — cai por artefactos, ruído, perda de definição ou geometria diferente.

| SSIM | Significado |
|---|---|
| > 0.98 | Estrutura preservada |
| 0.90 – 0.98 | Estrutura quase igual |
| < 0.90 | Estrutura claramente afectada |

**Combinação PSNR × SSIM:**

| PSNR | SSIM | Diagnóstico |
|---|---|---|
| Baixo | Alto | Mudança de **intensidade** (NoEC, exposição, gamma) |
| Baixo | Baixo | Artefactos **estruturais** (NaN, ruído, geometria errada) |
| Alto  | Alto | Quase idêntico |

### ΔE CIE76 (médio + máximo)

Distância perceptual em espaço Lab — aproximação de quão diferentes as cores são para o olho humano.

| ΔE | Significado |
|---|---|
| < 1   | Imperceptível |
| 1 – 2 | Limiar perceptível |
| 2 – 10 | Claramente diferente |
| 10 – 50 | Muito diferente |
| > 50 | Cores completamente distintas |

**Combinação médio × máximo:**

- Médio ≈ máximo → diferença de cor **espalhada** por todo o quadro.
- Médio << máximo → diferença **concentrada** em zonas específicas (highlights, bordas).

### Métricas legacy

- `RMSE legacy Y` — fórmula antiga `√(ΣSE)/N` (subestima); só serve para reprodutibilidade.
- `MinMaxScaled RMSE Y` — RMSE normalizado pelo range de Y da referência.

Não usar para análise nova; servem para comparar com números antigos.

---

## 4. Estatísticas da referência (Y min, max, mean)

Contexto para interpretar a magnitude do erro:

- **Cena escura** (Y mean baixo, ex: 0.1) — um RMSE 0.05 é proporcionalmente grande.
- **Cena clara** (Y mean alto, ex: 0.6) — o mesmo RMSE é proporcionalmente pequeno.

---

## 5. Histograma de |ΔY|

10 bins logarítmicos de erro de luminância por pixel. A **forma** diz o tipo de erro:

| Forma | Diagnóstico |
|---|---|
| Concentrado em `[0, 1e-4)` + cauda longa | **Local** — só algumas regiões mudam (highlights, sombras) |
| Distribuído ao longo dos bins | **Global** — shift de exposição, gamma ou cor uniforme |
| Dois picos | **Dois regimes** — ex: difuso vs especular afectados de formas diferentes |
| Algo em `[1.0, ∞)` overflow | **Extremos** — NaN, energia estourada, bug grave |

Soma de todos os bins = W × H (640 × 640 = 409 600).

---

## 6. Imagem-diferença (`diff_<TÍTULO>.ppm`)

Erro quadrático em Y por pixel, com gamma 0.5 aplicado para realçar diferenças pequenas:

- **Preto** = idêntico
- **Cinza** = diferença ligeira
- **Branco** = diferença extrema

Cruzar com a imagem original mostra **onde** as zonas problemáticas estão (esferas? bordas? fundo? sombras?).

---

## Pipeline de análise (4 passos)

1. **Histograma** → tipo de erro (local / global / bi-modal / extremos).
2. **PSNR + SSIM** → intensidade vs estrutura.
3. **Per-canal R/G/B** → luminância vs cor (e que cor).
4. **Imagem-diferença** → localização espacial.

---

## Padrões esperados por variante

| Variante | Padrão esperado |
|---|---|
| **NoEC** | PSNR ~25 dB, SSIM alto, R sobressai, histograma local, ΔE máx >> médio |
| **F constante** | Falta efeito Fresnel rasante → ΔE elevado nas bordas, médio próximo de máx |
| **F invertido** | Falta de brilho nas bordas + excesso de centro → padrão "invertido" no histograma |
| **G = 1** | Highlights ligeiramente mais brilhantes; PSNR alto, SSIM ≈ 1 |
| **D Beckmann** | Highlights mais concentrados que GGX → ΔE local no centro do lóbulo |
| **D Blinn-Phong** | Highlights ainda mais abruptos → cauda do histograma mais curta |
| **F exacto (IOR)** | Materiais com η diferente → erro varia por esfera, não é uniforme |
| **F SG (Karis)** | Quase indistinguível de Schlick → PSNR > 40 dB, SSIM ≈ 1 |
| **G CT 1982** | Penaliza menos com roughness alto → ligeiramente mais claro |
| **G Smith IBL** | k menor → G maior → resultado mais claro (não mais escuro) |
