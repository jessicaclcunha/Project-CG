# Project Planning: Implementação e Análise de BRDFs

**Deadline:** 22 de Junho
**Duração Estimada:** 12 Semanas
**Foco Principal:** Implementação de BRDFs, análise paramétrica, texturas, e cenas de demonstração.

---

## Estratégia Base e Stack Tecnológica
* **Linguagem:** C++ (ray tracer existente).
* **BRDFs base:** Phong (feito), Cook-Torrance GGX (a melhorar).
* **Texturas:** Imagens PPM mapeadas via UV.
* **Geometrias:** Esferas, triângulos, cubos (compostos por triângulos).
* **Ferramenta de visualização:** `brdf_viewer.html` (WebGL 2.0).
* **Build:** Makefile (`make`, `make run`, `make view`).

---

## Calendário de Desenvolvimento

### FASE 1 — Fundações e Melhorias

#### Semana 1: Melhorias CookTorrance + Formas Não-Esfera (March 16 to 22)

**Membro A:**
* [ ] Melhorar CookTorrance: remover F0 como parâmetro manual.
* [ ] Calcular F0 a partir de metallic + albedo (workflow PBR).
* [ ] Testar com materiais: plástico, ouro, cobre, borracha.

**Membro B:**
* [ ] Criar `AddBox()` com triângulos (cubo).
* [ ] Criar cena `PhongScene` com cubo + esfera lado a lado.
* [ ] Criar cena `CookTorranceScene` idem.

#### Semana 2: Multi-BRDF na Mesma Forma + Texturas em Cubos (March 23 to 29)

**Membro A:**
* [ ] Permitir diferentes materiais por face/triângulo do mesmo objeto.
* [ ] Criar cena demo: cubo com face metálica + face plástica.
* [ ] Testar com CookTorrance metallic vs dielectric.

**Membro B:**
* [ ] UV mapping para o cubo (`AddBoxUV`).
* [ ] Testar `PhongTexture` e `CookTorranceTexture` no cubo.
* [ ] Verificar que especular é independente da textura.

#### Semana 3: Oren-Nayar + Preparação Checkpoint 1 (March 30 to April 5) — 🟠 CHECKPOINT 1

**Membro A:**
* [ ] Implementar `OrenNayar.hpp` herdando de `BRDF`.
* [ ] Parâmetro σ (roughness difuso).
* [ ] Criar `OrenNayarScene` com variação de σ.
* [ ] Adicionar Oren-Nayar ao `brdf_viewer`.

**Membro B:**
* [ ] Gerar renders comparativos (Phong vs CookTorrance vs Oren-Nayar).
* [ ] Organizar imagens de teste.
* [ ] Documentar melhorias feitas no CookTorrance.
* [ ] Início da estrutura do relatório.

---

### FASE 2 — BRDFs Avançadas

#### Semana 4: Disney Principled (Parte 1) + Ward Anisotropic (April 6 to 12)

**Membro A:**
* [ ] Estudar o paper Disney (Burley 2012).
* [ ] Implementar subsurface, metallic, specular, specularTint.
* [ ] Criar `DisneyBRDF.hpp` com parâmetros base.

**Membro B:**
* [ ] Implementar `Ward.hpp`.
* [ ] Parâmetros: αx, αy (roughness anisotrópica).
* [ ] Criar `WardScene` com variação anisotrópica.
* [ ] Adicionar ao `brdf_viewer`.

#### Semana 5: Disney Principled (Parte 2) + Ashikhmin-Shirley (April 13 to 19)

**Membro A:**
* [ ] Adicionar roughness, anisotropic, sheen, sheenTint, clearcoat.
* [ ] Criar `DisneyScene` com presets (metal, plástico, tecido, etc.).
* [ ] Testar conservação de energia.

**Membro B:**
* [ ] Implementar `AshikhminShirley.hpp`.
* [ ] Especular anisotrópico + difuso dependente de Fresnel.
* [ ] Criar cena de teste.
* [ ] Adicionar ao `brdf_viewer`.

#### Semana 6: Integração + Preparação Checkpoint 2 (April 20 to 26) — 🟠 CHECKPOINT 2

**Membro A:**
* [ ] Cena com 6+ esferas, uma por BRDF.
* [ ] Mesmos parâmetros equivalentes para comparação direta.
* [ ] Gerar renders de alta qualidade.

**Membro B:**
* [ ] Gerar tabela comparativa de BRDFs.
* [ ] Análise de parâmetros (como cada um afeta o resultado).
* [ ] Expandir relatório com secções das novas BRDFs.

---

### FASE 3 — Cenas Complexas e Relatório

#### Semana 7: Blinn-Phong + Cenas Complexas Parte 1 (April 27 to May 3)

**Membro A:**
* [ ] Implementar `BlinnPhong.hpp` (half-vector).
* [ ] Comparar com Phong clássico.
* [ ] Documentar diferenças teóricas e visuais.

**Membro B:**
* [ ] Cornell Box com materiais variados por face.
* [ ] Cena com chão + objetos (esferas + cubos).
* [ ] Testar texturas + BRDFs diferentes na mesma cena.

#### Semana 8: GGX Multiscatter + Cenas Complexas Parte 2 (May 4 to 10)

**Membro A:**
* [ ] Implementar correção de energia (energy compensation) no CookTorrance.
* [ ] Comparar com/sem compensação.
* [ ] Documentar impacto visual.

**Membro B:**
* [ ] Cena showcase com todas as BRDFs em uso.
* [ ] Materiais realistas: metal escovado, cerâmica, veludo.
* [ ] Gerar galeria de renders finais.

#### Semana 9: Relatório — Teoria + Implementação (May 11 to 17)

**Membro A:**
* [ ] Fundamentação teórica de cada BRDF.
* [ ] Fórmulas matemáticas detalhadas.
* [ ] Comparação teórica (tabela de propriedades).

**Membro B:**
* [ ] Arquitetura do código (diagrama de classes).
* [ ] Detalhes de implementação relevantes.
* [ ] Decisões de design e trade-offs.

#### Semana 10: Relatório — Análise de Resultados (May 18 to 24)

**Membro A:**
* [ ] Comparação visual (imagens lado a lado).
* [ ] Análise paramétrica (variação de parâmetros).
* [ ] Performance / custo computacional de cada BRDF.

**Membro B:**
* [ ] Documentar cada cena criada.
* [ ] Explicar escolhas de materiais.
* [ ] Texturas e multi-material: resultados e discussão.

#### Semana 11: Polish do Código + Revisão do Relatório (May 25 to 31)

**Membro A:**
* [ ] Limpeza e refactor do código.
* [ ] Comentários em código.
* [ ] Garantir que todas as cenas compilam e correm.
* [ ] Verificar edge cases (roughness=0, etc.).

**Membro B:**
* [ ] Conclusões e trabalho futuro.
* [ ] Revisão integral do relatório.
* [ ] Verificar figuras, referências e formatação.

#### Semana 12: Revisão Final e Entrega (June 1 to 7) — 🔴 ENTREGA FINAL

**Membro A:**
* [ ] Reler relatório completo.
* [ ] Corrigir erros e inconsistências.
* [ ] Preparar entrega (zip, README atualizado).

**Membro B:**
* [ ] Testar build limpo (`make clean && make`).
* [ ] Verificar que todos os renders são reproduzíveis.
* [ ] Submeter entrega final.

---

## Resumo de BRDFs

| BRDF | Estado | Parâmetros Principais | Responsável |
|------|--------|-----------------------|-------------|
| Phong | ✅ Feito | Kd, Ks, ns | — |
| CookTorrance (GGX) | 🟡 A melhorar | roughness, metallic, albedo | Membro A (S1) |
| Oren-Nayar | ⬜ Por fazer | Kd, σ (roughness difuso) | Membro A (S3) |
| Disney Principled | ⬜ Por fazer | metallic, roughness, subsurface, sheen, clearcoat, ... | Membro A (S4-5) |
| Ward Anisotropic | ⬜ Por fazer | αx, αy, Kd, Ks | Membro B (S4) |
| Ashikhmin-Shirley | ⬜ Por fazer | nu, nv, Rd, Rs | Membro B (S5) |
| Blinn-Phong | ⬜ Por fazer | Kd, Ks, ns (half-vector) | Membro A (S7) |
