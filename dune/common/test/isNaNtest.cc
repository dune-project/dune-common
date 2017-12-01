#include <iostream>
#include <string>
#include <complex>

#include <dune/common/math.hh>

/*
template<class T,std::size_t S>
auto Dune::MathOverloads::isNaN(Dune::LoopSIMD<T,S> &v,Dune::PriorityTag<3>, Dune::MathOverloads::ADLTag) {
  Dune::LoopSIMD<bool,S> out;
  for(std::size_t i=0; i<S; i++) {
    out[i] = isNaN(v[i]);
  }
  return Dune::Simd::anyTrue(out);
}
*/

int main(){
  /*Dune::LoopSIMD<double,5> test1;
  test1.fill(PI);
  test1[1]*=2;
  test1[2]*=0.5;
  test1[3] = 0.;
  test1[4] = 1.;/

  test1[2] = sqrt(-1.0); //create NaN */
  std::cout << std::boolalpha
            << "isNaN(42): " << Dune::isNaN(42) << "\n"
            /*<< "isNaN(NaN): " << Dune::isNaN(sqrt(-1.0)) << "\n"
            << "isNaN(LoopSIMD) with NaN: " << isNaN(test1)*/
            //<< "CanCall<sqrt,std::string>: " << Dune::CanCall<sqrt,std::string>::value  << "\n"
            << "isComplex<std::complex>: " << Dune::isComplexLike<std::complex<int>>::value  << "\n"
            << std::endl;

  using std::sqrt;
}
