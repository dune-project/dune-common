// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_FORLOOP_HH
#define DUNE_COMMON_FORLOOP_HH

#include <utility>

/** \file
 * \brief A static for loop for template meta-programming
 */

namespace Dune
{

#ifndef DOXYGEN
  // GenericForLoop
  // --------------

  template< template< class, class > class Operation, template< int > class Value, int first, int last >
  class GenericForLoop
    : public Operation< Value< first >, GenericForLoop< Operation, Value, first+1, last > >
  {
    static_assert( (first <= last), "GenericForLoop: first > last" );
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

      template< typename... Params >
      static void apply ( Params&&... params )
      {
        A::apply( std::forward<Params>(params)... );
        B::apply( std::forward<Params>(params)... );
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
   * can be passed through the ForLoop to this function.
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
   * \note Since Dune 2.4, ForLoop uses variadic templates and perfect forwarding and
   *       thus supports arbitrary numbers of arguments to apply(), which can be any
   *       combination of lvalues and rvalues.
   *
   */
  template< template< int > class Operation, int first, int last >
  class ForLoop
    : public GenericForLoop< ForLoopHelper::Apply, Operation, first, last >
  {
    static_assert( (first <= last), "ForLoop: first > last" );
  };

}

#endif // #ifndef DUNE_COMMON_FORLOOP_HH
