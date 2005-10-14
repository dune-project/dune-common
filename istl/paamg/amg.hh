// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_AMG_AMG_HH
#define DUNE_AMG_AMG_HH

#include <memory>
#include <dune/istl/paamg/smoother.hh>
#include <dune/istl/paamg/transfer.hh>
#include <dune/istl/paamg/hierarchy.hh>
#include <dune/istl/solvers.hh>
#include <dune/istl/scalarproducts.hh>
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
    template<class M, class X, class Y, class CS, class S, class A=std::allocator<X> >
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

    private:
      /** @brief Multigrid cycle on a level. */
      void mgc(typename Hierarchy<Smoother,A>::Iterator& smoother,
               typename MatrixHierarchy::ParallelMatrixHierarchy::ConstIterator& matrix,
               typename MatrixHierarchy::AggregatesMapList::const_iterator& aggregates,
               typename Hierarchy<Domain,A>::Iterator& lhs,
               typename Hierarchy<Range,A>::Iterator& rhs,
               typename Hierarchy<Range,A>::Iterator& defect);

      /**  @brief The matrix we solve. */
      const MatrixHierarchy* matrices_;
      /** @brief The arguments to construct the smoother */
      SmootherArgs smootherArgs_;
      /** @brief The hierarchy of the smoothers. */
      Hierarchy<Smoother,A> smoothers_;
      /** @brief The solver of the coarsest level. */
      CoarseSolver* solver_;
      /** @brief The right hand side of our problem. */
      Hierarchy<Range,A>* rhs_;
      /** @brief The current defect. */
      Hierarchy<Range,A>* defect_;
      /** @brief The left approximate solution of our problem. */
      Hierarchy<Domain,A>* lhs_;
      /** @brief Gamma, 1 for V-cycle and 2 for W-cycle. */
      std::size_t gamma_;
      /** @brief The number of pre and postsmoothing steps. */
      std::size_t steps_;
      std::size_t level;
      SeqScalarProduct<X> ssp;
    };

    template<class M, class X, class Y, class CS, class S, class A>
    AMG<M,X,Y,CS,S,A>::AMG(const MatrixHierarchy& matrices, CoarseSolver& coarseSolver,
                           const SmootherArgs& smootherArgs,
                           std::size_t gamma, std::size_t smoothingSteps)
      : matrices_(&matrices), smootherArgs_(smootherArgs),
        smoothers_(), solver_(&coarseSolver), gamma_(gamma),
        steps_(smoothingSteps), ssp()
    {
      assert(matrices_->isBuilt());
      //printMatrix(matrices_->finest());

      // build the necessary smoother hierarchies
      matrices_->coarsenSmoother(smoothers_, smootherArgs_);

    }

    /** \copydoc Preconditioner::pre */
    template<class M, class X, class Y, class CS, class S, class A>
    void AMG<M,X,Y,CS,S,A>::pre(X& x, Y& b)
    {
      Y* copy = new Y(b);
      rhs_ = new Hierarchy<Range,A>(*copy);
      copy = new Y(b);
      defect_ = new Hierarchy<Range,A>(*copy);
      copy = new X(x);
      lhs_ = new Hierarchy<Domain,A>(*copy);
      matrices_->coarsenVector(*rhs_);
      matrices_->coarsenVector(*defect_);
      matrices_->coarsenVector(*lhs_);

      // Preprocess all smoothers
      typedef typename Hierarchy<Smoother,A>::Iterator Iterator;
      typedef typename Hierarchy<Range,A>::Iterator RIterator;
      typedef typename Hierarchy<Domain,A>::Iterator DIterator;
      Iterator coarsest = smoothers_.coarsest();
      Iterator smoother = smoothers_.finest();
      RIterator rhs = rhs_->finest();
      DIterator lhs = lhs_->finest();

      if(rhs!=rhs_->coarsest())
        // No smoothers!
        smoother->pre(*lhs,*rhs);

      if(smoother != coarsest)
        for(++smoother, ++lhs, ++rhs; smoother != coarsest; ++smoother, ++lhs, ++rhs)
          smoother->pre(*lhs,*rhs);
    }

    /** \copydoc Preconditioner::apply */
    template<class M, class X, class Y, class CS, class S, class A>
    void AMG<M,X,Y,CS,S,A>::apply(X& v, const Y& d)
    {
      typename Hierarchy<Smoother,A>::Iterator smoother = smoothers_.finest();
      typename MatrixHierarchy::ParallelMatrixHierarchy::ConstIterator matrix = matrices_->matrices().finest();
      typename MatrixHierarchy::AggregatesMapList::const_iterator aggregates = matrices_->aggregatesMaps().begin();
      typename Hierarchy<Domain,A>::Iterator lhs = lhs_->finest();
      typename Hierarchy<Range,A>::Iterator rhs = rhs_->finest();
      typename Hierarchy<Range,A>::Iterator defect = defect_->finest();

      *lhs = v;
      *rhs = d;

      level=0;
      mgc(smoother, matrix, aggregates, lhs, rhs, defect);
      v=*lhs;
    }

    template<class M, class X, class Y, class CS, class S, class A>
    void AMG<M,X,Y,CS,S,A>::mgc(typename Hierarchy<Smoother,A>::Iterator& smoother,
                                typename MatrixHierarchy::ParallelMatrixHierarchy::ConstIterator& matrix,
                                typename MatrixHierarchy::AggregatesMapList::const_iterator& aggregates,
                                typename Hierarchy<Domain,A>::Iterator& lhs,
                                typename Hierarchy<Range,A>::Iterator& rhs,
                                typename Hierarchy<Range,A>::Iterator& defect){
      if(matrix == matrices_->matrices().coarsest()) {
        // Solve directly
        InverseOperatorResult res;
        solver_->apply(*lhs, *rhs, res);

      }else{

        // presmoothing
        for(std::size_t i=0; i < steps_; ++i) {
          smoother->apply(*lhs, *rhs);
        }

        // calculate defect
        *defect = *rhs;
        matrix->matrix().mmv(static_cast<const Domain&>(*lhs), *defect);

        //restrict defect to coarse level right hand side.
        ++rhs;

        Transfer<typename MatrixHierarchy::AggregatesMap::AggregateDescriptor,Range>
        ::restrict (*(*aggregates), *rhs, static_cast<const Range&>(*defect));

        // prepare coarse system
        ++lhs;
        ++defect;
        ++matrix;
        ++level;
        *lhs=0;

        if(matrix != matrices_->matrices().coarsest()) {
          ++smoother;
          ++aggregates;
        }

        // next level
        mgc(smoother, matrix, aggregates, lhs, rhs, defect);

        if(matrix != matrices_->matrices().coarsest()) {
          --smoother;
          --aggregates;
        }
        --level;
        //prolongate and add the correction (update is in coarse left hand side)
        --matrix;

        typename Hierarchy<Domain,A>::Iterator coarseLhs = lhs--;
        Transfer<typename MatrixHierarchy::AggregatesMap::AggregateDescriptor,Range>
        ::prolongate(*(*aggregates), static_cast<const Domain&>(*coarseLhs), *lhs, 1.0);

        --rhs;
        --defect;

        // postsmoothing
        for(std::size_t i=0; i < steps_; ++i) {
          smoother->apply(*lhs, *rhs);
        }
      }
    }

    /** \copydoc Preconditioner::post */
    template<class M, class X, class Y, class CS, class S, class A>
    void AMG<M,X,Y,CS,S,A>::post(X& x)
    {
      // Postprocess all smoothers
      typedef typename Hierarchy<Smoother,A>::Iterator Iterator;
      typedef typename Hierarchy<Range,A>::Iterator RIterator;
      typedef typename Hierarchy<Domain,A>::Iterator DIterator;
      Iterator coarsest = smoothers_.coarsest();
      Iterator smoother = smoothers_.finest();
      DIterator lhs = lhs_->finest();

      smoother->post(*lhs);

      if(smoother != coarsest)
        for(++smoother; smoother != coarsest; ++smoother, ++lhs)
          smoother->post(*lhs);

      delete &(*lhs_->finest());
      delete lhs_;
      delete &(*defect_->finest());
      delete defect_;
      delete &(*rhs_->finest());
      delete rhs_;
    }
  } // end namespace Amg
} // end namespace Dune

#endif
