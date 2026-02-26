//
//  Reinhard.hpp
//  VI-RT-V4-PathTracing
//
//  Created by Luis Paulo Santos on 26/03/2025.
//
#ifndef ReinhardBasic_hpp
#define ReinhardBasic_hpp

#include "RGB.hpp"
    
class ReinhardBasic {
public:
    ReinhardBasic() {}
    
    void ToneMap(int const W, int const H, RGB *imageIn, RGB *imageOut) {
        for (int y = 0; y < H; y++) {
            int const row_off = y * W;
            for (int x = 0; x < W; x++) {
                int const offset = row_off + x;
                RGB Cin = imageIn[offset];
                float Lin = Cin.Y();  // Get luminance
                RGB const Cout = Cin / (1.0f + Lin);  // Basic Reinhard formula
                imageOut[offset] = Cout;
            }
        }
    }
};

#endif /* ReinhardBasic_hpp */
