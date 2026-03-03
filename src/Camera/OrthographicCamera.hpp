#ifndef OrthographicCamera_hpp
#define OrthographicCamera_hpp

#include "camera.hpp"
#include "ray.hpp"
#include "vector.hpp"

class OrthographicCamera: public Camera {
private:
    Point Eye, At;           // Camera position and look-at point
    Vector Up;               // Up vector
    Vector forward, right;   // Camera coordinate system
    
    // Orthographic projection parameters
    float left, right_bound, bottom, top;  // Viewing volume bounds
    int W, H;                              // Resolution
    
    // Pixel mapping
    Point viewport_topleft;     // Top-left corner of viewport in world space
    Vector pixel_delta_u;       // Offset to pixel to the right
    Vector pixel_delta_v;       // Offset to pixel below

public:
    // Constructor with viewing volume bounds
    OrthographicCamera(const Point _Eye, const Point _At, const Vector _Up, 
                      const int _W, const int _H,
                      const float _left, const float _right, 
                      const float _bottom, const float _top);
    
    // Constructor with automatic bounds based on distance and aspect ratio
    OrthographicCamera(const Point _Eye, const Point _At, const Vector _Up, 
                      const int _W, const int _H, const float _width);
    
    bool GenerateRay(const int x, const int y, Ray *r, const float *cam_jitter=NULL) override;
    void getResolution(int *_W, int *_H) override {*_W=W; *_H=H;}
    
    // Utility methods
    void setBounds(float _left, float _right, float _bottom, float _top);
    void getBounds(float *_left, float *_right, float *_bottom, float *_top);
};

#endif /* OrthographicCamera_hpp */