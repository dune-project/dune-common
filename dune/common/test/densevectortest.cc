// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <algorithm>

#include <dune/common/densevector.hh>
#include <dune/common/exceptions.hh>

class MyVector;

namespace Dune
{
  template< >
  struct DenseMatVecTraits< MyVector >
  {
    typedef MyVector derived_type;
    typedef double value_type;
    typedef unsigned int size_type;
  };
}

class MyVector : public Dune::DenseVector< MyVector >
{
public:
  MyVector ( unsigned int size, double v = 0 )
    : data_( size, v ) {}

  unsigned int size () const { return data_.size(); }

  double& operator[] ( unsigned int i ) { return data_[ i ]; }
  const double& operator[] ( unsigned int i ) const { return data_[ i ]; }
protected:
  std::vector< double > data_;
};


int main()
{
  try
  {
    unsigned int n = 15;
    MyVector v( n, 1 );
    if( ( v.end() - v.begin() ) < 0 )
      DUNE_THROW(Dune::Exception, "Negative value reported for end() - begin()" );

    return 0;
  } catch (Dune::Exception& e) {
    std::cerr << e << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }
}
