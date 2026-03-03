//
//  shader.hpp
//  VI-RT-LPS
//
//  Created by Luis Paulo Santos on 14/03/2023.
//

#ifndef shader_hpp
#define shader_hpp

#include "scene.hpp"
#include "RGB.hpp"

class Shader {
public:
    Scene *scene;
    Shader (Scene *_scene): scene(_scene) {}
    ~Shader () {}
    virtual RGB shade (bool intersected, Intersection isect, int depth) {return RGB();}
    // qualquer BRDF que eu vá criar vai herdar desta classe, ou seja, 
    // vai ser um shade, vai receber os parametros do shade e retornar 
    // o que o shade retorna
};

#endif /* shader_hpp */
