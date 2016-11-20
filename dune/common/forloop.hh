// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_FORLOOP_HH
#define DUNE_COMMON_FORLOOP_HH

#include <utility>

#include <dune/common/hybridutilities.hh>
#include <dune/common/std/utility.hh>

/** \file
 * \brief A static for loop for template meta-programming
 */

namespace Dune
{

  /** \class ForLoop
   * @brief A static loop using TMP
   *
   * The ForLoop takes a
   *   \code template<int i> class Operation \endcode
   * template argument with a static apply method
   * which is called for i=first...last (first<=last are int template arguments).
   * A specialization for class template class Operation for i=first
   * or i=last is not required. The class Operation must provide a
   * static void function apply(...). Arguments are perfectly forwarded
   * through the ForLoop to this function.
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
   */
  template< template< int > class Operation, int first, int last >
  struct ForLoop
  {
    static_assert( (first <= last), "ForLoop: first > last" );

    template<typename... Args>
    static void apply(Args&&... args)
    {
      Hybrid::forEach(Std::make_index_sequence<last+1-first>{},
        [&](auto i){Operation<i+first>::apply(args...);});
    }
  };

}

#endif // #ifndef DUNE_COMMON_FORLOOP_HH
