#include "primitive.h"
#include <math.h>

// Ray-sphere intersection
bool sphere_hit(const Sphere* sphere, const Ray* ray, float t_min, float t_max,
                HitRecord* rec) {
    // TODO: Implementasi ray-sphere intersection menggunakan persamaan kuadrat
    // Hint:
    // 1. Hitung vector dari ray origin ke sphere center: oc = ray->origin - sphere->center
    // 2. Setup persamaan kuadrat at² + bt + c = 0
    //    a = dot(ray->direction, ray->direction)
    //    half_b = dot(oc, ray->direction)
    //    c = dot(oc, oc) - radius²
    // 3. Hitung discriminant = half_b² - ac
    // 4. Jika discriminant < 0, tidak ada intersection (return false)
    // 5. Hitung kedua root: t = (-half_b ± √discriminant) / a
    // 6. Pilih root terdekat yang berada dalam range [t_min, t_max]
    // 7. Isi HitRecord:
    //    - rec->t = root yang valid
    //    - rec->point = ray_at(*ray, rec->t)
    //    - rec->normal = normalized(point - center)
    //    - rec->front_face = dot(ray->direction, normal) < 0

    // Vector dari origin ke center
    Vec3 oc = vec3_sub(ray->origin, sphere->center);
    
    // Koefisien persamaan kuadrat
    float a = vec3_dot(ray->direction, ray->direction);
    float half_b = vec3_dot(oc, ray->direction);
    float c = vec3_dot(oc, oc) - sphere->radius * sphere->radius;
    
    // Hitung diskriminan
    float discriminant = half_b * half_b - a * c;
    if (discriminant < 0.0f) {
        return false;
    }
    
    float sqrtd = sqrtf(discriminant);
    
    // Cari root terdekat dalam range [t_min, t_max]
    float root = (-half_b - sqrtd) / a;
    if (root < t_min || root > t_max) {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || root > t_max) {
            return false;
        }
    }
    
    // Isi hit record
    rec->t = root;
    rec->point = ray_at(*ray, rec->t);
    Vec3 outward_normal = vec3_div(vec3_sub(rec->point, sphere->center), sphere->radius);
    
    // Tentukan front face berdasarkan arah ray dan normal
    rec->front_face = vec3_dot(ray->direction, outward_normal) < 0.0f;
    rec->normal = rec->front_face ? outward_normal : vec3_scale(outward_normal, -1.0f);
    
    return true;
}

// Ray-triangle intersection (Möller-Trumbore algorithm)
bool triangle_hit(const Triangle* triangle, const Ray* ray, float t_min, float t_max,
                  HitRecord* rec) {
    // TODO: Implementasi Möller-Trumbore algorithm untuk ray-triangle intersection
    // Hint:
    // 1. Hitung edge vectors: edge1 = v1 - v0, edge2 = v2 - v0
    // 2. Hitung h = cross(ray->direction, edge2)
    // 3. Hitung a = dot(edge1, h)
    // 4. Jika |a| < EPSILON, ray parallel dengan triangle (return false)
    // 5. Hitung f = 1/a
    // 6. Hitung s = ray->origin - v0
    // 7. Hitung u = f * dot(s, h)
    // 8. Cek jika u < 0 atau u > 1 (di luar triangle)
    // 9. Hitung q = cross(s, edge1)
    // 10. Hitung v = f * dot(ray->direction, q)
    // 11. Cek jika v < 0 atau u + v > 1 (di luar triangle)
    // 12. Hitung t = f * dot(edge2, q)
    // 13. Cek jika t dalam range [t_min, t_max]
    // 14. Isi HitRecord dengan t, point, dan normal

    const float EPSILON = 0.0000001f;

    // TODO: Implementasi algoritma di sini

    return false; // Ganti dengan implementasi yang benar
}

// Generic primitive hit test
bool primitive_hit(const Primitive* prim, const Ray* ray, float t_min, float t_max,
                   HitRecord* rec) {
    bool hit = false;

    switch (prim->type) {
        case PRIMITIVE_SPHERE:
            hit = sphere_hit(&prim->sphere, ray, t_min, t_max, rec);
            break;
        case PRIMITIVE_TRIANGLE:
            hit = triangle_hit(&prim->triangle, ray, t_min, t_max, rec);
            break;
        default:
            return false;
    }

    if (hit) {
        rec->material = &prim->material;
    }

    return hit;
}