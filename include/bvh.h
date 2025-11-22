#ifndef BVH_H
#define BVH_H

#include "primitive.h"
#include <stdint.h>
#include <stdlib.h>

// BVH node structure
typedef struct BVHNode {
    AABB bounds;
    union {
        struct {
            struct BVHNode* left;
            struct BVHNode* right;
        };
        struct {
            uint32_t first_prim_idx;
            uint32_t prim_count;
        };
    };
    bool is_leaf;
} BVHNode;

// BVH acceleration structure
typedef struct {
    BVHNode* root;
    Primitive* primitives;
    uint32_t prim_count;
    BVHNode* nodes;
    uint32_t node_count;
    uint32_t* indices;  // Primitive indices for reordering
} BVH;

// BVH construction
BVH* bvh_create(Primitive* primitives, uint32_t count);
void bvh_destroy(BVH* bvh);

// BVH traversal
bool bvh_hit(const BVH* bvh, const Ray* ray, float t_min, float t_max,
             HitRecord* rec);

// Build BVH recursively
BVHNode* bvh_build_recursive(BVH* bvh, uint32_t* prim_indices,
                            uint32_t start, uint32_t end, uint32_t* node_idx);

// SAH (Surface Area Heuristic) for optimal splits
typedef struct {
    float cost;
    uint32_t split_axis;
    uint32_t split_pos;
} SplitCandidate;

SplitCandidate bvh_find_best_split(const BVH* bvh, uint32_t* prim_indices,
                                   uint32_t start, uint32_t end);

#endif // BVH_H