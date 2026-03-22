## BRDF Implementation Rules

- Every new BRDF inherits from the `BRDF` base class and implements the same material interface (`Ka`, `Kd`, `Ks`, `Kt`, `eta`). Add BRDF-specific params (e.g. `roughness`, `metallic`, `sigma`) on top.
- Textures affect **only the diffuse component** (`Kd`). Specular is always computed from material params, never from texture.
- UV mapping: textures are PPM images mapped via UV coordinates. Cubes need per-face UV (`AddBoxUV`).
- When creating a new BRDF: also create a matching test scene in `BuildScenes.hpp` that varies its key parameter (e.g. roughness 0.1→0.9 across multiple spheres).
- Always test energy conservation — reflected energy must not exceed incoming energy.

## Workflow

1. **Describe approach first** — before writing code, explain what will change and wait for confirmation.
2. **Minimal scope** — one BRDF or one feature per task. No unrelated refactors.
3. **Test renders** — after any material/shader change, verify with `make run` using a simple scene before moving to complex ones.
4. **Match existing style** — read surrounding code before writing. This is C++11, follow the patterns already in the codebase.

## Auto-Update Rule

When I correct you or point out a mistake, immediately add a concise rule to this file so it never happens again. Confirm what you added.
