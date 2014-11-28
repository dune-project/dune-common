// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/nullptr.hh>
#include <dune/common/unused.hh>

void basic_tests()
{
  typedef Dune::nullptr_t NULLPTR_T DUNE_UNUSED;
  char* ch = nullptr;    // ch has the null pointer value
  char* ch2 = 0;         // ch2 has the null pointer value
#ifdef FAIL
  int n = nullptr;       // error
  ++n;
#endif
  int n2 = 0;            // n2 is zero
  if( ch == 0 )          // evaluates to true
    ;
  if( ch == nullptr )    // evaluates to true
    ;
  if( nullptr == ch )    // evaluates to true
    ;
  if( ch )               // evaluates to false
    ;
  if( n2 == 0 )          // evaluates to true
    ;
  ch = ch2;
#ifdef FAIL
  if( n2 == nullptr )    // error
    ;
  if( nullptr )          // error, no conversion to bool
    ;
  if( nullptr == 0 )     // error
    ;
  // arithmetic
  nullptr = 0;           // error, nullptr is not an lvalue
  nullptr + 2;           // error
#endif
}

int main()
{
  basic_tests();
  return 0;
}
