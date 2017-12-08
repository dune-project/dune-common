#include <iostream>
#include <string>
#include <complex>

#include <dune/common/math.hh>

struct IsNaNTest {};

namespace Dune {
  namespace MathOverloads {
    constexpr auto isNaN(IsNaNTest &&t, PriorityTag<2>, ADLTag) {
      return true;
    }
  }
}

int main(){
  /*Dune::LoopSIMD<double,5> test1;
  test1.fill(PI);
  test1[1]*=2;
  test1[2]*=0.5;
  test1[3] = 0.;
  test1[4] = 1.;/

  test1[2] = sqrt(-1.0); //create NaN */
  int a = 42;
  const int b = 42;
  double c = sqrt(-1.0);
  IsNaNTest test;
  //Dune::isNaN(test);
  std::cout << std::boolalpha
            << "isNaN(int a = 42): " << Dune::isNaN(a) << "\n"
            << "isNaN(const int b = 42): " << Dune::isNaN(b) << "\n"
            << "isNaN(NaN): " << Dune::isNaN(c) << "\n"
            << "isNaN(42): " << Dune::isNaN(42) << "\n"
            << "isNaN(struct NanTest): " << Dune::isNaN(Dune::NanTest{}) << "\n"
            //<< "isNaN(struct IsNaNTest): " << Dune::isNaN(test) << "\n"
            //<< "isNaN(LoopSIMD) with NaN: " << isNaN(test1)*/
            << "canCallIsNaN<int>: " << Dune::canCallIsNaN<int>::value  << "\n"
            << "isComplex<std::complex>: " << Dune::isComplexLike<std::complex<int>>::value  << "\n"
            << "isComplex<int>: " << Dune::isComplexLike<int>::value  << "\n"
            << std::endl;

  using std::sqrt;
}
