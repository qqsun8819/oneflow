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
#include "oneflow/core/cuda/elementwise.cuh"

namespace oneflow {

template<typename FunctorT, typename T>
struct UnaryElemwiseXpuFunctor<DeviceType::kGPU, FunctorT, T> final {
  void operator()(DeviceCtx* ctx, int64_t elem_cnt, T* out, const T* in, FunctorT functor) {
    OF_CUDA_CHECK(cuda::elementwise::Unary(functor, elem_cnt, out, in, ctx->cuda_stream()));
  }
};

template<typename FunctorT, typename T>
struct BinaryElemwiseXpuFunctor<DeviceType::kGPU, FunctorT, T> final {
  void operator()(DeviceCtx* ctx, int64_t elem_cnt, T* out, const T* in_1, const T* in_2,
                  FunctorT functor) {
    OF_CUDA_CHECK(
        cuda::elementwise::Binary(functor, elem_cnt, out, in_1, in_2, ctx->cuda_stream()));
  }
};

}  // namespace oneflow
