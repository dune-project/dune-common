// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef AGGREGATES_HH
#define AGGREGATES_HH

#include "graph.hh"
#include <dune/common/poolallocator.hh>
#include <dune/common/sllist.hh>
#include <set>
#include <algorithm>

namespace Dune
{
  namespace amg
  {

    /**
     * @addtogroup ISTL_PAAMG
     *
     * @{
     */
    /** @file
     * @author Markus Blatt
     * @brief Provides classes for the Coloring process of AMG
     */

    /**
     * @brief Base class of all aggregation criterions.
     */
    template<class T>
    class AggregationCriterion : public T
    {

    public:
      /**
       * @brief The policy for calculating the dependency graph.
       */
      typedef T DependencyPolicy;

      /**
       * @brief Constructor.
       */
      AggregationCriterion()
        : maxDistance_(2), minAggregateSize_(4), maxAggregateSize_(8),
          connectivity_(27), debugLevel_(3)
      {}


      /**
       * @brief Get the maximal distance allowed between to nodes in a aggregate.
       *
       * The distance between two nodes in a aggregate is the minimal number of edges
       * it takes to travel from one node to the other without leaving the aggregate.
       * @return The maximum distance allowed.
       */
      int maxDistance() const { return maxDistance_;}

      /**
       * @brief Set the maximal distance allowed between to nodes in a aggregate.
       *
       * The distance between two nodes in a aggregate is the minimal number of edges
       * it takes to travel from one node to the other without leaving the aggregate.
       * The default value is 2.
       * @param distance The maximum distance allowed.
       */
      void setMaxDistance(int distance) { maxDistance_ = distance;}

      /**
       * @brief Get the minimum number of nodes a aggregate has to consist of.
       * @return The minimum number of nodes.
       */
      int minAggregateSize() const { return minAggregateSize_;}

      /**
       * @brief Set the minimum number of nodes a aggregate has to consist of.
       *
       * the default value is 4.
       * @return The minimum number of nodes.
       */
      void setMinAggregateSize(int size){ minAggregateSize_=size;}

      /**
       * @brief Get the maximum number of nodes a aggregate is allowed to have.
       * @return The maximum number of nodes.
       */
      int maxAggregateSize() const { return maxAggregateSize_;}

      /**
       * @brief Set the maximum number of nodes a aggregate is allowed to have.
       *
       * The default values is 6.
       * @param size The maximum number of nodes.
       */
      void setMaxAggregateSize(int size){ maxAggregateSize_ = size;}

      /**
       * @brief Get the maximum number of connections a aggregate is allowed to have.
       *
       * This limit exists to achieve sparsity of the coarse matrix. the default value is 15.
       *
       * @return The maximum number of connections a aggregate is allowed to have.
       */
      int maxConnectivity() const { return connectivity_;}

      /**
       * @brief Set the maximum number of connections a aggregate is allowed to have.
       *
       * This limit exists to achieve sparsity of the coarse matrix. the default value is 15.
       *
       * @param connectivity The maximum number of connections a aggregate is allowed to have.
       */
      void setMaxConnectivity(int connectivity){ connectivity_ = connectivity;}

      /**
       * @brief Set the debugging level.
       *
       * @param level If 0 no debugging output will be generated.
       */
      void setDebugLevel(int level)
      {
        debugLevel_ = level;
      }

      /**
       * @brief Get the debugging Level.
       *
       * @return 0 if no debugging output will be generated.
       */
      int debugLevel() const
      {
        return debugLevel_;
      }


    private:
      int maxDistance_, minAggregateSize_, maxAggregateSize_, connectivity_, debugLevel_;
    };

    class DependencyParameters
    {
    public:
      DependencyParameters()
        : alpha_(1.0/3.0), beta_(1.0E-5)
      {}

      void setBeta(double b)
      {
        beta_ = b;
      }

      double beta() const
      {
        return beta_;
      }

      void setAlpha(double a)
      {
        alpha_ = a;
      }

      double alpha() const
      {
        return alpha_;
      }

    private:
      double alpha_, beta_;
    };


    /**
     * @brief Dependency policy for symmetric matrices.
     */
    template<class G, class M, template<typename> class N >
    class SymmetricDependency : public DependencyParameters
    {
    public:
      /**
       * @brief The matrix type we build the dependency of.
       */
      typedef M Matrix;

      typedef G MatrixGraph;

      /**
       * @brief The norm to use for examining the matrix entries.
       */
      typedef N<typename Matrix::block_type> Norm;

      /**
       * @brief Constant Row iterator of the matrix.
       */
      typedef typename Matrix::row_type Row;

      /**
       * @brief Constant column iterator of the matrix.
       */
      typedef typename Matrix::ConstColIterator ColIter;

      /**
       * @brief Constant edge iterator oh the matrix graph.
       */
      typedef typename MatrixGraph::EdgeIterator EdgeIter;

      void init(MatrixGraph* graph, const Matrix* matrix);

      void initRow(const Row& row, int index);

      void examine(const ColIter& col);

      void examine(const EdgeIter& edge, const ColIter& col);

      bool isIsolated();
    private:
      /** @brief The graph we work on. */
      MatrixGraph *graph_;
      /** @brief The matrix we work on. */
      const Matrix* matrix_;
      /** @brief The current max value.*/
      double maxValue_;
      /** @brief The funktor for calculating the norm. */
      Norm norm_;
      /** @brief index of the currently evaluated row. */
      int row_;
      /** @brief The norm of the current diagonal. */
      double diagonal_;
    };

    template<class M>
    class FirstDiagonal
    {
    public:
      typename M::field_type operator()(const M& m) const
      {
        return m[0][0];
      }
    };


    template<class G, class M, template<typename>class Norm>
    class SymmetricCriterion : public AggregationCriterion<SymmetricDependency<G,M,Norm> >
    {};

    // forward declaration
    template<class G> class Aggregates;

    /**
     * @brief A class for temporarily storing the vertices of an
     * aggregate in.
     */
    template<class G>
    class Aggregate
    {

    public:

      /***
       * @brief The type of the matrix graph we work with.
       */
      typedef G MatrixGraph;
      /**
       * @brief The vertex descriptor type.
       */
      typedef typename MatrixGraph::VertexDescriptor Vertex;

      /**
       * @brief The allocator we use for our lists and the
       * set.
       */
      typedef PoolAllocator<Vertex,100*sizeof(int)> Allocator;

      /**
       * @brief The type of a single linked list of vertex
       * descriptors.
       */
      typedef SLList<Vertex,Allocator> VertexList;


      /**
       * @brief The type of a single linked list of vertex
       * descriptors.
       */
      typedef std::set<Vertex> VertexSet;

      typedef typename VertexList::const_iterator const_iterator;

      /**
       * @brief Constructor.
       * @param graph The matrix graph we work on.
       */
      Aggregate(const MatrixGraph& graph, Aggregates<MatrixGraph>& aggregates, VertexSet& connectivity);

      /**
       * @brief Reconstruct the aggregat from an seed node.
       *
       * Will determine all vertices of the same agggregate
       * and reference those.
       */
      void reconstruct(const Vertex& vertex);

      /**
       * @brief Initialize the aggregate with one vertex.
       */
      void seed(const Vertex& vertex);

      /**
       * @brief Add a vertex to the aggregate.
       */
      void add(const Vertex& vertex);

      /**
       * @brief Clear the aggregate.
       */
      void clear();

      /**
       * @brief Get the size of the aggregate.
       */
      int size();

      /**
       * @brief Get the id identifying the aggregate.
       */
      int id();

      const_iterator begin(){
        return vertices_.begin();
      }

      const_iterator end()
      {
        return vertices_.end();
      }

    private:
      /**
       * @brief The vertices of the aggregate.
       */
      VertexList vertices_;

      /**
       * @brief The number of the currently referenced
       * aggregate.
       */
      int id_;

      /**
       * @brief The matrix graph the aggregates live on.
       */
      const MatrixGraph& graph_;

      /**
       * @brief The aggregate mapping we build.
       */
      Aggregates<G>& aggregates_;

      /**
       * @brief The connections to other aggregates.
       */
      VertexSet& connected_;

    };

    /**
     * @brief Class providing information about the aggregates.
     */
    template<class G>
    class Aggregates
    {
    public:
      enum {
        /**
         * @brief Identifier of not yet aggregated vertices.
         */
        UNAGGREGATED = -1
      };

      /**
       * @brief The matrix graph type used.
       */
      typedef G MatrixGraph;

      /**
       * @brief The vertex identifier
       */
      typedef typename MatrixGraph::VertexDescriptor Vertex;

      typedef typename MatrixGraph::VertexDescriptor AggregateDescriptor;

      /**
       * @brief Constructor.
       */
      Aggregates();

      /**
       * @brief Destructor.
       */
      ~Aggregates();

      /**
       * @brief Build the aggregates.
       *
       * The template parameter C Is the type of the coarsening Criterion to
       * use.
       * @param m The matrix to build the aggregates accordingly.
       * @param graph A (sub) graph of the matrix.
       * @param c The coarsening criterion to use.
       */
      template<class C>
      void build(const typename C::Matrix& m, G& graph, const C& c);

      /**
       * @brief Get the aggregate the vertex belongs to.
       *
       * @param vertex The vertex descriptor we want the aggregate for.
       * @return The corresponding aggregate identifier.
       */
      const AggregateDescriptor& operator[](const Vertex& vertex) const;


      /**
       * @brief Get the aggregate the vertex belongs to.
       *
       * @param vertex The vertex descriptor we want the aggregate for.
       * @return The corresponding aggregate identifier.
       */
      AggregateDescriptor& operator[](const Vertex& vertex);

      /**
       * @brief Print the aggregates for a 2d cartesian structured grid.
       *
       * @param n The numeber of vertices in each grid directions.
       * @param os The output stream to use.
       */
      void print2d(int n, std::ostream& os);

    private:
      /**
       * @brief The allocator we use for our lists and the
       * set.
       */
      typedef PoolAllocator<Vertex,100*sizeof(int)> Allocator;

      /**
       * @brief The single linked list we use.
       */
      typedef SLList<Vertex,Allocator> VertexList;

      /**
       * @brief The set of vertices we use.
       */
      typedef std::set<Vertex> VertexSet;

      /**
       * @brief The graph we aggregate for.
       */
      MatrixGraph* graph_;

      /**
       * @brief The vertices of the current aggregate-
       */
      Aggregate<MatrixGraph>* aggregate_;

      /**
       * @brief The vertices of the current aggregate front.
       */
      VertexList front_;

      /**
       * @brief The set of connected vertices.
       */
      VertexSet connected_;

      /**
       * @brief The aggregates coresponding to the vertices.
       */
      AggregateDescriptor *aggregates_;

      /**
       * @brief Number of vertices mapped.
       */
      int size_;

      /**
       * @brief Stack.
       */
      class Stack
      {
      public:
        Stack(const MatrixGraph& graph, const Aggregates<G>& aggregates);
        ~Stack();
        void push(const Vertex& v);
        void fill();
        Vertex pop();
      private:
        enum { N = 1024 };

        /** @brief The graph we work on. */
        const MatrixGraph& graph_;
        /** @brief The aggregates information. */
        const Aggregates<G>& aggregates_;
        /** @brief The current size. */
        int size_;
        int maxSize_;
        /** @brief The index of the top element. */
        int head_;
        int filled_;

        /** @brief The values on the stack. */
        Vertex* vals_;

        void localPush(const Vertex& v);
      };

      friend class Stack;

      /**
       * @brief Build the dependency of the matrix graph.
       */
      template<class C>
      void buildDependency(MatrixGraph& graph,
                           const typename C::Matrix& matrix,
                           C criterion);

      /**
       * @brief Breadth first search within an aggregate
       *
       * @param start The vertex where the search should start
       * from. This does not need to belong to the aggregate.
       * @param aggregate The aggregate id.
       * @param graph The matrix graph to perform the search on.
       */
      int breadthFirstSearch(const Vertex& start, int aggregate,
                             MatrixGraph& graph);


      /**
       * @brief Breadth first search within an aggregate
       *
       * The template parameters: <br />
       * L A container type providing push_back(Vertex), and
       * pop_front() in case remove is true<br />
       * remove If true the entries in the visited list
       * will be removed.
       * @param start The vertex where the search should start
       * from. This does not need to belong to the aggregate.
       * @param aggregate The aggregate id.
       * @param graph The matrix graph to perform the search on.
       */
      template<bool remove, class L>
      int breadthFirstSearch(const Vertex& start, int aggregate,
                             MatrixGraph& graph, L& visited);

      /**
       * @brief Visits all neighbours of vertex belonging to a
       * specific aggregate.
       *
       * @param vertex The vertex whose neighbours we want to
       * visit.
       * @param aggregate The id of the aggregate.
       * @param visitor The visitor evaluated for each EdgeIterator
       * (by its method operator()(ConstEdgeIterator edge)
       */
      template<class V>
      void visitAggregateNeighbours(const Vertex& vertex, int aggregate,
                                    V& visitor) const;

      /**
       * @brief Visit all neighbour vertices of a vertex.
       *
       * @param vertex The vertex whose neighbours we want to
       * visit.
       * @param visitor The visitor evaluated for each EdgeIterator
       * (by its method operator()(const ConstEdgeIterator& edge)
       */
      template<class V>
      void visitNeighbours(const Vertex& vertex, V& visitor) const;

      /**
       * @brief An Adaptor for vsitors that only
       * evaluates edges pointing to a specific aggregate.
       */
      template<class V>
      class AggregateVisitor
      {
      public:
        /**
         * @brief The type of the adapted visitor
         */
        typedef V Visitor;
        /**
         * @brief Constructor.
         * @param aggregates The aggregate numbers of the
         * vertices.
         * @param  aggregate The id of the aggregate to visit.
         * @param visitor The visitor.
         */
        AggregateVisitor(const AggregateDescriptor* aggregates, int aggregate,
                         Visitor& visitor);

        /**
         * @brief Examine an edge.
         *
         * The edge will be examined by the adapted visitor if
         * it belongs to the right aggregate.
         */
        void operator()(const typename MatrixGraph::ConstEdgeIterator& edge);

      private:
        /** @brief Mapping of vertices to aggregates. */
        const AggregateDescriptor* aggregates_;
        /** @brief The aggregate id we want to visit. */
        int aggregate_;
        /** @brief The visitor to use on the aggregate. */
        Visitor& visitor_;
      };

      /**
       * @brief A simple counter functor.
       */
      class Counter
      {
      public:
        /** @brief Constructor */
        Counter();
        /** @brief Access the current count. */
        int value();

      protected:
        /** @brief Increment counter */
        void increment();
        /** @brief Decrement counter */
        void decrement();

      private:
        int count_;
      };


      /**
       * @brief Counts the number of edges to vertices belonging
       * to the aggregate front.
       */
      class FrontNeighbourCounter : public Counter
      {
      public:
        /**
         * @brief Constructor.
         * @param front The vertices of the front.
         */
        FrontNeighbourCounter(const MatrixGraph& front);

        void operator()(const typename MatrixGraph::ConstEdgeIterator& edge);

      private:
        const MatrixGraph& graph_;
      };

      /**
       * @brief Count the number of neighbours of a vertex that belong
       * to the aggregate front.
       */
      int noFrontNeighbours(const Vertex& vertex) const;

      /**
       * @brief Counter of TwoWayConnections.
       */
      class TwoWayCounter : public Counter
      {
      public:
        void operator()(const typename MatrixGraph::ConstEdgeIterator& edge);
      };

      /**
       * @brief Count the number of twoway connection from
       * a vertex to an aggregate.
       *
       * @param vertex The vertex whose connections are counted.
       * @param aggregate The id of the aggregate the connections
       * should point to.
       * @return The number of one way connections from the vertex to
       * the aggregate.
       */
      int twoWayConnections(const Vertex&, int aggregate) const;

      /**
       * @brief Counter of OneWayConnections.
       */
      class OneWayCounter : public Counter
      {
      public:
        void operator()(const typename MatrixGraph::ConstEdgeIterator& edge);
      };

      /**
       * @brief Count the number of oneway connection from
       * a vertex to an aggregate.
       *
       * @param vertex The vertex whose connections are counted.
       * @param aggregate The id of the aggregate the connections
       * should point to.
       * @return The number of one way connections from the vertex to
       * the aggregate.
       */
      int oneWayConnections(const Vertex&, int aggregate) const;

      /**
       * @brief Connectivity counter
       *
       * Increments count if the neighbour is already known as
       * connected or is not yet aggregated.
       */
      class ConnectivityCounter : public Counter
      {
      public:
        /**
         * @brief Constructor.
         * @param connected The set of connected aggregates.
         * @param aggregates The mapping of aggregates to vertices.
         */
        ConnectivityCounter(const VertexSet& connected, const AggregateDescriptor* aggregates);

        void operator()(const typename MatrixGraph::ConstEdgeIterator& edge);

      private:
        /** @brief The connected aggregates. */
        const VertexSet& connected_;
        /** @brief The mapping of vertices to aggregates. */
        const AggregateDescriptor* aggregates_;

      };

      /**
       * @brief Get the connectivity of a vertex.
       *
       * For each unaggregated neighbour or neighbour of an aggregate
       * that is already known as connected the count is increased by
       * one. In all other cases by two.
       *
       * @return The value of the connectivity.
       */
      int connectivity(const Vertex& vertex) const;

      /**
       * @brief Counts the edges depending on the dependency.
       *
       * If the inluence flag of the edge is set the counter is
       * increased and/or if the depends flag is set it is
       * incremented, too.
       */
      class DependencyCounter : public Counter
      {
      public:
        /**
         * @brief Constructor.
         * @param aggregates The mapping of the vertices to
         * aggregates.
         */
        DependencyCounter(const AggregateDescriptor *aggregates);

        void operator()(const typename MatrixGraph::ConstEdgeIterator& edge);

      private:
        const AggregateDescriptor* aggregates_;
      };

      /**
       * @brief Adds the targets of each edge to
       * the list of front vertices.
       *
       * Vertices already marked as front nodes will not get added.
       */
      class FrontMarker
      {
      public:
        /**
         * @brief Constructor.
         *
         * @param front The list to store the front vertices in.
         * @param graph The matrix graph we work on.
         */
        FrontMarker(VertexList& front, MatrixGraph& graph);

        void operator()(const typename MatrixGraph::ConstEdgeIterator& edge);

      private:
        /** @brief The list of front vertices. */
        VertexList& front_;
        /** @brief The matrix graph we work on. */
        MatrixGraph& graph_;
      };

      /**
       * @brief Mark the front of the current aggregate.
       *
       * The front are the direct (unaggregated) neighbours of
       * the aggregate vertices.
       */
      void markFront();

      /**
       * @brief Unmarks all front vertices.
       */
      void unmarkFront();

      /**
       * @brief counts the dependency between a vertex and unaggregated
       * neighbours.
       *
       * If the inluence flag of the edge is set the counter is
       * increased and/or if the depends flag is set it is
       * incremented, too.
       *
       * @param vertex The vertex whose neighbours we count.
       * @return The sum of the number of unaggregated
       * neighbours the vertex depends on and the number of unaggregated
       * neighbours the vertex influences.
       */
      int unusedNeighbours(const Vertex& vertex) const;

      /**
       * @brief Counts the number of neighbours belonging to an aggregate.
       *
       *
       * If the inluence flag of the edge is set the counter is
       * increased and/or if the depends flag is set it is
       * incremented, too.
       *
       * @param vertex The vertex whose neighbours we count.
       * @param aggregate The aggregate id.
       * @return The sum of the number of
       * neighbours belonging to the aggregate
       * the vertex depends on and the number of
       * neighbours of the aggregate the vertex influences.
       */
      int aggregateNeighbours(const Vertex& vertex, int aggregate) const;

      /**
       * @brief Checks wether a vertex is admisible to be added to an aggregate.
       */
      bool admissible(const Vertex& vertex, int aggregate) const;

      /**
       * @brief Push the neighbours of the current aggregate on the stack.
       *
       * @param stack The stack to push them on.
       * @param isolated If true only isolated vertices are push onto the stack.
       */
      void seedFromFront(Stack& stack,  bool isolated);

      /**
       * @brief The maximum distance of the vertex to any vertex in the
       * current aggregate.
       *
       * @return The maximum of all shortest paths from the vertex to any
       * vertex of the aggregate.
       */
      int distance(const Vertex& vertex);

      /**
       * @brief Find a strongly connected cluster of a vertex.
       *
       * @vertex The vertex whose neighbouring aggregate we search.
       * @return A vertex of neighbouring aggregate the vertex is allowed to
       * be added to.
       */
      Vertex mergeNeighbour(const Vertex& vertex) const;

      /**
       * @brief Grows the aggregate from a seed.
       *
       * @param seed The first vertex of the aggregate.
       */
      template<class C>
      void growAggregate(const Vertex& vertex, const C& c);
    };

    template<class G, class M, template<class> class N>
    inline void SymmetricDependency<G,M,N>::init(MatrixGraph* graph, const Matrix* matrix)
    {
      graph_ = graph;
      matrix_ = matrix;
    }

    template<class G, class M, template<class> class N>
    inline void SymmetricDependency<G,M,N>::initRow(const Row& row, int index)
    {
      maxValue_ = - std::numeric_limits<double>::max();
      row_ = index;
      diagonal_ = norm_(matrix_->operator[](row_)[row_]);
    }

    template<class G, class M, template<class> class N>
    inline void SymmetricDependency<G,M,N>::examine(const ColIter& col)
    {
      maxValue_ = std::max(maxValue_,
                           (norm_(*col) * norm_(matrix_->operator[](col.index())[row_]))/
                           (norm_(matrix_->operator[](col.index())[col.index()]) * diagonal_));
    }

    template<class G, class M, template<class> class N>
    inline void SymmetricDependency<G,M,N>::examine(const EdgeIter& edge, const ColIter& col)
    {
      if(norm_(matrix_->operator[](edge.target())[edge.source()]) * norm_(*col)/
         (norm_(matrix_->operator[](edge.target())[edge.target()]) * diagonal_) > alpha() * maxValue_) {
        edge.properties().setDepends();
        edge.properties().setInfluences();

        typename MatrixGraph::EdgeProperties& other = graph_->getEdgeProperties(edge.target(), edge.source());
        other.setInfluences();
        other.setDepends();
      }
    }

    template<class G, class M, template<class> class N>
    inline bool SymmetricDependency<G,M,N>::isIsolated()
    {
      return maxValue_  < beta();
    }

    template<class G>
    Aggregate<G>::Aggregate(const MatrixGraph& graph, Aggregates<G>& aggregates,
                            VertexSet& connected)
      : vertices_(), id_(-1), graph_(graph), aggregates_(aggregates),
        connected_(connected)
    {}

    template<class G>
    void Aggregate<G>::reconstruct(const Vertex& vertex)
    {
      assert(!graph_[vertex].excluded());
      vertices_.push_back(vertex);
      typedef typename VertexList::const_iterator iterator;
      iterator begin = vertices_.begin();
      iterator end   = vertices_.end();
      while(begin!=end) {
        //for();
        throw "Not yet implemented";
      }

    }

    template<class G>
    inline void Aggregate<G>::seed(const Vertex& vertex)
    {
      connected_.clear();
      vertices_.clear();
      connected_.insert(vertex);
      id_ = vertex;
      add(vertex);
    }


    template<class G>
    inline void Aggregate<G>::add(const Vertex& vertex)
    {
      vertices_.push_back(vertex);
      aggregates_[vertex]=id_;
      typedef typename MatrixGraph::ConstEdgeIterator iterator;
      const iterator end = graph_.endEdges(vertex);
      for(iterator edge = graph_.beginEdges(vertex); edge != end; ++edge)
        connected_.insert(aggregates_[edge.target()]);

    }
    template<class G>
    inline void Aggregate<G>::clear()
    {
      vertices_.clear();
      connected_.clear();
      id_=-1;
    }

    template<class G>
    inline int Aggregate<G>::size()
    {
      return vertices_.size();
    }

    template<class G>
    inline int Aggregate<G>::id()
    {
      return id_;
    }

    template<class G>
    Aggregates<G>::Aggregates()
      : graph_(0), aggregate_(0), front_(), connected_(), aggregates_(0), size_(-1)
    {}

    template<class G>
    Aggregates<G>::~Aggregates()
    {
      if(size_>0)
        delete[] aggregates_;
      size_=-1;
    }

    template<class G>
    template<class C>
    void Aggregates<G>::buildDependency(MatrixGraph& graph,
                                        const typename C::Matrix& matrix,
                                        C criterion)
    {
      // The Criterion we use for building the dependency.
      typedef C Criterion;

      //      assert(graph.isBuilt());
      typedef typename C::Matrix Matrix;
      typedef typename MatrixGraph::VertexIterator VertexIterator;

      criterion.init(&graph, &matrix);

      for(VertexIterator vertex = graph.begin(); vertex != graph.end(); ++vertex) {
        typedef typename Matrix::row_type Row;

        Row row = matrix[*vertex];

        // Tell the criterion what row we will examine now
        // This might for example be used for calculating the
        // maximum offdiagonal value
        criterion.initRow(row, *vertex);

        // On a first path all columns are examined. After this
        // the calculator should know whether the vertex is isolated.
        typedef typename Matrix::ConstColIterator ColIterator;
        ColIterator end = row.end();
        for(ColIterator col = row.begin(); col != end; ++col)
          if(col.index()!=*vertex)
            criterion.examine(col);

        // reset the vertex properties
        vertex.properties().reset();

        // Check whether the vertex is isolated.
        if(criterion.isIsolated()) {
          vertex.properties().setIsolated();
        }else{
          // Examine all the edges beginning at this vertex.
          typedef typename MatrixGraph::EdgeIterator EdgeIterator;
          typedef typename Matrix::ConstColIterator ColIterator;
          EdgeIterator end = vertex.end();
          ColIterator col = matrix[*vertex].begin();

          for(EdgeIterator edge = vertex.begin(); edge!= end; ++edge, ++col) {
            // Move to the right column.
            while(col.index()!=edge.target())
              ++col;
            criterion.examine(edge, col);
          }
        }

      }
    }

    template<class G>
    int Aggregates<G>::breadthFirstSearch(const Vertex& start, int aggregate,
                                          MatrixGraph& graph)
    {
      VertexList vlist;
      return breadthFirstSearch<true>(start, aggregate, graph, vlist);
    }

    template<class G>
    template<bool remove, class L>
    int Aggregates<G>::breadthFirstSearch(const Vertex& start, int aggregate,
                                          MatrixGraph& graph, L& visited)
    {
      typedef typename L::const_iterator iterator;
      int visitedSpheres = 0;

      visited.push_back(start);
      graph.getVertexProperties(start).setVisited();

      iterator current = visited.begin();
      iterator end = visited.end();
      int i=0, size=visited.size();

      // visit the neighbours of all vertices of the
      // current sphere.
      while(current != end) {

        for(; i<size; ++current, ++i) {
          typedef typename MatrixGraph::ConstEdgeIterator iterator;
          const iterator end = graph.endEdges(*current);

          for(iterator edge = graph.beginEdges(*current);
              edge != end; ++edge) {

            if(aggregates_[edge.target()]==aggregate &&
               !graph.getVertexProperties(edge.target()).visited()) {

              graph.getVertexProperties(edge.target()).setVisited();
              visited.push_back(edge.target());
            }
          }
        }
        end = visited.end();
        size = visited.size();
        if(current != end)
          visitedSpheres++;
      }

      for(current = visited.begin(); current != end;) {
        graph.getVertexProperties(*current).resetVisited();
        ++current;
        if(remove)
          visited.pop_front();
      }

      return visitedSpheres;
    }


    template<class G>
    template<class V>
    inline Aggregates<G>::AggregateVisitor<V>::AggregateVisitor(const AggregateDescriptor* aggregates,
                                                                int aggregate, V& visitor)
      : aggregates_(aggregates), aggregate_(aggregate), visitor_(visitor)
    {}

    template<class G>
    template<class V>
    inline void Aggregates<G>::AggregateVisitor<V>::operator()(const typename MatrixGraph::ConstEdgeIterator& edge)
    {
      if(aggregates_[edge.target()]==aggregate_)
        visitor_(edge);
    }

    template<class G>
    template<class V>
    inline void Aggregates<G>::visitAggregateNeighbours(const Vertex& vertex,
                                                        int aggregate,
                                                        V& visitor) const
    {
      // Only evaluates for edge pointing to the aggregate
      AggregateVisitor<V> v(this->aggregates_, aggregate, visitor);
      visitNeighbours(vertex, v);
    }

    template<class G>
    template<class V>
    inline void Aggregates<G>::visitNeighbours(const Vertex& vertex, V& visitor) const
    {
      typedef typename MatrixGraph::ConstEdgeIterator iterator;
      const iterator end = graph_->endEdges(vertex);
      for(iterator edge = graph_->beginEdges(vertex); edge != end; ++edge)
        visitor(edge);
    }

    template<class G>
    inline Aggregates<G>::Counter::Counter()
      : count_(0)
    {}

    template<class G>
    inline void Aggregates<G>::Counter::increment()
    {
      ++count_;
    }

    template<class G>
    inline void Aggregates<G>::Counter::decrement()
    {
      --count_;
    }
    template<class G>
    inline int Aggregates<G>::Counter::value()
    {
      return count_;
    }

    template<class G>
    inline void Aggregates<G>::TwoWayCounter::operator()(const typename MatrixGraph::ConstEdgeIterator& edge)
    {
      if(edge.properties().isTwoWay())
        Counter::increment();
    }

    template<class G>
    int Aggregates<G>::twoWayConnections(const Vertex& vertex, int aggregate) const
    {
      TwoWayCounter counter;
      visitAggregateNeighbours(vertex, aggregate, counter);
      return counter.value();
    }

    template<class G>
    int Aggregates<G>::oneWayConnections(const Vertex& vertex, int aggregate) const
    {
      OneWayCounter counter;
      visitAggregateNeighbours(vertex, aggregate, counter);
      return counter.value();
    }

    template<class G>
    inline void Aggregates<G>::OneWayCounter::operator()(const typename MatrixGraph::ConstEdgeIterator& edge)
    {
      if(edge.properties().isOneWay())
        Counter::increment();
    }

    template<class G>
    inline Aggregates<G>::ConnectivityCounter::ConnectivityCounter(const VertexSet& connected,
                                                                   const AggregateDescriptor* aggregates)
      : Counter(), connected_(connected), aggregates_(aggregates)
    {}


    template<class G>
    inline void Aggregates<G>::ConnectivityCounter::operator()(const typename MatrixGraph::ConstEdgeIterator& edge)
    {
      if(connected_.find(aggregates_[edge.target()]) != connected_.end() || aggregates_[edge.target()]==UNAGGREGATED)
        Counter::increment();
      else{
        Counter::increment();
        Counter::increment();
      }
    }

    template<class G>
    inline int Aggregates<G>::connectivity(const Vertex& vertex) const
    {
      ConnectivityCounter counter(connected_, aggregates_);
      visitNeighbours(vertex, counter);
      return counter.value();
    }

    template<class G>
    inline Aggregates<G>::DependencyCounter::DependencyCounter(const AggregateDescriptor* aggregates)
      : Counter(), aggregates_(aggregates)
    {}

    template<class G>
    inline void Aggregates<G>::DependencyCounter::operator()(const typename MatrixGraph::ConstEdgeIterator& edge)
    {
      if(edge.properties().depends())
        Counter::increment();
      if(edge.properties().influences())
        Counter::increment();
    }

    template<class G>
    int Aggregates<G>::unusedNeighbours(const Vertex& vertex) const
    {
      return aggregateNeighbours(vertex, UNAGGREGATED);
    }

    template<class G>
    int Aggregates<G>::aggregateNeighbours(const Vertex& vertex, int aggregate) const
    {
      DependencyCounter counter(aggregates_);
      visitAggregateNeighbours(vertex, aggregate, counter);
      return counter.value();
    }

    template<class G>
    int Aggregates<G>::distance(const Vertex& vertex)
    {
      return breadthFirstSearch(vertex, aggregate_->id(), *graph_);
    }

    template<class G>
    inline Aggregates<G>::FrontMarker::FrontMarker(VertexList& front, MatrixGraph& graph)
      : front_(front), graph_(graph)
    {}

    template<class G>
    inline void Aggregates<G>::FrontMarker::operator()(const typename MatrixGraph::ConstEdgeIterator& edge)
    {
      Vertex target = edge.target();

      if(!graph_.getVertexProperties(target).front()) {
        front_.push_back(target);
        graph_.getVertexProperties(target).setFront();
      }
    }


    template<class G>
    void Aggregates<G>::markFront()
    {
      front_.clear();
      FrontMarker frontBuilder(front_, *graph_);

      typedef typename Aggregate<G>::const_iterator Iterator;

      for(Iterator vertex=aggregate_->begin(); vertex != aggregate_->end(); ++vertex)
        visitAggregateNeighbours(*vertex, UNAGGREGATED, frontBuilder);
    }

    template<class G>
    inline bool Aggregates<G>::admissible(const Vertex& vertex, int aggregate) const
    {
      // Todo
      std::cerr<<" Admissible not yet implemented!"<<std::endl;

      return true;
    }

    template<class G>
    void Aggregates<G>::unmarkFront()
    {
      typedef typename VertexList::const_iterator Iterator;

      for(Iterator vertex=front_.begin(); vertex != front_.end(); ++vertex)
        graph_->getVertexProperties(*vertex).resetFront();

      front_.clear();
    }

    template<class G>
    inline typename G::VertexDescriptor Aggregates<G>::mergeNeighbour(const Vertex& vertex) const
    {
      typedef typename MatrixGraph::ConstEdgeIterator Iterator;

      Iterator end = graph_->endEdges(vertex);
      for(Iterator edge = graph_->beginEdges(vertex); edge != end; ++edge) {
        if(aggregates_[edge.target()] != UNAGGREGATED &&
           graph_->getVertexProperties(edge.target()).isolated() == graph_->getVertexProperties(edge.source()).isolated()) {
          if( graph_->getVertexProperties(vertex).isolated() ||
              ((edge.properties().depends() || edge.properties().influences())
               && admissible(vertex, aggregates_[edge.target()])))
            return edge.target();
        }
      }
      return -1;
    }

    template<class G>
    Aggregates<G>::FrontNeighbourCounter::FrontNeighbourCounter(const MatrixGraph& graph)
      : Counter(), graph_(graph)
    {}

    template<class G>
    void Aggregates<G>::FrontNeighbourCounter::operator()(const typename MatrixGraph::ConstEdgeIterator& edge)
    {
      if(graph_.getVertexProperties(edge.target()).front())
        Counter::increment();
    }

    template<class G>
    int Aggregates<G>::noFrontNeighbours(const Vertex& vertex) const
    {
      FrontNeighbourCounter counter(*graph_);
      visitNeighbours(vertex, counter);
      return counter.value();
    }

    template<class G>
    template<class C>
    void Aggregates<G>::growAggregate(const Vertex& seed, const C& c)
    {
      while(aggregate_->size() < c.minAggregateSize()) {
        int maxTwoCons=0, maxOneCons=0, maxNeighbours=-1, maxCon=-std::numeric_limits<int>::max();

        Vertex candidate = -1;

        unmarkFront();
        markFront();

        typedef typename VertexList::const_iterator Iterator;

        for(Iterator vertex = front_.begin(); vertex != front_.end(); ++vertex) {
          // Only nonisolated nodes are considered
          if(graph_->getVertexProperties(*vertex).isolated())
            continue;

          if(c.maxDistance() < distance(*vertex))
            continue;  // Distance too far

          int twoWayCons = twoWayConnections(*vertex, aggregate_->id());

          /* The case of two way connections. */
          if( maxTwoCons == twoWayCons && twoWayCons > 0) {
            int con = connectivity(*vertex);

            if(con == maxCon) {
              int neighbours = noFrontNeighbours(*vertex);

              if(neighbours > maxNeighbours) {
                maxNeighbours = neighbours;
                candidate = *vertex;
              }
            }else if( con > maxCon) {
              maxCon = con;
              maxNeighbours = noFrontNeighbours(*vertex);
              candidate = *vertex;
            }
          }else if(twoWayCons > maxTwoCons) {
            maxTwoCons = twoWayCons;
            maxCon = connectivity(*vertex);
            maxNeighbours = noFrontNeighbours(*vertex);
            candidate = *vertex;

            // two way connections preceed
            maxOneCons = std::numeric_limits<int>::max();
          }

          if(twoWayCons > 0)
            continue; // THis is a two-way node, skip tests for one way nodes

          /* The one way case */
          int oneWayCons = oneWayConnections(*vertex, aggregate_->id());

          if(oneWayCons==0)
            continue; // No strong connections, skip the tests.

          if(!admissible(*vertex, aggregate_->id()))
            continue;

          if( maxOneCons == oneWayCons && oneWayCons > 0) {
            int con = connectivity(*vertex);

            if(con == maxCon) {
              int neighbours = noFrontNeighbours(*vertex);

              if(neighbours > maxNeighbours) {
                maxNeighbours = neighbours;
                candidate = *vertex;
              }
            }else if( con > maxCon) {
              maxCon = con;
              maxNeighbours = noFrontNeighbours(*vertex);
              candidate = *vertex;
            }
          }else if(oneWayCons > maxOneCons) {
            maxOneCons = oneWayCons;
            maxCon = connectivity(*vertex);
            maxNeighbours = noFrontNeighbours(*vertex);
            candidate = *vertex;
          }
        }


        if(candidate < 0)
          break; // No more candidates found

        aggregate_->add(candidate);
      }
    }



    template<class G>
    template<class C>
    void Aggregates<G>::build(const typename C::Matrix& m, G& graph, const C& c)
    {
      // Stack for fast vertex access
      Stack stack_(graph, *this);

      graph_ = &graph;

      aggregate_ = new Aggregate<G>(graph, *this, connected_);

      // Allocate the mapping to aggregate.
      size_ = graph.maxVertex();
      aggregates_ = new AggregateDescriptor[size_];

      for(int i=0; i < size_; i++)
        aggregates_[i] = UNAGGREGATED;

      buildDependency(graph, m, c);

      int noAggregates, conAggregates, isoAggregates, oneAggregates;
      noAggregates = conAggregates = isoAggregates = oneAggregates = 0;

      while(true) {
        Vertex seed = stack_.pop();

        if(seed == -1)
          // No more unaggregated vertices. We are finished!
          break;
        else
          std::cout<<"seed="<<seed<<" ";

        // Debugging output
        if(c.debugLevel()==1 && (noAggregates+1)%10000 == 0)
          std::cout<<"c";

        aggregate_->seed(seed);


        if(graph.getVertexProperties(seed).isolated())
          throw "Juhu!";
        //aggregateIsolated();
        else
          growAggregate(seed, c);


        /* The rounding step. */
        while(aggregate_->size() < c.maxAggregateSize()) {

          unmarkFront();
          markFront();
          Vertex candidate = -1;

          typedef typename VertexList::const_iterator Iterator;

          for(Iterator vertex = front_.begin(); vertex != front_.end(); ++vertex) {

            if(graph.getVertexProperties(*vertex).isolated())
              continue; // No isolated nodes here

            if(distance(*vertex) > c.maxDistance())
              continue; // Distance too far

            if(twoWayConnections( *vertex, aggregate_->id() ) == 0 &&
               (oneWayConnections( *vertex, aggregate_->id() ) == 0 ||
                !admissible( *vertex, aggregate_->id()) ))
              continue;

            if(aggregateNeighbours(*vertex, aggregate_->id()) <= unusedNeighbours(*vertex))
              continue;

            candidate = *vertex;
            break;
          }

          if(candidate<0) break; // no more candidates found.

          aggregate_->add(candidate);

        }

        // try to merge aggregates consisting of only one nonisolated vertex with other aggregates
        if(aggregate_->size()==1)
          if(!graph.getVertexProperties(seed).isolated()) {
            int mergedNeighbour = mergeNeighbour(seed);

            if(mergedNeighbour > -1)
              aggregates_[seed] = aggregates_[mergedNeighbour];
            else{
              oneAggregates++;
              conAggregates++;
            }

          }else{
            oneAggregates++;
            isoAggregates++;
          }
        else{
          if(graph.getVertexProperties(seed).isolated())
            isoAggregates++;
          else
            conAggregates++;
        }
        std::cout<<"size"<<aggregate_->size()<<" ";
        unmarkFront();
        markFront();
        seedFromFront(stack_, graph.getVertexProperties(seed).isolated());
        unmarkFront();
      }

      if(c.debugLevel()>2) {
        std::cout<<"connected aggregates: "<<conAggregates;
        std::cout<<" isolated aggregates: "<<isoAggregates<<std::endl;
      }

      delete aggregate_;
    }

    template<class G>
    inline void Aggregates<G>::seedFromFront(Stack& stack_, bool isolated)
    {
      typedef typename VertexList::const_iterator Iterator;

      Iterator end= front_.end();
      int count=0;
      for(Iterator vertex=front_.begin(); vertex != end; ++vertex,++count)
        stack_.push(*vertex);
      if(count==0)
        std::cerr<< " no vertices pushed!"<<std::endl;
    }

    template<class G>
    inline const typename G::VertexDescriptor&
    Aggregates<G>::operator[](const Vertex& vertex) const
    {
      return aggregates_[vertex];
    }

    template<class G>
    inline typename G::VertexDescriptor&
    Aggregates<G>::operator[](const Vertex& vertex)
    {
      return aggregates_[vertex];
    }

    template<class G>
    Aggregates<G>::Stack::Stack(const MatrixGraph& graph, const Aggregates<G>& aggregates)
      : graph_(graph), aggregates_(aggregates), size_(0), maxSize_(0), head_(0), filled_(0)
    {
      vals_ = new  Vertex[N];
    }

    template<class G>
    Aggregates<G>::Stack::~Stack()
    {
      std::cout << "Max stack size was "<<maxSize_<<" filled="<<filled_<<std::endl;
      delete[] vals_;
    }

    template<class G>
    inline void Aggregates<G>::Stack::push(const Vertex & v)
    {
      if(aggregates_[v] == UNAGGREGATED)
        localPush(v);
    }

    template<class G>
    inline void Aggregates<G>::Stack::localPush(const Vertex & v)
    {
      vals_[head_] = v;
      size_ = std::min<int>(size_+1, N);
      head_ = (head_+N+1)%N;
    }

    template<class G>
    void Aggregates<G>::Stack::fill()
    {
      int isolated = 0, connected=0;
      int isoumin, umin;
      filled_++;

      head_ = size_ = 0;
      isoumin = umin = std::numeric_limits<int>::max();

      typedef typename MatrixGraph::ConstVertexIterator Iterator;

      const Iterator end = graph_.end();

      for(Iterator vertex = graph_.begin(); vertex != end; ++vertex) {
        // Skip already aggregated vertices
        if(aggregates_[*vertex] != UNAGGREGATED)
          continue;

        if(vertex.properties().isolated()) {
          isoumin = std::min(isoumin, aggregates_.unusedNeighbours(*vertex));
          isolated++;
        }else{
          umin = std::min(umin, aggregates_.unusedNeighbours(*vertex));
          connected++;
        }
      }

      if(connected + isolated == 0)
        // No unaggregated vertices.
        return;

      if(connected > 0) {
        // Connected vertices have higher priority.
        for(Iterator vertex = graph_.begin(); vertex != end; ++vertex)
          if(aggregates_[*vertex] == UNAGGREGATED && !vertex.properties().isolated()
             && aggregates_.unusedNeighbours(*vertex) == umin)
            localPush(*vertex);
      }else{
        for(Iterator vertex = graph_.begin(); vertex != end; ++vertex)
          if(aggregates_[*vertex] == UNAGGREGATED && vertex.properties().isolated()
             && aggregates_.unusedNeighbours(*vertex) == umin)
            localPush(*vertex);
      }
      maxSize_ = std::max(size_, maxSize_);
    }

    template<class G>
    inline typename G::VertexDescriptor Aggregates<G>::Stack::pop()
    {
      while(size_>0) {
        head_ = (head_ + N -1) % N;
        size_--;
        Vertex v = vals_[head_];
        if(aggregates_[v]==UNAGGREGATED)
          return v;
      }
      // Stack is empty try to fill it
      fill();

      // try again
      while(size_>0) {
        head_ = (head_ + N -1) % N;
        size_--;
        Vertex v = vals_[head_];
        if(aggregates_[v]==UNAGGREGATED)
          return v;
      }
      return -1;
    }

    template<class G>
    void Aggregates<G>::print2d(int n,  std::ostream& os)
    {
      std::ios_base::fmtflags oldOpts=os.flags();

      os.setf(std::ios_base::right, std::ios_base::adjustfield);

      int max=-1;
      int width=1;

      for(int i=0; i< size_; i++)
        max=std::max(max, aggregates_[i]);

      for(int i=10; i < 1000000; i*=10)
        if(max/i>0)
          width++;
        else
          break;

      std::cout<<"width="<<width<<std::endl;

      for(int j=0, entry=0; j < n; j++) {
        for(int i=0; i<n; i++, entry++) {
          os.width(width);
          os<<aggregates_[entry]<<" ";
        }

        os<<std::endl;
      }
      os<<std::endl;
      os.flags(oldOpts);
    }


  } // namespace amg

} // namespace Dune


#endif
