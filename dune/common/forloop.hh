// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_FORLOOP_HH
#define DUNE_COMMON_FORLOOP_HH

#include <dune/common/static_assert.hh>

namespace Dune
{

  // GenericForLoop
  // --------------

  template< template< class, class > class Operation, template< int > class Value, int first, int last >
  class GenericForLoop
    : public Operation< Value< first >, GenericForLoop< Operation, Value, first+1, last > >
  {
    dune_static_assert( (first <= last), "GenericForLoop: first > last" );
  };

  template< template< class, class > class Operation, template< int > class Value, int last >
  class GenericForLoop< Operation, Value, last, last >
    : public Value< last >
  {};



  // ForLoopHelper
  // -------------

  namespace ForLoopHelper
  {

    template< class A, class B >
    struct Apply
    {
      static void apply ()
      {
        A::apply();
        B::apply();
      }

      template< class T1 >
      static void apply ( T1 &p1 )
      {
        A::apply( p1 );
        B::apply( p1 );
      }

      template< class T1, class T2 >
      static void apply ( T1 &p1, T2 &p2 )
      {
        A::apply( p1, p2 );
        B::apply( p1, p2 );
      }

      template< class T1, class T2, class T3 >
      static void apply ( T1 &p1, T2 &p2, T3 &p3 )
      {
        A::apply( p1, p2, p3 );
        B::apply( p1, p2, p3 );
      }

      template< class T1, class T2, class T3, class T4 >
      static void apply ( T1 &p1, T2 &p2, T3 &p3, T4 &p4 )
      {
        A::apply( p1, p2, p3, p4 );
        B::apply( p1, p2, p3, p4 );
      }

      template< class T1, class T2, class T3, class T4, class T5 >
      static void apply ( T1 &p1, T2 &p2, T3 &p3, T4 &p4, T5 &p5 )
      {
        A::apply( p1, p2, p3, p4, p5 );
        B::apply( p1, p2, p3, p4, p5 );
      }

      template< class T1, class T2, class T3, class T4, class T5, class T6 >
      static void apply ( T1 &p1, T2 &p2, T3 &p3, T4 &p4, T5 &p5, T6 &p6 )
      {
        A::apply( p1, p2, p3, p4, p5, p6 );
        B::apply( p1, p2, p3, p4, p5, p6 );
      }

      template< class T1, class T2, class T3, class T4, class T5, class T6,
          class T7 >
      static void apply ( T1 &p1, T2 &p2, T3 &p3, T4 &p4, T5 &p5, T6 &p6,
                          T7 &p7 )
      {
        A::apply( p1, p2, p3, p4, p5, p6, p7 );
        B::apply( p1, p2, p3, p4, p5, p6, p7 );
      }
    };

  }



  /** \class ForLoop
   * @brief A static loop using TMP
   *
   * The ForLoop takes a
   *   \code template<int i> class Operation \endcode
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
   * \code
     template <class Foo>
     struct A
     {
     template <int i>
     struct Operation
     {
     template <class T>
     static void apply(const double &x,const T &t, T &ret)
     {
      ret = "hallo" + t;
     }
     };
     void useForLoop()
     {
     std::string world;
     ForLoop<Operation,1,10>::apply(1.,"hallo",world);
     }
     };
   * \endcode
   */

  template< template< int > class Operation, int first, int last >
  class ForLoop
    : public GenericForLoop< ForLoopHelper::Apply, Operation, first, last >
  {
    dune_static_assert( (first <= last), "ForLoop: first > last" );
  };

}

#endif // #ifndef DUNE_COMMON_FORLOOP_HH
