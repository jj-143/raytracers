#include <curand_kernel.h>

#include "CudaSampler.h"
#include "CudaUtils.h"
#include "Renderer.h"
#include "Sampler.h"
#include "Tracer.h"
#include "data.h"
#include "examples.h"
#include "math.h"
#include "raytracers.h"

RT_GLOBAL void color(float* framebuffer, int spp, int width, int height,
                     Scene* scene, Tracer::Type tracerType) {
  int i = threadIdx.x + blockIdx.x * blockDim.x;
  int j = threadIdx.y + blockIdx.y * blockDim.y;

  // Bound check
  if (i >= width || j >= height) return;

  Tracer tracer = Tracer::Create(tracerType);

  float halfFov = scene->camera.fov / 2.0f;
  float pixelSize = tan(halfFov) / width;

  Vec3f origin = scene->camera.pos;
  Vec3f color{0};

  for (int iSample = 0; iSample < spp; ++iSample) {
    float x = (2 * (i + 0.5) / (float)width - 1) * tan(halfFov);
    float y = -(2 * (j + 0.5) / (float)height - 1) * tan(halfFov) * height /
              (float)width;

    Vec2f uv = Sampler::Get2D();
    x += (uv.x - 0.5f) * pixelSize;
    y += (uv.y - 0.5f) * pixelSize;

    Vec3f dir = Vec3f(x, y, -1).normalize();

    color += tracer.trace(scene, origin, dir);
  }

  color *= (1.0f / spp);

  int idx = j * width * 3 + i * 3;

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

  // Prepare data to GPU
  data::UploadScene(project.scene.get());

  // Init Rand
  cuda_sampler::InitSampler(0, width, height);
  cuda_sampler::InitSamplerRand<<<blocks, threads>>>();
  checkCudaErrors(cudaGetLastError());
  checkCudaErrors(cudaDeviceSynchronize());

  // Render pixels
  int64_t now = nowMillis();
  color<<<blocks, threads>>>(framebuffer, spp, width, height,
                             data::GetGPUScene(), project.config.tracerType);
  checkCudaErrors(cudaGetLastError());
  checkCudaErrors(cudaDeviceSynchronize());

  // Report
  int64_t duration = nowMillis() - now;
  std::cout << "Render took " << duration << "ms" << std::endl;

  // Save render & Clean up
  cuda_sampler::DestroySampler();
  float* hostFB = copyFramebufferToHost(framebuffer, width, height);
  checkCudaErrors(cudaFree(framebuffer));

  gammaEncode(hostFB, width, height, 2.2);

  example::saveRender(hostFB, project);

  free(hostFB);
}
