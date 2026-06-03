//
//  main.cpp
//  RMSE
//
//  Created by Luis Paulo Santos on 01/04/2025.
//  Extended with multi-metric comparison (RMSE per-channel, PSNR, MAE,
//  max-diff, SSIM, deltaE76, histogram) and markdown report.
//

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include "ImagePPM.hpp"
#include "Metrics.hpp"

static void error_message(void) {
    fprintf(stderr,
        "Utilização: rmse_exec <img-variante.ppm> <img-ref.ppm> [<gamma>] [<out-diff.ppm>] [--report=<file.md>] [--title=<str>]\n"
        "    Imagens têm de ter as mesmas dimensões.\n"
        "    gamma default = 0.5 (aplicado à imagem-diferença).\n"
        "    out default = RMSE.ppm\n"
        "    --report=<file.md>  gera relatório markdown.\n"
        "    --title=<str>       título usado no relatório.\n");
}

static bool starts_with(const std::string& s, const std::string& p) {
    return s.size() >= p.size() && std::memcmp(s.data(), p.data(), p.size()) == 0;
}

int main(int argc, const char* argv[]) {
    std::vector<std::string> pos;
    std::string report;
    std::string title;
    for (int i = 1; i < argc; ++i) {
        std::string a(argv[i]);
        if (starts_with(a, "--report=")) report = a.substr(9);
        else if (starts_with(a, "--title=")) title = a.substr(8);
        else pos.push_back(a);
    }

    if (pos.size() < 2) {
        error_message();
        return 1;
    }

    std::string img_in_fn  = pos[0];
    std::string img_ref_fn = pos[1];
    float gamma = (pos.size() >= 3) ? atof(pos[2].c_str()) : 0.5f;
    std::string img_out_fn = (pos.size() >= 4) ? pos[3] : "RMSE.ppm";

    ImagePPM img_in, img_ref;
    if (!img_in.Load(img_in_fn))  return 1;
    if (!img_ref.Load(img_ref_fn)) return 1;

    if (img_ref.W != img_in.W || img_in.H != img_ref.H) {
        fprintf(stderr, "As 2 imagens de entrada têm dimensões diferentes!\n");
        return 1;
    }

    ImagePPM diff(img_ref.W, img_ref.H);
    ImageMetrics m;
    compute_metrics(img_in, img_ref, diff, m);

    diff.MonoGammaCorrect(gamma);
    diff.Save(img_out_fn);

    if (title.empty()) {
        size_t slash = img_in_fn.find_last_of('/');
        size_t dot   = img_in_fn.find_last_of('.');
        size_t start = (slash == std::string::npos) ? 0 : slash + 1;
        size_t end   = (dot == std::string::npos || dot < start) ? img_in_fn.size() : dot;
        title = img_in_fn.substr(start, end - start);
    }

    print_metrics(m, img_in_fn, img_ref_fn, img_out_fn, gamma, report);

    if (!report.empty()) {
        write_markdown_report(report, m, title, img_in_fn, img_ref_fn, img_out_fn);
    }

    return 0;
}
