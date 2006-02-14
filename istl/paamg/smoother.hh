// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_AMGSMOOTHER_HH
#define DUNE_AMGSMOOTHER_HH

#include <dune/istl/paamg/construction.hh>
#include <dune/istl/preconditioners.hh>
#include <dune/istl/schwarz.hh>

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
    template<class T>
    struct DefaultSmootherArgs
    {
      /**
       * @brief The type of the relaxation factor.
       */
      typedef T RelaxationFactor;

      /**
       * @brief The numbe of iterations to perform.
       */
      int iterations;
      /**
       * @brief The relaxation factor to use.
       */
      RelaxationFactor relaxationFactor;

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
      typedef DefaultSmootherArgs<typename T::matrix_type::field_type> Arguments;

    };

    template<class X, class Y, class C, class T>
    struct SmootherTraits<BlockPreconditioner<X,Y,C,T> >
    {
      typedef DefaultSmootherArgs<typename T::matrix_type::field_type> Arguments;

    };
    template<class T>
    class ConstructionTraits;

    /**
     * @brief Construction Arguments for the default smoothers
     */
    template<class T, class C=SequentialInformation>
    class DefaultConstructionArgs
    {
      typedef T Matrix;

      typedef DefaultSmootherArgs<typename Matrix::field_type> SmootherArgs;

    public:
      void setMatrix(const Matrix& matrix)
      {
        matrix_=&matrix;
      }

      const Matrix& getMatrix() const
      {
        return *matrix_;
      }

      void setArgs(const SmootherArgs& args)
      {
        args_=&args;
      }

      const SmootherArgs getArgs() const
      {
        return *args_;
      }

      void setComm(const C& comm)
      {
        comm_ = &comm;
      }

      const C& getComm() const
      {
        return *comm_;
      }


    private:
      const Matrix* matrix_;
      const SmootherArgs* args_;
      const C* comm_;
    };

    /**
     * @brief Policy for the construction of the SeqSSOR smoother
     */
    template<class M, class X, class Y>
    struct ConstructionTraits<SeqSSOR<M,X,Y> >
    {
      typedef DefaultConstructionArgs<M> Arguments;

      static inline SeqSSOR<M,X,Y>* construct(Arguments& args)
      {
        return new SeqSSOR<M,X,Y>(args.getMatrix(), args.getArgs().iterations,
                                  args.getArgs().relaxationFactor);
      }

    };

    /**
     * @brief Policy for the construction of the SeqJac smoother
     */
    template<class M, class X, class Y>
    struct ConstructionTraits<SeqJac<M,X,Y> >
    {
      typedef DefaultConstructionArgs<M> Arguments;

      static inline SeqJac<M,X,Y>* construct(Arguments& args)
      {
        return new SeqJac<M,X,Y>(args.getMatrix(), args.getArgs().iterations,
                                 args.getArgs().relaxationFactor);
      }

    };

    /**
     * @brief Policy for the construction of the ParSSOR smoother
     */
    template<class M, class X, class Y, class C>
    struct ConstructionTraits<ParSSOR<M,X,Y,C> >
    {
      typedef DefaultConstructionArgs<M,C> Arguments;

      static inline ParSSOR<M,X,Y,C>* construct(Arguments& args)
      {
        return new ParSSOR<M,X,Y,C>(args.getMatrix(), args.getArgs().iterations,
                                    args.getArgs().relaxationFactor,
                                    args.getComm());
      }
    };

    template<class X, class Y, class C, class T>
    struct ConstructionTraits<BlockPreconditioner<X,Y,C,T> >
    {
      typedef DefaultConstructionArgs<typename T::matrix_type,C> Arguments;

      static inline BlockPreconditioner<X,Y,C,T>* construct(Arguments& args)
      {
        return new BlockPreconditioner<X,Y,C,T>(*(new T(args.getMatrix(), args.getArgs().iterations,
                                                        args.getArgs().relaxationFactor)),
                                                args.getComm());
      }
    };
  } // namespace Amg
} // namespace Dune



#endif
