// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_TRIPEL_HH
#define DUNE_TRIPEL_HH

namespace Dune
{
  // a tripel class similar to std::pair
  template<typename T1, typename T2, typename T3>
  struct tripel
  {
    typedef T1 first_type;
    typedef T2 second_type;
    typedef T3 third_type;
    T1 first;
    T2 second;
    T3 third;
    tripel() {}
    tripel (const T1& t1, const T2& t2, const T3& t3)
      : first(t1), second(t2), third(t3)
    {}
    bool operator< (const tripel<T1,T2,T3>& y) const
    {
      if (first<y.first) return true;
      if (y.first<first) return false;

      if (second<y.second) return true;
      if (y.second<second) return false;

      if (third<y.third) return true;
      return false;
    }
    bool operator== (const tripel<T1,T2,T3>& y) const
    {
      if (first==y.first && second==y.second && third==y.third) return true;
      return false;
    }
  };

}

#endif
