#ifndef Linear_hpp
#define Linear_hpp

#include "RGB.hpp"
#include <cmath>
#include <algorithm>

class Linear {
private:
    float exposure;
    float gamma;
    
    float clamp01(float value) {
        return std::max(0.0f, std::min(1.0f, value));
    }
    
    float gammaCorrect(float value, float gamma_val) {
        return powf(std::max(0.0f, value), 1.0f / gamma_val);
    }
    
public:
    Linear(float _exposure = 1.0f, float _gamma = 2.2f) 
        : exposure(_exposure), gamma(_gamma) {}
    
    void ToneMap(int const W, int const H, RGB *imageIn, RGB *imageOut) {
        for (int y = 0; y < H; y++) {
            int const row_off = y * W;
            for (int x = 0; x < W; x++) {
                int const offset = row_off + x;
                RGB Cin = imageIn[offset];
                
                // Apply exposure adjustment
                RGB exposed;
                exposed.R = Cin.R * exposure;
                exposed.G = Cin.G * exposure;
                exposed.B = Cin.B * exposure;
                
                // Simple clamp and gamma correction
                RGB result;
                result.R = gammaCorrect(clamp01(exposed.R), gamma);
                result.G = gammaCorrect(clamp01(exposed.G), gamma);
                result.B = gammaCorrect(clamp01(exposed.B), gamma);
                
                imageOut[offset] = result;
            }
        }
    }
    
    void setExposure(float exp) { exposure = exp; }
    void setGamma(float g) { gamma = g; }
};

#endif /* Linear_hpp */