#include "BVHAccel.hpp"
#include <algorithm>
#include <iostream>
#include <climits>

// Construtor - segue estrutura do PBR book, mas com alterações para ser compatível com o código já existente
BVHAccel::BVHAccel(std::vector<Primitive*>& p, BRDF** mats, int maxPrimsInNode, SplitMethod splitMethod)
    : materials(mats), maxPrimsInNode(std::min(255, maxPrimsInNode)), primitives(p), splitMethod(splitMethod) {
    
    if (primitives.size() == 0) return;
    
    fprintf(stdout, "Building BVH with %zu primitives...\n", primitives.size());
    
    // Initialize primitiveInfo array
    std::vector<BVHPrimitiveInfo> primitiveInfo;
    primitiveInfo.reserve(primitives.size());
    
    for (size_t i = 0; i < primitives.size(); ++i) {
        primitiveInfo.emplace_back(i, primitives[i]->g->WorldBound());
    }
    
    // Build BVH tree
    int totalNodes = 0;
    std::vector<Primitive*> orderedPrims;
    orderedPrims.reserve(primitives.size());
    
    BVHBuildNode* root = recursiveBuild(primitiveInfo, 0, primitives.size(), 
                                   &totalNodes, orderedPrims);

    fprintf(stdout, "Root bounding box: min(%.1f,%.1f,%.1f) max(%.1f,%.1f,%.1f)\n",
        root->bounds.min.X, root->bounds.min.Y, root->bounds.min.Z,
        root->bounds.max.X, root->bounds.max.Y, root->bounds.max.Z);
    
    primitives.swap(orderedPrims);
    
    // Convert to linear representation  
    nodes = new LinearBVHNode[totalNodes];
    int offset = 0;
    flattenBVHTree(root, &offset);
    
    fprintf(stdout, "BVH built: %d nodes for %zu primitives\n", totalNodes, primitives.size());
}

BVHAccel::~BVHAccel() {
    delete[] nodes;
}

// Construção recursiva com SAH
BVHBuildNode* BVHAccel::recursiveBuild(
    std::vector<BVHPrimitiveInfo>& primitiveInfo,
    int start, int end, int* totalNodes,
    std::vector<Primitive*>& orderedPrims) {
    
    BVHBuildNode* node = new BVHBuildNode;
    (*totalNodes)++;
    
    // Compute bounds of all primitives
    BB bounds;
    bool first = true;
    for (int i = start; i < end; ++i) {
        if (first) {
            bounds = primitiveInfo[i].bounds;
            first = false;
        } else {
            bounds = Union(bounds, primitiveInfo[i].bounds);
        }
    }
    
    int nPrimitives = end - start;
    
    // Create leaf if few primitives
    if (nPrimitives == 1) {
        int firstPrimOffset = orderedPrims.size();
        for (int i = start; i < end; ++i) {
            int primNum = primitiveInfo[i].primitiveNumber;
            orderedPrims.push_back(primitives[primNum]);
        }
        node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
        return node;
    }
    
    // Compute bound of primitive centroids, choose split dimension
    BB centroidBounds;
    first = true;
    for (int i = start; i < end; ++i) {
        if (first) {
            centroidBounds.min = primitiveInfo[i].centroid;
            centroidBounds.max = primitiveInfo[i].centroid;
            first = false;
        } else {
            centroidBounds = Union(centroidBounds, primitiveInfo[i].centroid);
        }
    }
    int dim = MaximumExtent(centroidBounds);
    
    // Partition primitives into two sets
    int mid = (start + end) / 2;
    
    if (splitMethod == SplitMethod::Middle) {
        // Partition primitives through node's midpoint
        Point pmid = Centroid(centroidBounds);
        float midCoord = (dim == 0) ? pmid.X : (dim == 1) ? pmid.Y : pmid.Z;
        
        BVHPrimitiveInfo* midPtr = std::partition(
            &primitiveInfo[start], &primitiveInfo[end-1]+1,
            [dim, midCoord](const BVHPrimitiveInfo& pi) {
                float coord = (dim == 0) ? pi.centroid.X : 
                             (dim == 1) ? pi.centroid.Y : pi.centroid.Z;
                return coord < midCoord;
            });
        mid = midPtr - &primitiveInfo[0];
        
        if (mid != start && mid != end) {
            // Good split
        } else {
            // Fall back to equal counts
            mid = (start + end) / 2;
            std::nth_element(&primitiveInfo[start], &primitiveInfo[mid],
                           &primitiveInfo[end-1]+1,
                           [dim](const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b) {
                               float aCoord = (dim == 0) ? a.centroid.X : 
                                            (dim == 1) ? a.centroid.Y : a.centroid.Z;
                               float bCoord = (dim == 0) ? b.centroid.X : 
                                            (dim == 1) ? b.centroid.Y : b.centroid.Z;
                               return aCoord < bCoord;
                           });
        }
    } else {
        // Equal counts
        mid = (start + end) / 2;
        std::nth_element(&primitiveInfo[start], &primitiveInfo[mid],
                       &primitiveInfo[end-1]+1,
                       [dim](const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b) {
                           float aCoord = (dim == 0) ? a.centroid.X : 
                                        (dim == 1) ? a.centroid.Y : a.centroid.Z;
                           float bCoord = (dim == 0) ? b.centroid.X : 
                                        (dim == 1) ? b.centroid.Y : b.centroid.Z;
                           return aCoord < bCoord;
                       });
    }
    
    // Recursively build children
    node->InitInterior(dim,
        recursiveBuild(primitiveInfo, start, mid, totalNodes, orderedPrims),
        recursiveBuild(primitiveInfo, mid, end, totalNodes, orderedPrims));
    
    return node;
}

int BVHAccel::flattenBVHTree(BVHBuildNode* node, int* offset) {
    LinearBVHNode* linearNode = &nodes[*offset];
    linearNode->bounds = node->bounds;
    int myOffset = (*offset)++;
    
    if (node->nPrimitives > 0) {
        // Leaf node
        linearNode->primitivesOffset = node->firstPrimOffset;
        linearNode->nPrimitives = static_cast<uint16_t>(node->nPrimitives);
    } else {
        // Interior node
        linearNode->axis = static_cast<uint8_t>(node->splitAxis);
        linearNode->nPrimitives = 0;
        flattenBVHTree(node->children[0], offset);
        linearNode->secondChildOffset = flattenBVHTree(node->children[1], offset);
    }
    
    return myOffset;
}

bool BVHAccel::Intersect(const Ray& ray, Intersection* isect) const {
    if (!nodes) return false;
    
    bool hit = false;
    Vector invDir(1.0f / ray.dir.X, 1.0f / ray.dir.Y, 1.0f / ray.dir.Z);
    int dirIsNeg[3] = { invDir.X < 0, invDir.Y < 0, invDir.Z < 0 };
    
    // Traversal stack
    int toVisitOffset = 0, currentNodeIndex = 0;
    int nodesToVisit[64];
    
    while (true) {
        const LinearBVHNode* node = &nodes[currentNodeIndex];
        
        if (::IntersectP(node->bounds, ray, invDir, dirIsNeg)) {
            if (node->nPrimitives > 0) {
                // Intersect ray with primitives in leaf BVH node
                for (int i = 0; i < node->nPrimitives; ++i) {
                    // Criar uma intersecção temporária
                    Intersection temp_isect;
                    temp_isect.pix_x = isect->pix_x;
                    temp_isect.pix_y = isect->pix_y;
                    
                    // Guardar referência à primitiva
                    Primitive* prim = primitives[node->primitivesOffset + i];
                    
                    if (prim->g->intersect(ray, &temp_isect)) {
                        //Só atualizar se for mais próxima
                        if (!hit || temp_isect.depth < isect->depth) {
                            *isect = temp_isect;
                            isect->f = materials[prim->material_ndx];
                            hit = true;
                        }
                    }
                }
                if (toVisitOffset == 0) break;
                currentNodeIndex = nodesToVisit[--toVisitOffset];
            } else {
                // Put far BVH node on nodesToVisit stack, advance to near node
                if (dirIsNeg[node->axis]) {
                    nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                    currentNodeIndex = node->secondChildOffset;
                } else {
                    nodesToVisit[toVisitOffset++] = node->secondChildOffset;
                    currentNodeIndex = currentNodeIndex + 1;
                }
            }
        } else {
            if (toVisitOffset == 0) break;
            currentNodeIndex = nodesToVisit[--toVisitOffset];
        }
    }
    
    return hit;
}

// Shadow ray
bool BVHAccel::IntersectP(const Ray& ray) const {
    if (!nodes) return false;
    
    Vector invDir(1.0f / ray.dir.X, 1.0f / ray.dir.Y, 1.0f / ray.dir.Z);
    int dirIsNeg[3] = { invDir.X < 0, invDir.Y < 0, invDir.Z < 0 };
    
    int toVisitOffset = 0, currentNodeIndex = 0;
    int nodesToVisit[64];
    
    while (true) {
        const LinearBVHNode* node = &nodes[currentNodeIndex];
        
        // CORRIGIDO: usar função livre com :: para diferenciar
        if (::IntersectP(node->bounds, ray, invDir, dirIsNeg)) {
            if (node->nPrimitives > 0) {
                for (int i = 0; i < node->nPrimitives; ++i) {
                    Intersection temp_isect;
                    if (primitives[node->primitivesOffset + i]->g->intersect(ray, &temp_isect)) {
                        return true;  // Early exit for shadows
                    }
                }
                if (toVisitOffset == 0) break;
                currentNodeIndex = nodesToVisit[--toVisitOffset];
            } else {
                if (dirIsNeg[node->axis]) {
                    nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                    currentNodeIndex = node->secondChildOffset;
                } else {
                    nodesToVisit[toVisitOffset++] = node->secondChildOffset;
                    currentNodeIndex = currentNodeIndex + 1;
                }
            }
        } else {
            if (toVisitOffset == 0) break;
            currentNodeIndex = nodesToVisit[--toVisitOffset];
        }
    }
    
    return false;
}


// Shadow ray test com distância máxima
bool BVHAccel::IntersectP(const Ray& ray, float maxDist) const {
    if (!nodes) return false;
    
    Vector invDir(1.0f / ray.dir.X, 1.0f / ray.dir.Y, 1.0f / ray.dir.Z);
    int dirIsNeg[3] = { invDir.X < 0, invDir.Y < 0, invDir.Z < 0 };
    
    int toVisitOffset = 0, currentNodeIndex = 0;
    int nodesToVisit[64];
    
    while (true) {
        const LinearBVHNode* node = &nodes[currentNodeIndex];
        
        if (::IntersectP(node->bounds, ray, invDir, dirIsNeg)) {
            if (node->nPrimitives > 0) {
                // Testar primitivas
                for (int i = 0; i < node->nPrimitives; ++i) {
                    Intersection temp_isect;
                    if (primitives[node->primitivesOffset + i]->g->intersect(ray, &temp_isect)) {
                        // Verificar se está dentro da distância máxima
                        if (temp_isect.depth < maxDist) {
                            return true;  // Bloqueado!
                        }
                    }
                }
                if (toVisitOffset == 0) break;
                currentNodeIndex = nodesToVisit[--toVisitOffset];
            } else {
                // Interior node...
                if (dirIsNeg[node->axis]) {
                    nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                    currentNodeIndex = node->secondChildOffset;
                } else {
                    nodesToVisit[toVisitOffset++] = node->secondChildOffset;
                    currentNodeIndex = currentNodeIndex + 1;
                }
            }
        } else {
            if (toVisitOffset == 0) break;
            currentNodeIndex = nodesToVisit[--toVisitOffset];
        }
    }
    
    return false;  // Não bloqueado
}