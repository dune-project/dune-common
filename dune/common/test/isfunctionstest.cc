#include <iostream>
#include <cmath>
#include <dune/common/fvector.hh>

int main() {
  Dune::FieldVector<double,5> f = {1.0, 2.0, 3.0, 4.0, 5.0};
  std::cout << std::boolalpha << Dune::isFinite(f) << std::endl;
  f[3] = exp(800);
  f[4] = NAN;
  std::cout << std::boolalpha
            << Dune::isFinite(f) << "\n"
            << Dune::isNaN(f) << "\n"
            << std::endl;
  return 0;

}
