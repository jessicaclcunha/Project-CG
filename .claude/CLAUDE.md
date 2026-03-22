# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

```bash
make          # Build the project (output: ./build/apps/src)
make run      # Build, render, and display the result
make display  # Display a previously rendered image (skips rendering)
make clean    # Remove build artifacts

# RMSE comparison tool (in RMSE/ subdirectory)
cd RMSE && make
```

The renderer outputs a PPM file to `result/reference.ppm`. The Makefile uses `eog` to display it.

## Current Render Settings (main.cpp)

| Setting | Value |
|---|---|
| Resolution | 640×640 |
| Scene | `CookTorranceJustOneThing` |
| Shader | `DistributedShader` |
| SPP | 128 |
| Jitter | enabled |
| Camera | Perspective, FOV 60°, pos (0,0,−5) |

## Project Purpose

Academic ray tracing project focused on implementing multiple BRDFs, analysing how each responds to parameter changes, and producing comparative renders. Two-member team, deadline June 22.

**Current phase:** Improving CookTorrance to compute F0 from metallic + albedo (PBR workflow) instead of taking F0 as a manual parameter. Also adding box geometry (triangulated cube) and new scenes.

## Further Reading

- [architecture.md](architecture.md) — rendering pipeline, key types, lighting, acceleration
- [brdf-roadmap.md](brdf-roadmap.md) — BRDF implementation status and schedule
- [rules/brdf-implementation.md](rules/brdf-implementation.md) — BRDF coding rules and workflow
