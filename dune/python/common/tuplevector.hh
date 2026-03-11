// SPDX-FileCopyrightText: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PYTHON_COMMON_TVECTOR_HH
#define DUNE_PYTHON_COMMON_TVECTOR_HH

/**
 * @file tuplevector.hh
 * @brief Python bindings for TupleVector.
 */

#include <tuple>
#include <iostream>

#include <dune/python/pybind11/pybind11.h>
#include <dune/python/pybind11/cast.h>

#include <dune/common/classname.hh>
#include <dune/common/indices.hh>
#include <dune/common/tuplevector.hh>
#include <dune/common/hybridutilities.hh>

namespace Dune {
  namespace Python {

    /**
     * @brief Register TupleVector bindings.
     *
     * This function registers Python bindings for TupleVector.
     *
     * @tparam TV The type of the TupleVector.
     * @tparam options Additional options for pybind11 class.
     * @param scope The scope to register the bindings.
     * @param cls The pybind11 class to register the bindings.
     */
    template <class TV, class... options>
    void registerTupleVector(pybind11::handle scope, pybind11::class_<TV, options...> cls) {
      namespace py = pybind11;

      using py::operator"" _a;

      cls.def(py::init([](py::tuple x) {
        assert(std::tuple_size_v<TV> == x.size());
        return Dune::unpackIntegerSequence([&](auto... i) {
          return new TV((x[i].template cast<std::tuple_element_t<i, TV> >())...); },
                           std::make_index_sequence<std::tuple_size_v<TV> >{});
      }));

      cls.def("assign", [](TV &self, const TV &x) { self = x; }, "x"_a);
      cls.def("copy", [](const TV &self) { return new TV(self); });

      cls.def("__getitem__", [](const TV &self, size_t index) {

        if (index >= self.size())
          throw py::index_error();

        return Dune::Hybrid::switchCases(Dune::Hybrid::integralRange(Dune::index_constant<std::tuple_size_v<TV> >()),index, [&](auto i)  {
            return py::cast(self[i],py::return_value_policy::reference);
          },[]() {return py::object{};});
        },py::return_value_policy::reference_internal);

      cls.def("__setitem__", [&](TV &self, size_t index, const py::object &value) {
        if (index >= self.size())
          throw py::index_error();

        Dune::Hybrid::switchCases(Dune::Hybrid::integralRange(Dune::index_constant<std::tuple_size_v<TV> >()),index, [&](auto i) {
            try
            {
              self[i] = value.cast<std::tuple_element_t<i, TV>>();
            }
            catch(const py::cast_error & e)
            {
              std::cerr << "Your provide value is not of the correct type, which should be " << Dune::className<std::tuple_element_t<i, TV>>()<<" for your provided index "<< index<< std::endl;
              throw e;
            }
        });
      });

      cls.def("__len__", [](const TV &self) { return self.size(); });
    }
  }  // namespace Python
}  // namespace Dune

#endif  // #ifndef DUNE_PYTHON_COMMON_TVECTOR_HH
