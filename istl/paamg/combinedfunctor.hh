// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_AMG_COMBINEDFUNCTOR_HH
#define DUNE_AMG_COMBINEDFUNCTOR_HH

#include <dune/common/tuples.hh>
namespace Dune
{
  namespace Amg
  {

    template<typename T1, typename T2 = Nil, typename T3 = Nil,
        typename T4 = Nil, typename T5 = Nil,typename T6 = Nil,
        typename T7 = Nil, typename T8 = Nil, typename T9 = Nil>
    class CombinedFunctor : public Tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>
    {
    public:
      CombinedFunctor(const T1& t1=T1(), const T2& t2=T2(), const T3& t3=T3(),
                      const T4& t4=T4(), const T5& t5=T5(), const T6& t6=T6(),
                      const T7& t7=T7(), const T8& t8=T8(), const T9& t9=T8())
        : Tuple<T1,T2,T3,T4,T5,T6,T7,T8,T9>(t1, t2, t3,
                                            t4, t5, t6,
                                            t7, t8, t9)
      {}

      template<class T>
      void operator()(const T& t)
      {
        apply(*this, t);
      }

    private:
      template<class T, class TT1, class TT2>
      void apply(Pair<TT1,TT2>& pair, const T& t)
      {
        pair.first() (t);
        apply(pair.second(), t);
      }

      template<class T, class TT1>
      void apply(Pair<TT1,Nil>& pair, const T& t)
      {
        pair.first() (t);
      }
    };


  } //namespace Amg
} // namespace Dune
#endif
