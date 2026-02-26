# renderer
Initial version of the renderer used in Visualization and Illumnination : Masters in Informatics

# Execution

1. `make` to build the project.
1. `make run` to compute and display the image (requires a previous `make build`)
1. `make display` to only display the image (requires a previous `make run`)

# RMSE Evaluation

1. Compute the image on the renderer, lets imagine you give it the name \<output_image>
2. Move the image to the directory `samples/`
3. Run `./rmse_exec \<output_image> reference.ppm`

This will generate the comparisson result which should be compared to the time taken to render as explained in [samples.md](samples.md).
