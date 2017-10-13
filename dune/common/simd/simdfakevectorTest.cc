#include "simdfakevector.hh"
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

  std::cout << "Printing V: " << std::endl;
  V.print();

  W = ++V;
  std::cout << "Printing W = ++V: " << std::endl;
  V.print();
  W.print();

  V.fill(1);
  W = V++;
  std::cout << "Printing W = V++: " << std::endl;
  V.print();
  W.print();

  V.fill(1);
  V += 1;
  std::cout << "Printing V += 1: " << std::endl;
  V.print();

  V.fill(1);
  W = V + 1;
  std::cout << "Printing W = V + 1: " << std::endl;
  V.print();
  W.print();

  W = V + V + V;
  std::cout << "Printing W = V + V + V: " << std::endl;
  V.print();
  W.print();

  std::cout << "Printing boolean operations:" << std::endl;

  b3= (b1^true);
  b1.print();
  b3.print();

  V.fill(5);
 // b3 = V || 3;
 // b3.print();

  return 0;
}
