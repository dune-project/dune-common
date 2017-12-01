#include <iostream>
#include <string>
#include <complex>

#include <dune/common/math.hh>

int main(){
  int a = 42;
  const int b = 42;
  double c = sqrt(-1.0);
  std::complex<double> d(10.,c);
  std::cout << std::boolalpha
            << "isNaN(int): " << Dune::isNaN(a) << "\n"
            << "isNaN(const int): " << Dune::isNaN(b) << "\n"
            << "isNaN(42): " << Dune::isNaN(42) << "\n"
            << "isNaN(NaN): " << Dune::isNaN(c) << "\n"
            << "isNaN(std::complex<double> with NaN): " << Dune::isNaN(d) << "\n"
            << "isComplex<std::complex>: " << Dune::isComplexLike<std::complex<int>>::value  << "\n"
            << "isComplex<int>: " << Dune::isComplexLike<int>::value  << "\n"
            << std::endl;
}
