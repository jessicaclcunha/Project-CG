#ifndef FisheyeCamera_hpp
#define FisheyeCamera_hpp

#include "camera.hpp"
#include "ray.hpp"
#include "vector.hpp"
#include <cmath>

class FisheyeCamera: public Camera {
private:
    Point Eye, At;              // Camera position and look-at point
    Vector Up;                  // Up vector
    Vector forward, right, up;  // Camera coordinate system (orthonormal)
    
    int W, H;                   // Resolution
    float fov_angle;            // Field of view in radians (typically π for 180°)
    float max_radius;           // Maximum radius in normalized coordinates [0,1]
    
    // Fisheye projection types
    enum ProjectionType {
        EQUIDISTANT,    // r = f * θ (linear angle mapping)
        EQUISOLID,      // r = 2f * sin(θ/2) (equal area)
        ORTHOGRAPHIC,   // r = f * sin(θ) (orthographic projection)
        STEREOGRAPHIC   // r = 2f * tan(θ/2) (stereographic projection)
    };
    
    ProjectionType projection;

public:
    // Constructor with field of view in degrees
    FisheyeCamera(const Point _Eye, const Point _At, const Vector _Up,
                 const int _W, const int _H, 
                 const float _fov_degrees = 180.0f,
                 const ProjectionType _projection = STEREOGRAPHIC);
    
    // Constructor with field of view in radians
    FisheyeCamera(const Point _Eye, const Point _At, const Vector _Up,
                 const int _W, const int _H, 
                 const float _fov_radians,
                 const bool _is_radians,
                 const ProjectionType _projection = STEREOGRAPHIC);
    
    bool GenerateRay(const int x, const int y, Ray *r, const float *cam_jitter=NULL) override;
    void getResolution(int *_W, int *_H) override {*_W=W; *_H=H;}
    
    // Utility methods
    void setFOV(float fov_degrees);
    void setFOVRadians(float fov_radians);
    float getFOV() const { return fov_angle * 180.0f / M_PI; }
    void setProjectionType(ProjectionType proj) { projection = proj; }
    
private:
    // Convert pixel coordinates to fisheye ray direction
    Vector pixelToDirection(float pixel_x, float pixel_y);
    
    // Different fisheye projection models
    float equidistantProjection(float theta);
    float equisolidProjection(float theta);
    float orthographicProjection(float theta);
    float stereographicProjection(float theta);
};

#endif /* FisheyeCamera_hpp */