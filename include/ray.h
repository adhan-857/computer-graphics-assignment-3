#ifndef RAY_H
#define RAY_H

#include "vec3.h"

typedef struct {
    Vec3 origin;
    Vec3 direction;
} Ray;

static inline Ray ray_create(Vec3 origin, Vec3 direction) {
    return (Ray){origin, vec3_normalize(direction)};
}

static inline Vec3 ray_at(Ray r, float t) {
    return vec3_add(r.origin, vec3_scale(r.direction, t));
}

#endif // RAY_H