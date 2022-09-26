// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>

#include <dune/common/dynvector.hh>
#include <dune/common/exceptions.hh>

using Dune::DynamicVector;

template<class ct>
void dynamicVectorTest(int d) {
  ct a = 1;
  DynamicVector<ct> v(d,1);
  DynamicVector<ct> w(d,2);
  DynamicVector<ct> z(d,2);
  [[maybe_unused]] bool b;

  DynamicVector<ct> v2(v); // copy constructor
  if (v!=v2)
    DUNE_THROW(Dune::InvalidStateException,"Copy constructor does not work properly");

  DynamicVector<ct> v3(std::move(v2)); // move constructor
  if (v!=v3)
    DUNE_THROW(Dune::InvalidStateException,"Move constructor does not work properly");

  v2 = std::move(v3); // move assignment
  if (v!=v2)
    DUNE_THROW(Dune::InvalidStateException,"Move assignment does not work properly");


  // Test whether the norm methods compile
  (w+v).two_norm();
  (w+v).two_norm2();
  (w+v).one_norm();
  (w+v).one_norm_real();
  (w+v).infinity_norm();
  (w+v).infinity_norm_real();

  // test op(vec,vec)
  z = v + w;
  z = v - w;
  DynamicVector<ct> z2 = v + w;
  w -= v;
  w += v;

  // test op(vec,scalar)
  w +=a;
  w -= a;
  w *= a;
  w /= a;

  // test scalar product, axpy
  a = v * w;
  z = v.axpy(a,w);

  // test comparison
  b = (w != v);
  b = (w == v);


  // test istream operator
  std::stringstream s;
  for (int i=0; i<d; i++)
  {
    s << i << " ";
    v[i] = i;
  }
  s >> w;
  assert(v == w);

}

int main()
{
  try {
    for (int d=1; d<6; d++)
    {
      dynamicVectorTest<int>(d);
      dynamicVectorTest<float>(d);
      dynamicVectorTest<double>(d);
    }
  } catch (Dune::Exception& e) {
    std::cerr << e << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }
}
