# Resumo — Estudo CookTorrance

15 variantes testadas contra a referência `CookTorranceTestStandart` (4 esferas a roughness/metallic variados, iluminação por área). Resolução 640×640, SPP=128.

---

## Tabela master

| Variante | PSNR Y (dB) | SSIM Y | ΔE méd | ΔE máx | Tempo (s) | Status |
|---|---|---|---|---|---|---|
| **Standard** | — | — | — | — | 18.93 | base |
| NoEC | 26.07 | 0.984 | 1.03 | 69.92 | 18.29 | OK |
| F_Constante | 55.26 | 0.999 | 0.05 | 22.21 | 20.37 | rasantes |
| F_expoente | 55.20 | 0.999 | 0.05 | 34.14 | 19.12 | rasantes |
| F_invertido | 36.60 | 0.997 | 0.75 | 36.25 | 19.32 | OK |
| KD_metallic | 20.16 | 0.949 | 3.35 | 88.71 | 19.11 | OK |
| KD_lerp | 27.59 | 0.985 | 1.28 | 60.33 | 18.95 | OK |
| G1 | 38.45 | 0.995 | 0.35 | 40.57 | 19.26 | OK |
| G1L | 45.41 | 0.998 | 0.11 | 29.03 | 20.13 | OK |
| G_Kelemen | 48.07 | 0.999 | 0.11 | 23.23 | 20.18 | OK |
| D_Beckmann | 28.42 | 0.986 | 0.58 | 86.29 | 18.76 | OK |
| D_Blinn-Phong | 28.49 | 0.986 | 0.55 | 85.86 | 18.45 | OK |
| F_exacto | 31.46 | 0.986 | 1.43 | 93.78 | 19.34 | cena dif. |
| F_SG | 55.37 | 0.999 | 0.05 | 22.22 | 21.22 | OK |
| G_CT_1982 | 47.34 | 0.999 | 0.13 | 22.15 | 19.59 | OK |
| G_Smith_IBL | 45.25 | 0.998 | 0.13 | 20.24 | 20.26 | OK |

---

## Ranking de impacto (PSNR menor = maior diferença)

| # | Variante | PSNR | Magnitude |
|---|---|---|---|
| 1 | KD_metallic | 20.16 | Diferença gritante |
| 2 | NoEC | 26.07 | Visível |
| 3 | KD_lerp | 27.59 | Visível |
| 4 | D_Beckmann | 28.42 | Visível |
| 5 | D_Blinn-Phong | 28.49 | Visível |
| 6 | F_exacto | 31.46 | Ligeira |
| 7 | F_invertido | 36.60 | Ligeira |
| 8 | G1 | 38.45 | Ligeira |
| 9 | G_Smith_IBL | 45.25 | Quase idêntico |
| 10 | G1L | 45.41 | Quase idêntico |
| 11 | G_CT_1982 | 47.34 | Quase idêntico |
| 12 | G_Kelemen | 48.07 | Quase idêntico |
| 13 | F_expoente | 55.20 | Imperceptível |
| 14 | F_Constante | 55.26 | Imperceptível |
| 15 | F_SG | 55.37 | Imperceptível |

---

## Análise por categoria

### EC — Energy Conservation
- **NoEC** (PSNR 26 dB): R domina (esfera dourada), histograma local, ΔE máx 70 → confirma "excesso de energia em highlights"

### F — Fresnel (variantes erradas)
- **F_Constante / F_expoente**: PSNR ~55 dB → **diferença quase imperceptível**
- **F_invertido** (PSNR 36 dB): único do grupo F com impacto claro

### kD — Componente difusa
- **KD_metallic** (PSNR 20 dB): canal **B domina** (esferas azuis afectadas) — pior PSNR do estudo
- **KD_lerp** (PSNR 27 dB): comportamento semelhante ao NoEC

### G — Geometry (variantes erradas)
- **G1** (PSNR 38 dB): sem shadowing-masking, highlights mais brilhantes
- **G1L** (PSNR 45 dB): só falta o lambda V, impacto baixo
- **G_Kelemen** (PSNR 48 dB): boa aproximação heurística

### D — alternativas correctas
- **D_Beckmann / D_Blinn-Phong** (PSNR ~28 dB): muito semelhantes entre si, ΔE máx ~86 → cauda curta vs cauda longa do GGX

### F — alternativas correctas
- **F_SG** (PSNR 55 dB): **confirma que Schlick é aproximação excelente**
- **F_exacto** (PSNR 31 dB): cena com materiais diferentes (4 dielétricos com η variado), não comparável directamente

### G — alternativas correctas
- **G_CT_1982 / G_Smith_IBL** (PSNR ~45–47 dB): praticamente equivalentes ao Smith-GGX direct

---

## Tempos

| Métrica | Valor |
|---|---|
| Mais rápido | NoEC (18.29 s) |
| Mais lento | F_SG (21.22 s) |
| Diferença max | 2.93 s (~15%) |
| Média | ~19.3 s |

Nenhuma variante traz ganho de performance significativo — a complexidade adicional das alternativas correctas (F_SG, G_Kelemen) acrescenta ~10–15% de tempo sem alterar o resultado visual.

---

## Anomalias / observações

1. **F_Constante e F_expoente têm PSNR muito alto (~55 dB)**
   - Esperado: PSNR ~30 dB
   - Causa: **problema dos ângulos rasantes** (ver [docs/CookTorrance_Study_AngulosRasantes.md](../../CookTorrance_Study_AngulosRasantes.md)). O efeito Fresnel só se manifesta nas bordas das esferas — área muito pequena, contribuição negligenciável para RMSE/PSNR
   - **Não é bug** — o setup actual não destaca o erro

2. **F_exacto compara cenas diferentes**
   - Os materiais usados são 4 dielétricos com η = 1.33/1.5/1.77/2.5 (todos `metallic=0`)
   - O Standard tem materiais mistos (1 metálico dourado + dielétricos)
   - Canal B sobressai 3× porque as cenas têm composições cromáticas diferentes
   - O PSNR e ΔE medem **diferenças de cena**, não apenas a fórmula F

3. **D_Beckmann ≈ D_Blinn-Phong**: PSNR e SSIM praticamente iguais (28.4 dB / 0.986)
   - Confirma que ambas têm cauda curta vs GGX (cauda longa)

---

## Conclusões

- **kD é o termo mais crítico** para a aparência correcta (KD_metallic destrói o resultado)
- **F é o termo mais robusto** neste setup — quase qualquer aproximação funciona em ângulos não-rasantes
- **G tem múltiplas formulações equivalentes** (Smith direct, IBL, CT 1982, Kelemen, G1L) com diferenças < 3 dB
- **D dita a forma do highlight** — variar GGX por Beckmann/BP é a única mudança "correcta" com impacto visível claro
- **Schlick é uma aproximação validada** — F_SG confirma com ΔE 0.05
- **EC (energy conservation) é não-negociável** — sem ele, materiais perdem credibilidade física (NoEC: PSNR 26 dB)
- **Tempos são dominados pelo path-tracing**, não pela BRDF — ~10–15% de variação só

---

## Limitações do estudo

- Geometria só com esferas pequenas → Fresnel rasante invisível
- 1 setup de luz → não testa transporte indirecto rigoroso
- SPP=128 → algum ruído residual pode afectar ΔE máx
- F_exacto usa materiais diferentes → relatório não isola apenas a fórmula F
