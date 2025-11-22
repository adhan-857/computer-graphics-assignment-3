#ifndef PATHTRACER_H
#define PATHTRACER_H

#include "vec3.h"
#include "ray.h"
#include "primitive.h"
#include "material.h"
#include "camera.h"
#include "bvh.h"
#include "random.h"
#include <stdint.h>

// Scene structure
typedef struct {
    Primitive* primitives;
    uint32_t prim_count;
    uint32_t prim_capacity;
    BVH* bvh;
    Vec3 ambient_light;
} Scene;

// Render settings
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t samples_per_pixel;
    uint32_t max_depth;
    bool use_bvh;
    bool use_nee;  // Next event estimation
    uint32_t num_threads;
    volatile bool* cancel_flag;  // Pointer to cancel flag for early termination
} RenderSettings;

// Image buffer
typedef struct {
    Vec3* pixels;
    uint32_t width;
    uint32_t height;
} Image;

// Scene functions
Scene* scene_create(void);
void scene_destroy(Scene* scene);
void scene_add_sphere(Scene* scene, Vec3 center, float radius, Material mat);
void scene_add_triangle(Scene* scene, Vec3 v0, Vec3 v1, Vec3 v2, Material mat);
void scene_build_bvh(Scene* scene);

// Image functions
Image* image_create(uint32_t width, uint32_t height);
void image_destroy(Image* img);
void image_save_bmp(const Image* img, const char* filename);

// Path tracing functions
Vec3 trace_ray(const Scene* scene, const Ray* ray, RNG* rng,
               uint32_t depth, uint32_t max_depth);
void render_parallel(const Scene* scene, const Camera* camera,
                    const RenderSettings* settings, Image* output);

// Utility functions
Vec3 aces_tonemap(Vec3 color);

// Progress callback
typedef void (*progress_callback_t)(float progress);
void set_progress_callback(progress_callback_t callback);

#endif // PATHTRACER_H