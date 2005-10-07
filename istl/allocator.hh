// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALLOCATOR_HH
#define DUNE_ALLOCATOR_HH

#include <stdlib.h>


namespace Dune {

  /**
              @addtogroup ISTL
              @{
   */

  /**
      \brief Default allocator for ISTL.

     The default allocator for the sparse matrix vector classes:

      - uses malloc and free
      - member templates for type safety to the outside
          - is a singleton
          - illustrates state handling through counter
          - throws std::bad_alloc just as new does

   */
  class ISTLAllocator { // uses new and delete
  public:
    //! The size type
    typedef int size_type;

    //! The difference type to meassure the distance between two pointers
    typedef std::ptrdiff_t difference_type;

    //! allocate array of nmemb objects of type T
    template<class T>
    static T* malloc (std::size_t nmemb)
    {
      T* p = new T[nmemb];
      return p;
    }

    //! release memory previously allocated with malloc member
    template<class T>
    static void free (T* p)
    {
      delete [] p;
    }
  };

  /** @} end documentation */

} // end namespace

#endif
