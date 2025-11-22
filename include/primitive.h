#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include "vec3.h"
#include "ray.h"
#include "material.h"
#include <stdbool.h>
#include <float.h>

// Hit record stores intersection information
typedef struct HitRecord {
    Vec3 point;
    Vec3 normal;
    float t;
    bool front_face;
    const Material* material;
} HitRecord;

// Axis-aligned bounding box
typedef struct {
    Vec3 min;
    Vec3 max;
} AABB;

// Primitive types
typedef enum {
    PRIMITIVE_SPHERE,
    PRIMITIVE_TRIANGLE,
    PRIMITIVE_MESH
} PrimitiveType;

// Sphere primitive
typedef struct {
    Vec3 center;
    float radius;
} Sphere;

// Triangle primitive
typedef struct {
    Vec3 v0, v1, v2;
    Vec3 normal;  // Pre-computed normal
} Triangle;

// Generic primitive
typedef struct {
    PrimitiveType type;
    union {
        Sphere sphere;
        Triangle triangle;
    };
    Material material;
    AABB bounds;
} Primitive;

// AABB functions
static inline AABB aabb_empty() {
    return (AABB){
        vec3_create(FLT_MAX, FLT_MAX, FLT_MAX),
        vec3_create(-FLT_MAX, -FLT_MAX, -FLT_MAX)
    };
}

static inline AABB aabb_union(AABB a, AABB b) {
    return (AABB){
        vec3_create(
            fminf(a.min.x, b.min.x),
            fminf(a.min.y, b.min.y),
            fminf(a.min.z, b.min.z)
        ),
        vec3_create(
            fmaxf(a.max.x, b.max.x),
            fmaxf(a.max.y, b.max.y),
            fmaxf(a.max.z, b.max.z)
        )
    };
}

static inline AABB aabb_expand(AABB box, Vec3 point) {
    return (AABB){
        vec3_create(
            fminf(box.min.x, point.x),
            fminf(box.min.y, point.y),
            fminf(box.min.z, point.z)
        ),
        vec3_create(
            fmaxf(box.max.x, point.x),
            fmaxf(box.max.y, point.y),
            fmaxf(box.max.z, point.z)
        )
    };
}

static inline Vec3 aabb_center(AABB box) {
    return vec3_scale(vec3_add(box.min, box.max), 0.5f);
}

static inline float aabb_surface_area(AABB box) {
    Vec3 d = vec3_sub(box.max, box.min);
    return 2.0f * (d.x * d.y + d.y * d.z + d.z * d.x);
}

// Ray-AABB intersection (optimized slab method)
static inline bool aabb_hit(const AABB* box, const Ray* ray, float t_min, float t_max) {
    for (int a = 0; a < 3; a++) {
        float invD = 1.0f / ((float*)&ray->direction)[a];
        float t0 = (((float*)&box->min)[a] - ((float*)&ray->origin)[a]) * invD;
        float t1 = (((float*)&box->max)[a] - ((float*)&ray->origin)[a]) * invD;

        if (invD < 0.0f) {
            float temp = t0;
            t0 = t1;
            t1 = temp;
        }

        t_min = t0 > t_min ? t0 : t_min;
        t_max = t1 < t_max ? t1 : t_max;

        if (t_max <= t_min)
            return false;
    }
    return true;
}

// Sphere functions
static inline Sphere sphere_create(Vec3 center, float radius) {
    return (Sphere){center, radius};
}

static inline AABB sphere_bounds(const Sphere* s) {
    Vec3 r = vec3_create(s->radius, s->radius, s->radius);
    return (AABB){
        vec3_sub(s->center, r),
        vec3_add(s->center, r)
    };
}

// Ray-sphere intersection
bool sphere_hit(const Sphere* sphere, const Ray* ray, float t_min, float t_max,
                HitRecord* rec);

// Triangle functions
static inline Triangle triangle_create(Vec3 v0, Vec3 v1, Vec3 v2) {
    Triangle tri;
    tri.v0 = v0;
    tri.v1 = v1;
    tri.v2 = v2;
    Vec3 e1 = vec3_sub(v1, v0);
    Vec3 e2 = vec3_sub(v2, v0);
    tri.normal = vec3_normalize(vec3_cross(e1, e2));
    return tri;
}

static inline AABB triangle_bounds(const Triangle* t) {
    AABB box = aabb_empty();
    box = aabb_expand(box, t->v0);
    box = aabb_expand(box, t->v1);
    box = aabb_expand(box, t->v2);
    // Expand slightly to avoid numerical issues
    Vec3 epsilon = vec3_create(0.0001f, 0.0001f, 0.0001f);
    box.min = vec3_sub(box.min, epsilon);
    box.max = vec3_add(box.max, epsilon);
    return box;
}

// Ray-triangle intersection (Möller-Trumbore algorithm)
bool triangle_hit(const Triangle* triangle, const Ray* ray, float t_min, float t_max,
                  HitRecord* rec);

// Primitive creation
static inline Primitive primitive_sphere(Vec3 center, float radius, Material mat) {
    Primitive p;
    p.type = PRIMITIVE_SPHERE;
    p.sphere = sphere_create(center, radius);
    p.material = mat;
    p.bounds = sphere_bounds(&p.sphere);
    return p;
}

static inline Primitive primitive_triangle(Vec3 v0, Vec3 v1, Vec3 v2, Material mat) {
    Primitive p;
    p.type = PRIMITIVE_TRIANGLE;
    p.triangle = triangle_create(v0, v1, v2);
    p.material = mat;
    p.bounds = triangle_bounds(&p.triangle);
    return p;
}

// Generic primitive hit test
bool primitive_hit(const Primitive* prim, const Ray* ray, float t_min, float t_max,
                   HitRecord* rec);

#endif // PRIMITIVE_H