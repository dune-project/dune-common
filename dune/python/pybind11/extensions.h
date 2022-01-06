/*
    pybind11/extensions.h: Extensions to the C++11 python binding
    generator library for dune-fempy

    Copyright (c) 2016 Andreas Dedner <a.s.dedner@warwick.ac.uk>
    Copyright (c) 2016 Martin Nolte <nolte@mathematik.uni-freiburg.de>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE file.
*/

#pragma once

#include <stdexcept>
#include <type_traits>
#include <utility>

#include "pybind11.h"
#include "numpy.h"

PYBIND11_NAMESPACE_BEGIN(PYBIND11_NAMESPACE)

template <class T>
inline bool already_registered() {
  return static_cast<bool>(detail::get_type_info(typeid(T)));
}


template <class F>
inline void handle_buffer_format(const pybind11::buffer_info &info, F &&f) {
  if(info.format.size() > 2)
    throw std::runtime_error("Buffer format '" + info.format + "' not supported.");
  char format = info.format[0];
  if(format == '=' || format == '<')
    format = info.format[1];
  switch(format) {
  case 'h':
    return f(format_descriptor<short>());
  case 'H':
    return f(format_descriptor<unsigned short>());
  case 'i':
    return f(format_descriptor<int>());
  case 'I':
    return f(format_descriptor<unsigned int>());
  case 'l':
    return f(format_descriptor<long>());
  case 'L':
    return f(format_descriptor<unsigned long>());
  case 'q':
    return f(format_descriptor<long long>());
  case 'Q':
    return f(format_descriptor<unsigned long long>());
  case 'f':
    return f(format_descriptor<float>());
  case 'd':
    return f(format_descriptor<double>());
  default:
    throw std::runtime_error("Buffer format '" + info.format + "' not supported.");
  }
}

template <class T>
inline void implicitly_convert_facades() {
  auto implicit_caster = [](PyObject *obj, PyTypeObject *type) -> PyObject * {
    return getattr(obj, "__impl__", nullptr).release().ptr();
  };

  if(auto tinfo = detail::get_type_info(typeid(T)))
    tinfo->implicit_conversions.push_back(implicit_caster);
  else
    pybind11_fail("impplicitly_convert_facades: Unable to find type " + type_id<T>());
}


PYBIND11_NAMESPACE_END(PYBIND11_NAMESPACE)
