//
//  Texture.hpp
//  VI-RT
//
//  Textura partilhada: encapsula a amostragem (convenção ÚNICA de flip + clamp
//  + bilinear) e a partilha por ficheiro (cache). Substitui o bloco
//  `clamp → 1-v → floor → get` que estava duplicado em cada BRDF texturizada.
//

#ifndef Texture_hpp
#define Texture_hpp

#include "ImagePPM.hpp"
#include "RGB.hpp"
#include <string>
#include <map>
#include <algorithm>
#include <cmath>

class Texture {
    ImagePPM img;                                  // dados carregados (1×)
    explicit Texture(const std::string& filename) { img.Load(filename); }

public:
    // CACHE por nome de ficheiro: carrega 1× e partilha o mesmo objecto entre
    // todos os materiais que pedem a mesma textura (resolve a duplicação em
    // memória). Mantido vivo durante a execução (libertado pelo SO no fim).
    static Texture* get(const std::string& filename) {
        static std::map<std::string, Texture*> cache;
        std::map<std::string, Texture*>::iterator it = cache.find(filename);
        if (it != cache.end()) return it->second;
        Texture* t = new Texture(filename);
        cache[filename] = t;
        return t;
    }

    int width()  const { return img.W; }
    int height() const { return img.H; }

    // Amostragem UV — A ÚNICA convenção do projeto:
    //   - flip de V (PPM tem y=0 no topo; UV tem v=0 no fundo)  ← só aqui
    //   - clamp de u,v a [0,1]
    //   - filtragem BILINEAR (lerp dos 4 texels vizinhos)
    RGB sampleUV(float u, float v) const {
        if (img.W <= 0 || img.H <= 0) return RGB(0.f, 0.f, 0.f);

        u = std::max(0.f, std::min(u, 1.f));
        v = std::max(0.f, std::min(1.f - v, 1.f));   // flip ÚNICO

        // coordenada contínua em texels (centros em -0.5)
        float fx = u * img.W - 0.5f;
        float fy = v * img.H - 0.5f;
        int x0 = (int)std::floor(fx);
        int y0 = (int)std::floor(fy);
        float dx = fx - (float)x0;
        float dy = fy - (float)y0;

        // img.get() já faz clamp de borda → seguro fora dos limites
        RGB c00 = img.get(x0,     y0);
        RGB c10 = img.get(x0 + 1, y0);
        RGB c01 = img.get(x0,     y0 + 1);
        RGB c11 = img.get(x0 + 1, y0 + 1);

        RGB top    = c00 * (1.f - dx) + c10 * dx;
        RGB bottom = c01 * (1.f - dx) + c11 * dx;
        return top * (1.f - dy) + bottom * dy;
    }
};

#endif /* Texture_hpp */
