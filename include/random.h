#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>
#include <math.h>
#include "vec3.h"

// Define M_PI if not defined (C11 strict mode)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// PCG random number generator (fast and high quality)
typedef struct {
    uint64_t state;
    uint64_t inc;
} RNG;

// Initialize RNG with seed
static inline void rng_init(RNG* rng, uint64_t seed) {
    rng->state = seed;
    rng->inc = (seed << 1u) | 1u;
}

// Generate random uint32
static inline uint32_t rng_uint32(RNG* rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

// Generate random float in [0, 1)
static inline float rng_float(RNG* rng) {
    return (rng_uint32(rng) & 0xFFFFFF) / 16777216.0f;
}

// Generate random float in [min, max)
static inline float rng_float_range(RNG* rng, float min, float max) {
    return min + (max - min) * rng_float(rng);
}

// Generate random vector in unit sphere
static inline Vec3 rng_in_unit_sphere(RNG* rng) {
    while (1) {
        Vec3 p = vec3_create(
            rng_float_range(rng, -1.0f, 1.0f),
            rng_float_range(rng, -1.0f, 1.0f),
            rng_float_range(rng, -1.0f, 1.0f)
        );
        if (vec3_length_squared(p) < 1.0f)
            return p;
    }
}

// Generate random unit vector
static inline Vec3 rng_unit_vector(RNG* rng) {
    return vec3_normalize(rng_in_unit_sphere(rng));
}

// Generate random vector in unit disk (for DOF)
static inline Vec3 rng_in_unit_disk(RNG* rng) {
    while (1) {
        Vec3 p = vec3_create(
            rng_float_range(rng, -1.0f, 1.0f),
            rng_float_range(rng, -1.0f, 1.0f),
            0.0f
        );
        if (vec3_length_squared(p) < 1.0f)
            return p;
    }
}

#endif // RANDOM_H