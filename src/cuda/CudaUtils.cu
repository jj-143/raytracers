#include <iostream>

#include "CudaUtils.h"

void checkCuda(cudaError_t result, char const* const func,
               const char* const file, int const line) {
  if (result) {
    std::cerr << "CUDA error = " << static_cast<unsigned int>(result) << " at "
              << file << ":" << line << " '" << func << "' \n";
    // Make sure we call CUDA Device Reset before exiting
    cudaDeviceReset();
    exit(99);
  }
}

float* createFramebuffer(int width, int height) {
  float* framebuffer;
  size_t size = 3 * width * height * sizeof(float);
  checkCudaErrors(cudaMalloc(&framebuffer, size));
  return framebuffer;
}

float* copyFramebufferToHost(float* deviceFB, int width, int height) {
  float* hostFB = new float[3 * width * height];
  size_t size = 3 * width * height * sizeof(float);
  checkCudaErrors(cudaMemcpy(hostFB, deviceFB, size, cudaMemcpyDeviceToHost));
  return hostFB;
}
