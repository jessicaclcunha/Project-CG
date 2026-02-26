#ifndef ACES_hpp
#define ACES_hpp

#include "RGB.hpp"
#include <cmath>
#include <algorithm>

class ACES {
private:
    float exposure;
    float gamma;
    
    float clamp01(float value) {
        return std::max(0.0f, std::min(1.0f, value));
    }
    
    float gammaCorrect(float value, float gamma_val) {
        return powf(std::max(0.0f, value), 1.0f / gamma_val);
    }
    
    float acesCurve(float x) {
        const float a = 2.51f;
        const float b = 0.03f;
        const float c = 2.43f;
        const float d = 0.59f;
        const float e = 0.14f;
        
        return clamp01((x * (a * x + b)) / (x * (c * x + d) + e));
    }
    
public:
    ACES(float _exposure = 1.0f, float _gamma = 2.2f) 
        : exposure(_exposure), gamma(_gamma) {}
    
    void ToneMap(int const W, int const H, RGB *imageIn, RGB *imageOut) {
        for (int y = 0; y < H; y++) {
            int const row_off = y * W;
            for (int x = 0; x < W; x++) {
                int const offset = row_off + x;
                RGB Cin = imageIn[offset];
                
                // Apply exposure
                RGB exposed;
                exposed.R = Cin.R * exposure;
                exposed.G = Cin.G * exposure;
                exposed.B = Cin.B * exposure;
                
                // Apply ACES tone mapping curve
                RGB result;
                result.R = acesCurve(exposed.R);
                result.G = acesCurve(exposed.G);
                result.B = acesCurve(exposed.B);
                
                // Apply gamma correction
                result.R = gammaCorrect(result.R, gamma);
                result.G = gammaCorrect(result.G, gamma);
                result.B = gammaCorrect(result.B, gamma);
                
                imageOut[offset] = result;
            }
        }
    }
    
    void setExposure(float exp) { exposure = exp; }
    void setGamma(float g) { gamma = g; }
};

#endif /* ACES_hpp */