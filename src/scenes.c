#include "scenes.h"
#include "random.h"
#include <math.h>

// Create Cornell Box scene
Scene* create_cornell_box(void) {
    Scene* scene = scene_create();

    // Materials
    Material white = material_lambertian(vec3_create(0.73f, 0.73f, 0.73f));
    Material red = material_lambertian(vec3_create(0.65f, 0.05f, 0.05f));
    Material green = material_lambertian(vec3_create(0.12f, 0.45f, 0.15f));
    Material light = material_emissive(vec3_scale(vec3_create(1.0f, 1.0f, 1.0f), 15.0f));
    Material glass = material_dielectric(1.5f);
    Material metal = material_metal(vec3_create(0.7f, 0.6f, 0.5f), 0.0f);

    float size = 555.0f;

    // Walls (as two triangles each)
    // Floor
    scene_add_triangle(scene,
        vec3_create(0, 0, 0), vec3_create(size, 0, 0), vec3_create(size, 0, size),
        white);
    scene_add_triangle(scene,
        vec3_create(0, 0, 0), vec3_create(size, 0, size), vec3_create(0, 0, size),
        white);

    // Ceiling
    scene_add_triangle(scene,
        vec3_create(0, size, 0), vec3_create(size, size, size), vec3_create(size, size, 0),
        white);
    scene_add_triangle(scene,
        vec3_create(0, size, 0), vec3_create(0, size, size), vec3_create(size, size, size),
        white);

    // Back wall
    scene_add_triangle(scene,
        vec3_create(0, 0, size), vec3_create(size, 0, size), vec3_create(size, size, size),
        white);
    scene_add_triangle(scene,
        vec3_create(0, 0, size), vec3_create(size, size, size), vec3_create(0, size, size),
        white);

    // Left wall (green)
    scene_add_triangle(scene,
        vec3_create(0, 0, 0), vec3_create(0, 0, size), vec3_create(0, size, size),
        green);
    scene_add_triangle(scene,
        vec3_create(0, 0, 0), vec3_create(0, size, size), vec3_create(0, size, 0),
        green);

    // Right wall (red)
    scene_add_triangle(scene,
        vec3_create(size, 0, 0), vec3_create(size, size, 0), vec3_create(size, size, size),
        red);
    scene_add_triangle(scene,
        vec3_create(size, 0, 0), vec3_create(size, size, size), vec3_create(size, 0, size),
        red);

    // Light (smaller rectangle in the ceiling)
    float light_size = 130.0f;
    float light_x0 = (size - light_size) / 2.0f;
    float light_x1 = light_x0 + light_size;
    float light_z0 = (size - light_size) / 2.0f;
    float light_z1 = light_z0 + light_size;
    float light_y = size - 0.01f;

    scene_add_triangle(scene,
        vec3_create(light_x0, light_y, light_z0),
        vec3_create(light_x1, light_y, light_z0),
        vec3_create(light_x1, light_y, light_z1),
        light);
    scene_add_triangle(scene,
        vec3_create(light_x0, light_y, light_z0),
        vec3_create(light_x1, light_y, light_z1),
        vec3_create(light_x0, light_y, light_z1),
        light);

    // Add spheres
    scene_add_sphere(scene, vec3_create(185, 100, 185), 100, glass);
    scene_add_sphere(scene, vec3_create(370, 80, 370), 80, metal);

    // Set ambient light to zero for Cornell Box
    scene->ambient_light = vec3_create(0.0f, 0.0f, 0.0f);

    return scene;
}

// Create random spheres scene
Scene* create_random_spheres(void) {
    Scene* scene = scene_create();
    RNG rng;
    rng_init(&rng, 42);

    // Ground
    scene_add_sphere(scene, vec3_create(0, -1000, 0), 1000,
                    material_lambertian(vec3_create(0.5f, 0.5f, 0.5f)));

    // Random small spheres
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = rng_float(&rng);
            Vec3 center = vec3_create(a + 0.9f * rng_float(&rng), 0.2f, b + 0.9f * rng_float(&rng));

            if (vec3_length(vec3_sub(center, vec3_create(4, 0.2f, 0))) > 0.9f) {
                Material mat;

                if (choose_mat < 0.8f) {
                    // Diffuse
                    Vec3 albedo = vec3_mul(
                        vec3_create(rng_float(&rng), rng_float(&rng), rng_float(&rng)),
                        vec3_create(rng_float(&rng), rng_float(&rng), rng_float(&rng))
                    );
                    mat = material_lambertian(albedo);
                } else if (choose_mat < 0.95f) {
                    // Metal
                    Vec3 albedo = vec3_create(
                        0.5f * (1 + rng_float(&rng)),
                        0.5f * (1 + rng_float(&rng)),
                        0.5f * (1 + rng_float(&rng))
                    );
                    float roughness = 0.5f * rng_float(&rng);
                    mat = material_metal(albedo, roughness);
                } else {
                    // Glass
                    mat = material_dielectric(1.5f);
                }

                scene_add_sphere(scene, center, 0.2f, mat);
            }
        }
    }

    // Three large spheres
    scene_add_sphere(scene, vec3_create(0, 1, 0), 1.0f, material_dielectric(1.5f));
    scene_add_sphere(scene, vec3_create(-4, 1, 0), 1.0f,
                    material_lambertian(vec3_create(0.4f, 0.2f, 0.1f)));
    scene_add_sphere(scene, vec3_create(4, 1, 0), 1.0f,
                    material_metal(vec3_create(0.7f, 0.6f, 0.5f), 0.0f));

    // Sky light
    scene->ambient_light = vec3_create(0.7f, 0.8f, 1.0f);

    return scene;
}

// Create glass spheres scene
Scene* create_glass_spheres(void) {
    Scene* scene = scene_create();

    // Ground - much darker for better glass visibility
    scene_add_sphere(scene, vec3_create(0, -1000, 0), 1000,
                    material_lambertian(vec3_create(0.2f, 0.2f, 0.25f)));

    // Add large colored spheres in far background for interesting glass refraction
    scene_add_sphere(scene, vec3_create(0, 3, -15), 3.0f,
                    material_lambertian(vec3_create(0.9f, 0.2f, 0.2f)));  // Red
    scene_add_sphere(scene, vec3_create(10, 3, -15), 3.0f,
                    material_lambertian(vec3_create(0.2f, 0.9f, 0.2f)));  // Green
    scene_add_sphere(scene, vec3_create(10, 3, -5), 3.0f,
                    material_lambertian(vec3_create(0.2f, 0.4f, 0.9f)));  // Blue

    // Glass spheres in a grid
    for (int i = -3; i <= 3; i++) {
        for (int j = -3; j <= 3; j++) {
            if (i == 0 && j == 0) {
                // Center gold metal sphere for contrast
                scene_add_sphere(scene, vec3_create(i * 2.0f, 1.0f, j * 2.0f), 1.0f,
                                material_metal(vec3_create(1.0f, 0.85f, 0.3f), 0.1f));
            } else {
                // Glass spheres
                scene_add_sphere(scene, vec3_create(i * 2.0f, 1.0f, j * 2.0f), 1.0f,
                                material_dielectric(1.5f));
            }
        }
    }

    // Add bright area lights to illuminate the scene
    scene_add_sphere(scene, vec3_create(-8, 10, 0), 2.5f,
                    material_emissive(vec3_scale(vec3_create(1.0f, 0.95f, 0.9f), 15.0f)));
    scene_add_sphere(scene, vec3_create(8, 10, 0), 2.5f,
                    material_emissive(vec3_scale(vec3_create(0.9f, 0.95f, 1.0f), 15.0f)));

    // Darker ambient to emphasize glass effects
    scene->ambient_light = vec3_create(0.3f, 0.35f, 0.4f);

    return scene;
}

// Create metal spheres showcase scene
Scene* create_metal_spheres(void) {
    Scene* scene = scene_create();

    // Ground - darker to emphasize metal reflections
    scene_add_sphere(scene, vec3_create(0, -1000, 0), 1000,
                    material_lambertian(vec3_create(0.3f, 0.3f, 0.35f)));

    // Row 1: Chrome/Silver metals with low roughness (very reflective)
    scene_add_sphere(scene, vec3_create(-5, 1.0f, 0), 1.0f,
                    material_metal(vec3_create(0.95f, 0.95f, 0.95f), 0.0f));  // Perfect mirror chrome
    scene_add_sphere(scene, vec3_create(-2.5f, 1.0f, 0), 1.0f,
                    material_metal(vec3_create(0.9f, 0.9f, 0.95f), 0.05f));   // Slightly rough silver

    // Row 2: Gold metals
    scene_add_sphere(scene, vec3_create(0, 1.0f, 0), 1.0f,
                    material_metal(vec3_create(1.0f, 0.86f, 0.57f), 0.0f));   // Shiny gold

    // Row 3: Copper metals
    scene_add_sphere(scene, vec3_create(2.5f, 1.0f, 0), 1.0f,
                    material_metal(vec3_create(0.95f, 0.64f, 0.54f), 0.05f)); // Polished copper
    scene_add_sphere(scene, vec3_create(5, 1.0f, 0), 1.0f,
                    material_metal(vec3_create(0.9f, 0.7f, 0.6f), 0.1f));     // Brushed copper

    // Add some colored spheres in background for interesting reflections
    scene_add_sphere(scene, vec3_create(-3, 0.6f, -4), 0.6f,
                    material_lambertian(vec3_create(0.9f, 0.2f, 0.2f)));  // Red
    scene_add_sphere(scene, vec3_create(0, 0.6f, -4), 0.6f,
                    material_lambertian(vec3_create(0.2f, 0.9f, 0.2f)));  // Green
    scene_add_sphere(scene, vec3_create(3, 0.6f, -4), 0.6f,
                    material_lambertian(vec3_create(0.2f, 0.2f, 0.9f)));  // Blue

    // Stronger lighting for better reflections
    scene_add_sphere(scene, vec3_create(-5, 8, -3), 2.0f,
                    material_emissive(vec3_scale(vec3_create(1, 1, 1), 12.0f)));
    scene_add_sphere(scene, vec3_create(5, 8, -3), 2.0f,
                    material_emissive(vec3_scale(vec3_create(1, 1, 1), 12.0f)));

    // Brighter ambient for better visibility
    scene->ambient_light = vec3_create(0.5f, 0.55f, 0.6f);

    return scene;
}

// Create studio lighting scene with HDR range
// Features glass, metal materials with multiple emissive lights at different brightnesses
Scene* create_studio_lighting(void) {
    Scene* scene = scene_create();

    // Materials
    Material ground = material_lambertian(vec3_create(0.5f, 0.5f, 0.5f));
    Material glass = material_dielectric(1.5f);
    Material metal_gold = material_metal(vec3_create(1.0f, 0.85f, 0.57f), 0.1f);
    Material metal_chrome = material_metal(vec3_create(0.9f, 0.9f, 0.9f), 0.0f);

    // Create three emissive lights with different brightnesses for HDR showcase
    Material light_dim = material_emissive(vec3_scale(vec3_create(1.0f, 0.9f, 0.8f), 3.0f));    // Moderate
    Material light_bright = material_emissive(vec3_scale(vec3_create(1.0f, 0.7f, 0.3f), 10.0f)); // Bright
    Material light_very_bright = material_emissive(vec3_scale(vec3_create(1.0f, 1.0f, 1.0f), 30.0f)); // Very bright!

    // Ground plane (large sphere)
    scene_add_sphere(scene, vec3_create(0, -1000, 0), 1000, ground);

    // Central glass sphere - will show caustics and reflections
    scene_add_sphere(scene, vec3_create(0, 1, 0), 1.0f, glass);

    // Left: Gold metal sphere - will reflect lights
    scene_add_sphere(scene, vec3_create(-3, 1, 0), 1.0f, metal_gold);

    // Right: Chrome metal sphere - will reflect lights perfectly
    scene_add_sphere(scene, vec3_create(3, 1, 0), 1.0f, metal_chrome);

    // Back row: Three emissive spheres with different brightness levels

    // Dim light (background left)
    scene_add_sphere(scene, vec3_create(-3, 1.5f, -5), 1.0f, light_dim);

    // Bright light (background center)
    scene_add_sphere(scene, vec3_create(0, 2.0f, -5), 1.2f, light_bright);

    // Very bright light (background right)
    scene_add_sphere(scene, vec3_create(3, 1.5f, -5), 1.0f, light_very_bright);

    // Small bright accent lights for extra HDR detail
    scene_add_sphere(scene, vec3_create(-1.5f, 0.3f, 2), 0.3f, light_bright);
    scene_add_sphere(scene, vec3_create(1.5f, 0.3f, 2), 0.3f, light_very_bright);

    // Dark ambient to emphasize lights
    scene->ambient_light = vec3_create(0.02f, 0.02f, 0.03f);

    return scene;
}

// Create material blending showcase scene
// Features various blend modes and material combinations using vec3_lerp
Scene* create_material_blend(void) {
    Scene* scene = scene_create();

    // Ground plane - simple lambertian
    Material ground = material_lambertian(vec3_create(0.5f, 0.5f, 0.5f));
    scene_add_sphere(scene, vec3_create(0, -1000, 0), 1000, ground);

    // Center sphere: Vertical blend from red diffuse (bottom) to gold metal (top)
    Material center_blend = material_blend(
        MATERIAL_LAMBERTIAN, vec3_create(0.8f, 0.2f, 0.2f), 0.0f, 1.0f,  // Red diffuse
        MATERIAL_METAL, vec3_create(1.0f, 0.85f, 0.3f), 0.1f, 1.0f,      // Gold metal
        BLEND_VERTICAL, 0.0f, 2.0f  // Blend from Y=0 to Y=2
    );
    scene_add_sphere(scene, vec3_create(0, 1, 0), 1.0f, center_blend);

    // Left sphere: Vertical blend from green diffuse to chrome metal
    Material left_blend = material_blend(
        MATERIAL_LAMBERTIAN, vec3_create(0.2f, 0.8f, 0.2f), 0.0f, 1.0f,  // Green diffuse
        MATERIAL_METAL, vec3_create(0.9f, 0.9f, 0.9f), 0.0f, 1.0f,       // Chrome metal
        BLEND_VERTICAL, 0.0f, 2.0f
    );
    scene_add_sphere(scene, vec3_create(-2.5f, 1, 0), 1.0f, left_blend);

    // Right sphere: Vertical blend from blue diffuse to copper metal
    Material right_blend = material_blend(
        MATERIAL_LAMBERTIAN, vec3_create(0.2f, 0.4f, 0.8f), 0.0f, 1.0f,  // Blue diffuse
        MATERIAL_METAL, vec3_create(0.95f, 0.64f, 0.54f), 0.2f, 1.0f,    // Copper metal
        BLEND_VERTICAL, 0.0f, 2.0f
    );
    scene_add_sphere(scene, vec3_create(2.5f, 1, 0), 1.0f, right_blend);

    // Back left: Horizontal blend demonstration
    Material horizontal_blend = material_blend(
        MATERIAL_LAMBERTIAN, vec3_create(0.9f, 0.3f, 0.9f), 0.0f, 1.0f,  // Magenta diffuse
        MATERIAL_METAL, vec3_create(0.7f, 0.7f, 0.9f), 0.1f, 1.0f,       // Silver-blue metal
        BLEND_HORIZONTAL, -4.0f, -2.0f  // Blend from X=-4 to X=-2
    );
    scene_add_sphere(scene, vec3_create(-3, 0.7f, -2), 0.7f, horizontal_blend);

    // Back right: Radial blend from center
    Material radial_blend = material_blend(
        MATERIAL_METAL, vec3_create(1.0f, 0.95f, 0.8f), 0.0f, 1.0f,      // Bright metal center
        MATERIAL_LAMBERTIAN, vec3_create(0.3f, 0.2f, 0.1f), 0.0f, 1.0f,  // Dark diffuse outside
        BLEND_RADIAL, 0.0f, 4.0f  // Blend from distance 0 to 4
    );
    scene_add_sphere(scene, vec3_create(3, 0.7f, -2), 0.7f, radial_blend);

    // Small glass sphere in front for reference
    Material glass = material_dielectric(1.5f);
    scene_add_sphere(scene, vec3_create(0, 0.5f, 2), 0.5f, glass);

    // Lighting: Area light above (emissive sphere)
    Material light = material_emissive(vec3_scale(vec3_create(1.0f, 1.0f, 1.0f), 8.0f));
    scene_add_sphere(scene, vec3_create(-2, 5, -1), 1.5f, light);
    scene_add_sphere(scene, vec3_create(2, 5, -1), 1.5f, light);

    // Ambient light for subtle fill
    scene->ambient_light = vec3_create(0.3f, 0.35f, 0.4f);

    return scene;
}