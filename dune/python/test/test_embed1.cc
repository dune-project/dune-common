// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <iostream>

#include <dune/common/fvector.hh>
#include <dune/python/common/fvector.hh>
#include <dune/python/pybind11/embed.h>

int main()
{
  Py_SetProgramName(PYTHON_INTERPRETER);
  /*
     remark: combine getting the guard and loading
             dune.common in a single 'initialization' function -
             the dune.common module can also be used to register additional
             types - although a 'dummy' scope can also be used, i.e.,
             pybind11::handle scope;
  */
  pybind11::scoped_interpreter guard{};
  pybind11::module dcommon = pybind11::module::import("dune.common");
  auto global = pybind11::dict(pybind11::module::import("__main__").attr("__dict__"));

  // using the C++ registry functions directly:
  Dune::Python::registerFieldVector<double,2> ( dcommon );
  /*
      // instead of importing dune.common one could use the dummy scope and
      // the first step is done in 'dune.common' (_common.cc)
      Dune::Python::addToTypeRegistry<double>(Dune::Python::GenerateTypeName("double"));
      // now we want to pass a FV<double,2> to Python so need to register that
      Dune::Python::registerFieldVector<double,2> ( scope );
  */

  // first set of tests
  {
    // First example:
    // Call a C++ function that generats a FV,
    // call that in Python and compute squared 2-norm
    auto local = pybind11::dict();
    local["call_test"] = pybind11::cpp_function([&]() -> auto
        { return Dune::FieldVector<double,2>{4,2}; });
    auto result = pybind11::eval<pybind11::eval_statements>(
           "print('Example 1');\n"
           "x = call_test();\n"
           "norm2_x = x.two_norm2;\n"
           "print('results',x);",
       global, local
    );
    auto &x = local["x"].cast<Dune::FieldVector<double,2>&>();
    auto norm2_x = local["norm2_x"].cast<double>();
    if( !result.is( pybind11::none() )
         || (x != Dune::FieldVector<double,2>{4,2})
         || (norm2_x != 20) )
      std::cout << "Test 1 failed" << std::endl;

    // Second example:
    // Call a C++ function pssing in a FV reference and changing that
    // Note that the FV passed in is 'x&' generated in Example 2
    local["call_testref"] = pybind11::cpp_function([&]
        (Dune::FieldVector<double,2>& y) -> auto
        { y+=Dune::FieldVector<double,2>{-4,-2}; });
    auto resultref = pybind11::eval<pybind11::eval_statements>(
           "print('Example 2');\n"
           "call_testref(x);\n"
           "norm2_x = x.two_norm2;\n"
           "print('result',x);",
       global, local
    );
    norm2_x = local["norm2_x"].cast<double>();
    if( !resultref.is( pybind11::none() )
        || (x != Dune::FieldVector<double,2>{0,0})
        || (norm2_x != 0) )
      std::cout << "Test 2 failed" << std::endl;

    // Third example:
    // Construct a FV on C++ side and use that on Python side
    // Note: local["z"]=z generates a copy so to retrieve the changed
    // FV one needs to copy again using z = local["z"].cast<...>();
    Dune::FieldVector<double,2> z{4,2};
    local["call_testref2"] = pybind11::cpp_function([&]
        (Dune::FieldVector<double,2>& y) -> auto
        { y+=z; });
    local["z"] = z;
    auto resultref2 = pybind11::eval<pybind11::eval_statements>(
           "print('Example 3');\n"
           "import dune.common;\n"
           "zz=dune.common.FieldVector((2,4));\n"
           "call_testref2(zz);\n"
           "print('results',zz,'using',z);"
           "z *= 2\n"
           "print('changed z to',z);",
       global, local
    );
    z = local["z"].cast<Dune::FieldVector<double,2>>();
    std::cout << "change of z on C++ side:" << z << std::endl;
    auto &zz = local["zz"].cast<Dune::FieldVector<double,2>&>();
    if( !resultref2.is( pybind11::none() )
        || (zz != Dune::FieldVector<double,2>{6,6}) )
      std::cout << "Test 3 failed" << std::endl;

    // Example 4
    // Can also use pointer to a FV in the `local` dict soo that
    // changes on the Python side are available on the C++ side without copy
    auto newLocal = pybind11::dict(); // test with a new local dict
    Dune::FieldVector<double,2> fv2{4,2};
    newLocal["fv2"] = pybind11::cast(&fv2);
    auto resultFVptr = pybind11::eval<pybind11::eval_statements>(
           "print('Example 4');\n"
           "print('changed fv from',fv2,end=' -> ')\n"
           "fv2 *= 2\n"
           "print(fv2);",
       global, newLocal
    );
    std::cout << "C++ FV=" << fv2 << std::endl;
    if( !resultFVptr.is( pybind11::none() )
        || (fv2 != Dune::FieldVector<double,2>{8,4}) )
      std::cout << "Test 4 failed" << std::endl;
  }

  // the final example uses the `FieldVector` function from the
  // dune.common module - this approach requires JIT in dune-py and
  // is turned off for the general embedding tests
  if (false)
  {
    // Example 5
    // Similar to example 3 but without copying similar to Example 4
    auto pyfv = dcommon.attr("FieldVector")(std::vector<double>{4,2});
    Dune::FieldVector<double,2>& fv = pyfv.cast<Dune::FieldVector<double,2>&>();
    std::cout << "FV=" << pyfv << "==" << fv << std::endl;
    auto newLocal = pybind11::dict();
    newLocal["fv"] = pyfv;
    auto resultFVa = pybind11::eval<pybind11::eval_statements>(
           "print('Example 5a');\n"
           "print('changed fv from',fv,end=' -> ')\n"
           "fv *= 2\n"
           "print(fv);",
       global, newLocal
    );
    std::cout << "C++ FV=" << fv << std::endl;
    if( !resultFVa.is( pybind11::none() )
        || (fv != Dune::FieldVector<double,2>{8,4}) )
      std::cout << "Test 5a failed" << std::endl;
    auto resultFVb = pybind11::eval<pybind11::eval_statements>(
           "print('Example 5b');\n"
           "print('changed fv from',fv,end=' -> ')\n"
           "fv *= 2\n"
           "print(fv);",
       global, newLocal
    );
    std::cout << "C++ FV=" << fv << std::endl;
    if( !resultFVb.is( pybind11::none() )
        || (fv != Dune::FieldVector<double,2>{16,8}) )
      std::cout << "Test 5b failed" << std::endl;
  }
}
