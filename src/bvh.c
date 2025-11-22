#include "bvh.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

// Comparison function for qsort
typedef struct {
    uint32_t axis;
    const Primitive* primitives;
} SortContext;

static SortContext sort_ctx;

static int compare_primitives(const void* a, const void* b) {
    uint32_t idx_a = *(const uint32_t*)a;
    uint32_t idx_b = *(const uint32_t*)b;

    Vec3 center_a = aabb_center(sort_ctx.primitives[idx_a].bounds);
    Vec3 center_b = aabb_center(sort_ctx.primitives[idx_b].bounds);

    float val_a = ((float*)&center_a)[sort_ctx.axis];
    float val_b = ((float*)&center_b)[sort_ctx.axis];

    if (val_a < val_b) return -1;
    if (val_a > val_b) return 1;
    return 0;
}

// Find best split using SAH
SplitCandidate bvh_find_best_split(const BVH* bvh, uint32_t* prim_indices,
                                   uint32_t start, uint32_t end) {
    SplitCandidate best = {FLT_MAX, 0, start + (end - start) / 2};
    const uint32_t num_bins = 12;

    for (uint32_t axis = 0; axis < 3; axis++) {
        // Compute bounds for this subset
        AABB bounds = aabb_empty();
        for (uint32_t i = start; i < end; i++) {
            bounds = aabb_union(bounds, bvh->primitives[prim_indices[i]].bounds);
        }

        float axis_min = ((float*)&bounds.min)[axis];
        float axis_max = ((float*)&bounds.max)[axis];

        if (axis_max - axis_min < 0.0001f) continue;

        // Binning
        typedef struct {
            AABB bounds;
            uint32_t count;
        } Bin;

        Bin bins[12] = {0};
        float bin_width = (axis_max - axis_min) / num_bins;

        // Fill bins
        for (uint32_t i = start; i < end; i++) {
            Vec3 center = aabb_center(bvh->primitives[prim_indices[i]].bounds);
            float pos = ((float*)&center)[axis];
            uint32_t bin_idx = (uint32_t)((pos - axis_min) / bin_width);
            if (bin_idx >= num_bins) bin_idx = num_bins - 1;

            bins[bin_idx].count++;
            if (bins[bin_idx].count == 1) {
                bins[bin_idx].bounds = bvh->primitives[prim_indices[i]].bounds;
            } else {
                bins[bin_idx].bounds = aabb_union(bins[bin_idx].bounds,
                                                  bvh->primitives[prim_indices[i]].bounds);
            }
        }

        // Sweep to find best split
        for (uint32_t split_bin = 1; split_bin < num_bins; split_bin++) {
            AABB left_bounds = aabb_empty();
            AABB right_bounds = aabb_empty();
            uint32_t left_count = 0;
            uint32_t right_count = 0;

            for (uint32_t i = 0; i < split_bin; i++) {
                if (bins[i].count > 0) {
                    left_bounds = aabb_union(left_bounds, bins[i].bounds);
                    left_count += bins[i].count;
                }
            }

            for (uint32_t i = split_bin; i < num_bins; i++) {
                if (bins[i].count > 0) {
                    right_bounds = aabb_union(right_bounds, bins[i].bounds);
                    right_count += bins[i].count;
                }
            }

            if (left_count == 0 || right_count == 0) continue;

            // TODO: Implementasi SAH (Surface Area Heuristic) cost calculation
            // Hint:
            // 1. Hitung cost = traversal_cost + (left_count * left_area + right_count * right_area) / parent_area
            // 2. traversal_cost biasanya = 1.0f
            // 3. Gunakan aabb_surface_area() untuk menghitung area
            // 4. Jika cost lebih kecil dari best.cost, update best split
            // 5. Jangan lupa sort primitives dan set split_pos

            float cost = FLT_MAX; // TODO: Hitung SAH cost

            // TODO: Update best split jika cost lebih baik
        }
    }

    return best;
}

// Build BVH recursively
BVHNode* bvh_build_recursive(BVH* bvh, uint32_t* prim_indices,
                            uint32_t start, uint32_t end, uint32_t* node_idx) {
    // TODO: Implementasi recursive BVH construction
    // Hint:
    // 1. Alokasi node baru: BVHNode* node = &bvh->nodes[(*node_idx)++]
    // 2. Hitung bounding box untuk semua primitives di range [start, end)
    // 3. Jika jumlah primitives <= 2, buat leaf node:
    //    - Set node->is_leaf = true
    //    - Set node->first_prim_idx = start
    //    - Set node->prim_count = prim_count
    // 4. Jika bukan leaf:
    //    - Panggil bvh_find_best_split() untuk cari split terbaik
    //    - Set node->is_leaf = false
    //    - Recursive build left child: node->left = bvh_build_recursive(...)
    //    - Recursive build right child: node->right = bvh_build_recursive(...)
    // 5. Return node

    BVHNode* node = &bvh->nodes[(*node_idx)++];

    // TODO: Hitung bounds untuk node ini

    uint32_t prim_count = end - start;

    // TODO: Implementasi leaf node dan recursive split

    return node;
}

// Create BVH
BVH* bvh_create(Primitive* primitives, uint32_t count) {
    BVH* bvh = (BVH*)calloc(1, sizeof(BVH));
    bvh->primitives = primitives;
    bvh->prim_count = count;

    // Allocate nodes (worst case: 2N-1 nodes)
    bvh->nodes = (BVHNode*)calloc(2 * count - 1, sizeof(BVHNode));
    bvh->indices = (uint32_t*)malloc(count * sizeof(uint32_t));

    // Initialize indices
    for (uint32_t i = 0; i < count; i++) {
        bvh->indices[i] = i;
    }

    // Build tree
    uint32_t node_idx = 0;
    bvh->root = bvh_build_recursive(bvh, bvh->indices, 0, count, &node_idx);
    bvh->node_count = node_idx;

    // Reorder primitives according to indices
    Primitive* reordered = (Primitive*)malloc(count * sizeof(Primitive));
    for (uint32_t i = 0; i < count; i++) {
        reordered[i] = primitives[bvh->indices[i]];
    }
    memcpy(primitives, reordered, count * sizeof(Primitive));
    free(reordered);

    return bvh;
}

// Destroy BVH
void bvh_destroy(BVH* bvh) {
    if (bvh) {
        free(bvh->nodes);
        free(bvh->indices);
        free(bvh);
    }
}

// BVH traversal (iterative for performance)
bool bvh_hit(const BVH* bvh, const Ray* ray, float t_min, float t_max,
             HitRecord* rec) {
    // TODO: Implementasi BVH traversal algorithm
    // Hint:
    // 1. Gunakan stack untuk iterative traversal (sudah disediakan)
    // 2. Start dengan root node: stack[stack_ptr++] = bvh->root
    // 3. Loop selama stack tidak kosong:
    //    a. Pop node dari stack
    //    b. Test AABB intersection dengan aabb_hit()
    //    c. Jika leaf node:
    //       - Test semua primitives dalam leaf
    //       - Update closest_so_far jika hit
    //    d. Jika internal node:
    //       - Push kedua children ke stack
    // 4. Return true jika ada hit

    // Stack untuk iterative traversal (jangan diubah)
    BVHNode* stack[64];
    int stack_ptr = 0;

    bool hit_anything = false;
    float closest_so_far = t_max;

    // TODO: Implementasi traversal algorithm di sini

    return hit_anything;
}