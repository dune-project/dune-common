/*
    pybind11/eigen.h: Transparent conversion for dense and sparse Eigen matrices

    Copyright (c) 2016 Wenzel Jakob <wenzel.jakob@epfl.ch>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE file.
*/

#pragma once
#ifdef DUNE_USE_SYSTEM_PYBIND11
#include <pybind11/eigen.h>
#else
#include "eigen/matrix.h"
#endif // DUNE_USE_SYSTEM_PYBIND11
