// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_MATH_HH
#define DUNE_MATH_HH

#include <cmath>

namespace Dune
{

  template< class Field >
  struct MathematicalConstants;



  // StandardMathematicalConstants
  // -----------------------------

  template< class T >
  struct StandardMathematicalConstants
  {
    static T e ()
    {
      static const T e = std::exp( T( 1 ) );
      return e;
    }

    static T pi ()
    {
      static const T pi = std::acos( T( -1 ) );
      return pi;
    }
  };



  // MathematicalConstants for float
  // -------------------------------

  template<>
  struct MathematicalConstants< float >
    : public StandardMathematicalConstants< float >
  {};



  // MathematicalConstants for double
  // --------------------------------

  template<>
  struct MathematicalConstants< double >
    : public StandardMathematicalConstants< double >
  {};



  // MathematicalConstants for long double
  // -------------------------------------

  template<>
  struct MathematicalConstants< long double >
    : public StandardMathematicalConstants< long double >
  {};

}

#endif // #ifndef DUNE_MATH_HH
