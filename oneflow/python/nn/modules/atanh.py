"""
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
"""
import oneflow as flow
from oneflow.python.oneflow_export import oneflow_export, experimental_api
from oneflow.python.nn.module import Module
from oneflow.python.framework.tensor import register_tensor_op


class Atanh(Module):
    def __init__(self):
        super().__init__()
        self._op = flow.builtin_op("atanh").Input("x").Output("y").Build()

    def forward(self, x):
        return self._op(x)[0]


@oneflow_export("atanh")
@experimental_api
def atanh_op(input):
    r"""Returns a new tensor with the inverse hyperbolic tangent of the elements of :attr:`input`.

    .. math::
        \text{out}_{i} = \tanh^{-1}(\text{input}_{i})

    Args:
        input (Tensor): the input tensor.

    For example:

    .. code-block:: python

        >>> import oneflow.experimental as flow
        >>> import numpy as np
        >>> flow.enable_eager_execution()
        >>> np_arr = np.array([0.5, 0.6, 0.7]).astype(np.float32)
        >>> input = flow.Tensor(np_arr)
        >>> output = flow.atanh(input)
        >>> output
        tensor([0.5493, 0.6931, 0.8673], dtype=oneflow.float32)

    """

    return Atanh()(input)


@register_tensor_op("atanh")
@experimental_api
def atanh_op_tensor(x):
    r"""
    atanh() -> Tensor
    See :func:`oneflow.experimental.atanh`

    """

    return Atanh()(x)


@oneflow_export("arctanh")
@experimental_api
def arctanh_op(input):
    r"""

    Alias for :func:`oneflow.experimental.atanh`
    """

    return Atanh()(input)


@register_tensor_op("arctanh")
@experimental_api
def arctanh_op_tensor(input):
    r"""

    Alias for :func:`oneflow.experimental.atanh`
    """

    return Atanh()(input)


if __name__ == "__main__":
    import doctest

    doctest.testmod(raise_on_error=True)
