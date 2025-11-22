#include "material.h"
#include "primitive.h"
#include <math.h>

bool material_scatter(const Material* mat, const Ray* ray_in,
                     const HitRecord* rec, Vec3* attenuation,
                     Ray* scattered, RNG* rng) {
    switch (mat->type) {
        case MATERIAL_LAMBERTIAN: {
            // TODO: Implementasi Lambertian diffuse scattering
            // Hint:
            // 1. Generate random scatter direction dengan:
            //    Vec3 scatter_direction = vec3_add(rec->normal, rng_unit_vector(rng));
            // 2. Handle degenerate case (scatter_direction hampir nol)
            // 3. Buat scattered ray dari rec->point ke scatter_direction
            // 4. Set attenuation = mat->albedo (warna material)
            // 5. Return true untuk menandakan scatter berhasil

            // Cosine-weighted hemisphere sampling
            Vec3 scatter_direction = vec3_add(rec->normal, rng_unit_vector(rng));
            
            // Handle degenerate case (jika scatter_direction hampir nol)
            if (vec3_length_squared(scatter_direction) < 0.001f) {
                scatter_direction = rec->normal;
            }
            
            // Buat scattered ray
            *scattered = ray_create(rec->point, scatter_direction);
            
            // Set attenuation ke albedo material
            *attenuation = mat->albedo;
            
            return true;
        }

        case MATERIAL_METAL: {
            // TODO: Implementasi Metal reflection
            // Hint:
            // 1. Hitung reflected ray: vec3_reflect(vec3_normalize(ray_in->direction), rec->normal)
            // 2. Tambahkan roughness/fuzz: vec3_scale(rng_in_unit_sphere(rng), mat->roughness)
            // 3. Buat scattered ray dari reflected + fuzz
            // 4. Set attenuation = mat->albedo
            // 5. Return true hanya jika scattered ray mengarah ke luar (dot product > 0)

            return false; // Ganti dengan implementasi yang benar
        }

        case MATERIAL_DIELECTRIC: {
            // TODO: Implementasi Glass/dielectric dengan refraction
            // Hint:
            // 1. Set attenuation = vec3_create(1.0f, 1.0f, 1.0f) (glass tidak menyerap cahaya)
            // 2. Hitung refraction_ratio: jika front_face true gunakan 1/ior, else gunakan ior
            // 3. Normalize ray direction
            // 4. Coba refract dengan vec3_refract(), jika gagal lakukan total internal reflection
            // 5. Gunakan Schlick approximation untuk menentukan reflect vs refract
            // 6. Buat scattered ray dengan direction yang sudah ditentukan

            // Kode helper (jangan diubah):
            *attenuation = vec3_create(1.0f, 1.0f, 1.0f);
            float refraction_ratio = rec->front_face ? (1.0f / mat->ior) : mat->ior;

            // TODO: Implementasikan refraction logic di sini

            return false; // Ganti dengan implementasi yang benar
        }

        case MATERIAL_EMISSIVE: {
            // Emissive materials don't scatter
            return false;
        }

        case MATERIAL_BLEND: {
            // Calculate blend factor based on position
            float blend_factor = 0.0f;

            switch (mat->blend_mode) {
                case BLEND_VERTICAL:
                    // Blend based on Y coordinate
                    blend_factor = rec->point.y;
                    break;
                case BLEND_HORIZONTAL:
                    // Blend based on X coordinate
                    blend_factor = rec->point.x;
                    break;
                case BLEND_RADIAL:
                    // Blend based on distance from origin (XZ plane)
                    blend_factor = sqrtf(rec->point.x * rec->point.x +
                                       rec->point.z * rec->point.z);
                    break;
            }

            // Normalize blend_factor to [0, 1] range based on blend_min/max
            blend_factor = (blend_factor - mat->blend_min) / (mat->blend_max - mat->blend_min);
            blend_factor = fminf(fmaxf(blend_factor, 0.0f), 1.0f);  // Clamp to [0, 1]

            // Create blended material properties using vec3_lerp
            Vec3 blended_albedo = vec3_lerp(mat->albedo, mat->albedo2, blend_factor);
            float blended_roughness = mat->roughness + blend_factor * (mat->roughness2 - mat->roughness);
            float blended_ior = mat->ior + blend_factor * (mat->ior2 - mat->ior);

            // Choose material type based on blend_factor
            // If blend_factor < 0.5, use type1, otherwise use type2
            // This creates a smooth transition in behavior
            MaterialType active_type = (blend_factor < 0.5f) ? mat->blend_type1 : mat->blend_type2;

            // Scatter based on the active material type
            switch (active_type) {
                case MATERIAL_LAMBERTIAN: {
                    Vec3 scatter_direction = vec3_add(rec->normal, rng_unit_vector(rng));
                    if (vec3_length_squared(scatter_direction) < 0.001f) {
                        scatter_direction = rec->normal;
                    }
                    *scattered = ray_create(rec->point, scatter_direction);
                    *attenuation = blended_albedo;
                    return true;
                }

                case MATERIAL_METAL: {
                    Vec3 reflected = vec3_reflect(vec3_normalize(ray_in->direction), rec->normal);
                    Vec3 fuzz = vec3_scale(rng_in_unit_sphere(rng), blended_roughness);
                    *scattered = ray_create(rec->point, vec3_add(reflected, fuzz));
                    *attenuation = blended_albedo;
                    return vec3_dot(scattered->direction, rec->normal) > 0;
                }

                case MATERIAL_DIELECTRIC: {
                    *attenuation = vec3_create(1.0f, 1.0f, 1.0f);
                    float refraction_ratio = rec->front_face ? (1.0f / blended_ior) : blended_ior;
                    Vec3 unit_direction = vec3_normalize(ray_in->direction);
                    Vec3 direction;
                    Vec3 refracted;

                    if (vec3_refract(unit_direction, rec->normal, refraction_ratio, &refracted)) {
                        float cos_theta = fminf(-vec3_dot(unit_direction, rec->normal), 1.0f);
                        if (schlick(cos_theta, refraction_ratio) > rng_float(rng)) {
                            direction = vec3_reflect(unit_direction, rec->normal);
                        } else {
                            direction = refracted;
                        }
                    } else {
                        direction = vec3_reflect(unit_direction, rec->normal);
                    }

                    *scattered = ray_create(rec->point, direction);
                    return true;
                }

                default:
                    return false;
            }
        }

        default:
            return false;
    }
}