// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ALLOCATOR_HH__
#define __DUNE_ALLOCATOR_HH__

#include <stdlib.h>


namespace Dune {

  /** @defgroup ISTL Iterative Solvers Template Library
              @addtogroup ISTL
              @{
   */

  /** The default allocator for the sparse matrix vector classes:

      - uses malloc and free
      - member templates for type safety to the outside
          - is a singleton
          - illustrates state handling through counter
          - throws std::bad_alloc just as new does

   */
  class ISTL_Allocator { // uses standard malloc and free
  public:
    //! allocate array of nmemb objects of type T
    template<class T>
    static T* malloc (size_t nmemb)
    {
      T* p = static_cast<T*>(std::malloc(nmemb*sizeof(T)));
      if (p==NULL) throw std::bad_alloc();
      count++;
      return p;
    }

    //! release memory previously allocated with malloc member
    template<class T>
    static void free (T* p)
    {
      std::free(p);
      count--;
    }

    //! return number of allocated objects
    unsigned int nobjects ()
    {
      return count;
    }

  private:
    // just to demonstrate some state information
    static unsigned int count;
  };

  unsigned int ISTL_Allocator::count=0;

  /** @} end documentation */

} // end namespace

#endif
