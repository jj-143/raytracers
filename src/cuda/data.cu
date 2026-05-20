#include "Allocator.h"
#include "CudaUtils.h"
#include "Scene.h"
#include "data.h"

namespace {

Scene* devScene;

}  // namespace

namespace data {

Scene* GetGPUScene() { return devScene; }

template <typename T>
void Upload(T** dst, T* src) {
  if (!src) return;
  checkCudaErrors(cudaMalloc(dst, sizeof(T)));
  checkCudaErrors(cudaMemcpy(*dst, src, sizeof(T), cudaMemcpyHostToDevice));
}

void UploadScene(Scene* scene) {
  Upload<Scene>(&devScene, scene);

  printf("\nObjects: %d\n", scene->allocator.objectsSize);
  printf("\nLights: %d\n", scene->allocator.lightsSize);

  checkCudaErrors(cudaGetLastError());
  checkCudaErrors(cudaDeviceSynchronize());
}

}  // namespace data
