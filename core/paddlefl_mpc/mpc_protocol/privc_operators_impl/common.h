/* Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

// Description: 

#pragma once

namespace paddle {
namespace operators {
namespace privc {

struct GetMidDims {
    inline HOSTDEVICE void operator()(const framework::DDim &x_dims,
                         const framework::DDim &y_dims, const int axis,
                         int *pre, int *n, int *post)  {
        *pre = 1;
        *n = 1;
        *post = 1;
        for (int i = 0; i < axis; ++i) {
            (*pre) *= x_dims[i];
        }

        for (int i = 0; i < y_dims.size(); ++i) {
            PADDLE_ENFORCE_EQ(x_dims[i + axis], y_dims[i],
                              "Broadcast dimension mismatch.");
            (*n) *= y_dims[i];
        }

        for (int i = axis + y_dims.size(); i < x_dims.size(); ++i) {
            (*post) *= x_dims[i];
        }
    }
};

} // privc
} // operators
} // paddle

//#endif
