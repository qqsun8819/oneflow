/*
Copyright 2020 The OneFlow Authors. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef ONEFLOW_CORE_CUDA_BGEMM_H_
#define ONEFLOW_CORE_CUDA_BGEMM_H_

#ifdef WITH_CUDA

#include "oneflow/core/device/cuda_util.h"

namespace oneflow {
namespace cuda {
namespace blas {

template<typename T>
void bgemm(cublasHandle_t handle, char transa, char transb, int64_t m, int64_t n, int64_t k,
           T alpha, const T* a, int64_t lda, int64_t stridea, const T* b, int64_t ldb,
           int64_t strideb, T beta, T* c, int64_t ldc, int64_t stridec, int64_t batch_size);

}  // namespace blas
}  // namespace cuda
}  // namespace oneflow

#endif  // WITH_CUDA

#endif  // ONEFLOW_CORE_CUDA_BGEMM_H_
