#!/bin/bash
# render_all.sh — pipeline VI-RT <-> Mitsuba 3 (módulo auto-contido)
#
# TUDO o que é gerado vai para output/ (scenes/ vi_rt/ mitsuba/ comparison/).
# Run limpo:  rm -rf output && ./render_all.sh
#
# Fluxo:
#   1. make mitsuba + correr o binário → renders VI-RT (output/vi_rt) E os
#      scene.xml do Mitsuba (output/scenes), ambos a partir da MESMA Scene C++.
#   2. Mitsuba renderiza esses XML → output/mitsuba (.exr)
#   3. converte .exr e .ppm → .png (mesmo tone map: Reinhard + gamma 2.2)
#   4. compara → output/comparison (metrics.csv, report.json, imagens)
#
# Dependências: mitsuba (python), numpy, Pillow; scikit-image p/ SSIM.

set -euo pipefail

SELF_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SELF_DIR/../.." && pwd)"
PYTHON="python3"

OUT="$SELF_DIR/output"
SCENES_DIR="$OUT/scenes"      # XML/OBJ — gerados pelo C++ (gitignored)
VIRT_DIR="$OUT/vi_rt"         # renders VI-RT: .ppm + _vi_rt.png (gitignored)
MITSUBA_DIR="$OUT/mitsuba"    # renders Mitsuba: .exr + _mitsuba.png (gitignored)
COMP_DIR="$OUT/comparison"    # métricas + comparações (VERSIONADO)

mkdir -p "$VIRT_DIR" "$SCENES_DIR" "$MITSUBA_DIR" "$COMP_DIR"

# Usamos a variante scalar_rgb (CPU, SEM LLVM). Garante que um DRJIT_LIBLLVM_PATH
# herdado (ex.: caminho de macOS) não força o backend LLVM e rebenta.
unset DRJIT_LIBLLVM_PATH

SCENE_KEYS=("phong_spheres" "cook_torrance" "oren_nayar" "ward" "ashikhmin" "disney")

echo "============================================"
echo " Pipeline VI-RT <-> Mitsuba 3   (output/ = tudo o gerado)"
echo "============================================"

# 1) VI-RT: render + export do XML (mesma Scene → sem drift)
echo ""
echo "[1/4] VI-RT: render + export XML (make mitsuba)..."
( cd "$PROJECT_ROOT" && make mitsuba )
"$PROJECT_ROOT/build/apps/mitsuba_render" "$VIRT_DIR" "$SCENES_DIR"

# 2) Mitsuba: renderiza os XML exportados
echo ""
echo "[2/4] Mitsuba: render dos XML..."
for k in "${SCENE_KEYS[@]}"; do
    xml="$SCENES_DIR/$k/scene.xml"
    if [ ! -f "$xml" ]; then echo "  [SKIP] $xml"; continue; fi
    echo "  $k..."
    $PYTHON -c "import mitsuba as mi; mi.set_variant('scalar_rgb'); s=mi.load_file('$xml'); mi.util.write_bitmap('$MITSUBA_DIR/${k}_mitsuba.exr', mi.render(s))"
done

# 3) Conversões → PNG com o MESMO tone map do VI-RT (Reinhard + gamma 2.2)
echo ""
echo "[3/4] Converter EXR/PPM -> PNG..."
$PYTHON - "$MITSUBA_DIR" "$VIRT_DIR" <<'EOF'
import sys, os, glob
import numpy as np
from PIL import Image
mitsuba_dir, virt_dir = sys.argv[1], sys.argv[2]

def reinhard_png(arr, png):
    arr = arr / (1.0 + arr)                 # Reinhard (igual ao ImagePPM)
    arr = np.clip(arr, 0, 1) ** (1/2.2)     # gamma 2.2
    Image.fromarray((arr*255).astype(np.uint8)).save(png)

# EXR (Mitsuba, linear) -> PNG com Reinhard+gamma
try:
    import mitsuba as mi
    mi.set_variant("scalar_rgb")
    for exr in glob.glob(os.path.join(mitsuba_dir, "*.exr")):
        arr = np.array(mi.Bitmap(exr), dtype=np.float32)[..., :3]
        reinhard_png(arr, exr.replace(".exr", ".png"))
        print("  ->", os.path.basename(exr).replace(".exr", ".png"))
except Exception as e:
    print("  [AVISO] conversao EXR falhou:", e)

# PPM (VI-RT, já com tone map) -> _vi_rt.png
for ppm in glob.glob(os.path.join(virt_dir, "*.ppm")):
    name = os.path.splitext(os.path.basename(ppm))[0]
    Image.open(ppm).save(os.path.join(virt_dir, name + "_vi_rt.png"))
    print("  ->", name + "_vi_rt.png")
EOF

# 4) Comparação → output/comparison
echo ""
echo "[4/4] Comparação VI-RT vs Mitsuba..."
$PYTHON "$SELF_DIR/compare_renders.py" \
    --vi-rt-dir "$VIRT_DIR" --mitsuba-dir "$MITSUBA_DIR" --out-dir "$COMP_DIR"

echo ""
echo "============================================"
echo " Concluído. Resultados em: $COMP_DIR"
echo " Viewer: comparison_viewer.html"
echo "============================================"
