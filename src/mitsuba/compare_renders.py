"""
compare_renders.py
Compara imagens VI-RT vs Mitsuba 3:
  - Gera imagens lado a lado
  - Calcula RMSE, SSIM, PSNR
  - Produz um CSV com métricas
  - Gera um relatório JSON para o viewer HTML

Uso: python3 compare_renders.py [--dir DIR] [--out-dir DIR]
"""

import os
import json
import math
import argparse
import glob
from pathlib import Path

try:
    import numpy as np
    from PIL import Image, ImageDraw, ImageFont
    HAS_PIL = True
except ImportError:
    print("[ERRO] Pillow e numpy são necessários: pip install Pillow numpy")
    raise

try:
    from skimage.metrics import structural_similarity as ssim
    HAS_SSIM = True
except ImportError:
    HAS_SSIM = False
    print("[AVISO] scikit-image não disponível (SSIM não calculado): pip install scikit-image")


# ---------------------------------------------------------------------------
# Métricas
# ---------------------------------------------------------------------------

def load_image_rgb(path: str) -> np.ndarray:
    """Carrega imagem como array float32 [H,W,3] no intervalo [0,1]."""
    img = Image.open(path).convert("RGB")
    return np.asarray(img, dtype=np.float32) / 255.0


def compute_rmse(a: np.ndarray, b: np.ndarray) -> float:
    diff = a.astype(np.float64) - b.astype(np.float64)
    return float(np.sqrt(np.mean(diff ** 2)))


def compute_psnr(a: np.ndarray, b: np.ndarray, max_val: float = 1.0) -> float:
    mse = np.mean((a.astype(np.float64) - b.astype(np.float64)) ** 2)
    if mse < 1e-12:
        return float("inf")
    return float(10 * math.log10(max_val ** 2 / mse))


def compute_ssim(a: np.ndarray, b: np.ndarray) -> float:
    if not HAS_SSIM:
        return float("nan")
    score = ssim(a, b, data_range=1.0, channel_axis=2)
    return float(score)


def compute_diff_image(a: np.ndarray, b: np.ndarray, amplify: float = 5.0) -> np.ndarray:
    """Mapa de diferença amplificado, em escala de cores."""
    diff = np.abs(a - b).mean(axis=2)          # [H,W] luminância do erro
    diff_amp = np.clip(diff * amplify, 0, 1)
    # Colormap: azul (baixo erro) -> vermelho (alto erro)
    r = diff_amp
    g = np.clip(1.0 - 2 * np.abs(diff_amp - 0.5), 0, 1) * 0.5
    b_ch = 1.0 - diff_amp
    rgb = np.stack([r, g, b_ch], axis=2)
    return (rgb * 255).astype(np.uint8)


# ---------------------------------------------------------------------------
# Composição lado a lado
# ---------------------------------------------------------------------------

LABEL_H   = 32    # altura do cabeçalho de texto
SEPARATOR = 4     # pixels de separador

def add_label(img_arr: np.ndarray, text: str, bg=(20,20,20), fg=(240,240,240)) -> np.ndarray:
    """Adiciona uma faixa de texto no topo da imagem."""
    h, w = img_arr.shape[:2]
    canvas = np.zeros((h + LABEL_H, w, 3), dtype=np.uint8)
    canvas[:LABEL_H, :] = bg
    canvas[LABEL_H:, :] = img_arr if img_arr.dtype == np.uint8 else (img_arr * 255).astype(np.uint8)
    pil = Image.fromarray(canvas)
    draw = ImageDraw.Draw(pil)
    try:
        font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 18)
    except Exception:
        font = ImageFont.load_default()
    draw.text((8, 6), text, fill=fg, font=font)
    return np.asarray(pil)


def compose_comparison(vi_rt: np.ndarray, mitsuba: np.ndarray,
                       scene_name: str, metrics: dict) -> np.ndarray:
    """Cria imagem com 3 painéis: VI-RT | Mitsuba | Diferença."""
    # Redimensionar para o mesmo tamanho (usa o menor)
    h = min(vi_rt.shape[0], mitsuba.shape[0])
    w = min(vi_rt.shape[1], mitsuba.shape[1])

    vi_arr = (np.array(Image.fromarray((vi_rt  * 255).astype(np.uint8)).resize((w, h))) / 255.0)
    mi_arr = (np.array(Image.fromarray((mitsuba* 255).astype(np.uint8)).resize((w, h))) / 255.0)

    diff_arr = compute_diff_image(vi_arr, mi_arr)

    label_vi = f"VI-RT  (RMSE {metrics['rmse']:.4f} | PSNR {metrics['psnr']:.1f} dB)"
    label_mi = "Mitsuba 3  (referência)"
    label_df = f"Diferença ×5  (SSIM {metrics['ssim']:.4f})" if not math.isnan(metrics['ssim']) else "Diferença ×5"

    panel_vi = add_label(vi_arr, label_vi)
    panel_mi = add_label(mi_arr, label_mi)
    panel_df = add_label(diff_arr, label_df)

    sep = np.zeros((panel_vi.shape[0], SEPARATOR, 3), dtype=np.uint8)
    composite = np.concatenate([panel_vi, sep, panel_mi, sep, panel_df], axis=1)

    # Cabeçalho da cena
    title_h = 40
    title_bar = np.full((title_h, composite.shape[1], 3), 10, dtype=np.uint8)
    full = np.concatenate([title_bar, composite], axis=0)
    pil = Image.fromarray(full)
    draw = ImageDraw.Draw(pil)
    try:
        font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 22)
    except Exception:
        font = ImageFont.load_default()
    draw.text((12, 9), f"Cena: {scene_name}", fill=(255, 220, 80), font=font)
    return np.asarray(pil)


# ---------------------------------------------------------------------------
# Descoberta de pares de imagens
# ---------------------------------------------------------------------------

def find_pairs(vi_rt_dir: str, mitsuba_dir: str):
    """
    Encontra pares (cena, vi_rt_path, mitsuba_path) cruzando dois directórios:
      VI-RT:   <vi_rt_dir>/<cena>_vi_rt.png
      Mitsuba: <mitsuba_dir>/<cena>_mitsuba.png
    """
    vi_rt_files = glob.glob(os.path.join(vi_rt_dir, "*_vi_rt.png"))
    pairs = []
    for vi_path in sorted(vi_rt_files):
        base = os.path.basename(vi_path).replace("_vi_rt.png", "")
        mi_path = os.path.join(mitsuba_dir, f"{base}_mitsuba.png")
        if os.path.exists(mi_path):
            pairs.append((base, vi_path, mi_path))
        else:
            print(f"  [SKIP] Sem Mitsuba render para '{base}' ({mi_path} não existe)")
    return pairs


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Compara renders VI-RT vs Mitsuba 3")
    parser.add_argument("--vi-rt-dir",   default="output/vi_rt",
                        help="Directório com os PNGs do VI-RT (<cena>_vi_rt.png)")
    parser.add_argument("--mitsuba-dir", default="output/mitsuba",
                        help="Directório com os PNGs do Mitsuba (<cena>_mitsuba.png)")
    parser.add_argument("--out-dir",     default="output/comparison",
                        help="Directório de saída para comparações e métricas")
    parser.add_argument("--amplify", type=float, default=5.0,
                        help="Factor de amplificação do mapa de diferença (default: 5)")
    args = parser.parse_args()

    os.makedirs(args.out_dir, exist_ok=True)

    pairs = find_pairs(args.vi_rt_dir, args.mitsuba_dir)
    if not pairs:
        print(f"[ERRO] Nenhum par encontrado ('{args.vi_rt_dir}' x '{args.mitsuba_dir}')")
        print("       Corre primeiro: ./render_all.sh")
        return

    print(f"[INFO] {len(pairs)} par(es) encontrado(s)")
    print()

    all_metrics = []
    csv_lines   = ["cena,rmse,psnr_db,ssim"]

    for scene_name, vi_path, mi_path in pairs:
        print(f"  Comparando: {scene_name}")

        vi_img = load_image_rgb(vi_path)
        mi_img = load_image_rgb(mi_path)

        # Redimensionar para o mesmo tamanho
        h = min(vi_img.shape[0], mi_img.shape[0])
        w = min(vi_img.shape[1], mi_img.shape[1])
        vi_r = np.array(Image.fromarray((vi_img*255).astype(np.uint8)).resize((w,h)), dtype=np.float32) / 255
        mi_r = np.array(Image.fromarray((mi_img*255).astype(np.uint8)).resize((w,h)), dtype=np.float32) / 255

        metrics = {
            "rmse": compute_rmse(vi_r, mi_r),
            "psnr": compute_psnr(vi_r, mi_r),
            "ssim": compute_ssim(vi_r, mi_r),
        }

        print(f"    RMSE = {metrics['rmse']:.5f}")
        print(f"    PSNR = {metrics['psnr']:.2f} dB")
        if not math.isnan(metrics['ssim']):
            print(f"    SSIM = {metrics['ssim']:.4f}")

        # Imagem de comparação lado a lado
        comp = compose_comparison(vi_img, mi_img, scene_name, metrics)
        comp_path = os.path.join(args.out_dir, f"{scene_name}_comparison.png")
        Image.fromarray(comp).save(comp_path)
        print(f"    -> {comp_path}")

        # Diferença isolada
        diff = compute_diff_image(vi_r, mi_r, amplify=args.amplify)
        diff_path = os.path.join(args.out_dir, f"{scene_name}_diff.png")
        Image.fromarray(diff).save(diff_path)

        all_metrics.append({
            "scene":        scene_name,
            "vi_rt_png":    os.path.relpath(vi_path,   args.out_dir),
            "mitsuba_png":  os.path.relpath(mi_path,   args.out_dir),
            "comparison":   os.path.relpath(comp_path, args.out_dir),
            "diff":         os.path.relpath(diff_path, args.out_dir),
            **metrics,
        })
        csv_lines.append(f"{scene_name},{metrics['rmse']:.6f},{metrics['psnr']:.4f},{metrics['ssim']:.6f}")
        print()

    # CSV
    csv_path = os.path.join(args.out_dir, "metrics.csv")
    with open(csv_path, "w") as f:
        f.write("\n".join(csv_lines))
    print(f"[OK] Métricas CSV: {csv_path}")

    # JSON para o viewer HTML
    report = {
        "title":   "VI-RT vs Mitsuba 3 — Comparação de Renders",
        "scenes":  all_metrics,
    }
    json_path = os.path.join(args.out_dir, "report.json")
    with open(json_path, "w") as f:
        json.dump(report, f, indent=2)
    print(f"[OK] Relatório JSON: {json_path}")
    print(f"\n     Abre o viewer: open comparison_viewer.html")


if __name__ == "__main__":
    main()