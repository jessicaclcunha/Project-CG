# Resumo — Estudo Ward

7 variantes testadas contra a referência `WardTestStandart` (Ward original 1992, 4 esferas: iso liso 0.10/0.10, iso rugoso 0.40/0.40, aniso 0.10/0.40, aniso forte 0.05/0.50; metal cinzento escuro; point light `(0,2.5,−2)`). Resolução 640×640, SPP=218.

> Ward é **glossy especular anisotrópico** → o "estado" da imagem é highlights pequenos sobre corpos escuros (Kd=0.06). Isto inverte algumas intuições: o que muda *muitos píxeis* (o corpo difuso) pesa mais no RMSE global do que o que muda *só o highlight* (a anisotropia). Ver Anomalias.

> **Correcção de rótulo:** o cabeçalho de `Ward.hpp` diz "Geisler-Moroder & Dür 2010", mas o denominador é `√(NdotL·NdotV)` — isto é o **Ward original 1992**. A variante `GMD` implementa a verdadeira GMD. O standard deste estudo é, de facto, o Ward 1992.

---

## Tabela master

| Variante | PSNR Y (dB) | SSIM Y | ΔE méd | ΔE máx | RMSE Y | Tempo (s) | Status |
|---|---|---|---|---|---|---|---|
| **Standard** | — | — | — | — | — | 7.965 | base |
| NoDiff | 21.18 | 0.9227 | 2.70 | 49.38 | 0.08729 | 7.920 | OK |
| NoNorm | 30.24 | 0.9917 | 0.36 | 42.53 | 0.03077 | 7.477 | OK |
| IsoForced | 31.82 | 0.9911 | 0.21 | 52.24 | 0.02564 | 8.011 | OK |
| GMD | 45.06 | 0.9993 | 0.06 | 15.65 | 0.00559 | 7.452 | OK |
| Dür | 52.98 | 0.9998 | 0.03 | 5.76 | 0.00225 | 7.462 | OK |
| NoGeom | 53.47 | 0.9998 | 0.03 | 5.88 | 0.00212 | 7.570 | OK |
| Fresnel | 64.41 | 0.9999 | 0.01 | 4.66 | 0.00060 | 7.619 | OK |

---

## Ranking de impacto (PSNR menor = maior diferença)

| # | Variante | PSNR | Magnitude |
|---|---|---|---|
| 1 | NoDiff | 21.18 | Maior |
| 2 | NoNorm | 30.24 | Grande |
| 3 | IsoForced | 31.82 | Grande (local) |
| 4 | GMD | 45.06 | Pequena |
| 5 | Dür | 52.98 | Mínima |
| 6 | NoGeom | 53.47 | Mínima |
| 7 | Fresnel | 64.41 | Mínima |

> ΔE **máximo** dá um ranking diferente do PSNR: IsoForced (52.24) > NoDiff (49.38) > NoNorm (42.53). PSNR/RMSE medem o erro **global** (área); ΔE máx mede o pico **local**. A anisotropia é a maior mudança local mas concentrada nas 2 esferas aniso → pesa pouco no global.

---

## Análise por categoria

### Difuso e normalização (variantes erradas)
- **NoDiff** (PSNR 21, maior): remove `Kd/π`. Apesar de Kd ser baixo (0.06), a luz é intensa (320) e o difuso cobre **todo o corpo** das esferas, enquanto o especular é um ponto. Removê-lo escurece a área inteira → maior RMSE global. SSIM 0.92 (única abaixo de 0.99). Ver Anomalias.
- **NoNorm** (PSNR 30): remove `1/(αx·αy)`. As esferas lisas (α=0.10 → factor 1/0.01=100×) perdem quase todo o highlight; as rugosas escurecem menos. Forte nas zonas brilhantes de todas as esferas.

### Anisotropia (variante errada)
- **IsoForced** (PSNR 32 global, **ΔE máx 52 = o maior**): força αx=αy=√(αx·αy). Esf. 1/2 (já iso) inalteradas; esf. 3/4 colapsam de streak esticado para mancha redonda. É a **assinatura do Ward** — maior mudança *local*, mas só em 2 esferas → impacto global moderado.

### Termo geométrico (errada vs correctas)
- **NoGeom** (PSNR 53, mínima): remove `√(NdotL·NdotV)`. Como esse factor é ≤1, removê-lo **aumenta** o denominador → highlight ligeiramente mais escuro (não rebenta). Ver Anomalias.
- **Dür 2006** (PSNR 53, mínima): denom linear `(NdotL·NdotV)`. Simétrico do NoGeom — clareia um pouco. Magnitude quase igual (são as duas pequenas perturbações opostas do mesmo termo).
- **GMD 2010** (PSNR 45): denom `(NdotH)^4`. Diverge mais que Dür/NoGeom porque `(NdotH)^4` cai depressa fora do centro do highlight → maior diferença na saia do brilho. A mais diferente das 3 normalizações "correctas", como esperado.

### Fresnel (correcta)
- **Fresnel** (PSNR 64, **a menor de todas**): adiciona Schlick ao Ks. Com Ks=0.90, o termo `(1−Ks)=0.10` limita o ganho a rasante → efeito marginal. Ligeiramente mais forte no B (RMSE B 0.00077 vs R 0.00059).

---

## Tempos

| Métrica | Valor |
|---|---|
| Mais rápido | GMD (7.452 s) |
| Mais lento | IsoForced (8.011 s) |
| Diferença max | 0.56 s (~7%) |
| Média | ~7.7 s |

Ward ≈ Oren-Nayar em custo (~7.7 s vs ~7.9 s) e ~2.5× mais rápido que o CookTorrance (~19 s) — um `exp` por amostra, sem o ramo D·F·G. A complexidade da variante não altera o tempo (diferenças dentro do ruído).

---

## Anomalias / observações

1. **NoDiff é a MAIOR diferença (previa-se a menor)**
   - Raciocínio inicial: Kd baixo (0.06) → remover difuso muda pouco.
   - Realidade: a luz é intensa e o difuso ilumina **toda a superfície** visível; o especular é só um ponto. O difuso é a maior parte do sinal do corpo → removê-lo escurece tudo.
   - Confirmação: histograma com 18 025 píxeis em [0.3, 0.5) (escurecimento de área largo), SSIM 0.92 (estrutura mexida, não só intensidade). Lição: em material muito especular com corpo escuro, o difuso ainda domina a **área**, embora não o **brilho**.

2. **NoGeom NÃO rebenta — escurece ligeiramente (previa-se blow-up a rasante)**
   - Raciocínio inicial: sem `√(NdotL·NdotV)` no denom, a rasante (NdotL→0) explodiria (1/√→∞).
   - Realidade: removê-lo **aumenta** o denom (o factor é ≤1) → mais escuro, não mais claro. E a rasante já tem `exp(...)→0` (H longe de N) → o blow-up nunca acontece. Resultado: mínima diferença.

3. **PSNR e ΔE máx discordam (global vs local)**
   - IsoForced tem o **maior ΔE máx** (52) mas só o 3º PSNR. A anisotropia é a maior mudança *por píxel* mas confinada a 2 esferas.
   - NoDiff/NoNorm afectam *toda* a área → ganham no global (PSNR) apesar de picos menores.

4. **Dür ≈ NoGeom em magnitude**
   - Ambas perturbam o mesmo termo geométrico em direcções opostas (linear vs remover) por um factor `√(NdotL·NdotV)` → |Δ| semelhante (PSNR 53.0 vs 53.5). Coerente.

5. **As 3 normalizações "correctas" ordenam-se GMD > Dür > NoGeom-like**
   - Confirma o plano: divergem pouco no centro do highlight, mais na saia; a GMD `(NdotH)^4` é a que mais se afasta do original 1992.

---

## Conclusões

- **O difuso domina a área, o especular domina o brilho**: em Ward com corpo escuro, remover o difuso (NoDiff) muda mais píxeis que qualquer variante especular.
- **A anisotropia é uma mudança local intensa**: IsoForced lidera o ΔE máx mas não o RMSE global — é a assinatura do Ward, visível mas espacialmente concentrada.
- **A normalização `1/(αx·αy)` importa**: removê-la (NoNorm) apaga os highlights das superfícies lisas — 2º maior impacto.
- **As correcções de normalização são subtis**: Dür e GMD diferem do Ward 1992 só na saia do highlight (PSNR 45–53); a GMD é a mais distinta.
- **Fresnel é marginal aqui**: com Ks alto (0.90) o ganho a rasante é pequeno — relevante só em materiais menos reflectores.
- **Rótulo corrigido**: o standard é Ward 1992, não GMD 2010 (apesar do comentário no código).
- **Estudo válido**: 7 variantes, diferenças mensuráveis e fisicamente explicáveis, sem artefactos.

---

## Limitações do estudo

- 1 luz pontual frontal-superior → a rasante extrema (onde Dür/GMD/NoGeom mais divergem) aparece pouco; com luz lateral as 3 normalizações separar-se-iam mais.
- Material muito especular (Kd 0.06, Ks 0.90) → exagera o peso do NoDiff e minimiza o do Fresnel; outro balanço Kd/Ks daria outro ranking.
- Geometria só esferas → o highlight nunca atinge ângulos verdadeiramente rasantes prolongados (um plano inclinado mostraria melhor as normalizações).
- Diferenças globais subtis fora do NoDiff (PSNR 30–64) → confiar no RMSE/ΔE máx, não no olho.
