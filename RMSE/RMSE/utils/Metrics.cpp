#include "Metrics.hpp"
#include "ColorConvert.hpp"
#include "RGB.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <limits>

static int histogram_bin(float v) {
    static const float edges[10] = {
        0.f, 1e-4f, 1e-3f, 1e-2f, 1e-1f, 0.2f, 0.3f, 0.5f, 0.7f, 1.0f
    };
    if (v >= edges[9]) return 9; // overflow
    for (int i = 8; i >= 0; --i) {
        if (v >= edges[i]) return i;
    }
    return 0;
}

static float compute_ssim_tile(ImagePPM& var, ImagePPM& ref,
                               int x0, int y0, int tile) {
    const int W = ref.W;
    const int H = ref.H;
    float n = 0.f;
    float sumX = 0.f, sumY = 0.f;
    float sumXX = 0.f, sumYY = 0.f, sumXY = 0.f;
    for (int dy = 0; dy < tile; ++dy) {
        int y = y0 + dy;
        if (y >= H) break;
        for (int dx = 0; dx < tile; ++dx) {
            int x = x0 + dx;
            if (x >= W) break;
            float yref = ref.get(x, y).Y();
            float yvar = var.get(x, y).Y();
            sumX += yvar;
            sumY += yref;
            sumXX += yvar * yvar;
            sumYY += yref * yref;
            sumXY += yvar * yref;
            n += 1.f;
        }
    }
    if (n < 1.f) return 1.f;
    float muX = sumX / n;
    float muY = sumY / n;
    float varX = sumXX / n - muX * muX;
    float varY = sumYY / n - muY * muY;
    float covXY = sumXY / n - muX * muY;
    const float c1 = 0.01f * 0.01f;
    const float c2 = 0.03f * 0.03f;
    float num = (2.f * muX * muY + c1) * (2.f * covXY + c2);
    float den = (muX * muX + muY * muY + c1) * (varX + varY + c2);
    return (den > 0.f) ? num / den : 1.f;
}

void compute_metrics(ImagePPM& var, ImagePPM& ref,
                     ImagePPM& diff_out, ImageMetrics& m) {
    const int W = ref.W;
    const int H = ref.H;
    const float Npix = float(W * H);

    m.W = W;
    m.H = H;
    std::memset(m.histogram, 0, sizeof(m.histogram));

    // First pass: Y stats of reference
    m.averageY = 0.f;
    m.minY = m.maxY = ref.get(0, 0).Y();
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            float Y = ref.get(x, y).Y();
            m.averageY += Y;
            if (Y > m.maxY) m.maxY = Y;
            if (Y < m.minY) m.minY = Y;
        }
    }
    m.averageY /= Npix;
    float rangeY = m.maxY - m.minY;
    if (rangeY < 1e-9f) rangeY = 1.f; // avoid div by zero

    // Second pass: per-pixel errors + diff_out + Lab/deltaE + histogram
    double sumSE_R = 0.0, sumSE_G = 0.0, sumSE_B = 0.0, sumSE_Y = 0.0;
    double sumAE_R = 0.0, sumAE_G = 0.0, sumAE_B = 0.0, sumAE_Y = 0.0;
    float maxR = 0.f, maxG = 0.f, maxB = 0.f, maxY = 0.f;
    double sumMinMaxSE = 0.0;
    double sumDeltaE = 0.0;
    float maxDeltaE = 0.f;

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            RGB pref = ref.get(x, y);
            RGB pvar = var.get(x, y);

            float dR = pvar.R - pref.R;
            float dG = pvar.G - pref.G;
            float dB = pvar.B - pref.B;
            float aR = std::fabs(dR), aG = std::fabs(dG), aB = std::fabs(dB);

            sumSE_R += dR * dR;
            sumSE_G += dG * dG;
            sumSE_B += dB * dB;
            sumAE_R += aR;
            sumAE_G += aG;
            sumAE_B += aB;
            if (aR > maxR) maxR = aR;
            if (aG > maxG) maxG = aG;
            if (aB > maxB) maxB = aB;

            float Yref = pref.Y();
            float Yvar = pvar.Y();
            float dY = Yvar - Yref;
            float aY = std::fabs(dY);
            sumSE_Y += dY * dY;
            sumAE_Y += aY;
            if (aY > maxY) maxY = aY;

            // diff image (matches legacy: SE per pixel, stored monochrome)
            float SE = dY * dY;
            diff_out.set(x, y, RGB(SE, SE, SE));

            // min-max scaled SE on Y
            float mmYref = (Yref - m.minY) / rangeY;
            float mmYvar = (Yvar - m.minY) / rangeY;
            float mmd = mmYvar - mmYref;
            sumMinMaxSE += mmd * mmd;

            // histogram on |dY|
            m.histogram[histogram_bin(aY)] += 1;

            // deltaE
            float L1, a1, b1, L2, a2, b2;
            srgb_to_lab(pref.R, pref.G, pref.B, L1, a1, b1);
            srgb_to_lab(pvar.R, pvar.G, pvar.B, L2, a2, b2);
            float de = deltaE76(L1, a1, b1, L2, a2, b2);
            sumDeltaE += de;
            if (de > maxDeltaE) maxDeltaE = de;
        }
    }

    // Per-channel metrics
    m.R.rmse = std::sqrt(float(sumSE_R / Npix));
    m.G.rmse = std::sqrt(float(sumSE_G / Npix));
    m.B.rmse = std::sqrt(float(sumSE_B / Npix));
    m.R.mae  = float(sumAE_R / Npix);
    m.G.mae  = float(sumAE_G / Npix);
    m.B.mae  = float(sumAE_B / Npix);
    m.R.maxDiff = maxR;
    m.G.maxDiff = maxG;
    m.B.maxDiff = maxB;

    // Luminance metrics
    m.rmseY         = std::sqrt(float(sumSE_Y / Npix));
    m.rmseY_legacy  = std::sqrt(float(sumSE_Y)) / Npix;
    m.minMaxScaledRMSEY = std::sqrt(float(sumMinMaxSE)) / Npix;
    m.maeY          = float(sumAE_Y / Npix);
    m.maxDiffY      = maxY;

    // PSNR Y (MAX = 1.0 since PPM normalised to [0,1])
    float mseY = float(sumSE_Y / Npix);
    if (mseY <= 0.f) m.psnrY = std::numeric_limits<float>::infinity();
    else m.psnrY = 10.f * std::log10(1.f / mseY);

    // DeltaE
    m.deltaE_mean = float(sumDeltaE / Npix);
    m.deltaE_max  = maxDeltaE;

    // SSIM via non-overlapping 8x8 tiles
    const int tile = 8;
    double sumSSIM = 0.0;
    int nTiles = 0;
    for (int y0 = 0; y0 < H; y0 += tile) {
        for (int x0 = 0; x0 < W; x0 += tile) {
            sumSSIM += compute_ssim_tile(var, ref, x0, y0, tile);
            ++nTiles;
        }
    }
    m.ssimY = (nTiles > 0) ? float(sumSSIM / nTiles) : 1.f;
}

static const char* deltaE_qualifier(float de) {
    if (de < 1.f) return "imperceptível";
    if (de < 2.f) return "ligeiro";
    if (de < 10.f) return "perceptível";
    return "muito diferente";
}

static const char* psnr_qualifier(float psnr) {
    if (!std::isfinite(psnr)) return "idêntico";
    if (psnr > 40.f) return "quase idêntico";
    if (psnr > 30.f) return "diferença ligeira";
    if (psnr > 20.f) return "diferença visível";
    return "muito diferente";
}

void print_metrics(const ImageMetrics& m,
                   const std::string& var_name,
                   const std::string& ref_name,
                   const std::string& diff_name,
                   float gamma,
                   const std::string& report_name) {
    printf("========================================\n");
    printf("Comparação: %s vs %s\n", var_name.c_str(), ref_name.c_str());
    printf("========================================\n");
    printf("Resolução: %d×%d (%d píxeis)\n\n", m.W, m.H, m.W * m.H);

    printf("Referência (%s):\n", ref_name.c_str());
    printf("  Luminância Y: min=%.4f, max=%.4f, mean=%.4f\n\n",
           m.minY, m.maxY, m.averageY);

    printf("Métricas de erro:\n");
    printf("                   RMSE        MAE         Max\n");
    printf("  Canal R         %-10.6f  %-10.6f  %-10.6f\n",
           m.R.rmse, m.R.mae, m.R.maxDiff);
    printf("  Canal G         %-10.6f  %-10.6f  %-10.6f\n",
           m.G.rmse, m.G.mae, m.G.maxDiff);
    printf("  Canal B         %-10.6f  %-10.6f  %-10.6f\n",
           m.B.rmse, m.B.mae, m.B.maxDiff);
    printf("  Luminância Y    %-10.6f  %-10.6f  %-10.6f\n",
           m.rmseY, m.maeY, m.maxDiffY);
    printf("  RMSE legacy Y   %-10.8f (fórmula antiga)\n", m.rmseY_legacy);
    printf("  MinMaxScaled Y  %-10.8f (legacy)\n", m.minMaxScaledRMSEY);
    if (std::isfinite(m.psnrY))
        printf("  PSNR Y          %-7.2f dB (%s)\n", m.psnrY, psnr_qualifier(m.psnrY));
    else
        printf("  PSNR Y          inf      (imagens idênticas)\n");
    printf("\n");

    printf("Métricas perceptuais:\n");
    printf("  SSIM Y          %-7.4f (1.0 = idêntico)\n", m.ssimY);
    printf("  ΔE76 médio      %-7.2f (%s)\n", m.deltaE_mean, deltaE_qualifier(m.deltaE_mean));
    printf("  ΔE76 máx        %-7.2f\n\n", m.deltaE_max);

    // Histogram with ASCII bars
    static const char* labels[10] = {
        "[0,        1e-4)",
        "[1e-4,    1e-3)",
        "[1e-3,    1e-2)",
        "[1e-2,    1e-1)",
        "[1e-1,    0.2 )",
        "[0.2,     0.3 )",
        "[0.3,     0.5 )",
        "[0.5,     0.7 )",
        "[0.7,     1.0 )",
        "[1.0,     inf )"
    };
    int maxCount = 1;
    for (int i = 0; i < 10; ++i) if (m.histogram[i] > maxCount) maxCount = m.histogram[i];
    printf("Histograma |ΔY|:\n");
    for (int i = 0; i < 10; ++i) {
        int barLen = (m.histogram[i] * 30) / maxCount;
        printf("  %-16s %8d  ", labels[i], m.histogram[i]);
        for (int b = 0; b < barLen; ++b) printf("#");
        printf("\n");
    }
    printf("\n");
    printf("Imagem-diferença: %s (gamma=%.2f)\n", diff_name.c_str(), gamma);
    if (!report_name.empty())
        printf("Relatório markdown: %s\n", report_name.c_str());
    printf("========================================\n");
}

static std::string today_iso() {
    time_t t = time(nullptr);
    struct tm* tm_ = localtime(&t);
    char buf[16];
    strftime(buf, sizeof(buf), "%Y-%m-%d", tm_);
    return std::string(buf);
}

void write_markdown_report(const std::string& path,
                           const ImageMetrics& m,
                           const std::string& title,
                           const std::string& var_path,
                           const std::string& ref_path,
                           const std::string& diff_path) {
    std::ofstream f(path);
    if (!f.is_open()) {
        fprintf(stderr, "Não foi possível abrir %s para escrita\n", path.c_str());
        return;
    }

    f << "# Comparação: " << title << "\n\n";
    f << "- **Variante:** `" << var_path << "`\n";
    f << "- **Referência:** `" << ref_path << "`\n";
    f << "- **Data:** " << today_iso() << "\n";
    f << "- **Resolução:** " << m.W << "×" << m.H << "\n\n";

    f << "## Métricas per-canal\n\n";
    f << "| Métrica | R | G | B | Y |\n";
    f << "|---|---|---|---|---|\n";
    char row[256];
    snprintf(row, sizeof(row), "| RMSE | %.6f | %.6f | %.6f | %.6f |\n",
             m.R.rmse, m.G.rmse, m.B.rmse, m.rmseY);
    f << row;
    snprintf(row, sizeof(row), "| MAE  | %.6f | %.6f | %.6f | %.6f |\n",
             m.R.mae, m.G.mae, m.B.mae, m.maeY);
    f << row;
    snprintf(row, sizeof(row), "| Max  | %.6f | %.6f | %.6f | %.6f |\n\n",
             m.R.maxDiff, m.G.maxDiff, m.B.maxDiff, m.maxDiffY);
    f << row;

    f << "## Métricas adicionais\n\n";
    f << "| Métrica | Valor |\n";
    f << "|---|---|\n";
    snprintf(row, sizeof(row), "| RMSE legacy Y (fórmula antiga) | %.8f |\n", m.rmseY_legacy);
    f << row;
    snprintf(row, sizeof(row), "| MinMaxScaled RMSE Y | %.8f |\n", m.minMaxScaledRMSEY);
    f << row;
    if (std::isfinite(m.psnrY))
        snprintf(row, sizeof(row), "| PSNR Y | %.2f dB (%s) |\n", m.psnrY, psnr_qualifier(m.psnrY));
    else
        snprintf(row, sizeof(row), "| PSNR Y | ∞ (imagens idênticas) |\n");
    f << row;
    snprintf(row, sizeof(row), "| SSIM Y | %.4f |\n", m.ssimY);
    f << row;
    snprintf(row, sizeof(row), "| ΔE CIE76 médio | %.2f (%s) |\n", m.deltaE_mean, deltaE_qualifier(m.deltaE_mean));
    f << row;
    snprintf(row, sizeof(row), "| ΔE CIE76 máx | %.2f |\n\n", m.deltaE_max);
    f << row;

    f << "## Referência — estatísticas de luminância\n\n";
    f << "| | Valor |\n|---|---|\n";
    snprintf(row, sizeof(row), "| Y min | %.4f |\n", m.minY); f << row;
    snprintf(row, sizeof(row), "| Y max | %.4f |\n", m.maxY); f << row;
    snprintf(row, sizeof(row), "| Y mean | %.4f |\n\n", m.averageY); f << row;

    f << "## Histograma de \\|ΔY\\|\n\n";
    static const char* labels[10] = {
        "[0, 1e-4)",
        "[1e-4, 1e-3)",
        "[1e-3, 1e-2)",
        "[1e-2, 1e-1)",
        "[1e-1, 0.2)",
        "[0.2, 0.3)",
        "[0.3, 0.5)",
        "[0.5, 0.7)",
        "[0.7, 1.0)",
        "[1.0, ∞) overflow"
    };
    f << "| Intervalo | Píxeis |\n|---|---|\n";
    for (int i = 0; i < 10; ++i) {
        snprintf(row, sizeof(row), "| %s | %d |\n", labels[i], m.histogram[i]);
        f << row;
    }
    f << "\n";

    // image-diff path relative to the report (PNG; compare.sh converts PPM -> PNG)
    std::string diff_rel = diff_path;
    size_t slash = diff_path.find_last_of('/');
    if (slash != std::string::npos) diff_rel = diff_path.substr(slash + 1);
    size_t dot = diff_rel.find_last_of('.');
    if (dot != std::string::npos) diff_rel = diff_rel.substr(0, dot) + ".png";

    f << "## Imagem-diferença\n\n";
    f << "![diff](" << diff_rel << ")\n";

    f.close();
}
