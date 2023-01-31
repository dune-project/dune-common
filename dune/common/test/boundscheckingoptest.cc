// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>
#include "testsuite.hh"

int main() try {
  Dune::TestSuite t;
  Dune::FieldVector<double, 3> v1 = {1, 2, 3};
  Dune::FieldVector<double, 2> const v2 = {1, 2};

  t.checkThrow<Dune::RangeError>([&]{ v1+=v2;})<<"Add vectors of different sizes didn't throw.";
  t.checkThrow<Dune::RangeError>([&]{ v1-=v2;})<<"Subtract vectors of different sizes didn't throw.";
  t.checkThrow<Dune::RangeError>([&]{ return v1==v2;})<<"Check vectors of different sizes for equality didn't throw.";
  t.checkThrow<Dune::RangeError>([&]{ return v1!=v2;})<<"Check vectors of different sizes for inequality didn't throw.";
  t.checkThrow<Dune::RangeError>([&]{ return v1.axpy(2, v2);})<<"Apply axpy to vectors of different sizes didn't throw.";

  Dune::FieldMatrix<double, 1, 3> a = {{1, 2, 3}};
  Dune::FieldMatrix<double, 2, 3> const b = {{1, 2, 3}, {10, 20, 30}};

  t.checkThrow<Dune::RangeError>([&]{ a+=b;})<<"Add matrices of different sizes didn't throw.";
  t.checkThrow<Dune::RangeError>([&]{ a-=b;})<<"Subtract matrices of different sizes didn't throw.";
  t.checkThrow<Dune::RangeError>([&]{ (a == b);})<<"Comparing matrices of different sizes for equality didn't throw.";
  t.checkThrow<Dune::RangeError>([&]{ (a != b);})<<"Comparing matrices of different sizes for inequality didn't throw.";
  t.checkThrow<Dune::RangeError>([&]{a.axpy(2,b); })<<"Axpy for matrices of different sizes didn't throw.";

  Dune::FieldMatrix<double, 1, 3> c = {{1, 2, 3}};
  t.checkNoThrow([&]{ a+=c;})<<"Add vectors of the same sizes throws.";

  return t.exit();
} catch (Dune::Exception &e) {
  std::cerr << e << std::endl;
  return 1;
} catch (std::exception &e) {
  std::cerr << e.what() << std::endl;
  return 1;
}
