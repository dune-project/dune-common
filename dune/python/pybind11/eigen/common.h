// Copyright (c) 2023 The pybind Community.

#pragma once
#ifdef DUNE_USE_SYSTEM_PYBIND11
#include <pybind11/eigen/common.h>
#else
// Common message for `static_assert()`s, which are useful to easily
// preempt much less obvious errors.
#define PYBIND11_EIGEN_MESSAGE_POINTER_TYPES_ARE_NOT_SUPPORTED                                    \
    "Pointer types (in particular `PyObject *`) are not supported as scalar types for Eigen "     \
    "types."
#endif // DUNE_USE_SYSTEM_PYBIND11
