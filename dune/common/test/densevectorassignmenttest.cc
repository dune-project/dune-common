// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <iostream>

#include <dune/common/densevector.hh>
#include <dune/common/dynvector.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>

using namespace Dune;

template <class FirstVectorImp, class SecondVectorImp>
void assign(DenseVector<FirstVectorImp>& first, const DenseVector<SecondVectorImp>& second)
{
  first = second;
}

bool run()
{
  bool passed = true;
  FieldVector<double, 3> fvec1{1, 2, 3};
  DynamicVector<double> dynvec1{1, 2, 3};
  FieldVector<double, 3> fvec2;
  DynamicVector<double> dynvec2(3);
  // check mixed assignments
  assign(fvec2, dynvec1);
  assign(dynvec2, fvec1);
  for (size_t i = 0; i < 3; ++i) {
    if (fvec2[i] != dynvec1[i]) {
      std::cerr << "Assigning a DynamicVector to a FieldVector as DenseVectors does not work!"
      << std::endl << i << "-th entry after assignment is " << fvec2[i] << ", should be "
      << i+1 << "!" << std::endl;
      passed = false;
    }
    if (dynvec1[i] != dynvec2[i]) {
      std::cerr << "Assigning a FieldVector to a DynamicVector as DenseVectors does not work"
      << std::endl << i << "-th entry after assignment is " << dynvec1[i] << ", should be "
      << i+1 << "!" << std::endl;
      passed = false;
    }
  }
  return passed;
}

int main()
{
  bool passed = run();
  if (!passed)
    DUNE_THROW(Dune::Exception, "Test failed");
  return !passed;
}
