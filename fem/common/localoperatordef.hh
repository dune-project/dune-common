// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
//*************************************************
void LocalOperatorFunctionToCall () const
{
  _a.LocalOperatorFunctionToCall();
  _b.LocalOperatorFunctionToCall();
}

template <class Arg1>
void LocalOperatorFunctionToCall (Arg1 & arg1) const
{
#ifndef CALL_A_NOT
  _a.LocalOperatorFunctionToCall(arg1);
#endif
#ifndef CALL_B_NOT
  _b.LocalOperatorFunctionToCall(arg1);
#endif
}

template <class Arg1, class Arg2>
void LocalOperatorFunctionToCall (Arg1 & arg1, Arg2 & arg2) const
{
  _a.LocalOperatorFunctionToCall(arg1,arg2);
  _b.LocalOperatorFunctionToCall(arg1,arg2);
}

template <class Arg1, class Arg2, class Arg3>
void LocalOperatorFunctionToCall (Arg1 & arg1, Arg2 & arg2, Arg3 & arg3) const
{
  _a.LocalOperatorFunctionToCall(arg1,arg2,arg3);
  _b.LocalOperatorFunctionToCall(arg1,arg2,arg3);
}

template <class Arg1, class Arg2, class Arg3, class Arg4>
void LocalOperatorFunctionToCall (Arg1 & arg1, Arg2 & arg2, Arg3 & arg3 , Arg4 & arg4 ) const
{
  _a.LocalOperatorFunctionToCall(arg1,arg2,arg3,arg4);
  _b.LocalOperatorFunctionToCall(arg1,arg2,arg3,arg4);
}

template <class Arg1, class Arg2, class Arg3, class Arg4, class Arg5>
void LocalOperatorFunctionToCall (Arg1 & arg1, Arg2 & arg2, Arg3 & arg3 , Arg4 & arg4 , Arg5 & arg5 ) const
{
  _a.LocalOperatorFunctionToCall(arg1,arg2,arg3,arg4,arg5);
  _b.LocalOperatorFunctionToCall(arg1,arg2,arg3,arg4,arg5);
}

#undef LocalOperatorFunctionToCall
