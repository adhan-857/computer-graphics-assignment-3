#ifndef VEC3_H
#define VEC3_H

#include <math.h>
#include <immintrin.h>
#include <stdbool.h>
#include <stdio.h>

// 3D Vector structure aligned for SIMD
typedef struct {
    float x, y, z;
    float _padding;  // For 16-byte alignment
} __attribute__((aligned(16))) Vec3;

// Vector operations
static inline Vec3 vec3_create(float x, float y, float z) {
    return (Vec3){x, y, z, 0.0f};
}

static inline Vec3 vec3_add(Vec3 a, Vec3 b) {
    return vec3_create(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vec3 vec3_sub(Vec3 a, Vec3 b) {
    return vec3_create(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline Vec3 vec3_mul(Vec3 a, Vec3 b) {
    return vec3_create(a.x * b.x, a.y * b.y, a.z * b.z);
}

static inline Vec3 vec3_scale(Vec3 v, float s) {
    return vec3_create(v.x * s, v.y * s, v.z * s);
}

static inline Vec3 vec3_div(Vec3 v, float s) {
    float inv = 1.0f / s;
    return vec3_scale(v, inv);
}

static inline float vec3_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline Vec3 vec3_cross(Vec3 a, Vec3 b) {
    return vec3_create(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

static inline float vec3_length_squared(Vec3 v) {
    return vec3_dot(v, v);
}

static inline float vec3_length(Vec3 v) {
    return sqrtf(vec3_length_squared(v));
}

static inline Vec3 vec3_normalize(Vec3 v) {
    return vec3_div(v, vec3_length(v));
}

static inline Vec3 vec3_reflect(Vec3 v, Vec3 n) {
    return vec3_sub(v, vec3_scale(n, 2.0f * vec3_dot(v, n)));
}

static inline bool vec3_refract(Vec3 v, Vec3 n, float ni_over_nt, Vec3* refracted) {
    Vec3 uv = vec3_normalize(v);
    float dt = vec3_dot(uv, n);
    float discriminant = 1.0f - ni_over_nt * ni_over_nt * (1.0f - dt * dt);

    if (discriminant > 0) {
        *refracted = vec3_sub(
            vec3_scale(vec3_sub(uv, vec3_scale(n, dt)), ni_over_nt),
            vec3_scale(n, sqrtf(discriminant))
        );
        return true;
    }
    return false;
}

static inline Vec3 vec3_lerp(Vec3 a, Vec3 b, float t) {
    return vec3_add(vec3_scale(a, 1.0f - t), vec3_scale(b, t));
}


#endif // VEC3_H