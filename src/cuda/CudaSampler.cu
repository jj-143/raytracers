#include "CudaSampler.h"
#include "raytracers.h"

namespace {
RT_DEVICE CudaSampler sampler;
}

namespace cuda_sampler {

RT_DEVICE __noinline__ float Get1D() { return sampler.Get1D(); }
RT_DEVICE __noinline__ Vec2f Get2D() { return sampler.Get2D(); }

RT_GLOBAL void InitSamplerRand() { sampler.initRand(); }

void InitSampler(int seed, int width, int height) {
  CudaSampler temp;
  temp.init(seed, width, height);
  cudaMemcpyToSymbol(sampler, &temp, sizeof(CudaSampler));
}

void DestroySampler() {
  CudaSampler temp;
  cudaMemcpyFromSymbol(&temp, sampler, sizeof(CudaSampler));
  temp.free();
}

}  // namespace cuda_sampler
