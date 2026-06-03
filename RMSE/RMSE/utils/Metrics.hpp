#ifndef Metrics_hpp
#define Metrics_hpp

#include <string>
#include "ImagePPM.hpp"

struct ChannelMetrics {
    float rmse;
    float mae;
    float maxDiff;
};

struct ImageMetrics {
    int W, H;
    // Per-channel RGB
    ChannelMetrics R, G, B;
    // Luminance Y
    float rmseY;             // classical: sqrt(sum_SE / (W*H))
    float rmseY_legacy;      // legacy formula: sqrt(sum_SE) / (W*H)
    float minMaxScaledRMSEY; // legacy: normalized by reference Y range
    float maeY;
    float maxDiffY;
    float psnrY;             // dB; INFINITY if MSE == 0
    // Perceptual
    float ssimY;             // [-1, 1]; 1 = identical
    float deltaE_mean;       // CIE76 over all pixels
    float deltaE_max;
    // Reference Y stats
    float minY, maxY, averageY;
    // Histogram of |dY| in 10 bins.
    // Bins (intervals between edges): [0,1e-4), [1e-4,1e-3), [1e-3,1e-2),
    // [1e-2,1e-1), [1e-1,0.2), [0.2,0.3), [0.3,0.5), [0.5,0.7), [0.7,1.0),
    // [1.0,+inf) <- overflow
    int histogram[10];
};

void compute_metrics(ImagePPM& var, ImagePPM& ref,
                     ImagePPM& diff_out, ImageMetrics& m);

void print_metrics(const ImageMetrics& m,
                   const std::string& var_name,
                   const std::string& ref_name,
                   const std::string& diff_name,
                   float gamma,
                   const std::string& report_name);

void write_markdown_report(const std::string& path,
                           const ImageMetrics& m,
                           const std::string& title,
                           const std::string& var_path,
                           const std::string& ref_path,
                           const std::string& diff_path);

#endif /* Metrics_hpp */
