// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id : $
#ifndef DUNE_AMG_AMG_HH
#define DUNE_AMG_AMG_HH

#include <dune/istl/paamg/smoother.hh>
#include <dune/istl/paamg/transfer.hh>
#include <dune/istl/paamg/hierarchy.hh>
#include <dune/istl/solvers.hh>
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
     * @brief The AMG preconditioner.
     */
    template<class M, class X, class Y, class CS, class S, class A>
    class AMG : public Preconditioner<X,Y>
    {
    public:
      /** @brief The matrix type. */
      typedef M MatrixHierarchy;
      /** @brief The domain type. */
      typedef X Domain;
      /** @brief The range type. */
      typedef Y Range;
      /** @brief the type of the coarse solver. */
      typedef CS CoarseSolver;
      /** @brief The type of the smoother. */
      typedef S Smoother;

      /** @brief The argument type for the construction of the smoother. */
      typedef typename SmootherTraits<Smoother>::Arguments SmootherArgs;

      enum {
        /** @brief The solver category which is parallel. */
        category = SolverCategory::sequential
      };

      /**
       * @brief Constructor.
       * @param matrix The matrix we precondition for.
       * @param solverArgs The arguments needed for the construction of the
       * direct solver to use at the coarsest level.
       * @param smootherArgs The  arguments needed for thesmoother to use
       * for pre and post smoothing
       * @param gamma The number of subcycles. 1 for V-cycle, 2 for W-cycle.
       */
      AMG(const MatrixHierarchy& matrices, CoarseSolver& coarseSolver,
          const SmootherArgs& smootherArgs, std::size_t gamma,
          std::size_t smoothingSteps);

      void pre(Domain& x, Range& b);

      void apply(Domain& v, const Range& d);

      void post(Domain& x);

      void mgc(typename Hierarchy<Smoother,A>::Iterator& smoother,
               const typename MatrixHierarchy::Iterator& matrix,
               typename Hierarchy<Domain,A>::Iterator& lhs,
               typename Hierarchy<Domain,A>::Iterator& update,
               typename Hierarchy<Range,A>::Iterator& rhs,
               typename Hierarchy<Range,A>::Iterator& defect);

    private:
      /**  @brief The matrix we solve. */
      const MatrixHierarchy* matrices_;
      /** @brief The arguments to construct the smoother */
      SmootherArgs smootherArgs_;
      /** @brief The hierarchy of the smoothers. */
      Hierarchy<Smoother,A> smoothers_;
      /** @brief The solver of the coarsest level. */
      const CoarseSolver* solver_;
      /** @brief The right hand side of our problem. */
      Hierarchy<Range,A>* rhs_;
      /** @brief The current defect. */
      Hierarchy<Range,A>* defect_;
      /** @brief The left approximate solution of our problem. */
      Hierarchy<Domain,A>* lhs_;
      /** @brief The  vectors to store the update in. */
      Hierarchy<Domain,A>* update_;
      std::size_t gamma_;
      std::size_t steps_;
    };

    template<class M, class X, class Y, class CS, class S, class A>
    AMG<M,X,Y,CS,S,A>::AMG(const MatrixHierarchy& matrices, CoarseSolver& coarseSolver,
                           const SmootherArgs& smootherArgs,
                           std::size_t gamma, std::size_t smoothingSteps)
      : matrices_(&matrices), solver_(coarseSolver), smootherArgs_(smootherArgs),
        smoothers_(), solver_(0), rhs_(), defect_(), lhs_(), update_(), gamma_(gamma),
        steps_(smoothingSteps)
    {
      assert(matrices_.built());
      // Initialize smoother on the finest level if necessary
      if(smoothers_.levels()==0) {
        typename SmootherTraits<S>::Arguments args;
        args.setMatrix(matrices_.finest()->matrix());
        args.setArgs(smootherArgs_);
        smoothers_.addCoarser(args);
      }
      // build the necessary smoother hierarchies
      matrices_.coarsenSmoother(smoothers_, smootherArgs_);

    }

    template<class M, class X, class Y, class CS, class S, class A>
    void AMG<M,X,Y,CS,S,A>::pre(X& x, Y& b)
    {
      Y* copy = new Y(b);
      rhs_ = new Hierarchy<Range,A>(copy);
      copy = new Y(b);
      defect_ = new Hierarchy<Range,A>(copy);
      copy = new X(x);
      lhs_ = new Hierarchy<Domain,A>(copy);
      copy = new X(x);
      update_ = new Hierarchy<Domain,A>(copy);
      matrices_.coarsenVector(*rhs_);
      matrices_.coarsenVector(*defect_);
      matrices_.coarsenVector(*lhs_);
      matrices_.coarsenVector(*update_);

      // Preprocess all smoothers
      typedef typename Hierarchy<Smoother,A>::Iterator Iterator;
      typedef typename Hierarchy<Range,A>::Iterator RIterator;
      typedef typename Hierarchy<Domain,A>::Iterator DIterator;
      Iterator coarsest = smoothers_.coarsest();
      Iterator smoother = smoothers_.finest();
      RIterator rhs = rhs_.finest();
      DIterator lhs = lhs_.finest();

      smoother.pre(*lhs,*rhs);

      for(++smoother, ++lhs, ++rhs; smoother != coarsest; ++smoother, ++lhs, ++rhs)
        smoother.pre(*lhs,*rhs);
    }

    template<class M, class X, class Y, class CS, class S, class A>
    void AMG<M,X,Y,CS,S,A>::apply(X& v, const Y& d)
    {
      *(lhs_->finest())=v;
      *(rhs_->finest())=d;

      //Create the iterator over the levels

      // Coarsest level
      typename Hierarchy<Domain>::Iterator coarsest =
        mgc(smoothers_.finest, matrices_.finest(), lhs_.finest(),
            update_.finest(), rhs_.finest(), defect_.finest());
    }

    template<class M, class X, class Y, class CS, class S, class A>
    void AMG<M,X,Y,CS,S,A>::mgc(typename Hierarchy<Smoother,A>::Iterator& smoother,
                                const typename MatrixHierarchy::Iterator& matrix,
                                typename Hierarchy<Domain,A>::Iterator& lhs,
                                typename Hierarchy<Domain,A>::Iterator& update,
                                typename Hierarchy<Range,A>::Iterator& rhs,
                                typename Hierarchy<Range,A>::Iterator& defect)
    {
      if(matrix == matrices_.coarsest()) {
        // Solve directly
        InverseOperatorResult res;
        solver_.apply(*lhs, rhs, res);
      }else{
        // presmoothing
        for(int i=0; i < steps_; ++i) {
          // calculate defect
          *defect = *rhs;
          matrix().matrix().mmv(*lhs, *defect);

          // smooth
          smoother->apply(*update, *rhs);

          //add correction
          *lhs += *update;
        }

        // calculate defect
        *defect = *rhs;
        matrix().matrix().mmv(*lhs, *defect);

        //restrict defect to coarse level right hand side.
        ++rhs;

        Transfer<typename MatrixHierarchy::AggregatesMap::Vertex,Range>::restrict (matrix->aggregates(),
                                                                                   rhs,
                                                                                   defect);
        // prepare coarse system
        ++lhs=0;
        ++update;
        ++defect;
        ++smoother;
        ++matrix;

        // next level
        mgc(smoother, matrix, lhs, update, rhs, defect);


        //prolongate (coarse x is the new update)
        --matrix;
        --update;
        Transfer<typename MatrixHierarchy::AggregatesMap::Vertex,Range>::prolongate(matrix->aggregates(),
                                                                                    update,
                                                                                    lhs,
                                                                                    0.8);
        --lhs;
        --rhs;
        --defect;
        --smoother;

        // postsmoothing
        for(int i=0; i < steps_; ++i) {

          // calculate defect
          *defect = *rhs;
          matrix().matrix().mmv(*lhs, *defect);

          // smooth
          smoother->apply(*update, *defect);

          //add correction
          *lhs += *update;
        }
      }
    }

    template<class M, class X, class Y, class CS, class S, class A>
    void AMG<M,X,Y,CS,S,A>::post(X& x)
    {
      // Postprocess all smoothers
      typedef typename Hierarchy<Smoother,A>::Iterator Iterator;
      typedef typename Hierarchy<Range,A>::Iterator RIterator;
      typedef typename Hierarchy<Domain,A>::Iterator DIterator;
      Iterator coarsest = smoothers_.coarsest();
      Iterator smoother = smoothers_.finest();
      DIterator lhs = lhs_.finest();

      smoother.post(*lhs);

      for(++smoother; smoother != coarsest; ++smoother, ++lhs)
        smoother.post(*lhs);

      delete &(*update_.finest());
      delete update_;
      delete &(*lhs_.finest());
      delete lhs_;
      delete &(*defect_.finest());
      delete defect_;
      delete &(*rhs_.finest());
      delete rhs_;
    }
  } // end namespace Amg
} // end namespace Dune

#endif
