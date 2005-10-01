// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_AMGSMOOTHER_HH
#define DUNE_AMGSMOOTHER_HH

#include <dune/istl/paamg/construction.hh>
#include <dune/istl/preconditioners.hh>
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
     * @brief Classes for the generic construction of the smoothers.
     */

    /**
     * @brief The default class for the smoother arguments.
     */
    template<class M>
    struct DefaultSmootherArgs
    {
      /**
       * @brief The type of matrix the smoother is for.
       */
      typedef M Matrix;

      /**
       * @brief The numbe of iterations to perform.
       */
      int iterations;

      /**
       * @brief The relaxation factor to use.
       */
      typename M::field_type relaxationFactor;

      /**
       * @brief Default constructor.
       */
      DefaultSmootherArgs()
        : iterations(1), relaxationFactor(1.0)
      {}
    };

    /**
     * @brief Traits class for getting the attribute class of a smoother.
     */
    template<class T>
    struct SmootherTraits
    {
      typedef DefaultSmootherArgs<T> Arguments;

    };

    template<class T>
    class ConstructionTraits;

    /**
     * @brief Construction Arguments for the default smoothers
     */
    template<class T>
    class DefaultConstructionArgs
    {
      friend class ConstructionTraits<T>;

      typedef typename T::matrix_type Matrix;

    public:
      void setMatrix(const Matrix& matrix)
      {
        matrix_=&matrix;
      }

      void setArgs(const DefaultSmootherArgs<T>& args)
      {
        args_=&args;
      }
    private:
      const Matrix* matrix_;
      const DefaultSmootherArgs<T>* args_;
    };


    /**
     * @brief Policy for the construction of the SeqSSOR smoother
     */
    template<class M, class X, class Y>
    struct ConstructionTraits<SeqSSOR<M,X,Y> >
    {
      typedef DefaultConstructionArgs<SeqSSOR<M,X,Y> > Arguments;

      static inline SeqSSOR<M,X,Y>* construct(Arguments& args)
      {
        return new SeqSSOR<M,X,Y>(*(args.matrix_), args.args_->iterations,
                                  args.args_->relaxationFactor);
      }

    };
  } // namespace Amg
} // namespace Dune



#endif
