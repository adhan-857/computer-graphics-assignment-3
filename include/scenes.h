#ifndef SCENES_H
#define SCENES_H

#include "pathtracer.h"

// Scene creation functions
Scene* create_cornell_box(void);
Scene* create_random_spheres(void);
Scene* create_glass_spheres(void);  // Glass spheres with different materials
Scene* create_metal_spheres(void);  // Metal spheres showcase with reflections
Scene* create_studio_lighting(void);  // Studio lighting scene with glass and metal materials
Scene* create_material_blend(void);  // Material blending showcase with gradient materials

#endif // SCENES_H