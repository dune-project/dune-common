#include <iostream>

#include <dune/common/fvector.hh>
#include <dune/python/common/fvector.hh>
#include <dune/python/pybind11/embed.h>

int main()
{
  pybind11::scoped_interpreter guard{};
  auto global = pybind11::dict(pybind11::module::import("__main__").attr("__dict__"));
  {
#if 1 // using the C++ registry functions directly:
    // need some 'dummy' scope to register the FV to
    pybind11::handle scope;
    // the first step is done in 'dune.common' (_common.cc)
    Dune::Python::addToTypeRegistry<double>(Dune::Python::GenerateTypeName("double"));
    // now we want to pass a FV<double,2> to Python so need to register that
    Dune::Python::registerFieldVector<double,2> ( scope );
#else // go through dune.common module but still use C++ registry functions
    pybind11::module dcommon = pybind11::module::import("dune.common");
    Dune::Python::registerFieldVector<double,2> ( dcommon );
#endif
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
// remark: combine getting the guard, a dummy scope and possibly loading
//         dune.common in a single 'initialization' function?
