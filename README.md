# P-CG

# Execution

1. `make` to build the project
1. `make run` to compute and display the image (requires a previous `make build`)
1. `make display` to only display the image (requires a previous `make run`)
1. `make view`       → renderiza + abre viewer com reference.ppm carregado
1. `make view-only`  → abre viewer sem re-renderizar (útil para iterar nos shaders)

# RMSE Evaluation

1. Compute the image on the renderer, lets imagine you give it the name \<output_image>
2. Move the image to the directory `samples/`
3. Run `./rmse_exec \<output_image> reference.ppm`

This will generate the comparisson result which should be compared to the time taken to render as explained in [samples.md](samples.md).
