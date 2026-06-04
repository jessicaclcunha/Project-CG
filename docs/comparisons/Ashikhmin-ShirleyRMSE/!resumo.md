# Resumo — Estudo Ashikhmin-Shirley

7 variantes testadas contra a referência `AshikhminShirleyTestStandart` (AS 2000, point light, 4 esferas: 2 plásticos azuis Ks branco nu=10/80, 2 metais dourados Ks ouro — m3 escovado nu=500/nv=10 aniso, m4 polido nu=1000). Resolução 640×640, SPP=218.

> AS = **Phong anisotrópico** (especular + difuso com conservação de energia). 5 variantes "erradas" (ablações) + 2 "correctas" (formulações de Fresnel legítimas), à imagem de CT/ON/Ward.

> **Materiais com Ks alto** (plástico 0.50, metal 0.90): isto faz o termo de Fresnel pesar pouco (o ganho é ∝ (1−Ks)) — chave para ler os resultados de Fresnel. Ver Anomalias.

---

## Tabela master

| Variante | PSNR Y (dB) | SSIM Y | ΔE méd | ΔE máx | RMSE Y | Tempo (s) | Status |
|---|---|---|---|---|---|---|---|
| **Standard** | — | — | — | — | — | 9.757 | base |
| NoDiff | 18.59 | 0.9341 | 4.09 | 88.71 | 0.11757 | 8.695 | OK |
| LambDiff | 24.40 | 0.9741 | 1.84 | 59.88 | 0.06029 | 8.426 | OK |
| NoNorm | 33.39 | 0.9948 | 0.43 | 47.18 | 0.02141 | 8.605 | OK |
| FExact | 34.21 | 0.9962 | 0.40 | 37.43 | 0.01947 | 8.640 | OK |
| FInv | 46.55 | 0.9998 | 0.14 | 15.20 | 0.00470 | 8.912 | OK |
| FConst | 65.43 | 1.0000 | 0.02 | 5.74 | 0.00054 | 8.397 | OK |
| FSG | 65.42 | 1.0000 | 0.02 | 4.55 | 0.00054 | 8.279 | OK |

---

## Ranking de impacto (PSNR menor = maior diferença)

| # | Variante | PSNR | Magnitude | Tipo |
|---|---|---|---|---|
| 1 | NoDiff | 18.59 | Maior | errada |
| 2 | LambDiff | 24.40 | Grande | errada |
| 3 | NoNorm | 33.39 | Média | errada |
| 4 | FExact | 34.21 | Média | **correcta** |
| 5 | FInv | 46.55 | Pequena | errada |
| 6 | FConst | 65.43 | Mínima | errada |
| 7 | FSG | 65.42 | Mínima | correcta |

> Curiosidade: uma "correcta" (FExact) diverge **mais** que duas "erradas" (FInv, FConst). Ver Anomalias.

---

## Análise por categoria

### Difuso (variantes erradas)
- **NoDiff** (PSNR 19, maior): remove o difuso → corpos quase pretos (só highlight + ambiente). Como as esferas são maioritariamente iluminadas pelo difuso, é a maior mudança de área. SSIM 0.93 (a mais baixa). Mesmo padrão do Ward NoDiff.
- **LambDiff** (PSNR 24): troca o difuso com conservação de energia por Kd/π puro. Sem o factor (1−Rs) e sem os termos de Schlick → muda o corpo difuso inteiro. 2ª maior.

### Normalização (variante errada)
- **NoNorm** (PSNR 33): remove `√((nu+1)(nv+1))`. Os highlights de **alto nu** (metais m3=500, m4=1000) perdem o factor ~500–1000× → quase desaparecem; plásticos (nu=10/80) quase não mexem. **R dominante** (RMSE R 0.029 > B 0.012) porque os highlights mortos são os dos metais dourados (alto R).

### Fresnel (erradas + correctas)
- **FExact** (PSNR 34, **correcta mas a maior das de Fresnel**): Fresnel dieléctrico exacto (escalar, η=1.5). Dois efeitos: (a) tira a **cor** ao highlight do metal (dourado→cinza); (b) revela que o Ks=0.50 do plástico é art-directed — o F0 físico de η=1.5 é só **0.04** → highlight do plástico ~12× mais fraco. R dominante (perda da cor do metal).
- **FInv** (PSNR 47): inverte o Fresnel (brilho ao centro, escuro na borda). Mexe no centro do highlight (alto peso) mas limitado por (1−Ks) → moderada.
- **FConst** (PSNR 65, **quase invisível**): remove a variação angular de Fresnel. Com Ks alto, (1−Ks) é pequeno → o Fresnel já pesava pouco → tirá-lo quase não muda nada.

### Fresnel correcta "de validação"
- **FSG** (PSNR 65, mínima): spherical-gaussian (Karis) vs Schlick. Praticamente indistinguível — **valida o Schlick** como aproximação. Resultado de manual.

---

## Tempos

| Métrica | Valor |
|---|---|
| Mais rápido | FSG (8.279 s) |
| Mais lento | Standard (9.757 s) |
| Variantes | 8.28–8.91 s (apertado) |
| Média (variantes) | ~8.6 s |

O standard (9.76 s) destoa por ~1 s — provável ruído de sistema nessa corrida (a base não é mais cara que as variantes; FExact/FSG até têm `sqrt`/`expf` extra). AS fica em linha com Ward/ON (~8 s) e ~2× mais rápido que o CookTorrance (~19 s).

---

## Anomalias / observações

1. **FConst quase invisível (PSNR 65)** — esperar-se-ia que remover o Fresnel angular fosse visível (como no CT). Aqui não: o ganho de Fresnel é ∝ (1−Ks) e o Ks é alto (0.50 plástico, 0.90 metal) → o Fresnel contribui pouco → removê-lo quase não muda. **Lição:** o peso visual do Fresnel depende de quão baixo é o Ks/F0.

2. **FExact ("correcta") diverge mais que FInv/FConst ("erradas")** — contra-intuitivo. Causa dupla: o Fresnel dieléctrico escalar (a) não reproduz a **cor** do reflexo metálico (precisa de Fresnel complexo) e (b) usa o F0 físico 0.04, expondo que o Ks=0.50 do plástico é uma escolha artística. Ou seja, "exacto para dieléctricos" aplicado a metais + a um plástico art-directed afasta-se mais do que um simples tweak de Fresnel.

3. **NoDiff é a maior diferença** — igual ao Ward: em cena com corpo iluminado sobretudo por difuso, remover o difuso muda mais píxeis que qualquer variante especular. **Nuance teórica:** o NoDiff é, de facto, **fisicamente correcto para metais** (metal não tem difuso) — só é "errado" porque os nossos metais têm difuso art-directed por falta de IBL.

4. **NoNorm é cromático (R-dominante)** — os highlights que morrem são os de alto nu, que estão nos metais dourados → o erro concentra-se no R/G.

5. **FSG ≈ standard** — confirma que a forma exacta da curva de Fresnel é irrelevante; Schlick chega.

---

## Conclusões

- **Difuso domina a área, especular domina o brilho**: NoDiff/LambDiff (mexem no difuso de toda a superfície) lideram o RMSE; as variantes de Fresnel (highlight) pesam menos.
- **O peso do Fresnel depende do Ks**: com Ks alto, FConst é quase nula e FInv é moderada — o Fresnel só "aparece" quando o especular é fraco (Ks baixo).
- **Schlick é uma boa aproximação**: FSG idêntico; a curva exacta não importa visualmente.
- **Fresnel dieléctrico exacto não serve metais**: o FExact perde a cor do reflexo metálico — recordando que metais precisam de Fresnel colorido/complexo.
- **AS é barato**: ~8.5 s, em linha com Ward/ON.
- **Estudo válido**: 7 variantes, diferenças mensuráveis e fisicamente explicáveis, sem artefactos.

---

## Limitações do estudo

- **Ks art-directed altos** (plástico 0.50 ≫ 0.04 físico) → comprimem o sinal de Fresnel (FConst/FInv pequenos) e inflacionam o FExact. Com Ks físicos, o ranking de Fresnel mudaria.
- **Metais com difuso** (sem IBL, para serem visíveis) → o NoDiff/LambDiff medem efeito também no metal; com metal físico (Kd=0) só mediriam os plásticos.
- 1 luz pontual → a anisotropia (m3) vê-se mas a rasante extrema aparece pouco.
- Diferenças subtis fora do difuso (PSNR 33–65) → confiar no RMSE/ΔE máx, não no olho.
