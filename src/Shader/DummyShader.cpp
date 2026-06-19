//
//  DummyShader.cpp
//  VI-RT
//
//  Created by Luis Paulo Santos on 08/02/2025.
//

#include "DummyShader.hpp"

RGB DummyShader::shade(bool intersected, Intersection isect, int depth) {
    RGB color(((float) isect.pix_x) / W, ((float) isect.pix_y) / H, 0.);
    
    return color;
};
