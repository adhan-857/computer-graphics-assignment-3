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

    return false; // Ganti dengan implementasi yang benar
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