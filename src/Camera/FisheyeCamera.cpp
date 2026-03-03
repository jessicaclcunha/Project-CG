#include "FisheyeCamera.hpp"
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Constructor with FOV in degrees
FisheyeCamera::FisheyeCamera(const Point _Eye, const Point _At, const Vector _Up,
                           const int _W, const int _H,
                           const float _fov_degrees,
                           const ProjectionType _projection)
    : Eye(_Eye), At(_At), Up(_Up), W(_W), H(_H), projection(_projection) {
    
    // Convert FOV to radians
    fov_angle = _fov_degrees * M_PI / 180.0f;
    
    // Compute camera coordinate system
    forward = Vector(At.X - Eye.X, At.Y - Eye.Y, At.Z - Eye.Z);
    forward.normalize();
    
    right = forward.cross(Up);
    right.normalize();
    
    // Recompute up as exact cross product
    up = right.cross(forward);
    up.normalize();
    
    max_radius = 1.0f;
}

// Constructor with FOV in radians
FisheyeCamera::FisheyeCamera(const Point _Eye, const Point _At, const Vector _Up,
                           const int _W, const int _H,
                           const float _fov_radians,
                           const bool _is_radians,
                           const ProjectionType _projection)
    : Eye(_Eye), At(_At), Up(_Up), W(_W), H(_H), fov_angle(_fov_radians), projection(_projection) {
    
    // Compute camera coordinate system
    forward = Vector(At.X - Eye.X, At.Y - Eye.Y, At.Z - Eye.Z);
    forward.normalize();
    
    right = forward.cross(Up);
    right.normalize();
    
    up = right.cross(forward);
    up.normalize();
    
    max_radius = 1.0f;
}

bool FisheyeCamera::GenerateRay(const int x, const int y, Ray *r, const float *cam_jitter) {
    float pixel_x, pixel_y;
    
    // Calculate pixel coordinates with or without jittering
    if (cam_jitter == NULL) {
        pixel_x = (float)x + 0.5f;
        pixel_y = (float)y + 0.5f;
    } else {
        pixel_x = (float)x + cam_jitter[0];
        pixel_y = (float)y + cam_jitter[1];
    }
    
    // Convert pixel coordinates to normalized coordinates [-1, 1]
    int min_dim = std::min(W, H);
    float norm_x = (2.0f * pixel_x - W) / min_dim;
    float norm_y = -(2.0f * pixel_y - H) / min_dim;  // Negativo para corrigir o facto da imagem ficar invertida
    
    // Calculate distance from center
    float r_dist = sqrtf(norm_x * norm_x + norm_y * norm_y);
    
    // If outside the fisheye circle, return background ray
    if (r_dist > max_radius) {
        // Generate a ray that will hit background
        r->o = Eye;
        r->dir = Vector(0, 0, 1);  // Arbitrary direction for background
        r->pix_x = x;
        r->pix_y = y;
        r->FaceID = -1;
        r->propagating_eta = 1.0f;
        r->rtype = PRIMARY;
        return false;  // Indicates background
    }
    
    // Handle center point (avoid division by zero)
    if (r_dist < 1e-6f) {
        r->o = Eye;
        r->dir = forward;
        r->pix_x = x;
        r->pix_y = y;
        r->FaceID = -1;
        r->propagating_eta = 1.0f;
        r->rtype = PRIMARY;
        return true;
    }
    
    // Calculate angle from center based on projection model
    float theta;
    switch (projection) {
        case EQUIDISTANT:
            theta = r_dist * (fov_angle / 2.0f);
            break;
        case EQUISOLID:
            theta = 2.0f * asinf(r_dist * sinf(fov_angle / 4.0f));
            break;
        case ORTHOGRAPHIC:
            theta = asinf(r_dist * sinf(fov_angle / 2.0f));
            break;
        case STEREOGRAPHIC:
            theta = 2.0f * atanf(r_dist * tanf(fov_angle / 4.0f));
            break;
        default:
            theta = r_dist * (fov_angle / 2.0f);
            break;
    }
    
    // Calculate azimuth angle
    float phi = atan2f(norm_y, norm_x);
    
    // Convert spherical coordinates to direction vector in camera space
    float sin_theta = sinf(theta);
    float cos_theta = cosf(theta);
    float sin_phi = sinf(phi);
    float cos_phi = cosf(phi);
    
    // Direction in camera coordinate system
    Vector cam_dir(sin_theta * cos_phi,   // right component
                   sin_theta * sin_phi,   // up component  
                   cos_theta);            // forward component
    
    // Transform to world coordinates
    Vector world_dir = cam_dir.X * right + cam_dir.Y * up + cam_dir.Z * forward;
    world_dir.normalize();
    
    // Set up the ray
    r->o = Eye;
    r->dir = world_dir;
    r->pix_x = x;
    r->pix_y = y;
    r->FaceID = -1;
    r->propagating_eta = 1.0f;
    r->rtype = PRIMARY;
    
    return true;
}

Vector FisheyeCamera::pixelToDirection(float pixel_x, float pixel_y) {
    // This is now handled directly in GenerateRay
    return Vector(0, 0, 1);
}

float FisheyeCamera::equidistantProjection(float theta) {
    return theta;
}

float FisheyeCamera::equisolidProjection(float theta) {
    return 2.0f * sinf(theta / 2.0f);
}

float FisheyeCamera::orthographicProjection(float theta) {
    return sinf(theta);
}

float FisheyeCamera::stereographicProjection(float theta) {
    return 2.0f * tanf(theta / 2.0f);
}

void FisheyeCamera::setFOV(float fov_degrees) {
    fov_angle = fov_degrees * M_PI / 180.0f;
}

void FisheyeCamera::setFOVRadians(float fov_radians) {
    fov_angle = fov_radians;
}