//
//  image.hpp
//  VI-RT
//
//  Created by Luis Paulo Santos on 10/02/2023.
//

#ifndef image_hpp
#define image_hpp

#include "RGB.hpp"
#include <string>
#include <cstring>
#include <algorithm> 

typedef struct {
    unsigned char val[3];  // r,g,b
} char_pixel;

class Image {
protected:
    RGB *imagePlane;
    char_pixel *img2save;
public:
    int W,H;
    Image(): W(0),H(0),imagePlane(NULL) {}
    Image(const int W, const int H): W(W),H(H) {
        imagePlane = new RGB[W*H];
        memset((void *)imagePlane, 0, W*H*sizeof(RGB));
    }
    ~Image() {
        if (imagePlane!=NULL) delete[] imagePlane;
    }

    // get() com clamp: nunca acede fora do array, devolve o pixel de borda
    // quando u=1.0 exacto ou por erros de arredondamento
    RGB get (int x, int y) const {
        if (W == 0 || H == 0) return RGB(0.f, 0.f, 0.f);
        x = std::max(0, std::min(x, W - 1));
        y = std::max(0, std::min(y, H - 1));
        return imagePlane[y * W + x];
    }

    bool set (int x, int y, const RGB &rgb) {
        if (x >= W || y >= H || x < 0 || y < 0) return false;
        imagePlane[y*W+x] = rgb;
        return true;
    }
    bool add (int x, int y, const RGB &rgb) {
        if (x >= W || y >= H || x < 0 || y < 0) return false;
        imagePlane[y*W+x] += rgb;
        return true;
    }
    bool divide (int x, int y, const float alpha) {
        if (x >= W || y >= H || x < 0 || y < 0) return false;
        imagePlane[y*W+x] /= alpha;
        return true;
    }
    virtual bool Save (std::string filename) {return true;}
    virtual bool Load (std::string filename) {return true;}
    
};

#endif /* image_hpp */
