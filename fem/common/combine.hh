// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// no define of include variable because the file can be included more than
// once

#ifndef PARAM_CLASSNAME
#error "PARAM_CLASSNAME must be defined! \n"
#endif

#ifdef PARAM_CLASSNAME

// this call combines two objects in such a way that a certain method is
// can be called for both objects
// first A.method then B.method is called.
template <class A, class B >
class PARAM_CLASSNAME
#ifdef PARAM_INHERIT
  : public PARAM_INHERIT < PARAM_CLASSNAME < A , B > , typename A::Traits::ParamType >
#undef PARAM_INHERIT
#else
  : public ObjPointerStorage
#endif
{
public:

  //! Constructor for combinations with factors
  PARAM_CLASSNAME ( const A & a, const B & b ) : _a ( a ) , _b ( b )
  {
    //std::cout << "Constructor \n";
  }

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
