// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// no define of include variable because the file can be included more than
// once

#ifndef PARAM_CLASSNAME
#error "PARAM_CLASSNAME must be defined! \n"
#endif

#include "objpointer.hh"

#ifdef PARAM_CLASSNAME

template <class A, class B >
class PARAM_CLASSNAME
{
public:

  //! Constructor for combinations with factors
  PARAM_CLASSNAME ( const A & a, const B & b ) : _a ( a ) , _b ( b ) {}

#ifdef PARAM_FUNC_1
#define LocalOperatorFunctionToCall PARAM_FUNC_1
#include "localoperatordef.hh"
#undef PARAM_FUNC_1
#endif

#ifdef PARAM_FUNC_2
#define LocalOperatorFunctionToCall PARAM_FUNC_2
#include "localoperatordef.hh"
#undef PARAM_FUNC_2
#endif

#ifdef PARAM_FUNC_3
#ifdef PARAM_FUNC_3_ONLY_A
#define CALL_B_NOT
#endif
#define LocalOperatorFunctionToCall PARAM_FUNC_3
#include "localoperatordef.hh"
#undef PARAM_FUNC_3
#endif

#ifdef PARAM_FUNC_4
#define LocalOperatorFunctionToCall PARAM_FUNC_4
#include "localoperatordef.hh"
#undef PARAM_FUNC_4
#endif

#ifdef PARAM_FUNC_5
#define LocalOperatorFunctionToCall PARAM_FUNC_5
#include "localoperatordef.hh"
#undef PARAM_FUNC_5
#endif

private:
  //! space A and B
  const A & _a;
  const B & _b;
}; // end class PARAM_CLASSNAME

#undef PARAM_CLASSNAME
#endif

// end file
