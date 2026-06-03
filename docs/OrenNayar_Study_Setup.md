# Oren-Nayar — Estudo de Setup de Cena

Antes do estudo das variantes, é preciso escolher o **setup de cena** (luz, geometria, material, ambiente) que melhor revela os fenómenos do Oren-Nayar. Caso contrário, comparar variantes numa cena que não mostra o efeito dá diferenças artificialmente pequenas — foi o problema dos ângulos rasantes no CookTorrance.

---

## Os dois fenómenos a mostrar

| Fenómeno | Termo | Quando é visível |
|---|---|---|
| **Escurecimento** | A = 1 − 0.5σ²/(σ²+0.33) | sigma alto → superfície mais escura e "mate"; visível em todo o lado |
| **Retroreflexão** | B = 0.45σ²/(σ²+0.09)·max(0,cosΔφ)·sinα·tanβ | só no **back-scatter**: quando luz ≈ direcção da vista |

A assinatura clássica do ON é o **"efeito lua cheia"**: com luz vinda da câmara, a esfera achata e parece um disco uniforme, em vez do gradiente Lambertiano (centro claro → bordo escuro).

> Nota gamma/tone mapping: a correcção gamma (1/2.2) e o Reinhard são globais
> ([src/Image/ImagePPM.cpp](../src/Image/ImagePPM.cpp)), aplicados no save de qualquer
> render. Não há nada específico do ON a afinar aqui — é herdado.

---

## Eixo 1 — Direcção da luz  ✅ decisivo

Cada variante "vive" numa região diferente da esfera, e a luz tem de iluminar
essa região para o RMSE a medir:

| Variante | Onde difere | Precisa de |
|---|---|---|
| NoB | back-scatter (limbo virado à luz) | luz frontal |
| NoClamp | forward-scatter (lado oposto) | luz lateral |
| Full / FullInter | back **e** forward | ambas |
| AFixo | em todo o lado | qualquer |

| Opção | Resultado |
|---|---|
| Farol puro (luz ≈ câmara) | só back-scatter → mede NoB, **anula NoClamp** |
| Lateral puro | só forward-scatter → mede NoClamp, **anula NoB** |
| **Angular ~30° (do lado da câmara, off-axis)** | **Escolhido.** back **e** forward → mede TODAS |

**Decisão:** luz **angular**, PointLight em `(−2, 2, −2)` — do lado da câmara
(z<0, dá back-scatter no limbo) e em ângulo (cria terminador no lado oposto, dá
forward-scatter). Sem fill. É o único compromisso que torna as 7 variantes
mensuráveis.

> Nota: a demo `OrenNayarLambertVsON` usa um **farol puro** `(0, 0.5, −4.5)`
> sem fill — aí o objectivo é mostrar dramaticamente a retroreflexão (esfera
> achatada tipo "lua") ao olho, não medir. Demo e standard têm fins diferentes.

---

## Eixo 2 — Geometria

| Opção | Resultado |
|---|---|
| **Esfera** (escolhido) | Cobre toda a gama de ângulos de normal; mostra retroreflexão (achatamento) **e** escurecimento (terminador). Consistente com o CookTorrance |
| Plano rugoso a ângulo rasante | Mostra o **escurecimento** de forma mais forte numa área grande, mas precisa de **baixar a câmara** em `main.cpp` (Eye.y → ~0.05) e não mostra retroreflexão tão bem |

**Decisão:** **esfera** (4 esferas, sigma 0.3/0.5/0.7/0.9), igual ao CookTorrance — permite comparação directa e mostra os dois efeitos. O plano fica como opção suplementar (ver abaixo), não usado no estudo principal.

> Opção plano (não implementada): adicionar um plano grande ON e usar
> `const Point Eye = {0, 0.05f, −5}` em main.cpp. Paralelo ao
> [CookTorrance_Study_AngulosRasantes.md](CookTorrance_Study_AngulosRasantes.md).

---

## Eixo 3 — Material (Kd)

| Opção | Cena | Resultado |
|---|---|---|
| **Argila** `(0.7,0.5,0.3)` (escolhido) | `OrenNayarTestStandart` | Midtone quente; bom equilíbrio entre legibilidade do escurecimento e da retroreflexão |
| Veludo escuro `(0.18,0.10,0.22)` | `OrenNayarSetupVelvet` | Contraste relativo da retroreflexão **máximo** (showcase clássico), mas zonas escuras dificultam ler o escurecimento e dão mais ruído |
| Branco `(0.9,0.9,0.9)` | — | Lava o efeito; o escurecimento fica subtil |

**Decisão:** **argila** `(0.7, 0.5, 0.3)` para o standard. O veludo é o melhor "cartaz" do efeito mas pior para medir as duas componentes em simultâneo — fica como cena de demonstração.

---

## Eixo 4 — Ambiente

Ambient alto lava os dois efeitos (a luz uniforme elimina o contraste direccional do back-scatter). **Decisão:** ambient mínimo `(0.03, 0.03, 0.03)` — evita preto absoluto sem matar o efeito. Chão escuro `(0.07)` para não reflectir luz de volta.

---

## Setup final escolhido (`addONTestLighting` + standard)

| Parâmetro | Valor |
|---|---|
| Geometria | 4 esferas r=0.8, sigma 0.3/0.5/0.7/0.9 |
| Material | argila Kd `(0.7, 0.5, 0.3)`, Ka `(0.02, 0.01, 0.01)` |
| Key light | PointLight `(−2, 2, −2)`, 220 (angular ~30° → back + forward scatter) |
| Fill light | nenhuma (a luz angular já dá zona clara e escura) |
| Ambient | `(0.03, 0.03, 0.03)` |
| Chão | difuso `(0.07, 0.07, 0.07)` em y=−0.8 |
| Câmara | default de main.cpp: Eye `(0, 0.5, −5)` |

Este é o setup usado por `OrenNayarTestStandart` e por todas as variantes do estudo, garantindo que as diferenças medidas vêm da fórmula e não da cena.

---

## Nota metodológica

A exploração passou por várias iluminações até à decisão final:
- **Farol puro** (luz ≈ câmara): retroreflexão dramática (esfera achata como
  "lua"), óptimo para uma **demo**, mas anula o forward-scatter → a variante
  NoClamp daria diff≈0. Reservado para a demo `OrenNayarLambertVsON`.
- **Lateral puro**: terminador forte, mas sem back-scatter → a NoB daria
  diff≈0. Descartado.
- **Angular ~30° (escolhido)**: tem back **e** forward scatter → mede todas as
  variantes; e dá um look 3D natural com terminador.
- **Veludo**: contraste máximo da retroreflexão, mas zonas escuras escondem o
  escurecimento e dão mais ruído. Fica só como cena de demonstração.

O setup escolhido vive em `addONTestLighting` + `OrenNayarTestStandart`
([src/Scene/OrenNayarScenes.cpp](../src/Scene/OrenNayarScenes.cpp)), tal como no
CookTorrance se afinou directamente a cena standard. A demo `LambertVsON`
mostra o efeito ao olho (farol); o standard serve para medir (luz angular).
