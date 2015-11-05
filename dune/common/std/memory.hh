// -*- tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set ts=8 sw=2 et sts=2:
#ifndef DUNE_COMMON_STD_MEMORY_HH
#define DUNE_COMMON_STD_MEMORY_HH

#include <memory>
#include <utility>

namespace Dune
{

  namespace Std
  {

    // Helper struct to distinguish non-array, unknown bound
    // array, and known bound array types using SFINAE
    // following proposal N3656 by Stephan T. Lavavej.

    template<class T>
    struct MakeUniqueHelper
    {
      typedef std::unique_ptr<T> NonArrayUniquePtr;
    };

    template<class T>
    struct MakeUniqueHelper<T[]>
    {
      typedef std::unique_ptr<T[]> UnknownBoundArrayUniquePtr;
      typedef T RawType;
    };

    template<class T, size_t N>
    struct MakeUniqueHelper<T[N]>
    {
      typedef void KnownBoundArrayUniquePtr;
    };


    /** \brief Implementation of std::make_unique to be introduced in C++14
     *
     *  \tparam  T Nonarray type of object to be constructed
     *  \tparam  ...Args Parameter types for constructor of T
     *
     *  \param args Arguments to be passed to constructor of T
     *
     *  This fallback implementation using perfect forwarding
     *  as proposed by Herb Sutter in http://herbsutter.com/gotw/_102/
     */
    template<typename T, typename... Args>
    typename MakeUniqueHelper<T>::NonArrayUniquePtr
      make_unique(Args&&... args)
    {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    /** \brief Implementation of std::make_unique to be introduced in C++14
     *
     *  \tparam  T Array type of unknown bound
     *
     *  \param  n Size of array to allocate
     */
    template<typename T>
    typename MakeUniqueHelper<T>::UnknownBoundArrayUniquePtr
      make_unique(size_t n)
    {
      return std::unique_ptr<T>(new typename MakeUniqueHelper<T>::RawType[n]());
    }

    /** \brief Implementation of std::make_unique to be introduced in C++14
     *
     *  \tparam  T Array type of known bound
     *  \tparam  Args Dummy arguments
     *
     *  This is deleted, since, according to the standard this should not
     *  participate in overload resolution
     *
     *  \param args Dummy arguments
     */
    template<typename T, typename ...Args>
    typename MakeUniqueHelper<T>::KnownBoundArrayUniquePtr
      make_unique(Args&&... args) = delete;


  } // namespace Std

} // namespace Dune

#endif // #ifndef DUNE_COMMON_STD_MEMORY_HH
