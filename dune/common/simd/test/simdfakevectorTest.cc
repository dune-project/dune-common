#include <dune/common/simd/simdfakevector.hh>
#include <array>
#include <iostream>

using namespace Dune;

int main()
{
  simdfakevector<int, 5> V;
  simdfakevector<int, 5> W;
  simdfakevector<bool, 5> b1, b2, b3;
  V.fill(5);
  b1.fill(true);
  b1[2] = false;
  b2.fill(true);
  b2[0] = false;
  b2[2] = false;

  std::cout << "Printing V: " << V << std::endl;

  W = ++V;
  std::cout << "Printing W = ++V: " << V << W << std::endl;


  V.fill(1);
  W = V++;
  std::cout << "Printing W = V++: " << V << W << std::endl;

  V.fill(1);
  V += 1;
  std::cout << "Printing V += 1: " << V << std::endl;

  V.fill(1);
  W = V + 1;
  std::cout << "Printing W = V + 1: " << V << W << std::endl;

  W = V + V + V;
  std::cout << "Printing W = V + V + V: " << V << W << std::endl;

  b3= (b1^true);
  std::cout << "Printing boolean operations:" << b1 << b3 << std::endl;

  return 0;
}
