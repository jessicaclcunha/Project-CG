# Resumo — Estudo Oren-Nayar

7 variantes testadas contra a referência `OrenNayarTestStandart` (ON simplificado, 4 esferas a sigma 0.3/0.5/0.7/0.9, argila, luz angular ~30°). Resolução 640×640, SPP=218.

> ON é **difuso puro** → diferenças muito menores que o CookTorrance (que tinha highlights). O RMSE mede o que o olho não vê; "subtil" é resultado válido.

---

## Tabela master

| Variante | PSNR Y (dB) | SSIM Y | ΔE méd | ΔE máx | Tempo (s) | Status |
|---|---|---|---|---|---|---|
| **Standard** | — | — | — | — | 7.81 | base |
| Lambert | 38.37 | 0.9962 | 0.30 | 22.06 | 7.80 | OK |
| NoB | 35.16 | 0.9948 | 0.43 | 26.39 | 7.84 | OK |
| AFixo | 37.62 | 0.9994 | 0.36 | 11.04 | 8.05 | OK |
| NoClamp | 60.61 | 0.9999 | 0.02 | 9.17 | 7.96 | luz |
| Fujii | 38.87 | 0.9992 | 0.31 | 13.71 | 7.82 | OK |
| Full | 60.38 | 0.9999 | 0.02 | 9.35 | 7.98 | OK |
| FullInter | 49.68 | 0.9998 | 0.15 | 9.15 | 7.85 | OK |

---

## Ranking de impacto (PSNR menor = maior diferença)

| # | Variante | PSNR | Magnitude |
|---|---|---|---|
| 1 | NoB | 35.16 | Maior |
| 2 | AFixo | 37.62 | Média |
| 3 | Lambert | 38.37 | Média |
| 4 | Fujii | 38.87 | Média |
| 5 | FullInter | 49.68 | Pequena |
| 6 | Full | 60.38 | Mínima |
| 7 | NoClamp | 60.61 | Mínima |

---

## Análise por categoria

### Escurecimento + Retroreflexão (variantes erradas)
- **NoB** (PSNR 35, maior diff): remove a retroreflexão B. Isola o termo B sem cancelamento → maior impacto de todas
- **AFixo** (PSNR 38): remove o escurecimento A. Diff uniforme, Max baixo (0.08) → multiplicador global, sem picos
- **Lambert** (PSNR 38): remove A **e** B. Diff média porque A e B se cancelam parcialmente (ver Anomalias)

### Azimute (variante errada)
- **NoClamp** (PSNR 61, mínima): permite cosΔφ<0. Sob esta luz o azimute raramente fica negativo → quase nulo. Mensurável mas pequeno (seria maior com luz lateral)

### Alternativas correctas
- **Fujii** (PSNR 39): energy-preserving. Difere a sigma alto (~23%) — formulação A/B diferente do qualitative clássico (ver Anomalias)
- **Full** (PSNR 60, mínima): C1/C2/C3. O standard simplificado já tem C1 + parte do C2 → o acréscimo é mínimo
- **FullInter** (PSNR 50): Full + inter-reflexão L2. Maior acréscimo "correcto"; cromático (∝ Kd², ver Anomalias)

---

## Tempos

| Métrica | Valor |
|---|---|
| Mais rápido | Lambert (7.80 s) |
| Mais lento | AFixo (8.05 s) |
| Diferença max | 0.25 s (~3%) |
| Média | ~7.9 s |

ON é ~2.4× mais rápido que o CookTorrance (~7.9 s vs ~19 s) — difuso puro, sem o ramo especular D·F·G por amostra. A complexidade da variante quase não altera o tempo.

---

## Anomalias / observações

1. **NoB > Lambert (contra-intuitivo, mas correcto)**
   - Lambert remove A **e** B; NoB remove só B. Esperar-se-ia Lambert ≥ NoB
   - vs Lambert: A escurece (−) e B clareia (+) → **cancelam-se** → diff pequeno
   - vs NoB: resta só B isolado, sem cancelamento → diff maior
   - Conclusão: escurecimento e retroreflexão opõem-se; removê-los aos dois muda menos que remover um só

2. **FullInter é dominante no canal R**
   - RMSE R/G/B = 0.0055 / 0.0029 / 0.0012 → rácio **0.49 : 0.25 : 0.09 = Kd²** (argila 0.7/0.5/0.3²)
   - O termo L2 depende de Kd² → aparece quase só no R. Confirma que o L2 está correcto

3. **Fujii não é trivial (recalibração de expectativa)**
   - Esperava-se "quase idêntico" (paralelo ao F_SG do CT). Difere ~23% a sigma alto
   - Causa: a normalização energy-preserving de Fujii dá A/B diferentes do modelo qualitative clássico. Não é bug — são duas formulações legítimas distintas (o F_SG aproximava a MESMA curva; o Fujii é outra)

4. **Diferenças globalmente subtis**
   - SSIM ≥ 0.995 e ΔE médio ≤ 0.43 em todas → estrutura intacta, mudanças só de intensidade
   - Histogramas concentrados em [1e-2, 1e-1) → diferença difusa e suave (não localizada), típico de BRDF difusa — contraste com o CT, onde os highlights davam caudas longas

---

## Conclusões

- **kD/termos difusos**: o achado NoB>Lambert mostra que escurecimento e retroreflexão se opõem e cancelam parcialmente
- **O modelo simplificado é bom**: Full ≈ standard (PSNR 60) → C1 + parte do C2 capturam quase todo o single-scatter
- **A inter-reflexão (L2) é o maior acréscimo "correcto"** e é cromática (∝ Kd²)
- **Fujii é uma formulação distinta**, não uma aproximação trivial — diverge a roughness alta
- **NoClamp quase nulo** nesta geometria → o erro do azimute negativo raramente se manifesta com luz angular
- **ON é barato**: metade do tempo do CookTorrance, sem ramo especular
- **Estudo válido**: as 7 variantes deram diferença mensurável e fisicamente coerente

---

## Limitações do estudo

- Luz angular única → a NoClamp ficaria mais visível com luz lateral pura (forward-scatter)
- 1 material (argila) → o efeito cromático do L2 dependeria de outro Kd
- Geometria só com esferas → o escurecimento seria mais forte num plano a ângulo rasante
- Diferenças subtis (PSNR 35–60 dB) → confiar no RMSE, não no olho
