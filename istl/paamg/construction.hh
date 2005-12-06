// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_AMGCONSTRUCTION_HH
#define DUNE_AMGCONSTRUCTION_HH

#include <dune/istl/bvector.hh>
#include <dune/istl/operators.hh>
#include "pinfo.hh"

namespace Dune
{
  namespace Amg
  {

    /**
     * @addtogroup ISTL_PAAMG
     *
     * @{
     */

    /** @file
     * @author Markus Blatt
     * @brief Helper classes for the construction of classes without
     * empty constructor.
     */
    /**
     * @brief Traits class for generically constructing non default
     * constructable types.
     *
     * Needed because BCRSMatrix and Vector do a deep copy which is
     * too expensive.
     */
    template<typename T>
    class ConstructionTraits
    {
    public:
      /**
       * @brief A type holding all the arguments needed to call the
       * constructor.
       */
      typedef const void* Arguments;

      /**
       * @brief Construct an object with the specified arguments.
       *
       * In the default implementation the copy constructor is called.
       * @param object Pointer to the space allocated for the object.
       * @param arguments The arguments for the construction.
       */
      static inline T* construct(Arguments&  args)
      {
        return new T();
      }
    };

    template<class T>
    class ConstructionTraits<BlockVector<T> >
    {
    public:
      typedef const int Arguments;
      static inline BlockVector<T>* construct(Arguments& n)
      {
        return new BlockVector<T>(n);
      }
    };

    template<class M, class X, class Y>
    struct MatrixAdapterArgs
    {
      MatrixAdapterArgs(M& matrix, const SequentialInformation&)
        : matrix_(&matrix)
      {}

      M* matrix_;
    };

    template<class M, class X, class Y>
    class ConstructionTraits<MatrixAdapter<M,X,Y> >
    {
    public:
      typedef const MatrixAdapterArgs<M,X,Y> Arguments;

      static inline MatrixAdapter<M,X,Y>* construct(Arguments& args)
      {
        return new MatrixAdapter<M,X,Y>(*args.matrix_);
      }
    };

    /*
       template<class M, class T, class X, class Y>
       struct ParallelMatrixArgs
       {
       ParallelMatrixArgs(M& matrix, const ParallelInformation<T>& info)
        : matrix_(&matrix), info_(&info)
       {}

       M* matrix_;
       ParallelInformation<T>* info_;
       };

       template<class M, class T, class X, class Y>
       class ConstructionTraits<ParallelMatrix<M,T,X,Y> >
       {
       public:
       typedef const ParallelMatrixArgs<M,T,X,Y> Arguments;

       static inline ParallelMatrix<M,T,X,Y>* construct(Arguments& args)
       {
        return new MatrixAdapter<M,T,X,Y>(*args.matrix_, *args.info_);
       }
       };
     */
    template<>
    class ConstructionTraits<SequentialInformation>
    {
    public:
      typedef const SequentialInformation Arguments;

      static inline SequentialInformation* construct(Arguments& args)
      {
        return new SequentialInformation(args);
      }
    };


    template<typename T>
    class ConstructionTraits<ParallelInformation<T> >
    {
    public:
      typedef const ParallelInformation<T> Arguments;

      static inline ParallelInformation<T>* construct(Arguments& args)
      {
        return new ParallelInformation<T>(args);
      }
    };

    /** @} */
  } // namespace Amg
} // namespace Dune
#endif
