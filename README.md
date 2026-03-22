# renderer
Initial version of the renderer used in Visualization and Illumnination : Masters in Informatics

# Execution

1. `make` to build the project.
1. `make run` to compute and display the image (requires a previous `make build`)
1. `make display` to only display the image (requires a previous `make run`)


Melhor formas de comparação:

- Comparação direta Phong vs Cook-Torrance:
    - [Phong ns=50] [CookTorrance roughness=0.3]  ← materiais equivalentes
- Variação do parâmetro principal 
    - Phong: ns = 1, 5, 50, 500
    - Cook-Torrance: roughness = 1.0, 0.5, 0.2, 0.05
- Textura + especular
    - Mostrar que a textura afeta só o difuso — o especular (highlight) mantém-se independente da imagem. Esta é interessante visualmente.

- Phong.hpp — usa Kd (cor fixa RGB):
    color += Kd * (1.f / M_PI);
- PhongTexture.hpp — lê o Kd de uma imagem:
    RGB Kd_tex = Kd * texture.get(x, y);  // pixel da imagem
    color += Kd_tex * (1.f / M_PI);
### O especular é exatamente igual nos dois. A única diferença é de onde vem o Kd.

