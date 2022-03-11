# MegEngine is Licensed under the Apache License, Version 2.0 (the "License")
#
# Copyright (c) 2014-2021 Megvii Inc. All rights reserved.
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT ARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
import numpy as np
import pytest

import megengine.functional as F
import megengine.module as M
from megengine import Parameter, Tensor, amp, tensor


class MyModule(M.Module):
    class InnerModule(M.Module):
        def __init__(self):
            super().__init__()
            self.bn = M.BatchNorm2d(4)

        def forward(self, x):
            return self.bn(x)

    def __init__(self):
        super().__init__()
        self.i = self.InnerModule()
        self.conv = M.Conv2d(4, 4, 4, groups=2)
        self.bn = M.BatchNorm2d(4)
        self.param = Parameter(np.ones((1, 3, 1, 1), dtype=np.float32))
        self.buff = Tensor(np.ones((1, 3, 1, 1), dtype=np.float32))

    def forward(self, x):
        x = self.i(x)
        x = self.bn(x)
        return x


@pytest.mark.parametrize("is_inplace", [False, True])
def test_convert_module(is_inplace):
    m = MyModule()
    m = amp.convert_module_format(m, is_inplace)
    for name, param in m.named_tensors():
        assert param.format == "nhwc"