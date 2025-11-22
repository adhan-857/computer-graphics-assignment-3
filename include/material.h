#ifndef MATERIAL_H
#define MATERIAL_H

#include "vec3.h"
#include "ray.h"
#include "random.h"

typedef enum {
    MATERIAL_LAMBERTIAN,
    MATERIAL_METAL,
    MATERIAL_DIELECTRIC,
    MATERIAL_EMISSIVE,
    MATERIAL_BLEND
} MaterialType;

// Blend mode for blended materials
typedef enum {
    BLEND_VERTICAL,    // Blend based on Y coordinate (vertical gradient)
    BLEND_HORIZONTAL,  // Blend based on X coordinate
    BLEND_RADIAL       // Blend based on distance from origin
} BlendMode;

typedef struct {
    MaterialType type;
    Vec3 albedo;
    float roughness;  // For metal
    float ior;        // Index of refraction for dielectric
    Vec3 emission;    // For emissive materials

    // Blending properties (for MATERIAL_BLEND)
    MaterialType blend_type1;    // Type of first material
    MaterialType blend_type2;    // Type of second material
    Vec3 albedo2;                // Albedo for second material
    float roughness2;            // Roughness for second material
    float ior2;                  // IOR for second material
    BlendMode blend_mode;        // How to blend
    float blend_min;             // Start of blend range
    float blend_max;             // End of blend range
} Material;

// Forward declaration
typedef struct HitRecord HitRecord;

// Material creation functions
static inline Material material_lambertian(Vec3 albedo) {
    return (Material){
        .type = MATERIAL_LAMBERTIAN,
        .albedo = albedo,
        .roughness = 0.0f,
        .ior = 1.0f,
        .emission = vec3_create(0, 0, 0),
        .blend_type1 = MATERIAL_LAMBERTIAN,
        .blend_type2 = MATERIAL_LAMBERTIAN,
        .albedo2 = vec3_create(0, 0, 0),
        .roughness2 = 0.0f,
        .ior2 = 1.0f,
        .blend_mode = BLEND_VERTICAL,
        .blend_min = 0.0f,
        .blend_max = 1.0f
    };
}

static inline Material material_metal(Vec3 albedo, float roughness) {
    return (Material){
        .type = MATERIAL_METAL,
        .albedo = albedo,
        .roughness = roughness,
        .ior = 1.0f,
        .emission = vec3_create(0, 0, 0),
        .blend_type1 = MATERIAL_LAMBERTIAN,
        .blend_type2 = MATERIAL_LAMBERTIAN,
        .albedo2 = vec3_create(0, 0, 0),
        .roughness2 = 0.0f,
        .ior2 = 1.0f,
        .blend_mode = BLEND_VERTICAL,
        .blend_min = 0.0f,
        .blend_max = 1.0f
    };
}

static inline Material material_dielectric(float ior) {
    return (Material){
        .type = MATERIAL_DIELECTRIC,
        .albedo = vec3_create(1, 1, 1),
        .roughness = 0.0f,
        .ior = ior,
        .emission = vec3_create(0, 0, 0),
        .blend_type1 = MATERIAL_LAMBERTIAN,
        .blend_type2 = MATERIAL_LAMBERTIAN,
        .albedo2 = vec3_create(0, 0, 0),
        .roughness2 = 0.0f,
        .ior2 = 1.0f,
        .blend_mode = BLEND_VERTICAL,
        .blend_min = 0.0f,
        .blend_max = 1.0f
    };
}

static inline Material material_emissive(Vec3 emission) {
    return (Material){
        .type = MATERIAL_EMISSIVE,
        .albedo = vec3_create(0, 0, 0),
        .roughness = 0.0f,
        .ior = 1.0f,
        .emission = emission,
        .blend_type1 = MATERIAL_LAMBERTIAN,
        .blend_type2 = MATERIAL_LAMBERTIAN,
        .albedo2 = vec3_create(0, 0, 0),
        .roughness2 = 0.0f,
        .ior2 = 1.0f,
        .blend_mode = BLEND_VERTICAL,
        .blend_min = 0.0f,
        .blend_max = 1.0f
    };
}

// Blend between two materials based on position
// Example: material_blend(MATERIAL_LAMBERTIAN, red, 0.0f, 1.0f,
//                         MATERIAL_METAL, gold, 0.1f, 1.0f,
//                         BLEND_VERTICAL, -1.0f, 1.0f)
static inline Material material_blend(
    MaterialType type1, Vec3 albedo1, float param1, float ior1,
    MaterialType type2, Vec3 albedo2, float param2, float ior2,
    BlendMode mode, float blend_min, float blend_max)
{
    return (Material){
        .type = MATERIAL_BLEND,
        .albedo = albedo1,
        .roughness = param1,  // roughness for type1 if metal
        .ior = ior1,          // ior for type1 if dielectric
        .emission = vec3_create(0, 0, 0),
        .blend_type1 = type1,
        .blend_type2 = type2,
        .albedo2 = albedo2,
        .roughness2 = param2,
        .ior2 = ior2,
        .blend_mode = mode,
        .blend_min = blend_min,
        .blend_max = blend_max
    };
}

// Schlick approximation for Fresnel
static inline float schlick(float cosine, float ref_idx) {
    float r0 = (1.0f - ref_idx) / (1.0f + ref_idx);
    r0 = r0 * r0;
    return r0 + (1.0f - r0) * powf(1.0f - cosine, 5.0f);
}

// Material scattering
bool material_scatter(const Material* mat, const Ray* ray_in,
                     const HitRecord* rec, Vec3* attenuation,
                     Ray* scattered, RNG* rng);

#endif // MATERIAL_H