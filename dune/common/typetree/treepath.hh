// -*- tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=8 sw=2 sts=2:

#ifndef DUNE_COMMON_TYPETREE_TREEPATH_HH
#define DUNE_COMMON_TYPETREE_TREEPATH_HH

#include <cstddef>
#include <iostream>

#include <dune/common/documentation.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/indices.hh>
#include <dune/common/hybridutilities.hh>


namespace Dune {
  namespace TypeTree {

    //! \addtogroup TreePath
    //! \ingroup TypeTree
    //! \{

    namespace TreePathType {
      enum Type { fullyStatic, dynamic };
    }

    //! A hybrid version of TreePath that supports both compile time and run time indices.
    /**
     * A `HybridTreePath` supports storing a combination of run time and compile time indices.
     * This makes it possible to store the tree path to a tree node inside the tree node itself,
     * even if the path contains one or more `PowerNode`s, where each child must have exactly the
     * same type. At the same time, as much information as possible is kept accessible at compile
     * time, allowing for more efficient algorithms.
     *
     */
    template<typename... T>
    class HybridTreePath
    {
    public:
      //! An `index_sequence` for the entries in this `HybridTreePath`.
      using index_sequence = std::index_sequence_for<T...>;

      //! Default constructor
      constexpr HybridTreePath() = default;
      constexpr HybridTreePath(const HybridTreePath& tp) = default;
      constexpr HybridTreePath(HybridTreePath&& tp) = default;

      //! Constructor from a `std::tuple`
      explicit constexpr HybridTreePath(std::tuple<T...> t)
        : _data(t)
      {}

      //! Constructor from arguments
      template<typename... U, std::enable_if_t<(sizeof...(T) > 0 && sizeof...(U) == sizeof...(T)),bool> = true>
      explicit constexpr HybridTreePath(U... t)
        : _data(t...)
      {}

      //! Returns an index_sequence for enumerating the components of this HybridTreePath.
      constexpr static index_sequence enumerate()
      {
        return {};
      }

      //! Get the size (length) of this path.
      constexpr static std::size_t size()
      {
        return sizeof...(T);
      }

      //! Get the index value at position pos.
      template<std::size_t i>
      constexpr auto operator[](index_constant<i> pos) const
      {
        return std::get<i>(_data);
      }

      //! Get the index value at position pos.
      constexpr std::size_t operator[](std::size_t pos) const
      {
        std::size_t entry = 0;
        Hybrid::switchCases(enumerate(), pos, [&](auto i) {
          entry = this->element(i);
        });
        return entry;
      }

      //! Get the last index value.
      template<std::size_t i>
      constexpr auto element(index_constant<i> pos = {}) const
      {
        return std::get<i>(_data);
      }

      //! Get the index value at position pos.
      constexpr std::size_t element(std::size_t pos) const
      {
        std::size_t entry = 0;
        Hybrid::switchCases(enumerate(), pos, [&](auto i) {
          entry = this->element(i);
        });
        return entry;
      }

      //! Get the last index value.
      auto back() const
      {
        return std::get<sizeof...(T)-1>(_data);
      }

#ifndef DOXYGEN

      // I can't be bothered to make all the external accessors friends of HybridTreePath,
      // so we'll only hide the data tuple from the user in Doxygen.

      using Data = std::tuple<T...>;
      Data _data;

#endif // DOXYGEN

    };


    //! Constructs a new `HybridTreePath` from the given indices.
    /**
     * This function returns a new `HybridTreePath` with the given index values. It exists
     * mainly to avoid having to manually specify the exact type of the new object.
     */
    template<typename... T>
    constexpr auto hybridTreePath(const T&... t)
    {
      return HybridTreePath<T...>{t...};
    }

    //! Constructs a new `HybridTreePath` from the given indices.
    /**
     * This function returns a new `HybridTreePath` with the given index values. It exists
     * mainly to avoid having to manually specify the exact type of the new object.
     */
    template<typename... T>
    constexpr auto treePath(const T&... t)
    {
      return HybridTreePath<T...>{t...};
    }


    //! Returns the size (number of components) of the given `HybridTreePath`.
    template<typename... T>
    constexpr std::size_t treePathSize(const HybridTreePath<T...>&)
    {
      return sizeof...(T);
    }

    //! Returns a copy of the i-th element of the `HybridTreePath`.
    /**
     * As `HybridTreePath` instances should not be mutated after their creation, this function
     * returns a copy of the value. As values are either `std::integral_constant` or `std::size_t`, that's
     * just as cheap as returning a reference.
     *
     * The index for the entry can be passed either as a template parameter or as an `index_constant`:
     *
     * \code{.cc}
     * auto tp = hybridTreePath(1,2,3,4);
     * using namespace Dune::TypeTree::Indices;
     * // the following two lines are equivalent
     * std::cout << treePathEntry<2>(tp) << std::endl;
     * std::cout << treePathEntry(tp,_2) << std::endl;
     * \endcode
     *
     */
    template<std::size_t i, typename... T>
    constexpr auto treePathEntry(const HybridTreePath<T...>& tp, index_constant<i> = {})
      -> std::decay_t<decltype(std::get<i>(tp._data))>
    {
      return std::get<i>(tp._data);
    }

    //! Returns the index value of the i-th element of the `HybridTreePath`.
    /**
     * This function always returns the actual index value, irrespective of whether the
     * entry is a compile time index or a run time index.
     *
     * The index for the entry can be passed either as a template parameter or as an `index_constant`:
     *
     * \code{.cc}
     * auto tp = hybridTreePath(1,2,3,4);
     * using namespace Dune::TypeTree::Indices;
     * // the following two lines are equivalent
     * std::cout << treePathIndex<2>(tp) << std::endl;
     * std::cout << treePathIndex(tp,_2) << std::endl;
     * \endcode
     *
     */
    template<std::size_t i, typename... T>
    constexpr std::size_t treePathIndex(const HybridTreePath<T...>& tp, index_constant<i> = {})
    {
      return std::get<i>(tp._data);
    }

    //! Returns a copy of the last element of the `HybridTreePath`.
    /**
     * As `HybridTreePath` instances should not be mutated after their creation, this function
     * returns a copy of the value. As values are either `std::integral_constant` or `std::size_t`, that's
     * just as cheap as returning a reference.
     */
    template<typename... T, std::enable_if_t<(sizeof...(T) > 0),bool> = true>
    constexpr auto back(const HybridTreePath<T...>& tp)
      -> decltype(treePathEntry<sizeof...(T)-1>(tp))
    {
      return treePathEntry<sizeof...(T)-1>(tp);
    }

    //! Returns a copy of the first element of the `HybridTreePath`.
    /**
     * As `HybridTreePath` instances should not be mutated after their creation, this function
     * returns a copy of the value. As values are either `std::integral_constant` or `std::size_t`, that's
     * just as cheap as returning a reference.
     */
    template<typename... T>
    constexpr auto front(const HybridTreePath<T...>& tp)
      -> decltype(treePathEntry<0>(tp))
    {
      return treePathEntry<0>(tp);
    }

    //! Appends a run time index to a `HybridTreePath`.
    /**
     * This function returns a new `HybridTreePath` with the run time index `i` appended.
     */
    template<typename... T>
    constexpr auto push_back(const HybridTreePath<T...>& tp, std::size_t i)
    {
      return HybridTreePath<T...,std::size_t>{std::tuple_cat(tp._data,std::make_tuple(i))};
    }

    //! Appends a compile time index to a `HybridTreePath`.
    /**
     * This function returns a new `HybridTreePath` with the compile time index `i` appended.
     *
     * The value for the new entry can be passed either as a template parameter or as an `index_constant`:
     *
     * \code{.cc}
     * auto tp = hybridTreePath(1,2,3,4);
     * using namespace Dune::TypeTree::Indices;
     * // the following two lines are equivalent
     * auto tp_a = push_back<1>(tp);
     * auto tp_b = push_back(tp,_1);
     * \endcode
     *
     */
    template<std::size_t i, typename... T>
    constexpr auto push_back(const HybridTreePath<T...>& tp, index_constant<i> i_ = {})
    {
      return HybridTreePath<T...,index_constant<i>>{std::tuple_cat(tp._data,std::make_tuple(i_))};
    }

    //! Prepends a run time index to a `HybridTreePath`.
    /**
     * This function returns a new `HybridTreePath` with the run time index `i` prepended.
     */
    template<typename... T>
    constexpr auto push_front(const HybridTreePath<T...>& tp, std::size_t element)
    {
      return HybridTreePath<std::size_t,T...>{std::tuple_cat(std::make_tuple(element),tp._data)};
    }

    //! Prepends a compile time index to a `HybridTreePath`.
    /**
     * This function returns a new `HybridTreePath` with the compile time index `i` prepended.
     *
     * The value for the new entry can be passed either as a template parameter or as an `index_constant`:
     *
     * \code{.cc}
     * auto tp = hybridTreePath(1,2,3,4);
     * using namespace Dune::TypeTree::Indices;
     * // the following two lines are equivalent
     * auto tp_a = push_front<1>(tp);
     * auto tp_b = push_front(tp,_1);
     * \endcode
     *
     */
    template<std::size_t i, typename... T>
    constexpr auto push_front(const HybridTreePath<T...>& tp, index_constant<i> _i = {})
    {
      return HybridTreePath<index_constant<i>,T...>{std::tuple_cat(std::make_tuple(_i),tp._data)};
    }

#ifndef DOXYGEN

    namespace Impl {

      // end of recursion
      template<std::size_t i, typename... T>
      std::enable_if_t<(i == sizeof...(T))>
      print_hybrid_tree_path(std::ostream& os, const HybridTreePath<T...>& tp, index_constant<i> _i)
      {}

      // print current entry and recurse
      template<std::size_t i, typename... T>
      std::enable_if_t<(i < sizeof...(T))>
      print_hybrid_tree_path(std::ostream& os, const HybridTreePath<T...>& tp, index_constant<i> _i)
      {
        os << treePathIndex(tp,_i) << " ";
        print_hybrid_tree_path(os,tp,index_constant<i+1>{});
      }

    } // end namespace Impl

#endif // DOXYGEN

    //! Dumps a `HybridTreePath` to a stream.
    template<typename... T>
    std::ostream& operator<<(std::ostream& os, const HybridTreePath<T...>& tp)
    {
      os << "HybridTreePath< ";
      Impl::print_hybrid_tree_path(os, tp, index_constant<0>{});
      os << ">";
      return os;
    }

    //! \} group TypeTree

  } // namespace TypeTree
} //namespace Dune

#endif // DUNE_COMMON_TYPETREE_TREEPATH_HH
