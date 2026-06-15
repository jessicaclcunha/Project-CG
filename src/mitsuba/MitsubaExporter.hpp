//
//  MitsubaExporter.hpp
//  VI-RT — módulo Mitsuba
//
//  Exporta uma Scene do VI-RT para um scene.xml do Mitsuba 3 (opção 3).
//  Como lê a MESMA Scene que o renderer usa, a geometria/luz/câmara ficam
//  idênticas por construção (sem drift) — só o mapeamento de BSDF é aproximado.
//

#ifndef MitsubaExporter_hpp
#define MitsubaExporter_hpp

#include <string>
#include "vector.hpp"

class Scene;

// Parâmetros de câmara (espelham os usados no render VI-RT)
struct MiCamera {
    Point  eye;
    Point  at;
    Vector up;
    float  fov;     // graus (eixo Y)
    int    W, H;
    int    spp;     // amostras do Mitsuba (referência limpa, ex. 128)
};

// Escreve <dir>/scene.xml (+ .obj dos triângulos) equivalente à Scene dada.
// dir é criado se não existir.
void ExportMitsubaScene(const Scene& scene, const MiCamera& cam, const std::string& dir);

#endif /* MitsubaExporter_hpp */
