#include "OrthographicCamera.hpp"

// Constructor with explicit bounds
OrthographicCamera::OrthographicCamera(const Point _Eye, const Point _At, const Vector _Up, const int _W, const int _H, const float _left, const float _right, const float _bottom, const float _top)
    : Eye(_Eye), At(_At), Up(_Up), W(_W), H(_H), left(_left), right_bound(_right), bottom(_bottom), top(_top) {
    
    // Compute camera coordinate system
    forward = Vector(At.X - Eye.X, At.Y - Eye.Y, At.Z - Eye.Z);
    forward.normalize();
    
    right = forward.cross(Up);
    right.normalize();
    
    // Recompute UP as exact cross product right × forward
    Up = right.cross(forward);
    Up.normalize();
    
    // Calculate viewport dimensions in world space
    float viewport_width = right_bound - left;
    float viewport_height = top - bottom;
    
    // Calculate pixel deltas
    pixel_delta_u = (viewport_width / W) * right;
    pixel_delta_v = (viewport_height / H) * (-1.0f * Up);  // negative because screen Y goes down
    
    // Calculate top-left corner of viewport
    Vector viewport_center_offset = (left + viewport_width/2) * right + (bottom + viewport_height/2) * Up;
    Point viewport_center = Eye + viewport_center_offset;
    viewport_topleft = viewport_center - (viewport_width/2) * right - (viewport_height/2) * Up;
}

// Constructor with automatic bounds
OrthographicCamera::OrthographicCamera(const Point _Eye, const Point _At, const Vector _Up,
                                     const int _W, const int _H, const float _width)
    : Eye(_Eye), At(_At), Up(_Up), W(_W), H(_H) {
    
    // Compute camera coordinate system
    forward = Vector(At.X - Eye.X, At.Y - Eye.Y, At.Z - Eye.Z);
    forward.normalize();
    
    right = forward.cross(Up);
    right.normalize();
    
    // Recompute UP as exact cross product
    Up = right.cross(forward);
    Up.normalize();
    
    // Calculate bounds based on width and aspect ratio
    float aspect_ratio = (float)W / (float)H;
    float half_width = _width / 2.0f;
    float half_height = half_width / aspect_ratio;
    
    left = -half_width;
    right_bound = half_width;
    bottom = -half_height;
    top = half_height;
    
    // Calculate pixel deltas
    pixel_delta_u = (_width / W) * right;
    pixel_delta_v = (2.0f * half_height / H) * (-1.0f * Up);
    
    // Calculate top-left corner of viewport
    viewport_topleft = Eye + left * right + top * Up;
}

bool OrthographicCamera::GenerateRay(const int x, const int y, Ray *r, const float *cam_jitter) {
    Point pixel_coord;
    
    // Calculate pixel coordinates with or without jittering
    if (cam_jitter == NULL) {
        pixel_coord.X = (float)x + 0.5f;
        pixel_coord.Y = (float)y + 0.5f;
    } else {
        // Jittering for anti-aliasing
        pixel_coord.X = (float)x + cam_jitter[0];
        pixel_coord.Y = (float)y + cam_jitter[1];
    }
    
    // Calculate world position for this pixel
    Point world_position = viewport_topleft + 
                          pixel_coord.X * pixel_delta_u + 
                          pixel_coord.Y * pixel_delta_v;
    
    // For orthographic projection:
    // - Ray origin varies with pixel position
    // - Ray direction is always the forward direction
    r->o = world_position;
    r->dir = forward;  // All rays are parallel
    
    // Set ray metadata
    r->pix_x = x;
    r->pix_y = y;
    r->FaceID = -1;
    r->propagating_eta = 1.0f;
    r->rtype = PRIMARY;
    
    return true;
}

void OrthographicCamera::setBounds(float _left, float _right, float _bottom, float _top) {
    left = _left;
    right_bound = _right;
    bottom = _bottom;
    top = _top;
    
    // Recalculate viewport parameters
    float viewport_width = right_bound - left;
    float viewport_height = top - bottom;
    
    pixel_delta_u = (viewport_width / W) * right;
    pixel_delta_v = (viewport_height / H) * (-1.0f * Up);
    
    Vector viewport_center_offset = (left + viewport_width/2) * right + (bottom + viewport_height/2) * Up;
    Point viewport_center = Eye + viewport_center_offset;
    viewport_topleft = viewport_center - (viewport_width/2) * right - (viewport_height/2) * Up;
}

void OrthographicCamera::getBounds(float *_left, float *_right, float *_bottom, float *_top) {
    *_left = left;
    *_right = right_bound;
    *_bottom = bottom;
    *_top = top;
}