// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <iostream>

#include <dune/common/fvector.hh>
#include <dune/python/common/fvector.hh>
#include <dune/python/pybind11/embed.h>

#include<Python.h>
int main()
{
  Py_SetProgramName(PYTHON_INTERPRETER);

  pybind11::scoped_interpreter guard{};
  auto global = pybind11::dict(pybind11::module::import("__main__").attr("__dict__"));
  {
    pybind11::module dcommon = pybind11::module::import("dune.common");
    // the following not only returns a fv but more importantly registers
    // the FV (but leads to JIT (the first time it's called)
    // in contrast to the approach in 'test_embed1.cc').
    // Perhaps the use of 'std::vector' could be improved
    auto fv = dcommon.attr("FieldVector")(std::vector<double>{4,2});
    std::cout << "FV=" << fv << std::endl;

    // now execute Python code
    auto local = pybind11::dict();
    local["call_test"] = pybind11::cpp_function([&]() -> auto
        { return Dune::FieldVector<double,2>{4,2}; });
    auto result = pybind11::eval<pybind11::eval_statements>(
           "print('Hello World!');\n"
           "x = call_test();\n"
           "norm2_x = x.two_norm2;\n"
           "print(x);",
       global, local
    );
    auto x = local["x"].cast<Dune::FieldVector<double,2>>();
    if( !result.is( pybind11::none() ) || (x != Dune::FieldVector<double,2>{4,2}) )
      std::cout << "Test 1 failed" << std::endl;
    auto norm2_x = local["norm2_x"].cast<double>();
    if( !result.is( pybind11::none() ) || (norm2_x != 20) )
      std::cout << "Test 1 failed" << std::endl;
  }
}
