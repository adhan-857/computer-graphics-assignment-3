#ifndef CAMERA_H
#define CAMERA_H

#include "vec3.h"
#include "ray.h"
#include "random.h"
#include <math.h>

typedef struct {
    Vec3 origin;
    Vec3 lower_left_corner;
    Vec3 horizontal;
    Vec3 vertical;
    Vec3 u, v, w;
    float lens_radius;
} Camera;

static inline Camera camera_create(
    Vec3 lookfrom,
    Vec3 lookat,
    Vec3 vup,
    float vfov,  // Vertical field-of-view in degrees
    float aspect,
    float aperture,
    float focus_dist
) {
    Camera cam;

    float theta = vfov * M_PI / 180.0f;
    float half_height = tanf(theta / 2.0f);
    float half_width = aspect * half_height;

    cam.w = vec3_normalize(vec3_sub(lookfrom, lookat));
    cam.u = vec3_normalize(vec3_cross(vup, cam.w));
    cam.v = vec3_cross(cam.w, cam.u);

    cam.origin = lookfrom;
    cam.lower_left_corner = vec3_sub(
        vec3_sub(
            vec3_sub(cam.origin, vec3_scale(cam.u, half_width * focus_dist)),
            vec3_scale(cam.v, half_height * focus_dist)
        ),
        vec3_scale(cam.w, focus_dist)
    );

    cam.horizontal = vec3_scale(cam.u, 2.0f * half_width * focus_dist);
    cam.vertical = vec3_scale(cam.v, 2.0f * half_height * focus_dist);
    cam.lens_radius = aperture / 2.0f;

    return cam;
}

static inline Ray camera_get_ray(const Camera* cam, float s, float t, RNG* rng) {
    Vec3 rd = vec3_scale(rng_in_unit_disk(rng), cam->lens_radius);
    Vec3 offset = vec3_add(vec3_scale(cam->u, rd.x), vec3_scale(cam->v, rd.y));

    Vec3 ray_origin = vec3_add(cam->origin, offset);
    Vec3 ray_target = vec3_add(
        vec3_add(cam->lower_left_corner, vec3_scale(cam->horizontal, s)),
        vec3_scale(cam->vertical, t)
    );
    Vec3 ray_direction = vec3_sub(ray_target, ray_origin);

    return ray_create(ray_origin, ray_direction);
}

#endif // CAMERA_H