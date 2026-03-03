#ifndef ReinhardAdvanced_hpp
#define ReinhardAdvanced_hpp

#include "RGB.hpp"
#include <cmath>
#include <algorithm>

class ReinhardAdvanced {
private:
    float white_point;
    float gamma;
    
    // Utility functions
    float clamp01(float value) {
        return std::max(0.0f, std::min(1.0f, value));
    }
    
    float gammaCorrect(float value, float gamma_val) {
        return powf(std::max(0.0f, value), 1.0f / gamma_val);
    }
    
public:
    ReinhardAdvanced(float _white_point = 1.0f, float _gamma = 2.2f) 
        : white_point(_white_point), gamma(_gamma) {}
    
    void ToneMap(int const W, int const H, RGB *imageIn, RGB *imageOut) {
        float white_sq = white_point * white_point;
        
        for (int y = 0; y < H; y++) {
            int const row_off = y * W;
            for (int x = 0; x < W; x++) {
                int const offset = row_off + x;
                RGB Cin = imageIn[offset];
                
                // Apply advanced Reinhard formula per channel
                RGB result;
                result.R = Cin.R * (1.0f + Cin.R / white_sq) / (1.0f + Cin.R);
                result.G = Cin.G * (1.0f + Cin.G / white_sq) / (1.0f + Cin.G);
                result.B = Cin.B * (1.0f + Cin.B / white_sq) / (1.0f + Cin.B);
                
                // Apply gamma correction
                result.R = gammaCorrect(clamp01(result.R), gamma);
                result.G = gammaCorrect(clamp01(result.G), gamma);
                result.B = gammaCorrect(clamp01(result.B), gamma);
                
                imageOut[offset] = result;
            }
        }
    }
    
    void setWhitePoint(float wp) { white_point = wp; }
    void setGamma(float g) { gamma = g; }
};

#endif /* ReinhardAdvanced_hpp */