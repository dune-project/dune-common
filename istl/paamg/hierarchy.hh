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
    template<class M, class IS, class O, class A=std::allocator<M> >
    class MatrixHierarchy
    {
    public:
      /** @brief The type of the matrix. */
      typedef M Matrix;
      /** @brief The type of the index set. */
      typedef IS IndexSet;
      /** @brief The type of the remote indices. */
      typedef RemoteIndices<IndexSet> RemoteIndices;
      /** @brief The type of the Interface. */
      typedef Interface<IndexSet> Interface;
      /** @brief The type of the Communicator. */
      typedef BufferedCommunicator<IndexSet> Communicator;
      /** @brief The type of the parallel matrix. */
      typedef ParallelMatrix<Matrix,IndexSet,RemoteIndices> ParallelMatrix;
      /** @brief The flags identifying the overlap attributes */
      typedef O OverlapFlags;
      /** @brief The allocator to use. */
      typedef A Allocator;
      /** @brief The type of the aggregates map we use. */
      typedef AggregatesMap<typename MatrixGraph<M>::VertexDescriptor> AggregatesMap;
      /** @brief The type of the parallel matrix hierarchy. */
      typedef Hierarchy<ParallelMatrix,Allocator> ParallelMatrixHierarchy;

      /** @brief The type of the aggregates maps list. */
      typedef std::list<AggregatesMap*,Allocator> AggregatesMapList;
      /**
       * @brief Constructor
       * @param fineMatrix The matrix to coarsen.
       * @param indexSet The index set mapping the global indices to matrix rows.
       * @param remoteIndices Information about the remote indices.
       */
      MatrixHierarchy(const Matrix& fineMatrix,
                      const IndexSet& indexSet,
                      const RemoteIndices& remoteIndices,
                      Interface& interface);

      ~MatrixHierarchy();

      /**
       * @brief The matrix hierarchy using aggregation.
       *
       * @brief criterion The criterion describing the aggregation process.
       */
      template<typename T>
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

      const AggregatesMapList& aggregatesMaps() const;

    private:
      typedef typename ConstructionTraits<ParallelMatrix>::Arguments MatrixArgs;
      /** @brief The list of aggregates maps. */
      AggregatesMapList aggregatesMaps_;
      /** @brief The hierarchy of parallel matrices. */
      ParallelMatrixHierarchy matrices_;
      typedef Hierarchy<Interface,Allocator> InterfaceHierarchy;
      /** @brief The hierarchy of interfaces. */
      InterfaceHierarchy interfaces_;
      typedef Hierarchy<Communicator,Allocator> CommunicatorHierarchy;
      /** @brief The hierarchy of communicators. */
      CommunicatorHierarchy communicators_;

      /** @brief Whether the hierarchy wis built. */
      bool built_;

      template<class T>
      bool coarsenTargetReached(const T& crit,
                                const typename ParallelMatrixHierarchy::Iterator& matrix);
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


    template<class M, class IS, class O, class A>
    MatrixHierarchy<M,IS,O,A>::MatrixHierarchy(const Matrix& fineMatrix,
                                               const IndexSet& indexSet,
                                               const RemoteIndices& remoteIndices,
                                               Interface& interface)
      : matrices_(*new ParallelMatrix(fineMatrix,indexSet,remoteIndices)),
        interfaces_(interface), communicators_(*(new Communicator())), built_(false)
    {}

    template<class M, class IS, class O, class A>
    template<typename T>
    inline bool
    MatrixHierarchy<M,IS,O,A>::coarsenTargetReached(const T& crit,
                                                    const typename ParallelMatrixHierarchy::Iterator& matrix)
    {
      int nodes = matrix->matrix().N();
      int totalNodes;

      MPI_Allreduce(&nodes, &totalNodes, 1, MPI_INT, MPI_SUM, matrix->remoteIndices().communicator());

      return totalNodes < crit.coarsenTarget();
    }

    template<class M, class IS, class O, class A>
    template<typename T>
    void MatrixHierarchy<M,IS,O,A>::build(const T& criterion)
    {
      GalerkinProduct productBuilder;
      int procs;
      typedef typename ParallelMatrixHierarchy::Iterator MatIterator;
      typedef typename InterfaceHierarchy::Iterator InterfaceIterator;
      typedef typename CommunicatorHierarchy::Iterator CommIterator;

      MatIterator mlevel = matrices_.finest();
      InterfaceIterator iflevel = interfaces_.finest();
      CommIterator commlevel = communicators_.finest();

      MPI_Comm_size(mlevel->remoteIndices().communicator(), &procs);

      for(int level=0; level < criterion.maxLevel(); ++level, ++mlevel) {

        if(coarsenTargetReached(criterion, mlevel))
          // No further coarsening needed
          break;

        typedef MatrixGraph<const M> MatrixGraph;
        typedef SubGraph<MatrixGraph,std::vector<bool> > SubGraph;
        typedef PropertiesGraph<SubGraph,VertexProperties,EdgeProperties,
            IdentityMap,typename SubGraph::EdgeIndexMap> PropertiesGraph;
        typedef typename PropertiesGraph::VertexDescriptor Vertex;

        MatrixGraph mg(mlevel->matrix());
        std::vector<bool> excluded(mlevel->matrix().N());
        typedef typename IndexSet::const_iterator IndexIterator;
        IndexIterator iend = mlevel->indexSet().end();
        typename std::vector<bool>::iterator iter=excluded.begin();

        for(IndexIterator index = mlevel->indexSet().begin(); index != iend; ++index, ++iter)
          *iter = (OverlapFlags::contains(index->local().attribute()));

        SubGraph sg(mg, excluded);
        PropertiesGraph pg(sg, IdentityMap(), sg.getEdgeIndexMap());
        AggregatesMap* aggregatesMap=new AggregatesMap(pg.maxVertex());

        aggregatesMaps_.push_back(aggregatesMap);

        int noAggregates = aggregatesMap->buildAggregates(mlevel->matrix(), pg, criterion);

        if(noAggregates < criterion.coarsenTarget() && procs>1) {
          std::cerr<<"Accumulation to fewer processes not yet implemented!"<<std::endl;
          throw "Not implemented!";
        }

        IndexSet*      coarseIndices = new IndexSet();
        RemoteIndices* coarseRemote = new RemoteIndices(*coarseIndices, *coarseIndices,
                                                        mlevel->remoteIndices().communicator());

        typename PropertyMapTypeSelector<VertexVisitedTag,PropertiesGraph>::Type visitedMap =
          get(VertexVisitedTag(), pg);

        IndicesCoarsener<OverlapFlags,IndexSet>::coarsen(mlevel->indexSet(),
                                                         mlevel->remoteIndices(),
                                                         pg,
                                                         visitedMap,
                                                         *aggregatesMap,
                                                         *coarseIndices,
                                                         *coarseRemote);

        const void* args;
        communicators_.addCoarser(args);
        ++commlevel;
        interfaces_.addCoarser(args);
        ++iflevel;
        iflevel->build(*coarseRemote, NegateSet<OverlapFlags>(), OverlapFlags());
        typedef Dune::Amg::GlobalAggregatesMap<Vertex,IndexSet> GlobalMap;
        GlobalMap gmap(*aggregatesMap, *coarseIndices);

        commlevel->build<GlobalMap>(*iflevel);
        commlevel->template forward<Dune::Amg::AggregatesGatherScatter<Vertex,IndexSet> >(gmap);

        commlevel->free();

        std::vector<bool>& visited=excluded;

        typedef std::vector<bool>::iterator Iterator;
        typedef IteratorPropertyMap<Iterator, IdentityMap> VisitedMap2;
        Iterator end = visited.end();
        for(Iterator iter= visited.begin(); iter != end; ++iter)
          *iter=false;

        VisitedMap2 visitedMap2(visited.begin(), Dune::IdentityMap());

        Matrix* coarseMatrix = productBuilder.build(mlevel->matrix(), mg, visitedMap2,
                                                    mlevel->indexSet(),
                                                    *aggregatesMap,
                                                    OverlapFlags());
        productBuilder.calculate(mlevel->matrix(), *aggregatesMap, *coarseMatrix);

        matrices_.addCoarser(MatrixArgs(*coarseMatrix, *coarseIndices, *coarseRemote));
      }
      built_=true;
      AggregatesMap* aggregatesMap=new AggregatesMap(0);
      aggregatesMaps_.push_back(aggregatesMap);
    }

    template<class M, class IS, class R, class I>
    const typename MatrixHierarchy<M,IS,R,I>::ParallelMatrixHierarchy&
    MatrixHierarchy<M,IS,R,I>::matrices() const
    {
      return matrices_;
    }


    template<class M, class IS, class R, class I>
    const typename MatrixHierarchy<M,IS,R,I>::AggregatesMapList&
    MatrixHierarchy<M,IS,R,I>::aggregatesMaps() const
    {
      return aggregatesMaps_;
    }
    template<class M, class IS, class R, class I>
    MatrixHierarchy<M,IS,R,I>::~MatrixHierarchy()
    {
      typedef typename AggregatesMapList::reverse_iterator AggregatesMapIterator;
      typedef typename ParallelMatrixHierarchy::Iterator Iterator;

      AggregatesMapIterator amap = aggregatesMaps_.rbegin();

      for(Iterator level=matrices_.coarsest(), finest=matrices_.finest(); level != finest;  --level, ++amap) {
        delete *amap;
        if(level.isRedistributed()) {
          ParallelMatrix& mat = level.getRedistributed();
          delete &mat.remoteIndices();
          delete &mat.indexSet();
          delete &mat.matrix();
        }

        delete &level->remoteIndices();
        delete &level->indexSet();
        delete &level->matrix();
      }
    }

    template<class M, class IS, class R, class I>
    template<class V, class TA>
    void MatrixHierarchy<M,IS,R,I>::coarsenVector(Hierarchy<BlockVector<V,TA> >& hierarchy) const
    {
      assert(hierarchy.levels()==1);
      typedef typename ParallelMatrixHierarchy::ConstIterator Iterator;
      Iterator coarsest = matrices_.coarsest();
      int level=0;
      std::cout<<"Level "<<level<<" has "<<matrices_.finest()->matrix().N()<<" unknows!"<<std::endl;

      for(Iterator matrix = matrices_.finest(); matrix != coarsest;) {
        ++matrix;
        ++level;
        std::cout<<"Level "<<level<<" has "<<matrix->matrix().N()<<" unknows!"<<std::endl;
        hierarchy.addCoarser(matrix->matrix().N());
      }
    }

    template<class M, class IS, class R, class I>
    template<class S, class TA>
    void MatrixHierarchy<M,IS,R,I>::coarsenSmoother(Hierarchy<S,TA>& smoothers,
                                                    const typename SmootherTraits<S>::Arguments& sargs) const
    {
      assert(smoothers.levels()==0);
      typedef typename ParallelMatrixHierarchy::ConstIterator Iterator;
      typename ConstructionTraits<S>::Arguments cargs;
      cargs.setArgs(sargs);
      Iterator coarsest = matrices_.coarsest();
      int level=0;
      for(Iterator matrix = matrices_.finest(); matrix != coarsest; ++matrix) {
        std::cout<<"level "<<level++<<" "<<&matrix->matrix();
        cargs.setMatrix(matrix->matrix());
        smoothers.addCoarser(cargs);
      }
    }

    template<class M, class IS, class R, class I>
    void MatrixHierarchy<M,IS,R,I>::recalculateGalerkin()
    {
      typedef typename AggregatesMapList::iterator AggregatesMapIterator;
      typedef typename ParallelMatrixHierarchy::Iterator Iterator;

      AggregatesMapIterator amap = aggregatesMaps_.begin();
      GalerkinProduct productBuilder;

      for(Iterator level = matrices_.finest(), coarsest=matrices_.coarsest(); level!=coarsest; ++amap) {
        const Matrix& fine = level->matrix();
        ++level;
        //Matrix& coarse(level->matrix());
        productBuilder.calculate(fine, *(*amap), const_cast<Matrix&>(level->matrix()));

      }
    }

    template<class M, class IS, class R, class I>
    int MatrixHierarchy<M,IS,R,I>::levels() const
    {
      return matrices_.levels();
    }

    template<class M, class IS, class R, class I>
    bool MatrixHierarchy<M,IS,R,I>::isBuilt() const
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
      nonAllocated_ = finest_;
      coarsest_ = finest_;
      levels_ = 1;
    }

    template<class T, class A>
    Hierarchy<T,A>::~Hierarchy()
    {
      while(coarsest_) {
        Element* current = coarsest_;
        coarsest_ = coarsest_->finer_;
        if(current != nonAllocated_) {
          current->element_->~T();
          allocator_.deallocate(current, 1);
        }
        current->finer_=0;
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
        coarsest_->coarser_ = coarsest_->finer_ = 0;
      }else{
        coarsest_->coarser_ = allocator_.allocate(1,0);
        coarsest_->coarser_->finer_ = coarsest_;
        coarsest_ = coarsest_->coarser_;
        coarsest_->element_ = ConstructionTraits<MemberType>::construct(args);
      }
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
