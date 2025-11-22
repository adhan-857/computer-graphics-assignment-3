#include "pathtracer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <omp.h>
#include <float.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb.h"

// Progress callback
static progress_callback_t g_progress_callback = NULL;

void set_progress_callback(progress_callback_t callback) {
    g_progress_callback = callback;
}

// Scene creation and management
Scene* scene_create(void) {
    Scene* scene = (Scene*)calloc(1, sizeof(Scene));
    scene->prim_capacity = 128;
    scene->primitives = (Primitive*)malloc(scene->prim_capacity * sizeof(Primitive));
    scene->prim_count = 0;
    scene->bvh = NULL;
    scene->ambient_light = vec3_create(0.1f, 0.1f, 0.1f);
    return scene;
}

void scene_destroy(Scene* scene) {
    if (scene) {
        if (scene->bvh) {
            bvh_destroy(scene->bvh);
        }
        free(scene->primitives);
        free(scene);
    }
}

static void scene_grow_if_needed(Scene* scene) {
    if (scene->prim_count >= scene->prim_capacity) {
        scene->prim_capacity *= 2;
        scene->primitives = (Primitive*)realloc(scene->primitives,
                                                scene->prim_capacity * sizeof(Primitive));
    }
}

void scene_add_sphere(Scene* scene, Vec3 center, float radius, Material mat) {
    scene_grow_if_needed(scene);
    scene->primitives[scene->prim_count++] = primitive_sphere(center, radius, mat);
}

void scene_add_triangle(Scene* scene, Vec3 v0, Vec3 v1, Vec3 v2, Material mat) {
    scene_grow_if_needed(scene);
    scene->primitives[scene->prim_count++] = primitive_triangle(v0, v1, v2, mat);
}

void scene_build_bvh(Scene* scene) {
    if (scene->bvh) {
        bvh_destroy(scene->bvh);
    }
    scene->bvh = bvh_create(scene->primitives, scene->prim_count);
}

// Image management
Image* image_create(uint32_t width, uint32_t height) {
    Image* img = (Image*)malloc(sizeof(Image));
    img->width = width;
    img->height = height;
    img->pixels = (Vec3*)calloc(width * height, sizeof(Vec3));
    return img;
}

void image_destroy(Image* img) {
    if (img) {
        free(img->pixels);
        free(img);
    }
}

void image_save_bmp(const Image* img, const char* filename) {
    // Allocate RGB buffer (3 bytes per pixel)
    unsigned char* rgb_data = (unsigned char*)malloc(img->width * img->height * 3);
    if (!rgb_data) {
        fprintf(stderr, "Failed to allocate memory for BMP conversion\n");
        return;
    }

    // Convert Vec3 HDR pixels to RGB bytes
    for (uint32_t j = 0; j < img->height; j++) {
        for (uint32_t i = 0; i < img->width; i++) {
            Vec3 color = img->pixels[j * img->width + i];

            // ACES tone mapping and clamp
            color = aces_tonemap(color);
            color.x = fminf(fmaxf(color.x, 0.0f), 1.0f);
            color.y = fminf(fmaxf(color.y, 0.0f), 1.0f);
            color.z = fminf(fmaxf(color.z, 0.0f), 1.0f);

            // Convert to bytes
            int idx = (j * img->width + i) * 3;
            rgb_data[idx + 0] = (unsigned char)(color.x * 255.0f);
            rgb_data[idx + 1] = (unsigned char)(color.y * 255.0f);
            rgb_data[idx + 2] = (unsigned char)(color.z * 255.0f);
        }
    }

    // Write BMP file
    int result = stbi_write_bmp(filename, img->width, img->height, 3, rgb_data);

    if (!result) {
        fprintf(stderr, "Failed to write BMP file: %s\n", filename);
    }

    free(rgb_data);
}

// Utility functions
Vec3 aces_tonemap(Vec3 color) {
    // ACES Filmic Tone Mapping curve
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;

    color.x = (color.x * (a * color.x + b)) / (color.x * (c * color.x + d) + e);
    color.y = (color.y * (a * color.y + b)) / (color.y * (c * color.y + d) + e);
    color.z = (color.z * (a * color.z + b)) / (color.z * (c * color.z + d) + e);

    return color;
}

// Hit test for scene
static bool scene_hit(const Scene* scene, const Ray* ray, float t_min, float t_max,
                     HitRecord* rec) {
    if (scene->bvh) {
        return bvh_hit(scene->bvh, ray, t_min, t_max, rec);
    } else {
        // Brute force if no BVH
        bool hit_anything = false;
        float closest_so_far = t_max;

        for (uint32_t i = 0; i < scene->prim_count; i++) {
            if (primitive_hit(&scene->primitives[i], ray, t_min, closest_so_far, rec)) {
                hit_anything = true;
                closest_so_far = rec->t;
            }
        }

        return hit_anything;
    }
}

// Main path tracing function
Vec3 trace_ray(const Scene* scene, const Ray* ray, RNG* rng,
               uint32_t depth, uint32_t max_depth) {
    // TODO: Implement Russian roulette untuk early termination
    // Hint: Gunakan probabilitas untuk menghentikan ray setelah depth tertentu
    // - Jika depth >= max_depth, pertimbangkan untuk terminate
    // - Gunakan rng_float(rng) untuk generate random number [0,1]
    // - Return vec3_create(0, 0, 0) jika terminate

    // Russian roulette untuk early termination
    const uint32_t RR_START_DEPTH = 3;
    float p_continue = 1.0f;

    if (depth >= max_depth) {
        return vec3_create(0, 0, 0);
    }

    // Terapkan Russian Roulette setelah beberapa kali memantul
    if (depth >= RR_START_DEPTH) {
        p_continue = 0.95f; 
        if (rng_float(rng) > p_continue) {
            return vec3_create(0, 0, 0);
        }
    }

    HitRecord rec;

    // Test intersection dengan scene
    if (!scene_hit(scene, ray, 0.001f, FLT_MAX, &rec)) {
        // TODO: Return warna background/sky
        // Hint: Gunakan scene->ambient_light
        return scene->ambient_light;
    }

    // TODO: Handle material emissive (light source)
    // Hint:
    // - Cek apakah rec.material->type == MATERIAL_EMISSIVE
    // - Jika ya, return rec.material->emission

    // TODO: Scatter ray berdasarkan material
    // Hint:
    // - Deklarasikan Vec3 attenuation dan Ray scattered
    // - Panggil material_scatter() untuk mendapatkan scattered ray
    // - Jika scatter gagal (return false), return vec3_create(0, 0, 0)

    // Handle material emissive (light source)
    if (rec.material->type == MATERIAL_EMISSIVE) {
        return rec.material->emission;
    }

    Vec3 attenuation;
    Ray scattered;

    // TODO: Implementasi scatter dan recursive trace
    // Hint:
    // - Gunakan material_scatter(rec.material, ray, &rec, &attenuation, &scattered, rng)
    // - Jika berhasil, lakukan recursive trace dengan trace_ray()
    // - Kalikan hasil recursive dengan attenuation menggunakan vec3_mul()

    // Scatter ray berdasarkan material
    if (material_scatter(rec.material, ray, &rec, &attenuation, &scattered, rng)) {
        // Recursive trace
        Vec3 incoming = trace_ray(scene, &scattered, rng, depth + 1, max_depth);
        
        // Kalikan hasil recursive dengan attenuation
        Vec3 scattered_light = vec3_mul(attenuation, incoming);
        
        // Aplikasikan RR compensation
        if (depth >= RR_START_DEPTH) {
            scattered_light = vec3_scale(scattered_light, 1.0f / p_continue);
        }
        
        // Menambahkan emission
        return vec3_add(rec.material->emission, scattered_light);
    }

    // Jika tidak ada scatter (absorbed), return emission
    return rec.material->emission;
}

// Multi-threaded rendering with OpenMP
void render_parallel(const Scene* scene, const Camera* camera,
                    const RenderSettings* settings, Image* output) {
    uint32_t total_pixels = output->width * output->height;

    // Set number of threads
    omp_set_num_threads(settings->num_threads);

    // Shared counter for progress tracking
    uint32_t pixels_done = 0;

    #pragma omp parallel
    {
        RNG rng;
        rng_init(&rng, 42 + omp_get_thread_num() * 1000);

        #pragma omp for schedule(dynamic, 16) nowait
        for (uint32_t pixel_idx = 0; pixel_idx < total_pixels; pixel_idx++) {
            // Check cancel flag early - skip processing if cancelled
            if (settings->cancel_flag && *settings->cancel_flag) {
                continue;  // Skip this pixel
            }

            uint32_t i = pixel_idx % output->width;
            uint32_t j = pixel_idx / output->width;

            Vec3 color = vec3_create(0, 0, 0);

            // Multi-sampling
            for (uint32_t s = 0; s < settings->samples_per_pixel; s++) {
                // Check cancel during multi-sampling too
                if (settings->cancel_flag && *settings->cancel_flag) {
                    break;  // OK to break from inner loop
                }

                float u = (i + rng_float(&rng)) / (float)(output->width - 1);
                float v = (j + rng_float(&rng)) / (float)(output->height - 1);

                // Flip v for correct orientation
                v = 1.0f - v;

                Ray ray = camera_get_ray(camera, u, v, &rng);
                Vec3 sample_color = trace_ray(scene, &ray, &rng, 0, settings->max_depth);
                color = vec3_add(color, sample_color);
            }

            // Average samples
            color = vec3_div(color, (float)settings->samples_per_pixel);
            output->pixels[pixel_idx] = color;

            // Update progress every pixel (with atomic increment for thread safety)
            if (g_progress_callback) {
                uint32_t current_done;
                #pragma omp atomic capture
                current_done = ++pixels_done;

                // Only call callback every 1000 pixels to reduce overhead
                if (current_done % 1000 == 0) {
                    #pragma omp critical
                    {
                        g_progress_callback((float)current_done / total_pixels);
                    }
                }
            }
        }
    }
}