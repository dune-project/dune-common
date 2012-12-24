// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_FORLOOP_HH
#define DUNE_COMMON_FORLOOP_HH

/** \file
 * \brief A static for loop for template meta-programming
 */

#include <dune/common/static_assert.hh>

namespace Dune
{

#ifndef DOXYGEN
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

      template< class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8 >
      static void apply ( T1 &p1, T2 &p2, T3 &p3, T4 &p4, T5 &p5, T6 &p6,
                          T7 &p7, T8 &p8 )
      {
        A::apply( p1, p2, p3, p4, p5, p6, p7, p8 );
        B::apply( p1, p2, p3, p4, p5, p6, p7, p8 );
      }

      template< class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8, class T9 >
      static void apply ( T1 &p1, T2 &p2, T3 &p3, T4 &p4, T5 &p5, T6 &p6,
                          T7 &p7, T8 &p8, T9 &p9 )
      {
        A::apply( p1, p2, p3, p4, p5, p6, p7, p8, p9 );
        B::apply( p1, p2, p3, p4, p5, p6, p7, p8, p9 );
      }

      template< class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8, class T9, class T10 >
      static void apply ( T1 &p1, T2 &p2, T3 &p3, T4 &p4, T5 &p5, T6 &p6,
                          T7 &p7, T8 &p8, T9 &p9, T10 &p10 )
      {
        A::apply( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10 );
        B::apply( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10 );
      }

      template< class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8, class T9, class T10, class T11 >
      static void apply ( T1 &p1, T2 &p2, T3 &p3, T4 &p4, T5 &p5, T6 &p6,
                          T7 &p7, T8 &p8, T9 &p9, T10 &p10, T11 &p11 )
      {
        A::apply( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11 );
        B::apply( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11 );
      }

      template< class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8, class T9, class T10, class T11, class T12 >
      static void apply ( T1 &p1, T2 &p2, T3 &p3, T4 &p4, T5 &p5, T6 &p6,
                          T7 &p7, T8 &p8, T9 &p9, T10 &p10, T11 &p11,
                          T12 &p12 )
      {
        A::apply( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12 );
        B::apply( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12 );
      }

      template< class T1, class T2, class T3, class T4, class T5, class T6,
          class T7, class T8, class T9, class T10, class T11, class T12,
          class T13>
      static void apply ( T1 &p1, T2 &p2, T3 &p3, T4 &p4, T5 &p5, T6 &p6,
                          T7 &p7, T8 &p8, T9 &p9, T10 &p10, T11 &p11,
                          T12 &p12, T13 &p13 )
      {
        A::apply( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13 );
        B::apply( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13 );
      }
    };

  } // end namespace ForLoopHelper

#endif


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
   *
   * Example of usage:
   * \code
   * template<class Tuple>
   * struct PrintTupleTypes
   * {
   *   template <int i>
   *   struct Operation
   *   {
   *     template<class Stream>
   *     static void apply(Stream &stream, const std::string &prefix)
   *     {
   *       stream << prefix << i << ": "
   *              << className<typename tuple_element<i, Tuple>::type>()
   *              << std::endl;
   *     }
   *   };
   *   template<class Stream>
   *   static void print(Stream &stream, const std::string &prefix)
   *   {
   *     // cannot attach on-the-fly in the argument to ForLoop<..>::apply() since
   *     // that would yield an rvalue
   *     std::string extended_prefix = prefix+"  ";
   *
   *     stream << prefix << "tuple<" << std::endl;
   *     ForLoop<Operation, 0, tuple_size<Tuple>::value-1>::
   *       apply(stream, extended_prefix);
   *     stream << prefix << ">" << std::endl;
   *   }
   * };
   * \endcode
   *
   * \note Don't use any rvalues as the arguments to apply().
   *
   * Rvalues will bind to const-references, but not to references that are
   * non-const.  Since we do want to support modifiable arguments to apply(),
   * we have to use non-const references as arguments.  Supporting const
   * references as well would lead to an insane number of overloads which all
   * have to be written more-or-less by hand.
   *
   * Examples of rvalues are: literals (1.0, 0, "huhu"), the results of
   * functions returning an object (std::make_pair(0, 1.0)) and temporary
   * object constructions (std::string("hello"));
   */
  template< template< int > class Operation, int first, int last >
  class ForLoop
    : public GenericForLoop< ForLoopHelper::Apply, Operation, first, last >
  {
    dune_static_assert( (first <= last), "ForLoop: first > last" );
  };

}

#endif // #ifndef DUNE_COMMON_FORLOOP_HH
