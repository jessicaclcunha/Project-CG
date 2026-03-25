- As texturas são para ser colocadas nas formas e terá que funcionar como se fossem o objeto normal
- O mesmo objeto/forma (por exemplo: esfera) podem ter uma parte metálica e a outra não. Ou seja, diferentes "BRDFs" na mesma forma
- Testar com diferentes formas, sem ser esfera
- Melhorar a fórmula por trás do CookTorrance - Não usar F0, ser autocalculado.
- Implementar BRDFs mais complexas, com mts parametros, para brincar com elas.

---

Fresnel difuso vs especular inconsistente — O difuso usa Fresnel em NdotV, enquanto o especular usa Fresnel em VdotH. A abordagem mais rigorosa (tipo Unreal Engine) seria derivar kD = (1-F_specular)*(1-metallic) usando o F do especular calculado em VdotH. Na prática a vossa abordagem é uma aproximação comum e aceitável, mas vale a pena mencionar no relatório que é uma simplificação.

Edge case roughness=0 — Quando roughness=0, alpha=0, alpha²=0, e D pode dar 0/0 = NaN no caso em que NdotH=1. Isto está notado como tarefa da semana 11, mas convém ter em mente. Um clamp mínimo alpha = max(roughness², 0.001) resolveria.

CookTorranceTextureScene comentada — O código comentado nessa função usa a assinatura antiga de AddCookTorranceMat (que aceitava F0 como RGB). Isso está obsoleto desde o refactor da S1 e deve ser atualizado ou removido quando for descomentado.

brdf_viewer.html — O viewer ainda tem um slider manual de F0 separado do metallic. No C++, o F0 é derivado automaticamente. Isto não é um bug funcional, mas cria uma inconsistência entre o que se vê no viewer e o que o ray tracer faz. Vale a pena harmonizar eventualmente.