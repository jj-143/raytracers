#pragma once

#include <curand_kernel.h>

#include "CudaUtils.h"
#include "raytracers.h"

class CudaSampler {
 public:
  void init(int seed, int width, int height) {
    this->seed = seed;
    this->width = width;
    this->height = height;
    checkCudaErrors(
        cudaMalloc((void**)&states, width * height * sizeof(curandState)));
  }

  void free() {
    if (states) cudaFree(states);
  }

  RT_DEVICE float Get1D() const { return uniform(); }

  RT_DEVICE Vec2f Get2D() const { return {uniform(), uniform()}; }

  RT_DEVICE void initRand() {
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    int j = threadIdx.y + blockIdx.y * blockDim.y;

    // Bound check
    if (i >= width || j >= height) return;

    int pixelIndex = j * width + i;

    curand_init(seed, pixelIndex, 0, &states[pixelIndex]);
  }

 private:
  int width, height, seed;
  curandState* states;

  RT_DEVICE float uniform() const {
    curandState* state = &states[PixelIndex()];
    return curand_uniform(state);
  }

  RT_DEVICE int PixelIndex() const {
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    int j = threadIdx.y + blockIdx.y * blockDim.y;
    return j * width + i;
  }
};

namespace cuda_sampler {

RT_DEVICE __noinline__ float Get1D();
RT_DEVICE __noinline__ Vec2f Get2D();

RT_GLOBAL void InitSamplerRand();

void InitSampler(int seed, int width, int height);

void DestroySampler();

}  // namespace cuda_sampler
