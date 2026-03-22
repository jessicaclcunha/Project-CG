# Architecture

This is a C++11 ray tracing renderer. Rendering is configured entirely in `src/main.cpp` — scene selection, camera, shader, resolution, and samples-per-pixel (SPP) are all set there.

## Rendering Pipeline

1. **Scene** (`src/Scene/`) — holds primitives, lights, and materials. Pre-built scenes are in `BuildScenes.hpp`.
2. **Camera** (`src/Camera/`) — generates rays. `Perspective` supports depth-of-field (defocus radius + focus distance).
3. **Renderer** (`src/Renderer/`) — `StandardRenderer` drives the main loop with jittered multi-sampling.
4. **Shader** (`src/Shader/`) — shades each ray. Options:
   - `WhittedShader` — classic recursive ray tracing
   - `DistributedShader` — stochastic distributed ray tracing (current default)
   - `PathTracingShader` — Monte Carlo path tracing
   - `directLighting` / `AmbientShader` / `DummyShader` — simpler alternatives
5. **Image output** (`src/Image/`) — writes PPM; optional post-filters (Box, Median) and Reinhard tone mapping.

## Key Types

- `Ray` (`src/Rays/`) — carries type flag (PRIMARY, SHADOW, SPEC_REFL, SPEC_TRANS, DIFF_REFL).
- `Intersection` (`src/Rays/`) — hit point, normal, material reference.
- `BRDF` (`src/Primitive/`) — base material class. Concrete BRDFs: `Phong`, `CookTorrance`, and textured variants (`PhongTexture`, `CookTorranceTexture`, `DiffuseTexture`).
- `RGB` / `vector` (`src/utils/`) — color and math primitives used throughout.

## Acceleration

Triangle meshes use a BVH (`src/Primitive/`). Spheres are tested directly.

## Lighting

- `AmbientLight` — global constant
- `PointLight` — hard shadows
- `AreaLight` — soft shadows via distributed sampling

## BRDF Notes

- Cook-Torrance and Phong are interchangeable in scenes — they share the same `Ka`, `Kd`, `Ks`, `Kt`, `eta` material interface.
- Cook-Torrance adds a `roughness` parameter controlling microfacet distribution.
