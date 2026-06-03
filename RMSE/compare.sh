#!/bin/bash
# Comparar o render actual com o standard fixo.
#
# Uso: ./RMSE/compare.sh <titulo>
# Ex:  ./RMSE/compare.sh NoEC
#
# Assume:
#   build/apps/result/reference.ppm  → variante (render actual)
#   build/apps/result/standard.ppm   → referência fixa
#
# Output:
#   docs/comparisons/<titulo>.md       relatório markdown
#   docs/comparisons/diff_<titulo>.ppm imagem-diferença

set -e

if [ $# -lt 1 ]; then
    echo "Uso: $0 <titulo>"
    echo "Ex:  $0 NoEC"
    exit 1
fi

TITLE="$1"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
EXEC="$SCRIPT_DIR/rmse_exec"

VAR="$PROJECT_ROOT/build/apps/result/reference.ppm"
REF="$PROJECT_ROOT/build/apps/result/standard.ppm"

if [ ! -f "$VAR" ]; then
    echo "Não existe: $VAR" >&2
    echo "Renderiza a variante primeiro (make run)." >&2
    exit 1
fi
if [ ! -f "$REF" ]; then
    echo "Não existe: $REF" >&2
    echo "Gera o standard com:" >&2
    echo "  cp build/apps/result/reference.ppm build/apps/result/standard.ppm" >&2
    exit 1
fi

if [ ! -x "$EXEC" ]; then
    echo "Tool não compilada. Corre primeiro: cd RMSE && make" >&2
    exit 1
fi

# Pick Python with PIL for PPM -> PNG conversion (more reliable than ImageMagick)
PYBIN=""
if command -v python3 >/dev/null 2>&1 && python3 -c "from PIL import Image" 2>/dev/null; then
    PYBIN="python3"
elif command -v python >/dev/null 2>&1 && python -c "from PIL import Image" 2>/dev/null; then
    PYBIN="python"
else
    echo "Python com PIL (Pillow) não encontrado. Instala com:" >&2
    echo "  pip install pillow" >&2
    exit 1
fi

OUT_DIR="$PROJECT_ROOT/docs/comparisons"
mkdir -p "$OUT_DIR"

DIFF="$OUT_DIR/_${TITLE}.ppm"
DIFF_PNG="$OUT_DIR/_${TITLE}.png"
REPORT="$OUT_DIR/${TITLE}.md"

"$EXEC" "$VAR" "$REF" 0.5 "$DIFF" --report="$REPORT" --title="$TITLE"

"$PYBIN" -c "import sys; from PIL import Image; Image.open(sys.argv[1]).save(sys.argv[2])" "$DIFF" "$DIFF_PNG"

if [ ! -f "$DIFF_PNG" ]; then
    echo "Aviso: PNG não foi gerado. Verifica $PYBIN + PIL." >&2
else
    rm -f "$DIFF"
fi

echo ""
echo "Relatório: $REPORT"
echo "Imagem-diferença (PNG): $DIFF_PNG"
