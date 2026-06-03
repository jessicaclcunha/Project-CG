# CookTorrance — Estudo: Efeitos em Ângulos Rasantes

## Problema

Algumas das diferenças estudadas nas variantes de Fresnel (especialmente FConst vs Schlick)
são visualmente subtis porque o efeito de Fresnel só é forte quando VdotH → 0,
ou seja, em ângulos rasantes. Na cena actual (câmara frontal, 4 esferas), esse
ângulo ocorre apenas numa faixa estreita nas bordas das esferas — área pequena
e difícil de observar.

## Estado actual

Aguardar comparação com Mitsuba. Se o próprio Mitsuba não mostrar diferença clara
com a mesma cena, o problema é do setup de cena e não do código.
Implementar uma das soluções abaixo se necessário.

---

## Soluções Possíveis

### A — Plano horizontal visto a ângulo rasante

Adicionar um plano grande (floor) como objeto principal da cena, com a câmara
posicionada muito baixa a olhar horizontalmente ao longo da superfície.
Toda a superfície visível estará em ângulo rasante → Fresnel forte em toda a imagem.

```cpp
// Câmara rasante
const Point Eye = {0, 0.05f, -5}, At = {0, 0, 10};
// Adicionar plano CookTorrance liso (roughness=0.05, metallic=0) como objeto principal
```

### B — Baixar o viewpoint da câmara

Mudança mínima: baixar Eye.y para aproximar a câmara do nível do chão.
Isso aumenta a área rasante visível nas esferas (mais bordas expostas).

```cpp
// Opção atual:
const Point Eye = {0, 0.5f, -5}, At = {0, 0, 3};

// Câmara mais baixa:
const Point Eye = {0, 0.05f, -5}, At = {0, 0, 3};
```

### C — Cilindro (se implementado)

Um cilindro visto de frente cobre continuamente todos os ângulos desde
normal (centro) até rasante (bordas). Mais eficaz que uma esfera por
ter uma área lateral maior.

### D — Render multi-ângulo

Guardar renders do mesmo material visto de diferentes ângulos de câmara
(frontal, 45°, rasante) e comparar lado a lado. Permite ver a evolução
do Fresnel sem alterar a cena.

---

## Prioridade

Baixa. Verificar com Mitsuba primeiro. Se o Mitsuba também não mostrar a diferença,
implementar Solução A ou B antes de reavaliar as variantes de Fresnel.
