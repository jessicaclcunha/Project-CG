# Estudo Técnico e Teórico: Variáveis de Influência em BRDFs

Este documento serve como guia para o estudo e análise dos resultados das BRDFs implementadas no projeto. O foco não é apenas a implementação, mas a manipulação de variáveis para compreender o comportamento físico da luz.

---

## 1. Modelo de Phong (Empírico)
O objetivo aqui é observar a relação entre o brilho difuso e a concentração do lóbulo especular.

| Variável | Tipo/Intervalo | Significado Físico | Efeito Visual |
| :--- | :--- | :--- | :--- |
| **Ka** | RGB [0, 0.1] | Reflexão ambiente | Brilho mínimo nas zonas de sombra total. |
| **Kd** | RGB [0, 1] | Reflectância difusa | Cor base; uniforme em todas as direções. |
| **Ks_brdf** | RGB [0, 1] | Reflectância especular | Intensidade do "brilho" (highlight). |
| **ns** | float [1, ∞) | Expoente do lóbulo | **1-5:** Fosco; **100+:** Polido/Espelhado. |

**Plano de Estudo:**
* **Isolamento do Expoente:** Manter `Kd` e `Ks` fixos e variar apenas `ns` em potências de 10.
* **Conservação de Energia:** Testar o limite `Ks + Kd = 1`. Observar se há saturação artificial se a soma exceder 1.

---

## 2. Cook-Torrance (Físico - GGX)
Focado no *workflow* metálico e na distribuição de microfacetas.

| Variável | Tipo/Intervalo | Significado Físico | Efeito Visual |
| :--- | :--- | :--- | :--- |
| **Kd** | RGB [0, 1] | Albedo base | Cor difusa (dielétricos) ou cor do Fresnel (metais). |
| **Roughness** | float [0, 1] | Desvio de microfacetas | Controla a dispersão do lóbulo (α = roughness²). |
| **Metallic** | float [0, 1] | Natureza do material | **0:** Dielétrico (F0=0.04); **1:** Metal (F0=Albedo). |
| **Fresnel (F0)** | Implícito | Reflectância normal | Determina o brilho em ângulos rasantes (*grazing angles*). |

**Plano de Estudo:**
* **Progressão de Rugosidade:** 4 esferas com roughness [0.1, 0.3, 0.6, 0.9] com `metallic=1`.
* **Dielétrico vs Metal:** Comparar o mesmo Albedo com `metallic=0` e `metallic=1`. Observar como o highlight muda de branco para a cor do material.

---

## 3. Oren-Nayar (Rugosidade Difusa)
Ideal para estudar superfícies que não seguem a lei de Lambert (ex: gesso, pó).

| Variável | Tipo/Intervalo | Significado Físico | Efeito Visual |
| :--- | :--- | :--- | :--- |
| **Sigma (σ)** | float [0, 1] | Inclinação das facetas | **σ=0:** Lambertiano; **σ=0.9:** Retroreflexão máxima. |
| **Iluminação** | Posição | Ângulo de Incidência | Determina se a retroreflexão é captada pela câmara. |

**Plano de Estudo:**
* **Efeito Lua:** Comparar uma esfera Lambertiana com uma Oren-Nayar (σ=0.9). Usar uma luz frontal (próxima da câmara). A esfera Oren-Nayar deve parecer "achatada" e uniformemente iluminada.

---

## 4. Ward (Anisotrópico)
Estudo da reflexão direcional (ex: metal escovado).

| Variável | Tipo/Intervalo | Significado Físico | Efeito Visual |
| :--- | :--- | :--- | :--- |
| **alphaX** | float [0.01, 1] | Rugosidade em Tangente | Largura do highlight na direção X. |
| **alphaY** | float [0.01, 1] | Rugosidade em Bitangente | Largura do highlight na direção Y. |
| **Tangent** | Vector | Direção de polimento | Roda a orientação da elipse de reflexão. |

**Plano de Estudo:**
* **Anisotropia:** Fixar `alphaX=0.1` e variar `alphaY` de 0.1 a 0.9. Observar o alongamento do highlight.
* **Rotação:** Manter alphas assimétricos e rodar o vetor tangente.

---

## Resumo de Configurações de Iluminação

| BRDF | Luz Recomendada | Objetivo do Teste |
| :--- | :--- | :--- |
| **Phong** | PointLight | Ver precisão do ponto especular. |
| **Cook-Torrance** | AreaLight | Observar a cauda longa da distribuição GGX. |
| **Oren-Nayar** | PointLight Frontal | Maximizar a visualização da retroreflexão. |
| **Ward** | PointLight Lateral | Evidenciar o alongamento elíptico da reflexão. |

---
*Este guia deve ser utilizado para configurar as cenas e fundamentar as conclusões teóricas do projeto de Computação Gráfica.*