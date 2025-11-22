# C Path Tracer

A physically-based path tracer written in C with GTK3 GUI, featuring advanced material systems, BVH acceleration, and multi-threaded rendering.

## Features

### Rendering
- **Path Tracing**: Physically-based rendering with global illumination
- **Multi-threading**: OpenMP parallelization for fast rendering
- **BVH Acceleration**: Bounding Volume Hierarchy for efficient ray-object intersection
- **ACES Tone Mapping**: Hollywood-grade tone mapping for HDR to LDR conversion
- **Adaptive Sampling**: Configurable samples per pixel (1-10000)
- **Max Depth Control**: Adjustable ray bounce depth (1-100)

### Materials
- **Lambertian**: Diffuse materials with configurable albedo
- **Metal**: Reflective materials with roughness control
- **Dielectric**: Glass/refractive materials with configurable IOR
- **Emissive**: Light-emitting materials with adjustable brightness
- **Material Blending**: Position-based gradient blending between two material types
  - Vertical blend (Y-axis gradient)
  - Horizontal blend (X-axis gradient)
  - Radial blend (distance-based gradient)

### Scenes
1. **Cornell Box**: Classic Cornell Box with colored walls
2. **Random Spheres**: Large field of randomly placed spheres with various materials
3. **Glass Spheres**: 7x7 grid of glass spheres with colored background
4. **Metal Spheres**: Showcase of reflective metals (chrome, silver, gold, copper)
5. **Studio Lighting**: HDR lighting demonstration with glass and metal materials
6. **Material Blending**: Gradient material showcase using vec3_lerp

### User Interface
- **GTK3 GUI**: Native Linux GUI with real-time controls
- **Scene Selection**: Dropdown menu for quick scene switching
- **Render Settings**: Adjustable resolution, samples, and max depth
- **Progress Tracking**: Real-time progress bar during rendering
- **Image Export**: Save rendered images as BMP files

## Requirements

### Build Dependencies
- GCC (C11 support)
- GNU Make
- GTK3 development libraries
- OpenMP support
- Math library (libm)

### Ubuntu/Debian
```bash
sudo apt-get install build-essential libgtk-3-dev
```

### Fedora/RHEL
```bash
sudo dnf install gcc make gtk3-devel
```

### Arch Linux
```bash
sudo pacman -S base-devel gtk3
```

## Building

```bash
make
```

**Build output**: `pathtracer_gui`

### Clean build
```bash
make clean
make
```

### Release build
```bash
make release
```

## Usage

### Running the application
```bash
./pathtracer_gui
```

### GUI Controls
1. **Scene**: Select from 6 pre-configured scenes
2. **Width/Height**: Set output image resolution (default: 800x600)
3. **Samples**: Samples per pixel for anti-aliasing (1-10000)
4. **Max Depth**: Maximum ray bounce depth (1-100)
5. **Render**: Start rendering the selected scene
6. **Save Image**: Save the rendered image as BMP

## Scene Details

### Cornell Box
Classic Cornell Box with:
- Red left wall, green right wall
- White ceiling, floor, and back wall
- Two tall boxes with different materials
- Single area light on ceiling

### Random Spheres
Large-scale scene featuring:
- Randomly distributed small spheres
- Three large hero spheres (glass, diffuse, metal)
- Sky gradient ambient lighting
- Wide camera angle

### Glass Spheres
Refraction showcase with:
- 7x7 grid of glass spheres (49 spheres)
- Gold metal sphere at center
- Large colored background spheres (red, green, blue)
- Dual area lights
- Dark ground for contrast

### Metal Spheres
Reflective material showcase:
- 5 metal spheres: chrome, silver, gold, polished copper, brushed copper
- Small colored diffuse spheres in background
- Dual overhead lighting
- Dark ground for emphasis


### Studio Lighting
HDR lighting demonstration:
- Glass sphere (center), gold metal (left), chrome metal (right)
- Three emissive lights at different brightness levels (3�, 10�, 30�)
- Small accent lights
- Dark ambient for dramatic lighting


### Material Blending
Gradient material showcase:
- 6 spheres demonstrating vertical, horizontal, and radial blending
- Smooth transitions between diffuse and metallic materials
- Color gradients using vec3_lerp
- Dual overhead area lights


## Technical Details

### Architecture
```
c_pathtracer/
 include/          # Header files
   camera.h      # Camera with configurable FOV
   material.h    # Material system
   pathtracer.h  # Core rendering functions
   primitive.h   # Sphere primitives
   random.h      # RNG utilities
   ray.h         # Ray structure
   scenes.h      # Scene creation functions
   stb.h         # BMP image writer
   vec3.h        # 3D vector math
 src/              # Implementation files
   bvh.c         # BVH construction and traversal
   gui.c         # GTK3 GUI implementation
   main_gui.c    # Application entry point
   material.c    # Material scattering logic
   pathtracer.c  # Path tracing renderer
   primitive.c   # Ray-sphere intersection
   scenes.c      # Scene definitions
 Makefile          # Build configuration
 README.md         # This file
```

### Rendering Pipeline
1. **Scene Setup**: Create scene with primitives and materials
2. **BVH Construction**: Build acceleration structure
3. **Camera Setup**: Configure camera with optimal viewpoint
4. **Ray Generation**: Generate primary rays for each pixel
5. **Path Tracing**: Recursively trace rays with material scattering
6. **Tone Mapping**: Apply ACES tone mapping to HDR colors
7. **Image Output**: Save to BMP file

### Material Scattering

#### Lambertian (Diffuse)
- Cosine-weighted hemisphere sampling
- Random scatter direction based on surface normal
- Color modulation by albedo

#### Metal (Reflective)
- Perfect reflection with optional roughness
- Fuzzy reflections via random perturbation
- Mirror-like at roughness 0.0, matte at 1.0

#### Dielectric (Glass)
- Snell's law refraction
- Total internal reflection
- Schlick approximation for Fresnel effect
- Configurable index of refraction (IOR)

#### Emissive (Light)
- No scattering (absorbs all rays)
- Contributes emission color to ray path
- Enables area lighting

#### Blend (Gradient)
- Linear interpolation between two materials
- Position-based blend factor (vertical/horizontal/radial)
- Smooth transitions using vec3_lerp
- Blends albedo, roughness, and IOR

## License

This project is provided as-is for educational purposes.
