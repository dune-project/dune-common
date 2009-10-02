// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_FORLOOP_HH
#define DUNE_COMMON_FORLOOP_HH

#include <dune/common/static_assert.hh>

namespace Dune
{
  /** \class ForLoop
   * @brief A static loop using TMP
   *
   * The ForLoop takes a
   *   \code template<int i> class Operation
   * template argument with a static apply method
   * which is called for i=first...last (first<=last are int template arguments).
   * A specialization for class template class Operation for i=first
   * or i=last is not required. The class Operation must provide a
   * static void function apply(...). Arguments (as references)
   * can be passed through the ForLoop to this function
   * (up to 5 at the moment).
   *
   * It is possible to pass a subclass to the ForLoop
   * (since no specialization is needed).
   * Example of usage:
   * template <class Foo>
   * struct A
   * {
   *   template <int i>
   *   struct Operation
   *   {
   *     template <class T>
   *     static void apply(const double &x,const T &t, T &ret)
   *     {
   *       ret = "hallo" + t;
   *     }
   *   };
   *   void useForLoop()
   *   {
   *     std::string world;
   *     ForLoop<Operation,1,10>::apply(1.,"hallo",world);
   *   }
   * };
   */

  template< template< int > class Operation, int first, int last >
  struct ForLoop
  {
    static void apply ()
    {
      Operation< first >::apply();
      ForLoop< Operation, first+1, last >::apply();
    }

    template< class T1 >
    static void apply ( T1 &p1 )
    {
      Operation< first >::apply( p1 );
      ForLoop< Operation, first+1, last >::apply( p1 );
    }

    template< class T1, class T2 >
    static void apply ( T1 &p1, T2 &p2 )
    {
      Operation< first >::apply( p1, p2 );
      ForLoop< Operation, first+1, last >::apply( p1, p2 );
    }

    template< class T1, class T2, class T3 >
    static void apply ( T1 &p1, T2 &p2, T3 &p3 )
    {
      Operation< first >::apply( p1, p2, p3 );
      ForLoop< Operation, first+1, last >::apply( p1, p2, p3 );
    }

    template< class T1, class T2, class T3, class T4 >
    static void apply ( T1 &p1, T2 &p2, T3 &p3, T4 &p4 )
    {
      Operation< first >::apply( p1, p2, p3, p4 );
      ForLoop< Operation, first+1, last >::apply( p1, p2, p3, p4 );
    }

    template< class T1, class T2, class T3, class T4, class T5 >
    static void apply ( T1 &p1, T2 &p2, T3 &p3, T4 &p4, T5 &p5 )
    {
      Operation< first >::apply( p1, p2, p3, p4, p5 );
      ForLoop< Operation, first+1, last >::apply( p1, p2, p3, p4, p5 );
    }

  private:
    dune_static_assert( (first <= last), "ForLoop: first > last" );
  };

  template< template< int > class Operation, int last >
  struct ForLoop< Operation, last, last >
  {
    static void apply ()
    {
      Operation< last >::apply();
    }

    template< class T1 >
    static void apply ( T1 &p1 )
    {
      Operation< last >::apply( p1 );
    }

    template< class T1, class T2 >
    static void apply ( T1 &p1, T2 &p2 )
    {
      Operation< last >::apply( p1, p2 );
    }

    template< class T1, class T2, class T3 >
    static void apply ( T1 &p1, T2 &p2, T3 &p3 )
    {
      Operation< last >::apply( p1, p2, p3 );
    }

    template< class T1, class T2, class T3, class T4 >
    static void apply ( T1 &p1, T2 &p2, T3 &p3, T4 &p4 )
    {
      Operation< last >::apply( p1, p2, p3, p4 );
    }

    template< class T1, class T2, class T3, class T4, class T5 >
    static void apply ( T1 &p1, T2 &p2, T3 &p3, T4 &p4, T5 &p5 )
    {
      Operation< last >::apply( p1, p2, p3, p4, p5 );
    }
  };

}

#endif
