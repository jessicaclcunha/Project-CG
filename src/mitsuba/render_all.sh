#!/bin/bash
# render_all.sh
# Renderiza todas as cenas Mitsuba 3 e converte os .exr para .png
# Uso: ./render_all.sh [--scenes-dir DIR] [--vi-rt-results DIR] [--output DIR]
#
# Dependências: mitsuba, Python 3 com Pillow e numpy (pip install Pillow numpy)

set -euo pipefail

SCENES_DIR="mitsuba_scenes"
VIRI_RESULTS="build/apps/src/result"   # onde o VI-RT guarda os PPMs
OUTPUT_DIR="comparison_renders"
PYTHON="python3"

export DRJIT_LIBLLVM_PATH="/opt/homebrew/opt/llvm/lib/libLLVM.dylib"

# Parse de argumentos simples
while [[ $# -gt 0 ]]; do
    case $1 in
        --scenes-dir)   SCENES_DIR="$2";    shift 2 ;;
        --vi-rt-results) VIRI_RESULTS="$2"; shift 2 ;;
        --output)       OUTPUT_DIR="$2";    shift 2 ;;
        *) echo "Argumento desconhecido: $1"; exit 1 ;;
    esac
done

mkdir -p "$OUTPUT_DIR"

echo "============================================"
echo " Pipeline VI-RT <-> Mitsuba 3"
echo "============================================"
echo " Cenas:      $SCENES_DIR"
echo " VI-RT PPMs: $VIRI_RESULTS"
echo " Saída:      $OUTPUT_DIR"
echo ""

# ---------------------------------------------------------------------------
# 1. Gerar XMLs (se ainda não existirem)
# ---------------------------------------------------------------------------
if [ ! -d "$SCENES_DIR" ]; then
    echo "[1/4] Gerando cenas Mitsuba..."
    $PYTHON generate_mitsuba_scenes.py --output-dir "$SCENES_DIR"
else
    echo "[1/4] Cenas já existem em $SCENES_DIR (apaga a pasta para regenerar)"
fi

# ---------------------------------------------------------------------------
# 2. Renderizar com Mitsuba 3 (Via Python para evitar erro do LLVM)
# ---------------------------------------------------------------------------
echo ""
echo "[2/4] Renderizando com Mitsuba 3..."

SCENE_KEYS=("phong_spheres" "cook_torrance" "oren_nayar" "ward" "ashikhmin" "disney")

for scene_key in "${SCENE_KEYS[@]}"; do
    xml="$SCENES_DIR/$scene_key/scene.xml"
    exr_out="$OUTPUT_DIR/${scene_key}_mitsuba.exr"

    if [ ! -f "$xml" ]; then
        echo "  [SKIP] $xml não existe"
        continue
    fi

    echo "  Renderizando $scene_key..."
    # Usa o Python diretamente para forçar o uso de scalar_rgb (sem LLVM)
    $PYTHON -c "import mitsuba as mi; mi.set_variant('scalar_rgb'); scene = mi.load_file('$xml'); img = mi.render(scene); mi.util.write_bitmap('$exr_out', img)"
done

# ---------------------------------------------------------------------------
# 3. Converter EXR -> PNG com tone mapping
# ---------------------------------------------------------------------------
echo ""
echo "[3/4] Convertendo EXR -> PNG..."

$PYTHON - <<'EOF'
import os, sys, glob

try:
    import numpy as np
    from PIL import Image
    import OpenEXR, Imath
except ImportError:
    # Fallback: tentar com mitsuba Python API
    try:
        import mitsuba as mi
        mi.set_variant("scalar_rgb")
        USE_MI = True
    except ImportError:
        print("  [AVISO] Nem OpenEXR nem mitsuba Python disponíveis.")
        print("          Instala: pip install openexr mitsuba")
        sys.exit(0)
    USE_MI = False
    USE_MI = True
else:
    USE_MI = False

output_dir = "comparison_renders"

for exr_path in glob.glob(os.path.join(output_dir, "*.exr")):
    png_path = exr_path.replace(".exr", ".png")
    scene_name = os.path.basename(exr_path).replace("_mitsuba.exr", "")

    try:
        if USE_MI:
            import mitsuba as mi
            mi.set_variant("scalar_rgb")
            bmp = mi.Bitmap(exr_path)
            bmp = bmp.convert(mi.Bitmap.PixelFormat.RGB, mi.Struct.Type.UInt8, True)
            bmp.write(png_path)
        else:
            # OpenEXR + Pillow
            exr = OpenEXR.InputFile(exr_path)
            dw  = exr.header()["dataWindow"]
            w   = dw.max.x - dw.min.x + 1
            h   = dw.max.y - dw.min.y + 1
            pt  = Imath.PixelType(Imath.PixelType.FLOAT)
            r = np.frombuffer(exr.channel("R", pt), dtype=np.float32).reshape(h, w)
            g = np.frombuffer(exr.channel("G", pt), dtype=np.float32).reshape(h, w)
            b = np.frombuffer(exr.channel("B", pt), dtype=np.float32).reshape(h, w)
            # Reinhard tone mapping + gamma 2.2
            img = np.stack([r, g, b], axis=-1)
            img = img / (1.0 + img)
            img = np.clip(img, 0, 1) ** (1/2.2)
            img = (img * 255).astype(np.uint8)
            Image.fromarray(img).save(png_path)

        print(f"  -> {png_path}")
    except Exception as e:
        print(f"  [ERRO] {exr_path}: {e}")
EOF

# ---------------------------------------------------------------------------
# 4. Converter PPMs do VI-RT -> PNG
# ---------------------------------------------------------------------------
echo ""
echo "[4/4] Convertendo PPMs do VI-RT -> PNG..."

$PYTHON - <<EOF
import os, glob

try:
    from PIL import Image
except ImportError:
    print("  [AVISO] Pillow não instalado: pip install Pillow")
    exit()

vi_rt_results = "$VIRI_RESULTS"
output_dir    = "$OUTPUT_DIR"

ppms = glob.glob(os.path.join(vi_rt_results, "*.ppm"))
if not ppms:
    print(f"  [AVISO] Nenhum PPM encontrado em {vi_rt_results}")
    print("          Corre o VI-RT primeiro para gerar imagens de resultado.")
else:
    for ppm in ppms:
        name = os.path.splitext(os.path.basename(ppm))[0]
        out  = os.path.join(output_dir, f"{name}_vi_rt.png")
        try:
            Image.open(ppm).save(out)
            print(f"  -> {out}")
        except Exception as e:
            print(f"  [ERRO] {ppm}: {e}")
EOF

echo ""
echo "============================================"
echo " Concluído! Ficheiros em: $OUTPUT_DIR"
echo " Próximo passo: python3 compare_renders.py"
echo "============================================"