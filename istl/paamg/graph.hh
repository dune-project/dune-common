// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_AMG_GRAPH_HH
#define DUNE_AMG_GRAPH_HH

#include <cstddef>
#include <algorithm>
#include <vector>
#include <dune/common/typetraits.hh>
#include <dune/common/iteratorfacades.hh>
#include <dune/istl/istlexception.hh>
#include <dune/common/propertymap.hh>

namespace Dune
{
  namespace Amg
  {
    /**
     * @defgroup ISTL_PAAMG Parallel Algebraic Multigrid
     * @ingroup ISTL
     * @brief A Parallel Algebraic Multigrid based on Agglomeration.
     */
    /**
     * @addtogroup ISTL_PAAMG
     *
     * @{
     */
    /** @file
     * @author Markus Blatt
     * @brief Provides classes for building the matrix graph.
     *
     * During the coarsening process in AMG the matrix graph together
     * with the dependencies, what connections in the graph are considered
     * strong or weak, what vertices are isolated, etc., have to build.
     * This information will be contained in the MatrixGraph class.
     */

    /**
     * @brief The (undirected) graph of a matrix.
     *
     * The graph of a sparse matrix essentially describes the sparsity
     * pattern (nonzero entries) of a matrix.
     * It is assumed that the underlying sparsity pattern is symmetric,
     * i.e if entry a_ij is present in the storage scheme of the matrix
     * (i.e. nonzero) so is a_ji.
     *
     * The matrix entries can be accessed as weights of the vertices and
     * edges.
     */
    template<class M>
    class MatrixGraph
    {
    public:
      /**
       * @brief The type of the matrix we are a graph for.
       */
      typedef M Matrix;

      /**
       * @brief The type of the weights
       */
      typedef typename M::block_type Weight;

      /**
       * @brief The vertex descriptor.
       *
       * Each descriptor describes exactly one vertex.
       */
      typedef typename M::size_type VertexDescriptor;

      /**
       * @brief The edge descriptor.
       *
       * Each edge is identifies by exactly one descriptor.
       */
      typedef std::ptrdiff_t EdgeDescriptor;

      enum {
        /*
         * @brief Whether Matrix is mutable.
         */
        mutableMatrix = SameType<M, typename RemoveConst<M>::Type>::value
      };


      /**
       * @brief Iterator over all edges starting from a vertex.
       */
      template<class C>
      class EdgeIteratorT
      {

      public:
        /**
         *  @brief The mutable type of the container type.
         */
        typedef typename RemoveConst<C>::Type MutableContainer;
        /**
         * @brief The constant type of the container type.
         */
        typedef const typename RemoveConst<C>::Type ConstContainer;

        friend class EdgeIteratorT<MutableContainer>;
        friend class EdgeIteratorT<ConstContainer>;

        enum {
          /** @brief whether C is mutable. */
          isMutable = SameType<C, MutableContainer>::value
        };

        /**
         * @brief The column iterator of the matrix we use.
         */
        typedef typename SelectType<isMutable && C::mutableMatrix,typename Matrix::row_type::Iterator,
            typename Matrix::row_type::ConstIterator>::Type
        ColIterator;

        /**
         * @brief The matrix block type we use as weights.
         */
        typedef typename SelectType<isMutable && C::mutableMatrix,typename M::block_type,
            const typename M::block_type>::Type
        Weight;

        /**
         * @brief Constructor.
         * @param source The source vertex of the edges.
         * @param block The matrix column block the iterator is initialized to,
         * @param end The end iterator of the matrix row.
         * @param edge The edge descriptor of the current edge.
         */
        EdgeIteratorT(const VertexDescriptor& source, const ColIterator& block,
                      const ColIterator& end, const EdgeDescriptor& edge);

        /**
         * @brief Constructor for the end iterator.
         *
         * Variables not needed by operator== or operator!= will not be initialized.
         * @param block The matrix column block the iterator is initialized to.
         */
        EdgeIteratorT(const ColIterator& block);

        /**
         * @brief Copy Constructor.
         * @param other The iterator to copy.
         */
        template<class C1>
        EdgeIteratorT(const EdgeIteratorT<C1>& other);

        /**
         * @brief Access the edge weight
         */
        typename SelectType<SameType<C, typename RemoveConst<C>::Type>::value && C::mutableMatrix,
            typename M::block_type, const typename M::block_type>::Type&
        weight() const;

        /** @brief preincrement operator. */
        EdgeIteratorT<C>& operator++();

        /** @brief Inequality operator. */
        bool operator!=(const EdgeIteratorT<typename RemoveConst<C>::Type>& other) const;

        /** @brief Inequality operator. */
        bool operator!=(const EdgeIteratorT<const typename RemoveConst<C>::Type>& other) const;

        /** @brief Equality operator. */
        bool operator==(const EdgeIteratorT<typename RemoveConst<C>::Type>& other) const;

        /** @brief Equality operator. */
        bool operator==(const EdgeIteratorT<const typename RemoveConst<C>::Type>& other) const;

        /** @brief The index of the target vertex of the current edge. */
        VertexDescriptor target() const;

        /** @brief The index of the source vertex of the current edge. */
        VertexDescriptor source() const;

        /** @brief Get the edge descriptor. */
        const EdgeDescriptor& operator*() const;

        /** @brief Get the edge descriptor. */
        const EdgeDescriptor* operator->() const;

      private:
        /** @brief Start vertex of the edges. */
        VertexDescriptor source_;
        /** @brief The column iterator describing the current edge. */
        ColIterator block_;
        /***
         * @brief The column iterator positioned at the end of the row
         * of vertex source_
         */
        ColIterator blockEnd_;
        /** @brief The edge descriptor. */
        EdgeDescriptor edge_;
      };

      /**
       * @brief The vertex iterator type of the graph.
       */
      template<class C>
      class VertexIteratorT
      {
      public:
        /**
         *  @brief The mutable type of the container type.
         */
        typedef typename RemoveConst<C>::Type MutableContainer;
        /**
         * @brief The constant type of the container type.
         */
        typedef const typename RemoveConst<C>::Type ConstContainer;

        friend class VertexIteratorT<MutableContainer>;
        friend class VertexIteratorT<ConstContainer>;

        enum {
          /** @brief whether C is mutable. */
          isMutable = SameType<C, MutableContainer>::value
        };

        /**
         * @brief Constructor.
         * @param graph The graph we are a vertex iterator for.
         * @param current The current vertex to position on.
         */
        explicit VertexIteratorT(C* graph, const VertexDescriptor& current);

        /**
         * @brief Constructor for the end iterator.
         *
         * only operator== or operator!= may be called safely on an
         * iterator constructed this way.
         * @param graph The graph we are a vertex iterator for.
         * @param current The current vertex to position on.
         */
        explicit VertexIteratorT(const VertexDescriptor& current);

        VertexIteratorT(const VertexIteratorT<MutableContainer>& other);

        /**
         * @brief Move to the next vertex.
         * @return This iterator positioned at the next vertex.
         */
        VertexIteratorT<C>& operator++();

        /** @brief Inequality operator. */
        bool operator!=(const VertexIteratorT<ConstContainer>& other) const;

        /** @brief Equality operator. */
        bool operator==(const VertexIteratorT<ConstContainer>& other) const;

        /** @brief Inequality operator. */
        bool operator!=(const VertexIteratorT<MutableContainer>& other) const;

        /** @brief Equality operator. */
        bool operator==(const VertexIteratorT<MutableContainer>& other) const;

        /** @brief Access the weight of the vertex. */
        typename SelectType<SameType<C, typename RemoveConst<C>::Type>::value  && C::mutableMatrix,
            typename M::block_type, const typename M::block_type>::Type&
        weight() const;

        /**
         * @brief Get the descriptor of the current vertex.
         * @return The index of the currently referenced vertex.
         */
        const VertexDescriptor& operator*() const;

        /**
         * @brief Get an iterator over all edges starting at the
         * current vertex.
         * @return Iterator position on the first edge to another vertex.
         */
        EdgeIteratorT<C> begin() const;

        /**
         * @brief Get an iterator over all edges starting at the
         * current vertex.
         * @return Iterator position on the first edge to another vertex.
         */
        EdgeIteratorT<C> end() const;

      private:
        C* graph_;
        VertexDescriptor current_;
      };

      /**
       * @brief The constant edge iterator type.
       */
      typedef EdgeIteratorT<const MatrixGraph<Matrix> > ConstEdgeIterator;

      /**
       * @brief The mutable edge iterator type.
       */
      typedef EdgeIteratorT<MatrixGraph<Matrix> > EdgeIterator;

      /**
       * @brief The constant vertex iterator type.
       */
      typedef VertexIteratorT<const MatrixGraph<Matrix> > ConstVertexIterator;

      /**
       * @brief The mutable vertex iterator type.
       */
      typedef VertexIteratorT<MatrixGraph<Matrix> > VertexIterator;

      /**
       * @brief Constructor.
       * @param matrix The matrix we are a graph for.
       */
      MatrixGraph(Matrix& matrix);

      /**
       * @brief Destructor.
       */
      ~MatrixGraph();

      /**
       * @brief Get an iterator over the vertices.
       * @return A vertex Iterator positioned at the first vertex.
       */
      VertexIterator begin();

      /**
       * @brief Get an iterator over the vertices.
       * @return A vertex Iterator positioned behind the last vertex.
       */
      VertexIterator end();

      /**
       * @brief Get an iterator over the vertices.
       * @return A vertex Iterator positioned at the first vertex.
       */
      ConstVertexIterator begin() const;

      /**
       * @brief Get an iterator over the vertices.
       * @return A vertex Iterator positioned behind the last vertex.
       */
      ConstVertexIterator end() const;

      /**
       * @brief Get an iterator over the edges starting at a vertex.
       * @param source The vertex where the edges should start.
       * @return An edge iterator positioned at the first edge starting
       * from vertex source.
       */
      EdgeIterator beginEdges(const VertexDescriptor& source);

      /**
       * @brief Get an iterator over the edges starting at a vertex.
       * @param source The vertex where the edges should start.
       * @return An edge iterator positioned behind the last edge starting
       * from vertex source.
       */
      EdgeIterator endEdges(const VertexDescriptor& source);


      /**
       * @brief Get an iterator over the edges starting at a vertex.
       * @param source The vertex where the edges should start.
       * @return An edge iterator positioned at the first edge starting
       * from vertex source.
       */
      ConstEdgeIterator beginEdges(const VertexDescriptor& source) const;

      /**
       * @brief Get an iterator over the edges starting at a vertex.
       * @param source The vertex where the edges should start.
       * @return An edge iterator positioned behind the last edge starting
       * from vertex source.
       */
      ConstEdgeIterator endEdges(const VertexDescriptor& source) const;

      /**
       * @brief Get the underlying matrix.
       * @return The matrix of the graph.
       */
      Matrix& matrix();

      /**
       * @brief Get the underlying matrix.
       * @return The matrix of the graph.
       */
      const Matrix& matrix() const;

      /**
       * @brief Get the number of vertices in the graph.
       */
      int noVertices() const;

      /**
       * @brief Get the maximal vertex descriptor.
       *
       * @return The minimum value v such that for
       * all vertices w in the graph w&lt;v holds.
       */
      VertexDescriptor maxVertex() const;

      /**
       * @brief Get the number of edges in the graph.
       */
      int noEdges() const;

      /**
       * @brief Find the descriptor of an edge.
       * @param source The source vertex of the edge we search for.
       * @param target The target vertex of the edge we search for.
       * @return The edge we found.
       */
      const EdgeDescriptor findEdge(const VertexDescriptor& source,
                                    const VertexDescriptor& target) const;

    private:
      /** @brief The matrix we are the graph for. */
      Matrix& matrix_;
      /** @brief The edge descriptor of the first edge of each row. */
      EdgeDescriptor* start_;
    };

    /**
     * @brief A subgraph of a graph.
     *
     * This is a (cached) view of a graph where certain
     * vertices and edges pointing to and leading from them
     * are skipped.
     *
     * The vertex descriptors are not changed.
     */
    template<class G, class T>
    class SubGraph
    {
    public:
      /**
       * @brief The type of the graph we are a sub graph for.
       */
      typedef G Graph;

      /**
       * @brief Random access container providing information about
       * which vertices are excluded.
       */
      typedef T Excluded;

      /**
       * @brief The vertex descriptor.
       */
      typedef typename Graph::VertexDescriptor VertexDescriptor;

      typedef VertexDescriptor* EdgeDescriptor;

      /**
       * @brief An index map for mapping the edges to indices.
       *
       * This should be used for attaching properties to a SubGraph
       * using VertexPropertiesGraph od PropertiesGraph.
       */
      class EdgeIndexMap
      {
      public:
        typedef ReadablePropertyMapTag Category;

        EdgeIndexMap(const EdgeDescriptor& firstEdge)
          : firstEdge_(firstEdge)
        {}

        std::size_t operator[](const EdgeDescriptor& edge) const
        {
          return edge-firstEdge_;
        }
      private:
        /** @brief The first edge of the graph. */
        EdgeDescriptor firstEdge_;
        /** @brief Protect default construction. */
        EdgeIndexMap()
        {}
      };

      /**
       * @brief Get an edge index map for the graph.
       * @return An edge index map for the graph.
       */
      EdgeIndexMap getEdgeIndexMap();

      /**
       * @brief The edge iterator of the graph.
       */
      class EdgeIterator : public RandomAccessIteratorFacade<EdgeIterator,const EdgeDescriptor>
      {
      public:
        /**
         * @brief Constructor.
         * @param source The source vertex of the edge.
         * @param firstEdge Pointer to the beginning of the graph's edge array.
         */
        explicit EdgeIterator(const VertexDescriptor& source, const EdgeDescriptor& edge);

        /**
         * @brief Constructor for the end iterator.
         *
         * Only operator== or operator!= can be called safely on an iterator constructed
         * this way!
         * @param firstEdge Pointer to the beginning of the graph's edge array.
         */
        explicit EdgeIterator(const EdgeDescriptor& edge);

        /** @brief Equality operator. */
        bool equals(const EdgeIterator& other) const;

        /** @brief Preincrement operator. */
        EdgeIterator& increment();

        /** @brief Preincrement operator. */
        EdgeIterator& decrement();

        EdgeIterator& advance(std::ptrdiff_t n);

        /** @brief The descriptor of the current edge. */
        const EdgeDescriptor& dereference() const;

        /** @brief The index of the target vertex of the current edge. */
        const VertexDescriptor& target() const;

        /** @brief The index of the source vertex of the current edge. */
        const VertexDescriptor& source() const;

        std::ptrdiff_t distanceTo(const EdgeIterator& other) const;

      private:
        /** @brief The source vertex of the edge. */
        VertexDescriptor source_;
        /**
         * @brief The offset of the current edge to the first
         * one starting at the vertex source_.
         */
        EdgeDescriptor edge_;
      };

      /**
       * @brief The vertex iterator of the graph.
       */
      class VertexIterator
        : public ForwardIteratorFacade<VertexIterator,const VertexDescriptor>
      {
      public:
        /**
         * @param Constructor.
         * @param current The position of the iterator.
         * @param end The last vertex of the graph.
         */
        explicit VertexIterator(const SubGraph<G,T>* graph, const VertexDescriptor& current,
                                const VertexDescriptor& end);


        /**
         * @param Constructor for end iterator.
         *
         * Use with care! All operations except operator== or operator!= will fail!
         * @param current The position of the iterator.
         */
        explicit VertexIterator(const VertexDescriptor& current);

        /** @brief Preincrement operator. */
        VertexIterator& increment();

        /** @brief Equality iterator. */
        bool equals(const VertexIterator& other) const;

        /**
         * @brief Get the descriptor of the current vertex.
         * @return The index of the currently referenced vertex.
         */
        const VertexDescriptor& dereference() const;

        /**
         * @brief Get an iterator over all edges starting at the
         * current vertex.
         * @return Iterator position on the first edge to another vertex.
         */
        EdgeIterator begin() const;

        /**
         * @brief Get an iterator over all edges starting at the
         * current vertex.
         * @return Iterator position on the first edge to another vertex.
         */
        EdgeIterator end() const;

      private:
        /** @brief The graph we are a vertex iterator for. */
        const SubGraph<Graph,T>* graph_;
        /** @brief The current position. */
        VertexDescriptor current_;
        /** @brief The number of vertices of the graph. */
        VertexDescriptor end_;
      };

      /**
       * @brief The constant edge iterator type.
       */
      typedef EdgeIterator ConstEdgeIterator;

      /**
       * @brief The constant vertex iterator type.
       */
      typedef VertexIterator ConstVertexIterator;

      /**
       * @brief Get an iterator over the vertices.
       * @return A vertex Iterator positioned at the first vertex.
       */
      ConstVertexIterator begin() const;

      /**
       * @brief Get an iterator over the vertices.
       * @return A vertex Iterator positioned behind the last vertex.
       */
      ConstVertexIterator end() const;

      /**
       * @brief Get an iterator over the edges starting at a vertex.
       * @param source The vertex where the edges should start.
       * @return An edge iterator positioned at the first edge starting
       * from vertex source.
       */
      ConstEdgeIterator beginEdges(const VertexDescriptor& source) const;

      /**
       * @brief Get an iterator over the edges starting at a vertex.
       * @param source The vertex where the edges should start.
       * @return An edge iterator positioned behind the last edge starting
       * from vertex source.
       */
      ConstEdgeIterator endEdges(const VertexDescriptor& source) const;

      /**
       * @brief Get the number of vertices in the graph.
       */
      int noVertices() const;

      /**
       * @brief Get the maximal vertex descriptor.
       *
       * @return The minimum value v such that for
       * all vertices w in the graph w&lt;v holds.
       */
      VertexDescriptor maxVertex() const;

      /**
       * @brief Get the number of edges in the graph.
       */
      int noEdges() const;
      /**
       * @brief Find the descriptor of an edge.
       * @param source The source vertex of the edge we search for.
       * @param target The target vertex of the edge we search for.
       * @return The edge we found.
       */
      const EdgeDescriptor& findEdge(const VertexDescriptor& source,
                                     const VertexDescriptor& target) const;
      /**
       * @brief Constructor.
       *
       * @param graph The graph we are a sub graph for.
       * @param excluded If excluded[i] is true then vertex i will not appear
       * in the sub graph.
       */
      SubGraph(const Graph& graph, const T& excluded);

      /**
       * @brief Destructor.
       */
      ~SubGraph();

    private:
      /** @brief flags indication which vertices are excluded. */
      const T& excluded_;
      /** @brief The number of vertices in this sub graph. */
      int noVertices_;
      /** @brief Vertex behind the last valid vertex of this sub graph. */
      VertexDescriptor endVertex_;
      /** @brief The number of edges in this sub graph.*/
      int noEdges_;
      /**
       * @brief The maximum vertex descriptor of the graph
       * we are a subgraph for.
       */
      VertexDescriptor maxVertex_;
      /** @brief The edges of this sub graph. */
      VertexDescriptor* edges_;
      /** @brief The start of the out edges of each vertex. */
      int* start_;
      /** @brief The edge behind the last out edge of each vertex. */
      int* end_;
    };


    /**
     * @brief Attaches properties to the vertices of a graph.
     */
    template<class G, class VP, class VM=IdentityMap>
    class VertexPropertiesGraph
    {
    public:
      /**
       * @brief The graph we attach properties to.
       */
      typedef G Graph;

      /**
       * @brief The vertex descriptor.
       */
      typedef typename Graph::VertexDescriptor VertexDescriptor;

      /**
       * @brief The edge descritor.
       */
      typedef typename Graph::EdgeDescriptor EdgeDescriptor;

      /**
       * @brief The type of the properties of the vertices.
       */
      typedef VP VertexProperties;

      /**
       * @brief The type of the map for converting the VertexDescriptor
       * to std::size_t
       *
       * Has to provide the following method:
       * std::size_t operator[](const VertexDescriptor& vertex)
       *
       * The following condition has to be met:
       * Let v1 and v2 be two vertex descriptors with v1 < v2 and map be the
       * index map. Then map[v1]<map[v2] has to hold.
       */
      typedef VM VertexMap;

      /**
       * @brief The type of the mutable edge iterator.
       */
      typedef typename Graph::EdgeIterator EdgeIterator;

      /**
       * @brief The type of the constant edge iterator.
       */
      typedef typename Graph::ConstEdgeIterator ConstEdgeIterator;

      /**
       * @brief Get the mutable edge iterator over edges starting at a vertex.
       * @return  An edge iterator over edges starting at a vertex
       * positioned at the first edge.
       */
      EdgeIterator beginEdges(const VertexDescriptor& source);

      /**
       * @brief Get the mutable edge iterator over edges starting at a vertex.
       * @return  An edge iterator over edges starting at a vertex
       * positioned after the last edge.
       */
      EdgeIterator endEdges(const VertexDescriptor& source);

      /**
       * @brief Get the mutable edge iterator over edges starting at a vertex.
       * @return  An edge iterator over edges starting at a vertex
       * positioned at the first edge.
       */
      ConstEdgeIterator beginEdges(const VertexDescriptor& source) const;

      /**
       * @brief Get the mutable edge iterator over edges starting at a vertex.
       * @return  An edge iterator over edges starting at a vertex
       * positioned after the last edge.
       */
      ConstEdgeIterator endEdges(const VertexDescriptor& source) const;


      template<class C>
      class VertexIteratorT
        : public SelectType<SameType<typename RemoveConst<C>::Type,
                  C>::value,
              typename Graph::VertexIterator,
              typename Graph::ConstVertexIterator>::Type
      {
        friend class VertexIteratorT<const typename RemoveConst<C>::Type>;
        friend class VertexIteratorT<typename RemoveConst<C>::Type>;
      public:
        /**
         * @brief The father class.
         */
        typedef typename SelectType<SameType<typename RemoveConst<C>::Type,
                C>::value,
            typename Graph::VertexIterator,
            typename Graph::ConstVertexIterator>::Type
        Father;

        /**
         * @brief The class of the edge iterator.
         */
        typedef typename SelectType<SameType<typename RemoveConst<C>::Type,
                C>::value,
            typename Graph::EdgeIterator,
            typename Graph::ConstEdgeIterator>::Type
        EdgeIterator;

        /**
         * @brief Constructor.
         * @param iter The iterator of the underlying graph.
         * @param graph The property graph over whose vertices we iterate.
         */
        explicit VertexIteratorT(const Father& iter,
                                 C* graph);


        /**
         * @brief Constructor for the end iterator.
         *
         * Only operator!= or operator== can be calles safely on an iterator
         * constructed this way.
         * @param iter The iterator of the underlying graph.
         */
        explicit VertexIteratorT(const Father& iter);

        /**
         * @brief Copy Constructor.
         * @param other The iterator to copy.
         */
        template<class C1>
        VertexIteratorT(const VertexIteratorT<C1>& other);

        /**
         * @brief Get the properties of the current Vertex.
         */
        typename SelectType<SameType<C,typename RemoveConst<C>::Type>::value,
            VertexProperties&,
            const VertexProperties&>::Type
        properties() const;

        /**
         * @brief Get an iterator over the edges starting from the current vertex.
         * @return An iterator over the edges starting from the current vertex
         * positioned at the first edge.
         */
        EdgeIterator begin() const;

        /**
         * @brief Get an iterator over the edges starting from the current vertex.
         * @return An iterator over the edges starting from the current vertex
         * positioned after the last edge.
         */
        EdgeIterator end() const;

      private:
        /**
         * @brief The graph over whose vertices we iterate.
         */
        C* graph_;
      };

      /**
       * @brief The type of the mutable Vertex iterator.
       */
      typedef VertexIteratorT<VertexPropertiesGraph<Graph,
              VertexProperties,VM> > VertexIterator;

      /**
       * @brief The type of the constant Vertex iterator.
       */
      typedef VertexIteratorT<const VertexPropertiesGraph<Graph,
              VertexProperties,VM> > ConstVertexIterator;

      /**
       * @brief Get an iterator over the vertices.
       * @return A vertex Iterator positioned at the first vertex.
       */
      VertexIterator begin();

      /**
       * @brief Get an iterator over the vertices.
       * @return A vertex Iterator positioned behind the last vertex.
       */
      VertexIterator end();

      /**
       * @brief Get an iterator over the vertices.
       * @return A vertex Iterator positioned at the first vertex.
       */
      ConstVertexIterator begin() const;

      /**
       * @brief Get an iterator over the vertices.
       * @return A vertex Iterator positioned behind the last vertex.
       */
      ConstVertexIterator end() const;

      /**
       * @brief Get the properties associated with a vertex.
       * @param vertex The descriptor identifying the vertex.
       * @return The properties of the vertex.
       */
      VertexProperties& getVertexProperties(const VertexDescriptor& vertex);

      /**
       * @brief Get the properties associated with a vertex.
       * @param vertex The descriptor identifying the vertex.
       * @return The properties of the vertex.
       */
      const VertexProperties& getVertexProperties(const VertexDescriptor& vertex) const;

      /**
       * @brief Get the graph the properties are attached to.
       * @return The underlying graph.
       */
      const Graph& graph() const;

      /**
       * @brief Get the number of vertices in the graph.
       */
      int noVertices() const;

      /**
       * @brief Get the maximal vertex descriptor.
       *
       * @return The minimum value v such that for
       * all vertices w in the graph w&lt;v holds.
       */
      VertexDescriptor maxVertex() const;

      /**
       * @brief Constructor.
       * @param graph The graph we attach properties to.
       * @param vmap The vertex map.
       */
      VertexPropertiesGraph(Graph& graph, const VertexMap vmap=VertexMap());

    private:
      VertexPropertiesGraph(const VertexPropertiesGraph&)
      {}

      /** @brief The graph the properties are attached to. */
      Graph& graph_;
      /** @brief The vertex map. */
      VertexMap vmap_;
      /** @brief The vertex properties. */
      std::vector<VertexProperties> vertexProperties_;

    };

    /**
     * @brief Attaches properties to the edges and vertices of a graph.
     */
    template<class G, class VP, class EP, class VM=IdentityMap, class EM=IdentityMap>
    class PropertiesGraph
    {
    public:
      /**
       * @brief The graph we attach properties to.
       */
      typedef G Graph;

      /**
       * @brief The vertex descriptor.
       */
      typedef typename Graph::VertexDescriptor VertexDescriptor;

      /**
       * @brief The edge descritor.
       */
      typedef typename Graph::EdgeDescriptor EdgeDescriptor;

      /**
       * @brief The type of the properties of the vertices.
       */
      typedef VP VertexProperties;

      /**
       * @brief The type of the map for converting the VertexDescriptor
       * to std::size_t
       *
       * Has to provide the following method:
       * std::size_t operator[](const VertexDescriptor& vertex)
       *
       * The following condition has to be met:
       * Let v1 and v2 be two vertex descriptors with v1 < v2 and map be the
       * index map. Then map[v1]<map[v2] has to hold.
       */
      typedef VM VertexMap;

      /**
       * @brief The type of the properties of the edges;
       */
      typedef EP EdgeProperties;


      /**
       * @brief The type of the map for converting the EdgeDescriptor
       * to std::size_t.
       *
       * Has to provide the following method:
       * std::size_t operator[](const EdgeDescriptor& vertex)
       *
       * The following condition has to be met:
       * Let e1 and e2 be two edge descriptors, e1 < e2, and map be the
       * index map. Then map[v1]<map[v2] has to hold.
       */
      typedef EM EdgeMap;

      template<class C>
      class EdgeIteratorT
        :  public SelectType<SameType<typename RemoveConst<C>::Type,
                  C>::value,
              typename Graph::EdgeIterator,
              typename Graph::ConstEdgeIterator>::Type
      {

        friend class EdgeIteratorT<const typename RemoveConst<C>::Type>;
        friend class EdgeIteratorT<typename RemoveConst<C>::Type>;
      public:
        /**
         * @brief The father class.
         */
        typedef typename SelectType<SameType<typename RemoveConst<C>::Type,
                C>::value,
            typename Graph::EdgeIterator,
            typename Graph::ConstEdgeIterator>::Type
        Father;

        /**
         * @param Constructor.
         * @param iter The iterator of the underlying graph.
         * @param graph The graph over whose edges we iterate.
         */
        explicit EdgeIteratorT(const Father& iter,
                               C* graph);

        /**
         * @param Constructor for the end iterator.
         *
         * Only operator== or operator!= should be called on
         * an iterator constructed this way.
         * @param iter The iterator of the underlying graph.
         * @param graph The graph over whose edges we iterate.
         */
        explicit EdgeIteratorT(const Father& iter);

        /**
         * @brief Copy constructor.
         * @param other the iterator to copy.
         */
        template<class C1>
        EdgeIteratorT(const EdgeIteratorT<C1>& other);

        /**
         * @brief Get the properties of the current edge.
         */
        typename SelectType<SameType<C,typename RemoveConst<C>::Type>::value,
            EdgeProperties&,
            const EdgeProperties&>::Type
        properties() const;

      private:
        /**
         * @brief The graph over whose edges we iterate.
         */
        C* graph_;
      };

      /**
       * @brief The type of the mutable edge iterator.
       */
      typedef EdgeIteratorT<PropertiesGraph<Graph,
              VertexProperties,
              EdgeProperties,VM,EM> > EdgeIterator;

      /**
       * @brief The type of the constant edge iterator.
       */
      typedef EdgeIteratorT<const PropertiesGraph<Graph,
              VertexProperties,
              EdgeProperties,VM,EM> > ConstEdgeIterator;

      /**
       * @brief Get the mutable edge iterator over edges starting at a vertex.
       * @return  An edge iterator over edges starting at a vertex
       * positioned at the first edge.
       */
      EdgeIterator beginEdges(const VertexDescriptor& source);

      /**
       * @brief Get the mutable edge iterator over edges starting at a vertex.
       * @return  An edge iterator over edges starting at a vertex
       * positioned after the last edge.
       */
      EdgeIterator endEdges(const VertexDescriptor& source);

      /**
       * @brief Get the mutable edge iterator over edges starting at a vertex.
       * @return  An edge iterator over edges starting at a vertex
       * positioned at the first edge.
       */
      ConstEdgeIterator beginEdges(const VertexDescriptor& source) const;

      /**
       * @brief Get the mutable edge iterator over edges starting at a vertex.
       * @return  An edge iterator over edges starting at a vertex
       * positioned after the last edge.
       */
      ConstEdgeIterator endEdges(const VertexDescriptor& source) const;


      template<class C>
      class VertexIteratorT
        : public SelectType<SameType<typename RemoveConst<C>::Type,
                  C>::value,
              typename Graph::VertexIterator,
              typename Graph::ConstVertexIterator>::Type
      {
        friend class VertexIteratorT<const typename RemoveConst<C>::Type>;
        friend class VertexIteratorT<typename RemoveConst<C>::Type>;
      public:
        /**
         * @brief The father class.
         */
        typedef typename SelectType<SameType<typename RemoveConst<C>::Type,
                C>::value,
            typename Graph::VertexIterator,
            typename Graph::ConstVertexIterator>::Type
        Father;

        /**
         * @brief Constructor.
         * @param iter The iterator of the underlying graph.
         * @param graph The property graph over whose vertices we iterate.
         */
        explicit VertexIteratorT(const Father& iter,
                                 C* graph);


        /**
         * @brief Constructor for the end iterator.
         *
         * Only operator!= or operator== can be called safely on an iterator
         * constructed this way.
         * @param iter The iterator of the underlying graph.
         */
        explicit VertexIteratorT(const Father& iter);

        /**
         * @brief Copy Constructor.
         * @param other The iterator to copy.
         */
        template<class C1>
        VertexIteratorT(const VertexIteratorT<C1>& other);

        /**
         * @brief Get the properties of the current Vertex.
         */
        typename SelectType<SameType<C,typename RemoveConst<C>::Type>::value,
            VertexProperties&,
            const VertexProperties&>::Type
        properties() const;

        /**
         * @brief Get an iterator over the edges starting from the current vertex.
         * @return An iterator over the edges starting from the current vertex
         * positioned at the first edge.
         */
        EdgeIteratorT<C> begin() const;

        /**
         * @brief Get an iterator over the edges starting from the current vertex.
         * @return An iterator over the edges starting from the current vertex
         * positioned after the last edge.
         */
        EdgeIteratorT<C> end() const;

      private:
        /**
         * @brief The graph over whose vertices we iterate.
         */
        C* graph_;
      };

      /**
       * @brief The type of the mutable Vertex iterator.
       */
      typedef VertexIteratorT<PropertiesGraph<Graph,
              VertexProperties,
              EdgeProperties,VM,EM> > VertexIterator;

      /**
       * @brief The type of the constant Vertex iterator.
       */
      typedef VertexIteratorT<const PropertiesGraph<Graph,
              VertexProperties,
              EdgeProperties,VM,EM> > ConstVertexIterator;

      /**
       * @brief Get an iterator over the vertices.
       * @return A vertex Iterator positioned at the first vertex.
       */
      VertexIterator begin();

      /**
       * @brief Get an iterator over the vertices.
       * @return A vertex Iterator positioned behind the last vertex.
       */
      VertexIterator end();

      /**
       * @brief Get an iterator over the vertices.
       * @return A vertex Iterator positioned at the first vertex.
       */
      ConstVertexIterator begin() const;

      /**
       * @brief Get an iterator over the vertices.
       * @return A vertex Iterator positioned behind the last vertex.
       */
      ConstVertexIterator end() const;

      /**
       * @brief Get the properties associated with a vertex.
       * @param vertex The descriptor identifying the vertex.
       * @return The properties of the vertex.
       */
      VertexProperties& getVertexProperties(const VertexDescriptor& vertex);

      /**
       * @brief Get the properties associated with a vertex.
       * @param vertex The descriptor identifying the vertex.
       * @return The properties of the vertex.
       */
      const VertexProperties& getVertexProperties(const VertexDescriptor& vertex) const;

      /**
       * @brief Get the properties associated with a edge.
       * @param edge The descriptor identifying the edge.
       * @return The properties of the edge.
       */
      EdgeProperties& getEdgeProperties(const EdgeDescriptor& edge);


      /**
       * @brief Get the properties associated with a edge.
       * @param edge The descriptor identifying the edge.
       * @return The properties of the edge.
       */
      const EdgeProperties& getEdgeProperties(const EdgeDescriptor& edge) const;

      /**
       * @brief Get the properties associated with a edge.
       * @param source The descriptor identifying the source vertex of the edge.
       * @param target The descriptor identifying the target vertex of the edge.
       * @return The properties of the edge.
       */
      EdgeProperties& getEdgeProperties(const VertexDescriptor& source,
                                        const VertexDescriptor& target);

      /**
       * @brief Get the properties associated with a edge.
       * @param source The descriptor identifying the source vertex of the edge.
       * @param target The descriptor identifying the target vertex of the edge.
       * @return The properties of the edge.
       */
      const EdgeProperties& getEdgeProperties(const VertexDescriptor& source,
                                              const VertexDescriptor& target) const;

      /**
       * @brief Get the graph the properties are attached to.
       * @return The underlying graph.
       */
      const Graph& graph() const;

      /**
       * @brief Get the number of vertices in the graph.
       */
      int noVertices() const;

      /**
       * @brief Get the maximal vertex descriptor.
       *
       * @return The minimum value v such that for
       * all vertices w in the graph w&lt;v holds.
       */
      VertexDescriptor maxVertex() const;

      /**
       * @brief Constructor.
       * @param graph The graph we attach properties to.
       * @param
       */
      PropertiesGraph(Graph& graph, const VertexMap& vmap=VertexMap(),
                      const EdgeMap& emap=EdgeMap());

    private:
      PropertiesGraph(const PropertiesGraph&);

      /** @brief The graph the properties are attached to. */
      Graph& graph_;
      /** @brief The vertex properties. */
      /** @brief The mapping of the vertices to indices. */
      VertexMap vmap_;
      std::vector<VertexProperties> vertexProperties_;
      /** @brief The mapping of the edges to indices. */
      EdgeMap emap_;
      /** @brief The edge properties. */
      std::vector<EdgeProperties> edgeProperties_;

    };


    /**
     * @brief Wrapper to access the internal edge properties of a graph
     * via operator[]()
     */
    template<typename G>
    class GraphVertexPropertiesSelector
    {
    public:
      /**
       * @brief The type of the graph with internal properties.
       */
      typedef G Graph;
      /**
       * @brief The type of the vertex properties.
       */
      typedef typename G::VertexProperties VertexProperties;
      /**
       * @brief The vertex descriptor.
       */
      typedef typename G::VertexDescriptor Vertex;

      /**
       * @brief Constructor.
       * @param g The graph whose properties we access.
       */
      GraphVertexPropertiesSelector(G& g)
        : graph_(g)
      {}
      /**
       * @brief Default constructor.
       */
      GraphVertexPropertiesSelector()
        : graph_(0)
      {}


      /**
       * @brief Get the properties associated to a vertex.
       * @param vertex The vertex whose Properties we want.
       */
      VertexProperties& operator[](const Vertex& vertex) const
      {
        return graph_->getVertexProperties(vertex);
      }
    private:
      Graph* graph_;
    };

    /**
     * @brief Wrapper to access the internal vertex properties of a graph
     * via operator[]()
     */
    template<typename G>
    class GraphEdgePropertiesSelector
    {
    public:
      /**
       * @brief The type of the graph with internal properties.
       */
      typedef G Graph;
      /**
       * @brief The type of the vertex properties.
       */
      typedef typename G::EdgeProperties EdgeProperties;
      /**
       * @brief The edge descriptor.
       */
      typedef typename G::EdgeDescriptor Edge;

      /**
       * @brief Constructor.
       * @param g The graph whose properties we access.
       */
      GraphEdgePropertiesSelector(G& g)
        : graph_(g)
      {}
      /**
       * @brief Default constructor.
       */
      GraphEdgePropertiesSelector()
        : graph_(0)
      {}

      /**
       * @brief Get the properties associated to a vertex.
       * @param edge The edge whose Properties we want.
       */
      EdgeProperties& operator[](const Edge& edge) const
      {
        return graph_->getEdgeProperties(edge);
      }
    private:
      Graph* graph_;
    };


    /**
     * @brief Visit all neighbour vertices of a vertex in a graph.
     *
     * @param vertex The vertex whose neighbours we want to
     * visit.
     * @param visitor The visitor evaluated for each EdgeIterator
     * (by its method operator()(const ConstEdgeIterator& edge)
     */
    template<class G, class V>
    void visitNeighbours(const G& graph, const typename G::VertexDescriptor& vertex,
                         V& visitor);

    template<class M>
    MatrixGraph<M>::MatrixGraph(M& matrix)
      : matrix_(matrix)
    {
      if(matrix_.N()!=matrix_.M())
        DUNE_THROW(ISTLError, "Matrix has to have as columns as rows!");

      start_ = new EdgeDescriptor[matrix_.N()+1];

      typedef typename M::ConstIterator Iterator;
      Iterator row = matrix_.begin();
      start_[row.index()] = 0;

      for(Iterator row=matrix_.begin(); row != matrix_.end(); ++row)
        start_[row.index()+1] = start_[row.index()] + row->size();
    }

    template<class M>
    MatrixGraph<M>::~MatrixGraph()
    {
      delete[] start_;
    }

    template<class M>
    inline int MatrixGraph<M>::noEdges() const
    {
      return start_[matrix_.N()];
    }

    template<class M>
    inline int MatrixGraph<M>::noVertices() const
    {
      return matrix_.N();
    }

    template<class M>
    inline typename MatrixGraph<M>::VertexDescriptor MatrixGraph<M>::maxVertex() const
    {
      return matrix_.N();
    }

    template<class M>
    const typename MatrixGraph<M>::EdgeDescriptor
    MatrixGraph<M>::findEdge(const VertexDescriptor& source,
                             const VertexDescriptor& target) const
    {
      int offset = matrix_[source].find(target).offset();
#ifdef DUNE_ISTL_WITH_CHECKING
      // diagonal is assumed to exist, so search for it
      // If not present this should throw an exception
      typename M::ConstColIterator found = matrix_[source].find(source);
      if(found == matrix_[source].end())
        DUNE_THROW(ISTLError, "Every matrix row is assumed to have a diagonal!");
#endif
      if(target>source)
        offset--;

      assert(offset<noEdges());

      return start_[source]+offset;
    }


    template<class M>
    inline M& MatrixGraph<M>::matrix()
    {
      return matrix_;
    }

    template<class M>
    inline const M& MatrixGraph<M>::matrix() const
    {
      return matrix_;
    }

    template<class M>
    template<class C>
    MatrixGraph<M>::EdgeIteratorT<C>::EdgeIteratorT(const VertexDescriptor& source, const ColIterator& block,
                                                    const ColIterator& end, const EdgeDescriptor& edge)
      : source_(source), block_(block), blockEnd_(end), edge_(edge)
    {
      if(block_!=blockEnd_ && block_.index() == source_) {
        // This is the edge from the diagonal to the diagonal. Skip it.
        ++block_;
      }
    }

    template<class M>
    template<class C>
    MatrixGraph<M>::EdgeIteratorT<C>::EdgeIteratorT(const ColIterator& block)
      : block_(block)
    {}

    template<class M>
    template<class C>
    template<class C1>
    MatrixGraph<M>::EdgeIteratorT<C>::EdgeIteratorT(const EdgeIteratorT<C1>& other)
      : source_(other.source_), block_(other.block_), blockEnd_(other.blockEnd_), edge_(other.edge_)
    {}


    template<class M>
    template<class C>
    inline typename SelectType<SameType<C, typename RemoveConst<C>::Type>::value && C::mutableMatrix,
        typename M::block_type, const typename M::block_type>::Type&
    MatrixGraph<M>::EdgeIteratorT<C>::weight() const
    {
      return *block_;
    }

    template<class M>
    template<class C>
    inline MatrixGraph<M>::EdgeIteratorT<C>& MatrixGraph<M>::EdgeIteratorT<C>::operator++()
    {
      ++block_;
      ++edge_;

      if(block_!=blockEnd_ && block_.index() == source_) {
        // This is the edge from the diagonal to the diagonal. Skip it.
        ++block_;
      }

      return *this;
    }

    template<class M>
    template<class C>
    inline bool MatrixGraph<M>::EdgeIteratorT<C>::operator!=(const MatrixGraph<M>::EdgeIteratorT<typename RemoveConst<C>::Type>& other) const
    {
      return block_!=other.block_;
    }

    template<class M>
    template<class C>
    inline bool MatrixGraph<M>::EdgeIteratorT<C>::operator!=(const MatrixGraph<M>::EdgeIteratorT<const typename RemoveConst<C>::Type>& other) const
    {
      return block_!=other.block_;
    }

    template<class M>
    template<class C>
    inline bool MatrixGraph<M>::EdgeIteratorT<C>::operator==(const MatrixGraph<M>::EdgeIteratorT<typename RemoveConst<C>::Type>& other) const
    {
      return block_==other.block_;
    }

    template<class M>
    template<class C>
    inline bool MatrixGraph<M>::EdgeIteratorT<C>::operator==(const MatrixGraph<M>::EdgeIteratorT<const typename RemoveConst<C>::Type>& other) const
    {
      return block_==other.block_;
    }

    template<class M>
    template<class C>
    inline typename MatrixGraph<M>::VertexDescriptor MatrixGraph<M>::EdgeIteratorT<C>::target() const
    {
      return block_.index();
    }

    template<class M>
    template<class C>
    inline typename MatrixGraph<M>::VertexDescriptor MatrixGraph<M>::EdgeIteratorT<C>::source() const
    {
      return source_;
    }

    template<class M>
    template<class C>
    inline const typename MatrixGraph<M>::EdgeDescriptor& MatrixGraph<M>::EdgeIteratorT<C>::operator*() const
    {
      return edge_;
    }

    template<class M>
    template<class C>
    inline const typename MatrixGraph<M>::EdgeDescriptor* MatrixGraph<M>::EdgeIteratorT<C>::operator->() const
    {
      return &edge_;
    }

    template<class M>
    template<class C>
    MatrixGraph<M>::VertexIteratorT<C>::VertexIteratorT(C* graph,
                                                        const VertexDescriptor& current)
      : graph_(graph), current_(current)
    {}


    template<class M>
    template<class C>
    MatrixGraph<M>::VertexIteratorT<C>::VertexIteratorT(const VertexDescriptor& current)
      : current_(current)
    {}

    template<class M>
    template<class C>
    MatrixGraph<M>::VertexIteratorT<C>::VertexIteratorT(const VertexIteratorT<MutableContainer>& other)
      : graph_(other.graph_), current_(other.current_)
    {}

    template<class M>
    template<class C>
    inline bool MatrixGraph<M>::VertexIteratorT<C>::operator!=(const VertexIteratorT<MutableContainer>& other) const
    {
      return current_ != other.current_;
    }

    template<class M>
    template<class C>
    inline bool MatrixGraph<M>::VertexIteratorT<C>::operator!=(const VertexIteratorT<ConstContainer>& other) const
    {
      return current_ != other.current_;
    }


    template<class M>
    template<class C>
    inline bool MatrixGraph<M>::VertexIteratorT<C>::operator==(const VertexIteratorT<MutableContainer>& other) const
    {
      return current_ == other.current_;
    }

    template<class M>
    template<class C>
    inline bool MatrixGraph<M>::VertexIteratorT<C>::operator==(const VertexIteratorT<ConstContainer>& other) const
    {
      return current_ == other.current_;
    }

    template<class M>
    template<class C>
    inline MatrixGraph<M>::VertexIteratorT<C>& MatrixGraph<M>::VertexIteratorT<C>::operator++()
    {
      ++current_;
      return *this;
    }

    template<class M>
    template<class C>
    inline typename SelectType<SameType<C, typename RemoveConst<C>::Type>::value && C::mutableMatrix,
        typename M::block_type, const typename M::block_type>::Type&
    MatrixGraph<M>::VertexIteratorT<C>::weight() const
    {
      return graph_->matrix()[current_][current_];
    }

    template<class M>
    template<class C>
    inline const typename MatrixGraph<M>::VertexDescriptor&
    MatrixGraph<M>::VertexIteratorT<C>::operator*() const
    {
      return current_;
    }

    template<class M>
    template<class C>
    inline MatrixGraph<M>::EdgeIteratorT<C>
    MatrixGraph<M>::VertexIteratorT<C>::begin() const
    {
      return graph_->beginEdges(current_);
    }

    template<class M>
    template<class C>
    inline MatrixGraph<M>::EdgeIteratorT<C>
    MatrixGraph<M>::VertexIteratorT<C>::end() const
    {
      return graph_->endEdges(current_);
    }

    template<class M>
    inline MatrixGraph<M>::VertexIteratorT<MatrixGraph<M> >
    MatrixGraph<M>::begin()
    {
      return VertexIterator(this,0);
    }

    template<class M>
    inline MatrixGraph<M>::VertexIteratorT<MatrixGraph<M> >
    MatrixGraph<M>::end()
    {
      return VertexIterator(matrix_.N());
    }


    template<class M>
    inline MatrixGraph<M>::VertexIteratorT<const MatrixGraph<M> >
    MatrixGraph<M>::begin() const
    {
      return ConstVertexIterator(this, 0);
    }

    template<class M>
    inline MatrixGraph<M>::VertexIteratorT<const MatrixGraph<M> >
    MatrixGraph<M>::end() const
    {
      return ConstVertexIterator(matrix_.N());
    }

    template<class M>
    inline MatrixGraph<M>::EdgeIteratorT<MatrixGraph<M> >
    MatrixGraph<M>::beginEdges(const VertexDescriptor& source)
    {
      return EdgeIterator(source, matrix_.operator[](source).begin(),
                          matrix_.operator[](source).end(), start_[source]);
    }

    template<class M>
    inline MatrixGraph<M>::EdgeIteratorT<MatrixGraph<M> >
    MatrixGraph<M>::endEdges(const VertexDescriptor& source)
    {
      return EdgeIterator(matrix_.operator[](source).end());
    }


    template<class M>
    inline MatrixGraph<M>::EdgeIteratorT<const MatrixGraph<M> >
    MatrixGraph<M>::beginEdges(const VertexDescriptor& source) const
    {
      return ConstEdgeIterator(source, matrix_.operator[](source).begin(),
                               matrix_.operator[](source).end(), start_[source]);
    }

    template<class M>
    inline MatrixGraph<M>::EdgeIteratorT<const MatrixGraph<M> >
    MatrixGraph<M>::endEdges(const VertexDescriptor& source) const
    {
      return ConstEdgeIterator(matrix_.operator[](source).end());
    }


    template<class G, class T>
    SubGraph<G,T>::EdgeIterator::EdgeIterator(const VertexDescriptor& source,
                                              const EdgeDescriptor& edge)
      : source_(source), edge_(edge)
    {}


    template<class G, class T>
    SubGraph<G,T>::EdgeIterator::EdgeIterator(const EdgeDescriptor& edge)
      : edge_(edge)
    {}

    template<class G, class T>
    typename SubGraph<G,T>::EdgeIndexMap SubGraph<G,T>::getEdgeIndexMap()
    {
      return EdgeIndexMap(edges_);
    }

    template<class G, class T>
    inline bool SubGraph<G,T>::EdgeIterator::equals(const EdgeIterator & other) const
    {
      return other.edge_==edge_;
    }

    template<class G, class T>
    inline typename SubGraph<G,T>::EdgeIterator& SubGraph<G,T>::EdgeIterator::increment()
    {
      ++edge_;
      return *this;
    }

    template<class G, class T>
    inline typename SubGraph<G,T>::EdgeIterator& SubGraph<G,T>::EdgeIterator::decrement()
    {
      --edge_;
      return *this;
    }

    template<class G, class T>
    inline typename SubGraph<G,T>::EdgeIterator& SubGraph<G,T>::EdgeIterator::advance(std::ptrdiff_t n)
    {
      edge_+=n;
      return *this;
    }
    template<class G, class T>
    inline const typename G::VertexDescriptor& SubGraph<G,T>::EdgeIterator::source() const
    {
      return source_;
    }

    template<class G, class T>
    inline const typename G::VertexDescriptor& SubGraph<G,T>::EdgeIterator::target() const
    {
      return *edge_;
    }


    template<class G, class T>
    inline const typename SubGraph<G,T>::EdgeDescriptor& SubGraph<G,T>::EdgeIterator::dereference() const
    {
      return edge_;
    }

    template<class G, class T>
    inline std::ptrdiff_t SubGraph<G,T>::EdgeIterator::distanceTo(const EdgeIterator & other) const
    {
      return other.edge_-edge_;
    }

    template<class G, class T>
    SubGraph<G,T>::VertexIterator::VertexIterator(const SubGraph<G,T>* graph,
                                                  const VertexDescriptor& current,
                                                  const VertexDescriptor& end)
      : graph_(graph), current_(current), end_(end)
    {
      // Skip excluded vertices
      typedef typename T::const_iterator Iterator;

      for(Iterator vertex = graph_->excluded_.begin();
          current_ != end_ && *vertex;
          ++vertex)
        ++current_;
      assert(current_ == end_ || !graph_->excluded_[current_]);
    }

    template<class G, class T>
    SubGraph<G,T>::VertexIterator::VertexIterator(const VertexDescriptor& current)
      : current_(current)
    {}

    template<class G, class T>
    inline typename SubGraph<G,T>::VertexIterator& SubGraph<G,T>::VertexIterator::increment()
    {
      ++current_;
      //Skip excluded vertices
      while(current_ != end_ && graph_->excluded_[current_])
        ++current_;

      assert(current_ == end_ || !graph_->excluded_[current_]);
      return *this;
    }

    template<class G, class T>
    inline bool SubGraph<G,T>::VertexIterator::equals(const VertexIterator & other) const
    {
      return current_==other.current_;
    }

    template<class G, class T>
    inline const typename G::VertexDescriptor& SubGraph<G,T>::VertexIterator::dereference() const
    {
      return current_;
    }

    template<class G, class T>
    inline typename SubGraph<G,T>::EdgeIterator SubGraph<G,T>::VertexIterator::begin() const
    {
      return graph_->beginEdges(current_);
    }

    template<class G, class T>
    inline typename SubGraph<G,T>::EdgeIterator SubGraph<G,T>::VertexIterator::end() const
    {
      return graph_->endEdges(current_);
    }

    template<class G, class T>
    inline typename SubGraph<G,T>::VertexIterator SubGraph<G,T>::begin() const
    {
      return VertexIterator(this, 0, endVertex_);
    }


    template<class G, class T>
    inline typename SubGraph<G,T>::VertexIterator SubGraph<G,T>::end() const
    {
      return VertexIterator(endVertex_);
    }


    template<class G, class T>
    inline typename SubGraph<G,T>::EdgeIterator SubGraph<G,T>::beginEdges(const VertexDescriptor& source) const
    {
      return EdgeIterator(source, edges_+start_[source]);
    }

    template<class G, class T>
    inline typename SubGraph<G,T>::EdgeIterator SubGraph<G,T>::endEdges(const VertexDescriptor& source) const
    {
      return EdgeIterator(edges_+end_[source]);
    }

    template<class G, class T>
    int SubGraph<G,T>::noVertices() const
    {
      return noVertices_;
    }

    template<class G, class T>
    inline typename SubGraph<G,T>::VertexDescriptor SubGraph<G,T>::maxVertex() const
    {
      return maxVertex_;
    }

    template<class G, class T>
    inline int SubGraph<G,T>::noEdges() const
    {
      return noEdges_;
    }

    template<class G, class T>
    inline const typename SubGraph<G,T>::EdgeDescriptor& SubGraph<G,T>::findEdge(const VertexDescriptor & source,
                                                                                 const VertexDescriptor & target) const
    {
      const EdgeDescriptor& edge = std::lower_bound(edges_+start_[source], edges_+end_[source], target);
#ifdef DUNE_ISTL_WITH_CHECKING
      if(edge==edges_+end_[source] || *edge!=target)
        DUNE_THROW(ISTLError, "No such edge found!");
#endif

      return edge;
    }

    template<class G, class T>
    SubGraph<G,T>::~SubGraph()
    {
      delete[] edges_;
      delete[] end_;
      delete[] start_;
    }

    template<class G, class T>
    SubGraph<G,T>::SubGraph(const G& graph, const T& excluded)
      : excluded_(excluded), noVertices_(0), endVertex_(0), maxVertex_(graph.maxVertex())
    {
      start_ = new int[graph.noVertices()];
      end_ = new int[graph.noEdges()];
      edges_ = new VertexDescriptor[graph.noEdges()];

      VertexDescriptor* edge=edges_;

      typedef typename Graph::ConstVertexIterator Iterator;
      Iterator endVertex=graph.end();

      for(Iterator vertex = graph.begin(); vertex != endVertex; ++vertex)
        if(excluded_[*vertex])
          start_[*vertex]=end_[*vertex]=-1;
        else{
          ++noVertices_;
          endVertex_ = std::max(*vertex, endVertex_);

          start_[*vertex] = edge-edges_;

          typedef typename Graph::ConstEdgeIterator Iterator;
          Iterator endEdge = vertex.end();

          for(Iterator iter=vertex.begin(); iter!= endEdge; ++iter)
            if(!excluded[iter.target()]) {
              *edge = iter.target();
              ++edge;
            }

          end_[*vertex] = edge - edges_;

          // Sort the edges
          std::sort(edges_+start_[*vertex], edge);
        }
      noEdges_ = edge-edges_;
      ++endVertex_;
    }

    template<class G, class V, class VM>
    inline typename VertexPropertiesGraph<G,V,VM>::EdgeIterator
    VertexPropertiesGraph<G,V,VM>::beginEdges(const VertexDescriptor& source)
    {
      return graph_.beginEdges(source);
    }

    template<class G, class V, class VM>
    inline typename VertexPropertiesGraph<G,V,VM>::EdgeIterator
    VertexPropertiesGraph<G,V,VM>::endEdges(const VertexDescriptor& source)
    {
      return graph_.endEdges(source);
    }

    template<class G, class V, class VM>
    typename VertexPropertiesGraph<G,V,VM>::ConstEdgeIterator
    inline VertexPropertiesGraph<G,V,VM>::beginEdges(const VertexDescriptor& source) const
    {
      return graph_.beginEdges(source);
    }

    template<class G, class V, class VM>
    typename VertexPropertiesGraph<G,V,VM>::ConstEdgeIterator
    VertexPropertiesGraph<G,V,VM>::endEdges(const VertexDescriptor& source) const
    {
      return graph_.endEdges(source);
    }

    template<class G, class V, class VM>
    template<class C>
    VertexPropertiesGraph<G,V,VM>::VertexIteratorT<C>
    ::VertexIteratorT(const Father& iter,
                      C* graph)
      : Father(iter), graph_(graph)
    {}

    template<class G, class V, class VM>
    template<class C>
    VertexPropertiesGraph<G,V,VM>::VertexIteratorT<C>
    ::VertexIteratorT(const Father& iter)
      : Father(iter)
    {}

    template<class G, class V, class VM>
    template<class C>
    template<class C1>
    VertexPropertiesGraph<G,V,VM>::VertexIteratorT<C>
    ::VertexIteratorT(const VertexIteratorT<C1>& other)
      : Father(other), graph_(other.graph_)
    {}

    template<class G, class V, class VM>
    template<class C>
    typename SelectType<SameType<C,typename RemoveConst<C>::Type>::value,
        V&, const V&>::Type
    inline VertexPropertiesGraph<G,V,VM>::VertexIteratorT<C>::properties() const
    {
      return graph_->getVertexProperties(Father::operator*());
    }

    template<class G, class V, class VM>
    template<class C>
    typename SelectType<SameType<typename RemoveConst<C>::Type,
            C>::value,
        typename G::EdgeIterator,
        typename G::ConstEdgeIterator>::Type
    inline VertexPropertiesGraph<G,V,VM>::VertexIteratorT<C>::begin() const
    {
      return graph_->beginEdges(Father::operator*());
    }

    template<class G, class V, class VM>
    template<class C>
    typename SelectType<SameType<typename RemoveConst<C>::Type,
            C>::value,
        typename G::EdgeIterator,
        typename G::ConstEdgeIterator>::Type
    inline VertexPropertiesGraph<G,V,VM>::VertexIteratorT<C>::end() const
    {
      return graph_->endEdges(Father::operator*());
    }

    template<class G, class V, class VM>
    inline typename VertexPropertiesGraph<G,V,VM>::VertexIterator VertexPropertiesGraph<G,V,VM>::begin()
    {
      return VertexIterator(graph_.begin(), this);
    }

    template<class G, class V, class VM>
    inline typename VertexPropertiesGraph<G,V,VM>::VertexIterator VertexPropertiesGraph<G,V,VM>::end()
    {
      return VertexIterator(graph_.end());
    }


    template<class G, class V, class VM>
    inline typename VertexPropertiesGraph<G,V,VM>::ConstVertexIterator VertexPropertiesGraph<G,V,VM>::begin() const
    {
      return ConstVertexIterator(graph_.begin(), this);
    }

    template<class G, class V, class VM>
    inline typename VertexPropertiesGraph<G,V,VM>::ConstVertexIterator VertexPropertiesGraph<G,V,VM>::end() const
    {
      return ConstVertexIterator(graph_.end());
    }

    template<class G, class V, class VM>
    inline V& VertexPropertiesGraph<G,V,VM>::getVertexProperties(const VertexDescriptor& vertex)
    {
      return vertexProperties_[vmap_[vertex]];
    }

    template<class G, class V, class VM>
    inline const V& VertexPropertiesGraph<G,V,VM>::getVertexProperties(const VertexDescriptor& vertex) const
    {
      return vertexProperties_[vmap_[vertex]];
    }

    template<class G, class V, class VM>
    inline const G& VertexPropertiesGraph<G,V,VM>::graph() const
    {
      return graph_;
    }

    template<class G, class V, class VM>
    inline int VertexPropertiesGraph<G,V,VM>::noVertices() const
    {
      return graph_.noVertices();
    }


    template<class G, class V, class VM>
    inline typename VertexPropertiesGraph<G,V,VM>::VertexDescriptor VertexPropertiesGraph<G,V,VM>::maxVertex() const
    {
      return graph_.maxVertex();
    }

    template<class G, class V, class VM>
    VertexPropertiesGraph<G,V,VM>::VertexPropertiesGraph(Graph& graph, const VM vmap)
      : graph_(graph), vmap_(vmap), vertexProperties_(vmap_[graph_.maxVertex()], V())
    {}

    template<class G, class V, class E, class VM, class EM>
    template<class C>
    PropertiesGraph<G,V,E,VM,EM>::EdgeIteratorT<C>::EdgeIteratorT(const Father& iter,
                                                                  C* graph)
      : Father(iter), graph_(graph)
    {}

    template<class G, class V, class E, class VM, class EM>
    template<class C>
    PropertiesGraph<G,V,E,VM,EM>::EdgeIteratorT<C>::EdgeIteratorT(const Father& iter)
      : Father(iter)
    {}

    template<class G, class V, class E, class VM, class EM>
    template<class C>
    template<class C1>
    PropertiesGraph<G,V,E,VM,EM>::EdgeIteratorT<C>::EdgeIteratorT(const EdgeIteratorT<C1>& other)
      : Father(other), graph_(other.graph_)
    {}

    template<class G, class V, class E, class VM, class EM>
    template<class C>
    inline typename SelectType<SameType<C,typename RemoveConst<C>::Type>::value,E&,const E&>::Type
    PropertiesGraph<G,V,E,VM,EM>::EdgeIteratorT<C>::properties() const
    {
      return graph_->getEdgeProperties(Father::operator*());
    }

    template<class G, class V, class E, class VM, class EM>
    inline typename PropertiesGraph<G,V,E,VM,EM>::EdgeIterator
    PropertiesGraph<G,V,E,VM,EM>::beginEdges(const VertexDescriptor& source)
    {
      return EdgeIterator(graph_.beginEdges(source), this);
    }

    template<class G, class V, class E, class VM, class EM>
    inline typename PropertiesGraph<G,V,E,VM,EM>::EdgeIterator
    PropertiesGraph<G,V,E,VM,EM>::endEdges(const VertexDescriptor& source)
    {
      return EdgeIterator(graph_.endEdges(source));
    }

    template<class G, class V, class E, class VM, class EM>
    typename PropertiesGraph<G,V,E,VM,EM>::ConstEdgeIterator
    inline PropertiesGraph<G,V,E,VM,EM>::beginEdges(const VertexDescriptor& source) const
    {
      return ConstEdgeIterator(graph_.beginEdges(source), this);
    }

    template<class G, class V, class E, class VM, class EM>
    typename PropertiesGraph<G,V,E,VM,EM>::ConstEdgeIterator
    PropertiesGraph<G,V,E,VM,EM>::endEdges(const VertexDescriptor& source) const
    {
      return ConstEdgeIterator(graph_.endEdges(source));
    }

    template<class G, class V, class E, class VM, class EM>
    template<class C>
    PropertiesGraph<G,V,E,VM,EM>::VertexIteratorT<C>
    ::VertexIteratorT(const Father& iter,
                      C* graph)
      : Father(iter), graph_(graph)
    {}

    template<class G, class V, class E, class VM, class EM>
    template<class C>
    PropertiesGraph<G,V,E,VM,EM>::VertexIteratorT<C>
    ::VertexIteratorT(const Father& iter)
      : Father(iter)
    {}

    template<class G, class V, class E, class VM, class EM>
    template<class C>
    template<class C1>
    PropertiesGraph<G,V,E,VM,EM>::VertexIteratorT<C>
    ::VertexIteratorT(const VertexIteratorT<C1>& other)
      : Father(other), graph_(other.graph_)
    {}

    template<class G, class V, class E, class VM, class EM>
    template<class C>
    inline typename SelectType<SameType<C,typename RemoveConst<C>::Type>::value,
        V&, const V&>::Type
    PropertiesGraph<G,V,E,VM,EM>::VertexIteratorT<C>::properties() const
    {
      return graph_->getVertexProperties(Father::operator*());
    }

    template<class G, class V, class E, class VM, class EM>
    template<class C>
    inline PropertiesGraph<G,V,E,VM,EM>::EdgeIteratorT<C>
    PropertiesGraph<G,V,E,VM,EM>::VertexIteratorT<C>::begin() const
    {
      return graph_->beginEdges(Father::operator*());
    }

    template<class G, class V, class E, class VM, class EM>
    template<class C>
    inline PropertiesGraph<G,V,E,VM,EM>::EdgeIteratorT<C>
    PropertiesGraph<G,V,E,VM,EM>::VertexIteratorT<C>::end() const
    {
      return graph_->endEdges(Father::operator*());
    }

    template<class G, class V, class E, class VM, class EM>
    inline typename PropertiesGraph<G,V,E,VM,EM>::VertexIterator PropertiesGraph<G,V,E,VM,EM>::begin()
    {
      return VertexIterator(graph_.begin(), this);
    }

    template<class G, class V, class E, class VM, class EM>
    inline typename PropertiesGraph<G,V,E,VM,EM>::VertexIterator PropertiesGraph<G,V,E,VM,EM>::end()
    {
      return VertexIterator(graph_.end());
    }


    template<class G, class V, class E, class VM, class EM>
    inline typename PropertiesGraph<G,V,E,VM,EM>::ConstVertexIterator PropertiesGraph<G,V,E,VM,EM>::begin() const
    {
      return ConstVertexIterator(graph_.begin(), this);
    }

    template<class G, class V, class E, class VM, class EM>
    inline typename PropertiesGraph<G,V,E,VM,EM>::ConstVertexIterator PropertiesGraph<G,V,E,VM,EM>::end() const
    {
      return ConstVertexIterator(graph_.end());
    }

    template<class G, class V, class E, class VM, class EM>
    inline V& PropertiesGraph<G,V,E,VM,EM>::getVertexProperties(const VertexDescriptor& vertex)
    {
      return vertexProperties_[vmap_[vertex]];
    }

    template<class G, class V, class E, class VM, class EM>
    inline const V& PropertiesGraph<G,V,E,VM,EM>::getVertexProperties(const VertexDescriptor& vertex) const
    {
      return vertexProperties_[vmap_[vertex]];
    }

    template<class G, class V, class E, class VM, class EM>
    inline E& PropertiesGraph<G,V,E,VM,EM>::getEdgeProperties(const EdgeDescriptor& edge)
    {
      return edgeProperties_[emap_[edge]];
    }

    template<class G, class V, class E, class VM, class EM>
    inline const E& PropertiesGraph<G,V,E,VM,EM>::getEdgeProperties(const EdgeDescriptor& edge) const
    {
      return edgeProperties_[emap_[edge]];
    }

    template<class G, class V, class E, class VM, class EM>
    inline E& PropertiesGraph<G,V,E,VM,EM>::getEdgeProperties(const VertexDescriptor& source,
                                                              const VertexDescriptor& target)
    {
      return edgeProperties_[emap_[graph_.findEdge(source,target)]];
    }

    template<class G, class V, class E, class VM, class EM>
    inline const E& PropertiesGraph<G,V,E,VM,EM>::getEdgeProperties(const VertexDescriptor& source,
                                                                    const VertexDescriptor& target) const
    {
      return edgeProperties_[emap_[graph_.findEdge(source,target)]];
    }

    template<class G, class V, class E, class VM, class EM>
    inline const G& PropertiesGraph<G,V,E,VM,EM>::graph() const
    {
      return graph_;
    }

    template<class G, class V, class E, class VM, class EM>
    inline int PropertiesGraph<G,V,E,VM,EM>::noVertices() const
    {
      return graph_.noVertices();
    }


    template<class G, class V, class E, class VM, class EM>
    inline typename PropertiesGraph<G,V,E,VM,EM>::VertexDescriptor PropertiesGraph<G,V,E,VM,EM>::maxVertex() const
    {
      return graph_.maxVertex();
    }

    template<class G, class V, class E, class VM, class EM>
    PropertiesGraph<G,V,E,VM,EM>::PropertiesGraph(Graph& graph, const VM& vmap, const EM& emap)
      : graph_(graph), vmap_(vmap), vertexProperties_(vmap_[graph_.maxVertex()], V()),
        emap_(emap), edgeProperties_(graph_.noEdges(), E())
    {}

    template<class G, class V>
    inline void visitNeighbours(const G& graph, const typename G::VertexDescriptor& vertex,
                                V& visitor)
    {
      typedef typename G::ConstEdgeIterator iterator;
      const iterator end = graph.endEdges(vertex);
      for(iterator edge = graph.beginEdges(vertex); edge != end; ++edge)
        visitor(edge);
    }


    /** @} */
  }
}
#endif
