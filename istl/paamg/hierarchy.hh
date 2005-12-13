// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_AMGHIERARCHY_HH
#define DUNE_AMGHIERARCHY_HH

#include <list>
#include <memory>
#include "pmatrix.hh"
#include "aggregates.hh"
#include "graph.hh"
#include "galerkin.hh"
#include "renumberer.hh"
#include "graphcreator.hh"
#include <dune/common/stdstreams.hh>
#include <dune/common/timer.hh>
#include <dune/istl/bvector.hh>
#include <dune/istl/indexset.hh>
#include <dune/istl/remoteindices.hh>
#include <dune/istl/interface.hh>
#include <dune/istl/communicator.hh>
#include <dune/istl/remoteindices.hh>
#include <dune/istl/interface.hh>
#include <dune/istl/communicator.hh>
#include <dune/istl/paamg/dependency.hh>
#include <dune/istl/paamg/graph.hh>
#include <dune/istl/paamg/indicescoarsener.hh>
#include <dune/istl/paamg/globalaggregates.hh>
#include <dune/istl/paamg/construction.hh>
#include <dune/istl/paamg/smoother.hh>

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
     * @brief Provides a classes representing the hierarchies in AMG.
     */
    /**
     * @brief A hierarchy of coantainers (e.g. matrices or vectors)
     *
     * Because sometimes a redistribution of the parallel data might be
     * advisable one can add redistributed version of the container at
     * each level.
     */
    template<typename T, typename A=std::allocator<T> >
    class Hierarchy
    {
    public:
      /**
       * @brief The type of the container we store.
       */
      typedef T MemberType;

      template<typename T1, typename T2>
      class LevelIterator;

    private:
      /**
       * @brief An element in the hierarchy.
       */
      struct Element
      {
        friend class LevelIterator<Hierarchy<T,A>, T>;
        friend class LevelIterator<const Hierarchy<T,A>, const T>;

        /** @brief The next coarser element in the list. */
        Element* coarser_;

        /** @brief The next finer element in the list. */
        Element* finer_;

        /** @brief Pointer to the element. */
        MemberType* element_;

        /** @brief The redistributed version of the element. */
        MemberType* redistributed_;
      };
    public:
      //       enum{
      //        /**
      //         * @brief If true only the method addCoarser will be usable
      //         * otherwise only the method addFiner will be usable.
      //         */
      //        coarsen = b
      //          };

      /**
       * @brief The allocator to use for the list elements.
       */
      typedef typename A::template rebind<Element>::other Allocator;

      typedef typename ConstructionTraits<T>::Arguments Arguments;

      /**
       * @brief Construct a new hierarchy.
       * @param first The first element in the hierarchy.
       */
      Hierarchy(MemberType& first);

      /**
       * @brief Construct a new hierarchy.
       * @param first The first element in the hierarchy.
       */
      Hierarchy();

      /**
       * @brief Add an element on a coarser level.
       * @param args The arguments needed for the construction.
       */
      void addCoarser(Arguments& args);


      /**
       * @brief Add an element on a finer level.
       * @param args The arguments needed for the construction.
       */
      void addFiner(Arguments& args);

      /**
       * @brief Iterator over the levels in the hierarchy.
       *
       * operator++() moves to the next coarser level in the hierarchy.
       * while operator--() moved to the next finer level in the hierarchy.
       */
      template<class C, class T1>
      class LevelIterator
        : public BidirectionalIteratorFacade<LevelIterator<C,T1>,T1,T1&>
      {
        friend class LevelIterator<typename Dune::RemoveConst<C>::Type,
            typename Dune::RemoveConst<T1>::Type >;
        friend class LevelIterator<const typename Dune::RemoveConst<C>::Type,
            const typename Dune::RemoveConst<T1>::Type >;

      public:
        /** @brief Constructor. */
        LevelIterator()
          : element_(0)
        {}

        LevelIterator(Element* element)
          : element_(element)
        {}

        /** @brief Copy constructor. */
        LevelIterator(const LevelIterator<typename Dune::RemoveConst<C>::Type,
                          typename Dune::RemoveConst<T1>::Type>& other)
          : element_(other.element_)
        {}

        /** @brief Copy constructor. */
        LevelIterator(const LevelIterator<const typename Dune::RemoveConst<C>::Type,
                          const typename Dune::RemoveConst<T1>::Type>& other)
          : element_(other.element_)
        {}

        /**
         * @brief Equality check.
         */
        bool equals(const LevelIterator<typename Dune::RemoveConst<C>::Type,
                        typename Dune::RemoveConst<T1>::Type>& other) const
        {
          return element_ == other.element_;
        }

        /**
         * @brief Equality check.
         */
        bool equals(const LevelIterator<const typename Dune::RemoveConst<C>::Type,
                        const typename Dune::RemoveConst<T1>::Type>& other) const
        {
          return element_ == other.element_;
        }

        /** @brief Dereference the iterator. */
        T1& dereference() const
        {
          return *(element_->element_);
        }

        /** @brief Move to the next coarser level */
        void increment()
        {
          element_ = element_->coarser_;
        }

        /** @brief Move to the next fine level */
        void decrement()
        {
          element_ = element_->finer_;
        }

        /**
         * @brief Check whether there was a redistribution at the current level.
         * @return True if there is a redistributed version of the conatainer at the current level.
         */
        bool isRedistributed() const
        {
          return element_->redistributed_;
        }

        /**
         * @brief Get the redistributed container.
         * @return The redistributed container.
         */
        T1& getRedistributed() const
        {
          assert(element_->redistributed_);
          return *element_->redistributed_;
        }

      private:
        Element* element_;
      };

      /** @brief Type of the mutable iterator. */
      typedef LevelIterator<Hierarchy<T,A>,T> Iterator;

      /** @brief Type of the const iterator. */
      typedef LevelIterator<const Hierarchy<T,A>, const T> ConstIterator;

      /**
       * @brief Get an iterator positioned at the finest level.
       * @return An iterator positioned at the finest level.
       */
      Iterator finest();

      /**
       * @brief Get an iterator positioned at the coarsest level.
       * @return An iterator positioned at the coarsest level.
       */
      Iterator coarsest();


      /**
       * @brief Get an iterator positioned at the finest level.
       * @return An iterator positioned at the finest level.
       */
      ConstIterator finest() const;

      /**
       * @brief Get an iterator positioned at the coarsest level.
       * @return An iterator positioned at the coarsest level.
       */
      ConstIterator coarsest() const;

      /**
       * @brief Get the number of levels in the hierarchy.
       * @return The number of levels.
       */
      int levels() const;

      /** @brief Destructor. */
      ~Hierarchy();

    private:
      /** @brief The finest element in the hierarchy. */
      Element* finest_;
      /** @brief The coarsest element in the hierarchy. */
      Element* coarsest_;
      /** @brief Whether the first element was not allocated by us. */
      Element* nonAllocated_;
      /** @brief The allocator for the list elements. */
      Allocator allocator_;
      /** @brief The number of levels in the hierarchy. */
      int levels_;
    };

    /**
     * @brief The hierarchies build by the coarsening process.
     *
     * Namely a hierarchy of matrices, index sets, remote indices,
     * interfaces and communicators.
     */
    template<class M, class PI, class A=std::allocator<M> >
    class MatrixHierarchy
    {
    public:
      /** @brief The type of the matrix operator. */
      typedef M MatrixOperator;

      /** @brief The type of the matrix. */
      typedef typename MatrixOperator::matrix_type Matrix;

      /** @brief The type of the index set. */
      typedef PI ParallelInformation;

      /** @brief The allocator to use. */
      typedef A Allocator;

      /** @brief The type of the aggregates map we use. */
      typedef AggregatesMap<typename MatrixGraph<Matrix>::VertexDescriptor> AggregatesMap;

      /** @brief The type of the parallel matrix hierarchy. */
      typedef Hierarchy<MatrixOperator,Allocator> ParallelMatrixHierarchy;

      /** @brief The type of the parallel informarion hierarchy. */
      typedef Hierarchy<ParallelInformation,Allocator> ParallelInformationHierarchy;

      /** @brief Allocator for pointers. */
      typedef typename Allocator::template rebind<AggregatesMap*>::other AAllocator;

      /** @brief The type of the aggregates maps list. */
      typedef std::list<AggregatesMap*,AAllocator> AggregatesMapList;

      /**
       * @brief Constructor
       * @param fineMatrix The matrix to coarsen.
       * @param indexSet The index set mapping the global indices to matrix rows.
       * @param remoteIndices Information about the remote indices.
       */
      MatrixHierarchy(const MatrixOperator& fineMatrix,
                      const ParallelInformation& pinfo=ParallelInformation());


      ~MatrixHierarchy();

      /**
       * @brief The matrix hierarchy using aggregation.
       *
       * @brief criterion The criterion describing the aggregation process.
       */
      template<typename O, typename T>
      void build(const T& criterion);

      void recalculateGalerkin();

      template<class V, class TA>
      void coarsenVector(Hierarchy<BlockVector<V,TA> >& hierarchy) const;

      template<class S, class TA>
      void coarsenSmoother(Hierarchy<S,TA>& smoothers,
                           const typename SmootherTraits<S>::Arguments& args) const;

      /**
       * @brief Get the number of levels in the hierarchy.
       * @return The number of levels.
       */
      int levels() const;

      /**
       * @brief Whether the hierarchy wis built.
       * @return true if the ::coarsen method was called.
       */
      bool isBuilt() const;

      const ParallelMatrixHierarchy& matrices() const;

      const ParallelInformationHierarchy& parallelInformation() const;

      const AggregatesMapList& aggregatesMaps() const;

    private:
      typedef typename ConstructionTraits<MatrixOperator>::Arguments MatrixArgs;
      /** @brief The list of aggregates maps. */
      AggregatesMapList aggregatesMaps_;
      /** @brief The hierarchy of parallel matrices. */
      ParallelMatrixHierarchy matrices_;
      /** @brief The hierarchy of the parallel information. */
      ParallelInformationHierarchy parallelInformation_;

      /** @brief Whether the hierarchy was built. */
      bool built_;

      template<class T>
      bool coarsenTargetReached(const T& crit,
                                const typename ParallelMatrixHierarchy::Iterator& matrix,
                                const ParallelInformation& info);
    };

    template<class T>
    class CoarsenCriterion : public T
    {
    public:
      /**
       * @brief Set the maximum number of levels allowed in the hierarchy.
       */
      void setMaxLevel(int l)
      {
        maxLevel_ = l;
      }
      /**
       * @brief Get the maximum number of levels allowed in the hierarchy.
       */
      int maxLevel() const
      {
        return maxLevel_;
      }

      /**
       * @brief Set the maximum number of unknowns allowed on the coarsest level.
       */
      void setCoarsenTarget(int nodes)
      {
        coarsenTarget_ = nodes;
      }

      /**
       * @brief Get the maximum number of unknowns allowed on the coarsest level.
       */
      int coarsenTarget() const
      {
        return coarsenTarget_;
      }

      CoarsenCriterion(int maxLevel=100, int coarsenTarget=1000)
        : T(), maxLevel_(maxLevel), coarsenTarget_(coarsenTarget)
      {}

    private:
      /**
       * @brief The maximum number of levels allowed in the hierarchy.
       */
      int maxLevel_;
      /**
       * @brief The maximum number of unknowns allowed on the coarsest level.
       */
      int coarsenTarget_;
    };


    template<class M, class IS, class A>
    MatrixHierarchy<M,IS,A>::MatrixHierarchy(const MatrixOperator& fineOperator,
                                             const ParallelInformation& pinfo)
      : matrices_(const_cast<MatrixOperator&>(fineOperator)),
        parallelInformation_(const_cast<ParallelInformation&>(pinfo))
    {
      IsTrue<static_cast<int>(MatrixOperator::category) == static_cast<int>(SolverCategory::sequential) ||
          static_cast<int>(MatrixOperator::category) == static_cast<int>(SolverCategory::overlapping)>::yes();
      IsTrue<static_cast<int>(MatrixOperator::category) == static_cast<int>(ParallelInformation::category)>::yes();
    }

    template<class M, class IS, class A>
    template<typename T>
    inline bool
    MatrixHierarchy<M,IS,A>::coarsenTargetReached(const T& crit,
                                                  const typename ParallelMatrixHierarchy::Iterator& matrix,
                                                  const ParallelInformation& info)
    {
      int nodes = matrix->getmat().N();
      int totalNodes = info.globalSum(nodes);

      return totalNodes < crit.coarsenTarget();
    }

    template<class M, class IS, class A>
    template<typename O, typename T>
    void MatrixHierarchy<M,IS,A>::build(const T& criterion)
    {
      typedef O OverlapFlags;
      typedef typename ParallelMatrixHierarchy::Iterator MatIterator;
      typedef typename ParallelInformationHierarchy::Iterator PInfoIterator;

      GalerkinProduct productBuilder;
      MatIterator mlevel = matrices_.finest();
      PInfoIterator infoLevel = parallelInformation_.finest();


      int procs;
      int level = 0;


      infoLevel->processes(&procs);

      for(; level < criterion.maxLevel(); ++level, ++mlevel) {

        dinfo<<"Level "<<level<<" has "<<mlevel->getmat().N()<<" unknows!"<<std::endl;


        if(coarsenTargetReached(criterion, mlevel, *infoLevel))
          // No further coarsening needed
          break;

        typedef PropertiesGraphCreator<MatrixOperator> GraphCreator;
        typedef typename GraphCreator::PropertiesGraph PropertiesGraph;
        typedef typename GraphCreator::MatrixGraph MatrixGraph;
        typedef typename GraphCreator::GraphTuple GraphTuple;

        typedef typename PropertiesGraph::VertexDescriptor Vertex;

        std::vector<bool> excluded(mlevel->getmat().N(), false);

        GraphTuple graphs = GraphCreator::create(*mlevel, excluded, *infoLevel, OverlapFlags());

        AggregatesMap* aggregatesMap=new AggregatesMap(Element<1>::get(graphs)->maxVertex());

        aggregatesMaps_.push_back(aggregatesMap);

        Timer watch;
        watch.reset();
        int noAggregates = aggregatesMap->buildAggregates(mlevel->getmat(), *(Element<1>::get(graphs)), criterion);

        if(noAggregates < criterion.coarsenTarget() && procs>1) {
          DUNE_THROW(NotImplemented, "Accumulation to fewer processes not yet implemented!");
        }
        dinfo << "Building aggregates took "<<watch.elapsed()<<" seconds."<<std::endl;

        ParallelInformation* coarseInfo = new ParallelInformation(infoLevel->communicator());

        typename PropertyMapTypeSelector<VertexVisitedTag,PropertiesGraph>::Type visitedMap =
          get(VertexVisitedTag(), *(Element<1>::get(graphs)));

        watch.reset();
        int aggregates = IndicesCoarsener<ParallelInformation,OverlapFlags>
                         ::coarsen(*infoLevel,
                                   *(Element<1>::get(graphs)),
                                   visitedMap,
                                   *aggregatesMap,
                                   *coarseInfo);

        GraphCreator::free(graphs);

        parallelInformation_.addCoarser(*coarseInfo);

        PInfoIterator fineInfo = infoLevel;

        ++infoLevel;

        dinfo<<" Coarsening of index sets took "<<watch.elapsed()<<" seconds."<<std::endl;

        watch.reset();

        AggregatesPublisher<Vertex,OverlapFlags,ParallelInformation>::publish(*aggregatesMap,
                                                                              *infoLevel,
                                                                              mlevel->getmat().N());

        dinfo<<"Communicating global aggregate numbers took "<<watch.elapsed()<<" seconds."<<std::endl;

        watch.reset();
        std::vector<bool>& visited=excluded;

        typedef std::vector<bool>::iterator Iterator;
        typedef IteratorPropertyMap<Iterator, IdentityMap> VisitedMap2;
        Iterator end = visited.end();
        for(Iterator iter= visited.begin(); iter != end; ++iter)
          *iter=false;

        VisitedMap2 visitedMap2(visited.begin(), Dune::IdentityMap());

        typename MatrixOperator::matrix_type* coarseMatrix;

        coarseMatrix = productBuilder.build(mlevel->getmat(), *(Element<0>::get(graphs)), visitedMap2,
                                            *fineInfo,
                                            *aggregatesMap,
                                            aggregates,
                                            OverlapFlags());

        delete Element<0>::get(graphs);
        productBuilder.calculate(mlevel->getmat(), *aggregatesMap, *coarseMatrix);

        dinfo<<"Calculation of Galerkin product took "<<watch.elapsed()<<" seconds."<<std::endl;

        matrices_.addCoarser(MatrixArgs(*coarseMatrix, *infoLevel));
      }
      built_=true;
      AggregatesMap* aggregatesMap=new AggregatesMap(0);
      aggregatesMaps_.push_back(aggregatesMap);

      if(level==criterion.maxLevel())
        dinfo<<"Level "<<level<<" has "<<mlevel->getmat().N()<<" unknows!"<<std::endl;
    }

    template<class M, class IS, class A>
    const typename MatrixHierarchy<M,IS,A>::ParallelMatrixHierarchy&
    MatrixHierarchy<M,IS,A>::matrices() const
    {
      return matrices_;
    }

    template<class M, class IS, class A>
    const typename MatrixHierarchy<M,IS,A>::ParallelInformationHierarchy&
    MatrixHierarchy<M,IS,A>::parallelInformation() const
    {
      return parallelInformation_;
    }

    template<class M, class IS, class A>
    const typename MatrixHierarchy<M,IS,A>::AggregatesMapList&
    MatrixHierarchy<M,IS,A>::aggregatesMaps() const
    {
      return aggregatesMaps_;
    }
    template<class M, class IS, class A>
    MatrixHierarchy<M,IS,A>::~MatrixHierarchy()
    {
      typedef typename AggregatesMapList::reverse_iterator AggregatesMapIterator;
      typedef typename ParallelMatrixHierarchy::Iterator Iterator;
      typedef typename ParallelInformationHierarchy::Iterator InfoIterator;

      AggregatesMapIterator amap = aggregatesMaps_.rbegin();
      InfoIterator info = parallelInformation_.coarsest();
      int i=0;
      for(Iterator level=matrices_.coarsest(), finest=matrices_.finest(); level != finest;  --level, --info, ++amap) {
        std::cout<<"Freeing level "<<i++<<std::endl;
        (*amap)->free();
        delete *amap;
        delete &level->getmat();
      }
      delete *amap;
    }

    template<class M, class IS, class A>
    template<class V, class TA>
    void MatrixHierarchy<M,IS,A>::coarsenVector(Hierarchy<BlockVector<V,TA> >& hierarchy) const
    {
      assert(hierarchy.levels()==1);
      typedef typename ParallelMatrixHierarchy::ConstIterator Iterator;
      Iterator coarsest = matrices_.coarsest();
      int level=0;
      Dune::dverb<<"Level "<<level<<" has "<<matrices_.finest()->getmat().N()<<" unknows!"<<std::endl;

      for(Iterator matrix = matrices_.finest(); matrix != coarsest;) {
        ++matrix;
        ++level;
        Dune::dverb<<"Level "<<level<<" has "<<matrix->getmat().N()<<" unknows!"<<std::endl;
        hierarchy.addCoarser(matrix->getmat().N());
      }
    }

    template<class M, class IS, class A>
    template<class S, class TA>
    void MatrixHierarchy<M,IS,A>::coarsenSmoother(Hierarchy<S,TA>& smoothers,
                                                  const typename SmootherTraits<S>::Arguments& sargs) const
    {
      assert(smoothers.levels()==0);
      typedef typename ParallelMatrixHierarchy::ConstIterator Iterator;
      typename ConstructionTraits<S>::Arguments cargs;
      cargs.setArgs(sargs);
      Iterator coarsest = matrices_.coarsest();

      for(Iterator matrix = matrices_.finest(); matrix != coarsest; ++matrix) {
        cargs.setMatrix(matrix->getmat());
        smoothers.addCoarser(cargs);
      }
    }

    template<class M, class IS, class A>
    void MatrixHierarchy<M,IS,A>::recalculateGalerkin()
    {
      typedef typename AggregatesMapList::iterator AggregatesMapIterator;
      typedef typename ParallelMatrixHierarchy::Iterator Iterator;

      AggregatesMapIterator amap = aggregatesMaps_.begin();
      GalerkinProduct productBuilder;

      for(Iterator level = matrices_.finest(), coarsest=matrices_.coarsest(); level!=coarsest; ++amap) {
        const Matrix& fine = level->getmat();
        ++level;
        //Matrix& coarse(level->getmat());
        productBuilder.calculate(fine, *(*amap), const_cast<Matrix&>(level->getmat()));

      }
    }

    template<class M, class IS, class A>
    int MatrixHierarchy<M,IS,A>::levels() const
    {
      return matrices_.levels();
    }

    template<class M, class IS, class A>
    bool MatrixHierarchy<M,IS,A>::isBuilt() const
    {
      return built_;
    }

    template<class T, class A>
    Hierarchy<T,A>::Hierarchy()
      : finest_(0), coarsest_(0), nonAllocated_(0), allocator_(), levels_(0)
    {}

    template<class T, class A>
    Hierarchy<T,A>::Hierarchy(MemberType& first)
      : allocator_()
    {
      finest_ = allocator_.allocate(1,0);
      finest_->element_ = &first;
      finest_->redistributed_ = 0;
      nonAllocated_ = finest_;
      coarsest_ = finest_;
      coarsest_->coarser_ = coarsest_->finer_ = 0;
      levels_ = 1;
    }

    template<class T, class A>
    Hierarchy<T,A>::~Hierarchy()
    {
      while(coarsest_) {
        Element* current = coarsest_;
        coarsest_ = coarsest_->finer_;
        if(current != nonAllocated_) {
          delete current->element_;
        }
        allocator_.deallocate(current, 1);
        //coarsest_->coarser_ = 0;
      }
    }

    template<class T, class A>
    int Hierarchy<T,A>::levels() const
    {
      return levels_;
    }

    template<class T, class A>
    void Hierarchy<T,A>::addCoarser(Arguments& args)
    {
      if(!coarsest_) {
        assert(!finest_);
        coarsest_ = allocator_.allocate(1,0);
        coarsest_->element_ = ConstructionTraits<MemberType>::construct(args);
        finest_ = coarsest_;
        coarsest_->finer_ = 0;
        coarsest_->redistributed_ = 0;
      }else{
        coarsest_->coarser_ = allocator_.allocate(1,0);
        coarsest_->coarser_->finer_ = coarsest_;
        coarsest_ = coarsest_->coarser_;
        coarsest_->element_ = ConstructionTraits<MemberType>::construct(args);
        finest_->redistributed_ = 0;
      }
      coarsest_->coarser_=0;
      ++levels_;
    }

    template<class T, class A>
    void Hierarchy<T,A>::addFiner(Arguments& args)
    {
      if(!finest_) {
        assert(!coarsest_);
        finest_ = allocator_.allocate(1,0);
        finest_->element = ConstructionTraits<T>::construct(args);
        coarsest_ = finest_;
        coarsest_->coarser_ = coarsest_->finer_ = 0;
      }else{
        finest_->finer_ = allocator_.allocate(1,0);
        finest_->finer_->coarser_ = finest_;
        finest_ = finest_->finer_;
        finest_->finer = 0;
        finest_->element = ConstructionTraits<T>::construct(args);
      }
      ++levels_;
    }

    template<class T, class A>
    typename Hierarchy<T,A>::Iterator Hierarchy<T,A>::finest()
    {
      return Iterator(finest_);
    }

    template<class T, class A>
    typename Hierarchy<T,A>::Iterator Hierarchy<T,A>::coarsest()
    {
      return Iterator(coarsest_);
    }

    template<class T, class A>
    typename Hierarchy<T,A>::ConstIterator Hierarchy<T,A>::finest() const
    {
      return ConstIterator(finest_);
    }

    template<class T, class A>
    typename Hierarchy<T,A>::ConstIterator Hierarchy<T,A>::coarsest() const
    {
      return ConstIterator(coarsest_);
    }
    /** @} */
  } // namespace Amg
} // namespace Dune

#endif
