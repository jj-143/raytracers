#pragma once

#if defined(__CUDACC__) and defined(GPU_RENDER)
#define RT_DEVICE_HOST __device__ __host__
#define RT_DEVICE __device__
#define RT_GLOBAL __global__
#else
#define RT_DEVICE_HOST
#define RT_DEVICE
#define RT_GLOBAL
#endif
