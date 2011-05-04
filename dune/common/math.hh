// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_MATH_HH
#define DUNE_MATH_HH

#include <cmath>

namespace Dune
{

  template< class Field >
  struct MathematicalConstants;



  // MathematicalConstants for double
  // --------------------------------

  template<>
  struct MathematicalConstants< double >
  {
    static double e () { return std::exp( 1.0 ); }
    static double pi () { return std::acos( -1.0 ); }
  };



  // MathematicalConstants for float
  // -------------------------------

  template<>
  struct MathematicalConstants< float >
  {
    static float e () { return std::exp( 1.0f ); }
    static float pi () { return std::acos( -1.0f ); }
  };

}

#endif // #ifndef DUNE_MATH_HH
