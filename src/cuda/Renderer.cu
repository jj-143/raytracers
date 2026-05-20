#include <curand_kernel.h>

#include "CudaSampler.h"
#include "CudaUtils.h"
#include "Renderer.h"
#include "data.h"
#include "examples.h"
#include "math.h"
#include "raytracers.h"

RT_GLOBAL void color(float* framebuffer, int spp, int width, int height,
                     Scene* scene) {
  int i = threadIdx.x + blockIdx.x * blockDim.x;
  int j = threadIdx.y + blockIdx.y * blockDim.y;

  // Bound check
  if (i >= width || j >= height) return;

  int idx = j * width * 3 + i * 3;

  Vec3f color{(float)i / width, (float)j / height, 0};

  framebuffer[idx] = color[0];
  framebuffer[idx + 1] = color[1];
  framebuffer[idx + 2] = color[2];
}

void renderWithGPU(const example::Project& project) {
  int tx = 8;
  int ty = 8;

  int width = project.config.width;
  int height = project.config.height;
  int spp = project.config.spp;

  dim3 blocks(width / tx + 1, height / ty + 1);  // (65, 65) for 512x512
  dim3 threads(tx, ty);                          // (8, 8)

  float* framebuffer = createFramebuffer(width, height);

  // Load Scene & Upload
  data::UploadScene(project.scene.get());

  // Init Rand
  cuda_sampler::InitSampler(0, width, height);
  cuda_sampler::InitSamplerRand<<<blocks, threads>>>();
  checkCudaErrors(cudaGetLastError());
  checkCudaErrors(cudaDeviceSynchronize());

  color<<<blocks, threads>>>(framebuffer, spp, width, height,
                             data::GetGPUScene());
  checkCudaErrors(cudaGetLastError());
  checkCudaErrors(cudaDeviceSynchronize());

  // Save render & Clean up
  cuda_sampler::DestroySampler();
  float* hostFB = copyFramebufferToHost(framebuffer, width, height);
  checkCudaErrors(cudaFree(framebuffer));

  example::saveRender(hostFB, project);

  free(hostFB);
}
