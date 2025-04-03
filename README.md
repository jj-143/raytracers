# Raytracers

A collection of ray tracer implementations in C++

![SimplePathtracer][simple_r20]

## Overview

This is a renderer featuring 3 ray tracer implementations based on popular
learning materials, to compare and understand different approaches to ray
tracing.

To replicate the Diffuse and Roughness parameters of Blender's
[Principled BSDF][], `GlossyDiffuseLambertBSDF` was implemented based on
[Glossy-diffuse slab][] from [OpenPBR Surface][].

## Features

- Mesh
  - Plane, Sphere
- Material
  - Emission, PhongMaterial, LambertBSDF, MetalBSDF(0% roughness),
    DielectricBSDF(0% roughness), GlossyDiffuseLambertBSDF(thin dielectric
    microfacet coat + Lambertian substrate, GGX Distribution)
- Light
  - AreaLight
- Sampler
  - RandomSampler, StratefiedSampler
- Integrator
  - _WhittedRaytracer_ - Whitted-style recursive ray tracer, based on
    _[tinyraytracer][]_.
    Uses PhongMaterial and PointLight.
  - _RecursivePathtracer_ - Path tracer using Monte Carlo with mixed sampling,
    based on _Ray Tracing: The Rest of Your Life_ from
    _[Ray Tracing in One Weekend][]_ series. Uses BSDF Material and AreaLight.
  - _SimplePathtracer_ - Path tracer using Monte Carlo with direct light
    sampling, a simplified version of _SimplePathIntegrator_ from
    _[Physically Based Rendering: From Theory To Implementation][pbrt]_.
    Most of the features in the literature, such as spectral rendering, were
    omitted for simplicity.
    RGB rendering (as _RecursivePathtacer_), Material, and AreaLight.

## Development

### Requirements

- Make
- CMake

### Build

#### Linux

```bash
mkdir build
cd build
cmake ..
make
```

#### Windows, tested on MinGW (UCRT)

```bash
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make.exe
```

Or, use VS Code with CMake Tools extension (MinGW Kit).

### Usage

```
raytracers [options] <example>

Positional arguments:
  example            "whitted", "recursive", "simple" (roughness=0.2),
                     "simple0" (roughness=0), "simple10" (roughness=0.1)
                     [default: "simple"]

Optional arguments:
  --spp <spp>        number of samples per pixel [default: 10]
  --width <width>    width of rendered image [default: 512]
  --height <height>  height of rendered image [default: 512]
  --seed <seed>      seed for RNG [default: 0]
```

## Render Samples - 20% Roughness

[Cornell box data][cornell_box_data]

[Blender file][]

**WhittedRaytracer, 4 samples**

![whitted][]

**RecursivePathtracer, 1000 samples**

![recursive_r20][]

**SimplePathtracer, 1000 samples**

![simple_r20][]

**Blender Cycles (reference), 1000 samples**

![cycles_r20][]

[Principled BSDF]: https://docs.blender.org/manual/en/4.3/render/shader_nodes/shader/principled.html
[Glossy-diffuse slab]: https://academysoftwarefoundation.github.io/OpenPBR/#model/basesubstrate/glossy-diffuse
[OpenPBR Surface]: https://academysoftwarefoundation.github.io/OpenPBR/
[tinyraytracer]: https://github.com/ssloy/tinyraytracer
[Ray Tracing in One Weekend]: https://raytracing.github.io/
[pbrt]: https://www.pbrt.org/
[cornell_box_data]: https://www.graphics.cornell.edu/online/box/data.html

<!-- Assets -->

[Blender file]: Reference.blend
[whitted]: docs/whitted_raytracer_4spp.png
[recursive_r20]: docs/recursive_pathtracer_1000spp.png
[simple_r20]: docs/simple_pathtracer_r20_1000spp.png
[cycles_r20]: docs/cycles_r20_1000spp.png
