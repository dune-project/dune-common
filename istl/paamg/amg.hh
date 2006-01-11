// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_AMG_AMG_HH
#define DUNE_AMG_AMG_HH

#include <memory>
#include <dune/common/exceptions.hh>
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
    template<class M, class X, class S, class PI=SequentialInformation,
        class A=std::allocator<X> >
    class AMG : public Preconditioner<X,X>
    {
    public:
      /** @brief The matrix operator type. */
      typedef M Matrix;
      /** @brief The type of the parallel information */
      typedef PI ParallelInformation;
      /** @brief The matrix type. */
      typedef MatrixHierarchy<M, ParallelInformation, A> MatrixHierarchy;

      typedef typename MatrixHierarchy::ParallelInformationHierarchy ParallelInformationHierarchy;

      /** @brief The domain type. */
      typedef X Domain;
      /** @brief The range type. */
      typedef X Range;
      /** @brief the type of the coarse solver. */
      typedef InverseOperator<X,X> CoarseSolver;
      /** @brief The type of the smoother. */
      typedef S Smoother;

      /** @brief The argument type for the construction of the smoother. */
      typedef typename SmootherTraits<Smoother>::Arguments SmootherArgs;

      enum {
        /** @brief The solver category. */
        category = S::category
      };

      enum GridFlag { owner, overlap};

      /**
       * @brief Construct a new amg with a specific coarse solver.
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

      /**
       * @brief Construct an AMG with an inexact coarse solver based on the smoother.
       *
       * As coarse solver a prconditioned CG method with the smoother as preconditioner
       * will be used. The matrix hierarchy is built automatically.
       * @param fineOperator The operator on the fine level.
       * @param pinfo The information about the parallel distribution of the data.
       * @param criterion The coarsen criterion.
       * @param smootherArgs The arguments for constructing the smoothers.
       * @param gamma 1 for V-cycle, 2 for W-cycle
       * @param smoothingSteps The number of smoothing steps for pre and postsmoothing.
       */
      template<class C>
      AMG(const Matrix& fineOperator, const ParallelInformation& pinfo=PI(), const C& criterion,
          const SmootherArgs& smootherArgs=SmootherArgs(), std::size_t gamma=1,
          std::size_t smoothingSteps=2);

      ~AMG();

      void pre(Domain& x, Range& b);

      void apply(Domain& v, const Range& d);

      void post(Domain& x);

    private:
      /** @brief Multigrid cycle on a level. */
      void mgc(typename Hierarchy<Smoother,A>::Iterator& smoother,
               typename MatrixHierarchy::ParallelMatrixHierarchy::ConstIterator& matrix,
               typename ParallelInformationHierarchy::Iterator& pinfo,
               typename MatrixHierarchy::AggregatesMapList::const_iterator& aggregates,
               typename Hierarchy<Domain,A>::Iterator& lhs,
               typename Hierarchy<Range,A>::Iterator& rhs,
               typename Hierarchy<Range,A>::Iterator& defect);

      //      void setupIndices(typename Matrix::ParallelIndexSet& indices, const Matrix& matrix);

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
      /** @brief Scalar product on the coarse level. */
      ScalarProduct<X>* scalarProduct_;
      /** @brief Gamma, 1 for V-cycle and 2 for W-cycle. */
      std::size_t gamma_;
      /** @brief The number of pre and postsmoothing steps. */
      std::size_t steps_;
      std::size_t level;
      bool buildHierarchy_;

      typedef MatrixAdapter<typename Matrix::matrix_type,Domain,Range> MatrixAdapter;
      Smoother *coarseSmoother_;
    };

    template<class M, class X, class S, class P, class A>
    AMG<M,X,S,P,A>::AMG(const MatrixHierarchy& matrices, CoarseSolver& coarseSolver,
                        const SmootherArgs& smootherArgs,
                        std::size_t gamma, std::size_t smoothingSteps)
      : matrices_(&matrices), smootherArgs_(smootherArgs),
        smoothers_(), solver_(&coarseSolver), scalarProduct_(0),
        gamma_(gamma), steps_(smoothingSteps), buildHierarchy_(false)
    {
      assert(matrices_->isBuilt());
      //printMatrix(matrices_->finest());

      // build the necessary smoother hierarchies
      matrices_->coarsenSmoother(smoothers_, smootherArgs_);

    }

    template<class M, class X, class S, class P, class A>
    template<class C>
    AMG<M,X,S,P,A>::AMG(const Matrix& matrix,
                        const P& pinfo,
                        const C& criterion,
                        const SmootherArgs& smootherArgs,
                        std::size_t gamma, std::size_t smoothingSteps)
      : smootherArgs_(smootherArgs),
        smoothers_(), scalarProduct_(0), gamma_(gamma),
        steps_(smoothingSteps), buildHierarchy_(true)
    {
      IsTrue<static_cast<int>(M::category)==static_cast<int>(S::category)>::yes();
      IsTrue<static_cast<int>(P::category)==static_cast<int>(S::category)>::yes();
      MatrixHierarchy* matrices = new MatrixHierarchy(const_cast<Matrix&>(matrix), pinfo);

      matrices->template build<EmptySet<int> >(criterion);

      matrices_ = matrices;
      // build the necessary smoother hierarchies
      matrices_->coarsenSmoother(smoothers_, smootherArgs_);

    }

    template<class M, class X, class S, class P, class A>
    AMG<M,X,S,P,A>::~AMG()
    {
      if(buildHierarchy_) {
        delete matrices_;
      }
      if(scalarProduct_)
        delete scalarProduct_;
    }

    /** \copydoc Preconditioner::pre */
    template<class M, class X, class S, class P, class A>
    void AMG<M,X,S,P,A>::pre(Domain& x, Range& b)
    {
      Range* copy = new Range(b);
      rhs_ = new Hierarchy<Range,A>(*copy);
      copy = new Range(b);
      defect_ = new Hierarchy<Range,A>(*copy);
      Domain* dcopy = new Domain(x);
      lhs_ = new Hierarchy<Domain,A>(*dcopy);
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
        smoother->pre(*lhs,*rhs);

      if(smoother != coarsest)
        for(++smoother, ++lhs, ++rhs; smoother != coarsest; ++smoother, ++lhs, ++rhs)
          smoother->pre(*lhs,*rhs);

      if(buildHierarchy_) {
        // Create the coarse Solver
        SmootherArgs sargs(smootherArgs_);
        sargs.iterations = 1;

        typename ConstructionTraits<Smoother>::Arguments cargs;
        cargs.setArgs(sargs);
        cargs.setMatrix(matrices_->matrices().coarsest()->getmat());

        coarseSmoother_ = ConstructionTraits<Smoother>::construct(cargs);
        scalarProduct_ = new OverlappingSchwarzScalarProduct<X,P>(*matrices_->parallelInformation().coarsest());

        solver_ = new CGSolver<X>(const_cast<M&>(*matrices_->matrices().coarsest()),
                                  static_cast<OverlappingSchwarzScalarProduct<X,P>&>(*scalarProduct_),
                                  *coarseSmoother_, 1E-12, 10000, 0);
      }
    }

    /** \copydoc Preconditioner::apply */
    template<class M, class X, class S, class P, class A>
    void AMG<M,X,S,P,A>::apply(Domain& v, const Range& d)
    {
      typename Hierarchy<Smoother,A>::Iterator smoother = smoothers_.finest();
      typename MatrixHierarchy::ParallelMatrixHierarchy::ConstIterator matrix = matrices_->matrices().finest();
      typename ParallelInformationHierarchy::Iterator pinfo = matrices_->parallelInformation().finest();
      typename MatrixHierarchy::AggregatesMapList::const_iterator aggregates = matrices_->aggregatesMaps().begin();
      typename Hierarchy<Domain,A>::Iterator lhs = lhs_->finest();
      typename Hierarchy<Range,A>::Iterator rhs = rhs_->finest();
      typename Hierarchy<Range,A>::Iterator defect = defect_->finest();

      *lhs = v;
      *rhs = d;

      level=0;
      mgc(smoother, matrix, pinfo, aggregates, lhs, rhs, defect);
      v=*lhs;
    }

    template<class M, class X, class S, class P, class A>
    void AMG<M,X,S,P,A>::mgc(typename Hierarchy<Smoother,A>::Iterator& smoother,
                             typename MatrixHierarchy::ParallelMatrixHierarchy::ConstIterator& matrix,
                             typename ParallelInformationHierarchy::Iterator& pinfo,
                             typename MatrixHierarchy::AggregatesMapList::const_iterator& aggregates,
                             typename Hierarchy<Domain,A>::Iterator& lhs,
                             typename Hierarchy<Range,A>::Iterator& rhs,
                             typename Hierarchy<Range,A>::Iterator& defect){
      if(matrix == matrices_->matrices().coarsest()) {
        // Solve directly
        InverseOperatorResult res;
        solver_->apply(*lhs, *rhs, res);
        if(!res.converged)
          DUNE_THROW(MathError, "Coarse solver did not converge");
      }else{

        // presmoothing
        for(std::size_t i=0; i < steps_; ++i) {
          smoother->apply(*lhs, *rhs);
        }

        // calculate defect
        *defect = *rhs;
        matrix->getmat().mmv(static_cast<const Domain&>(*lhs), *defect);

        //restrict defect to coarse level right hand side.
        ++rhs;
        ++pinfo;

        Transfer<typename MatrixHierarchy::AggregatesMap::AggregateDescriptor,Range,ParallelInformation>
        ::restrict (*(*aggregates), *rhs, static_cast<const Range&>(*defect), *pinfo);

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
        mgc(smoother, matrix, pinfo, aggregates, lhs, rhs, defect);

        if(matrix != matrices_->matrices().coarsest()) {
          --smoother;
          --aggregates;
        }
        --level;
        //prolongate and add the correction (update is in coarse left hand side)
        --matrix;
        --pinfo;

        typename Hierarchy<Domain,A>::Iterator coarseLhs = lhs--;
        Transfer<typename MatrixHierarchy::AggregatesMap::AggregateDescriptor,Range,ParallelInformation>
        ::prolongate(*(*aggregates), *coarseLhs, *lhs, 1.6);

        --rhs;
        --defect;

        // postsmoothing
        for(std::size_t i=0; i < steps_; ++i) {
          smoother->apply(*lhs, *rhs);
        }
      }
    }

    /** \copydoc Preconditioner::post */
    template<class M, class X, class S, class P, class A>
    void AMG<M,X,S,P,A>::post(Domain& x)
    {
      if(buildHierarchy_) {
        delete solver_;
        delete coarseSmoother_;
      }

      // Postprocess all smoothers
      typedef typename Hierarchy<Smoother,A>::Iterator Iterator;
      typedef typename Hierarchy<Range,A>::Iterator RIterator;
      typedef typename Hierarchy<Domain,A>::Iterator DIterator;
      Iterator coarsest = smoothers_.coarsest();
      Iterator smoother = smoothers_.finest();
      DIterator lhs = lhs_->finest();

      if(lhs!= lhs_->coarsest())
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
