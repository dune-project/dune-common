// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_AMG_GRAPH_HH
#define DUNE_AMG_GRAPH_HH

#include <iostream>

namespace Dune
{
  namespace amg
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
     * @brief The weighted matrix graph with properties attached to the vertices
     * and edges.
     *
     * This class contains information about the graph of a matrix.
     * It holds information about the vertices and edges in the graph.
     * At each vertex and edge the values of the matrix are represented as
     * weights.
     */
    template<class M, class VP, class EP>
    class Graph
    {
    private:
      /**
       * @brief All the edges of a matrix graph.
       */
      class Edges
      {
      public:
        typedef const EP* const_iterator;
        typedef EP* iterator;

        /**
         * @brief Iterator for allocating space for the edge properties.
         */
        class BuildIterator
        {
        public:
          /** @brief Constructor. */
          BuildIterator(Edges& edges)
            : edges_(edges), currentVertex_(0), noEdges_(0)
          {

            if(edges_.vertices_>=0)
              edges_.start_[currentVertex_]=noEdges_;
          }
          /**
           * @brief Save Edge information for the current vertex and inkrement iterator.
           */
          BuildIterator& operator++()
          {
            ++currentVertex_;

            if(currentVertex_<edges_.vertices_)
              edges_.start_[currentVertex_]=noEdges_;
            else{
              edges_.start_[currentVertex_]=noEdges_;
              edges_.edges_ = new EP[noEdges_];
              edges_.edgesBuilt_ = true;
            }
            return *this;
          }

          /**
           * @brief Set the number of edges for the current vertex.
           * @param edges The number of edges.
           */
          void setNoEdges(int edges)
          {
            noEdges_+=edges;
          }

        private:
          /** @brief The edges we build. */
          Edges& edges_;
          /** @brief The current vertex. */
          int currentVertex_;
          /** @brief The total number of edges. */
          int noEdges_;
        };

        /**
         * @brief Free allocated memory.
         */
        void free();
        /**
         * @brief Reserve space for edges.
         * @param vertices The number of vertices the graph contains.
         */
        void reserveVertices(int vertices);

        /**
         * @brief Get an iterator for building the edge structure.
         * @return An Iterator for building then edge structure.
         */
        BuildIterator begin();

        iterator operator[](int i);

        const_iterator operator[](int i) const;
        /**
         * @brief Constructor.
         */
        Edges();

        /**
         * @brief Destructor.
         */
        ~Edges();

      private:
        /** @brief The indices of the first edge for each vertex.*/
        int* start_;
        /** @brief All edge properties ordered by source vertices. */
        EP* edges_;
        /** @brief True if the edges array is allocated. */
        bool edgesBuilt_;
        /** @brief True if the start array is allocated. */
        bool startBuilt_;
        /** @brief The number of vertices of the graph. */
        int vertices_;

      };

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
       * @brief The type of the properties of the vertices.
       */
      typedef VP VertexProperties;

      /**
       * @brief The type of the vertex descriptor.
       */
      typedef int VertexDescriptor;

      /**
       * @brief The type of the properties of the edges.
       */
      typedef EP EdgeProperties;

      class EdgeIterator;

      /**
       * @brief Const iterator over the edges starting at specific vertex of the graph.
       */
      class ConstEdgeIterator
      {
        typedef typename Matrix::row_type::ConstIterator ColIterator;
        friend class EdgeIterator;

      public:
        /**
         * @brief Constructor an iterator over all edges starting from a specific source vertex.
         *
         * @param source The index of the source vertex.
         * @param edge The edge the iterator should point to.
         * @param block The corresponding matrix row block.
         * @param blockEnd Pointer to the end of the matrix row.
         */
        ConstEdgeIterator(VertexDescriptor source, const typename Edges::const_iterator edge,
                          const ColIterator& block, const ColIterator& blockEnd);

        ConstEdgeIterator(const EdgeIterator& other);

        /**
         * @brief Access the edge properties
         */
        const EdgeProperties& properties() const;

        /**
         * @brief Access the edge weight
         */
        const Weight& weight() const;

        /** @brief preincrement operator. */
        ConstEdgeIterator& operator++();

        /** @brief Inequality operator. */
        bool operator!=(const EdgeIterator& other) const;

        /** @brief Inequality operator. */
        bool operator!=(const ConstEdgeIterator& other) const;

        /** @brief The index of the target vertex of the current edge. */
        VertexDescriptor target() const;

      private:
        /** The current edge. */
        typename Edges::const_iterator edge_;
        /** The matrix row iterator. */
        ColIterator block_;
        /** End of the matrix row. */
        ColIterator blockEnd_;
        /** @brief The source index of the edges. */
        VertexDescriptor source_;
      };

      /**
       * @brief Iterator over the edges starting at specific vertex of the graph.
       */
      class EdgeIterator
      {
        typedef typename Matrix::row_type::Iterator ColIterator;
        friend class ConstEdgeIterator;

      public:
        /**
         * @brief Constructor an iterator over all edges starting from a specific source vertex.
         *
         * @param source The index of the source vertex.
         * @param edge The edge the iterator should point to.
         * @param block The corresponding matrix row block.
         * @param blockEnd Pointer to the end of the matrix row.
         */
        EdgeIterator(VertexDescriptor source, const typename Edges::iterator edge, const ColIterator& block,
                     const ColIterator& blockEnd);

        /**
         * @brief Access the edge properties.
         */
        EdgeProperties& properties() const;

        /**
         * @brief Access the edge weight.
         */
        Weight& weight() const;

        /** @brief preincrement operator. */
        EdgeIterator& operator++();

        /** @brief Inequality operator. */
        bool operator!=(const EdgeIterator& other) const;

        /** @brief Inequality operator. */
        bool operator!=(const ConstEdgeIterator& other) const;

        /** @brief The index of the target vertex of the current edge. */
        VertexDescriptor target() const;

      private:
        /** The current edge. */
        typename Edges::iterator edge_;
        /** The matrix row iterator. */
        ColIterator block_;
        /** End of the matrix row. */
        ColIterator blockEnd_;
        /** @brief The source of the edges. */
        VertexDescriptor source_;
      };

      class ConstVertexIterator;

      /**
       * @brief Iterator over the matrix graph vertices.
       *
       * Provides access to the vertex indices and iterators
       * over the edges starting at each vertex.
       */
      class VertexIterator
      {
        friend class ConstVertexIterator;

      public:
        /**
         * @brief Constructor.
         *
         * @param graph Reference to surrounding graph.
         * @param currentVertex The current vertex index.
         */
        VertexIterator(Graph<M,VP,EP>* graph, VertexDescriptor currentVertex);

        /** @brief Preincrement operator. */
        VertexIterator& operator++();

        /** @brief Inequality operator. */
        bool operator!=(const VertexIterator& other) const;

        /** @brief Equality operator. */
        bool operator==(const VertexIterator& other) const;

        /** @brief Inequality operator. */
        bool operator!=(const ConstVertexIterator& other) const;

        /** @brief Equality operator. */
        bool operator==(const ConstVertexIterator& other) const;

        /** @brief Access the properties attached to the vertex. */
        VertexProperties& properties() const;

        /** @brief Access the weight of the vertex. */
        Weight& weight() const;

        /**
         * @brief Get the descriptor of the current vertex.
         * @return The index of the currently referenced vertex.
         */
        VertexDescriptor index() const;

        /**
         * @brief Get an iterator over all edges starting at the
         * current vertex.
         * @return Iterator position on the first edge to another vertex.
         */
        ConstEdgeIterator begin() const;

        /**
         * @brief Get an iterator over all edges starting at the
         * current vertex.
         * @return Iterator position on the first edge to another vertex.
         */
        ConstEdgeIterator end() const;

      private:
        /** @brief Reference to the outer graph. */
        Graph<M,VP,EP>* graph_;
        /** @brief Current vertex index. */
        VertexDescriptor current_;

      };

      /**
       * @brief Const iterator over the matrix graph vertices.
       *
       * Provides access to the vertex indices and iterators
       * over the edges starting at each vertex.
       */
      class ConstVertexIterator
      {
        friend class VertexIterator;

      public:
        /**
         * @brief Constructor.
         *
         * @param graph Reference to surrounding graph.
         * @param currentVertex The current vertex index.
         */
        ConstVertexIterator(const Graph<M,VP,EP>* graph, VertexDescriptor currentVertex);

        /**
         * @brief Copy Constructor.
         *
         * @param other The iterator to copy
         */
        ConstVertexIterator(const VertexIterator& other);

        /** @brief Preincrement operator. */
        ConstVertexIterator& operator++();

        /** @brief Inequality operator. */
        bool operator!=(const VertexIterator& other) const;

        /** @brief Inequality operator. */
        bool operator!=(const ConstVertexIterator& other) const;

        /** @brief Equality operator. */
        bool operator==(const VertexIterator& other) const;

        /** @brief Equality operator. */
        bool operator==(const ConstVertexIterator& other) const;

        /** @brief Access the properties attached to the vertex. */
        const VertexProperties& properties() const;

        /** @brief Access the weight of the vertex. */
        const Weight& weight() const;

        /**
         * @brief Get the descriptor of the current vertex.
         * @return The index of the currently referenced vertex.
         */
        VertexDescriptor index() const;

        /**
         * @brief Get an iterator over all edges starting at the
         * current vertex.
         * @return Iterator position on the first edge to another vertex.
         */
        ConstEdgeIterator begin() const;

        /**
         * @brief Get an iterator over all edges starting at the
         * current vertex.
         * @return Iterator position on the first edge to another vertex.
         */
        ConstEdgeIterator end() const;

      private:
        /** @brief Reference to the outer graph. */
        const Graph<M,VP,EP>* graph_;
        /** @brief Current vertex index. */
        VertexDescriptor current_;
      };

      /**
       * @brief Constructor.
       */
      Graph();

      /**
       * @brief Destructor.
       */
      ~Graph();

      /**
       * @brief Build the data structures of the graph.
       *
       * Allocates the space need to store the graph.
       * @param matrix The matrix whose graph we store.
       */
      void build(const Matrix& matrix);

      /**
       * @brief Frees the space allocated by the build method.
       */
      void free();

      /**
       * @brief Get the underlying matrix.
       * @return The underlying matrix.
       */
      const Matrix& matrix() const;

      /**
       * @brief Get an iterator over the edges of the vertex positioned at the first edge.
       * @param vertex The descriptor of the vertex whose edges we want.
       */
      ConstEdgeIterator beginEdges(VertexDescriptor vertex) const;

      /**
       * @brief Get an iterator over the edges of the vertex positioned at the first edge.
       * @param vertex The descriptor of vertex whose edges we want.
       */
      EdgeIterator beginEdges(VertexDescriptor vertex);

      /**
       * @brief Get an end iterator over the edges of the vertex.
       * @param vertex The descriptor of the vertex whose edges we want.
       */
      ConstEdgeIterator endEdges(VertexDescriptor vertex) const;

      /**
       * @brief Get an end iterator over the edges of the vertex.
       * @param vertex The descriptor of the vertex whose edges we want.
       */
      EdgeIterator endEdges(VertexDescriptor vertex);

      /**
       * @brief Random access to edges in the graph.
       * @param source The index of the source vertex.
       * @param target The index of the target vertex.
       * @return The edge pointing from source to target.
       */
      EdgeProperties& operator()(VertexDescriptor source, VertexDescriptor target);

      /**
       * @brief Random access to edges in the graph.
       * @param source The index of the source vertex.
       * @param target The index of the target vertex.
       * @return The edge pointing from source to target.
       */
      const EdgeProperties& operator()(VertexDescriptor source, VertexDescriptor target) const;

      /**
       * @brief Random access to the vertex properties of the graph.
       * @param index The index of the vertex.
       * @return The properties corresponding to that vertex.
       */
      VertexProperties& operator()(VertexDescriptor index);

      /**
       * @brief Random access to the vertex properties of the graph.
       * @param index The index of the vertex.
       * @return The properties corresponding to that vertex.
       */
      const VertexProperties& operator()(VertexDescriptor index) const;
      /**
       * @brief Get iterator over the vertices.
       * @retune An iterator over the vertices positioned at the
       * first vertex.
       */
      ConstVertexIterator begin() const;

      /**
       * @brief Get end iterator over the vertices.
       * @return An iterator over the vertices positioned behind the
       * last vertex.
       */
      ConstVertexIterator end() const;

      /**
       * @brief Get an iterator over the vertices.
       * @return An iterator over the vertices positioned at the
       * first vertex.
       */
      VertexIterator begin();

      /**
       * @brief Get end iterator over the vertices.
       * @return An iterator over the vertices positioned behind the
       * last vertex.
       */
      VertexIterator end();

      /**
       * @brief Print the matrix graph.
       * @param os The output stream to use.
       */
      void print(std::ostream& os) const;

    private:
      /** @brief the matrix we are a graph for. */
      const Matrix* matrix_;
      /** @brief The vertices of the graph. */
      VertexProperties* vertexProperties_;
      /** @brief The number of vertices of the graph. */
      int noVertices_;
      /** @brief  The edges of the graph. */
      Edges edges_;
      /** @brief True if the graph is built. */
      bool built_;
    };

    template<class M, class VP, class EP>
    Graph<M,VP,EP>::Graph()
      : matrix_(0), vertexProperties_(0), noVertices_(0), built_(false)
    {}

    template<class M, class VP, class EP>
    inline void Graph<M,VP,EP>::build(const Matrix& matrix)
    {
      // Setup the edges for each vertex.
      typedef typename Matrix::ConstRowIterator RowIterator;
      noVertices_ = matrix.N();
      vertexProperties_ = new VertexProperties[matrix.N()];
      edges_.reserveVertices(matrix.N());
      const RowIterator end = matrix.end();
      typename Edges::BuildIterator edgeBuilder = edges_.begin();

      for(RowIterator row = matrix.begin(); row != end; ++row,++edgeBuilder) {
        edgeBuilder.setNoEdges(row->size());
      }

      matrix_ = &matrix;
    }

    template<class M, class VP, class EP>
    inline void Graph<M,VP,EP>::free()
    {
      delete[] vertexProperties_;
      edges_.free();
      matrix_ = 0;
      noVertices_ = 0;
      built_=false;
    }

    template<class M, class VP, class EP>
    Graph<M,VP,EP>::~Graph()
    {
      if(built_)
        free();
    }

    template<class M, class VP, class EP>
    inline const M& Graph<M,VP,EP>::matrix() const
    {
      return *matrix_;
    }

    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::ConstEdgeIterator Graph<M,VP,EP>::beginEdges(VertexDescriptor row) const
    {
      return ConstEdgeIterator(row, edges_[row], matrix_[row].begin(), matrix_[row].end());
    }

    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::ConstEdgeIterator Graph<M,VP,EP>::endEdges(VertexDescriptor row) const
    {
      return ConstEdgeIterator(row, edges_[row+1], matrix_[row].end(), matrix_[row].end());
    }

    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::EdgeIterator Graph<M,VP,EP>::beginEdges(VertexDescriptor row)
    {
      return EdgeIterator(row, edges_[row], matrix_[row].begin(), matrix_[row].end());
    }

    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::EdgeIterator Graph<M,VP,EP>::endEdges(VertexDescriptor row)
    {
      return EdgeIterator(row, edges_[row+1], matrix_[row].end(), matrix_[row].end());
    }

    template<class M, class VP, class EP>
    inline EP& Graph<M,VP,EP>::operator()(VertexDescriptor source, VertexDescriptor target)
    {
      return edges_[source] + matrix[source].find(target).offset();
    }


    template<class M, class VP, class EP>
    inline const EP& Graph<M,VP,EP>::operator()(VertexDescriptor source, VertexDescriptor target) const
    {
      return edges_[source] + matrix[source].find(target).offset();
    }

    template<class M, class VP, class EP>
    inline Graph<M,VP,EP>::ConstEdgeIterator::ConstEdgeIterator(VertexDescriptor source, const typename Edges::const_iterator edge,
                                                                const ColIterator& block, const ColIterator& blockEnd) :
      edge_(edge), block_(block), blockEnd_(blockEnd), source_(source)
    {
      if(block_!=blockEnd_ && block.index() == source_) {
        // This is the diagonal and not a edge. Skip it.
        ++block_;
        ++edge_;
      }
    }

    template<class M, class VP, class EP>
    inline Graph<M,VP,EP>::ConstEdgeIterator::ConstEdgeIterator(const EdgeIterator& other)
      : edge_(other.edge_), block_(other.block_), source_(other.source_)
    {}

    template<class M, class VP, class EP>
    inline const EP & Graph<M,VP,EP>::ConstEdgeIterator::properties() const
    {
      return *edge_;
    }


    template<class M, class VP, class EP>
    inline const typename M::block_type& Graph<M,VP,EP>::ConstEdgeIterator::weight() const
    {
      return *block_;
    }

    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::ConstEdgeIterator& Graph<M,VP,EP>::ConstEdgeIterator::operator++()
    {
      ++block_;
      ++edge_;

      if(block_!=blockEnd_ && block_.index() == source_) {
        // This is the edge from the diagonal to the diagonal. Skip it.
        ++block_;
        ++edge_;
      }

      return *this;
    }

    template<class M, class VP, class EP>
    inline bool Graph<M,VP,EP>::ConstEdgeIterator::operator!=(const Graph<M,VP,EP>::EdgeIterator& other) const
    {
      return block_!=other.block_;
    }

    template<class M, class VP, class EP>
    inline bool Graph<M,VP,EP>::ConstEdgeIterator::operator!=(const Graph<M,VP,EP>::ConstEdgeIterator& other) const
    {
      return block_!=other.block_;
    }

    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::VertexDescriptor Graph<M,VP,EP>::ConstEdgeIterator::target() const
    {
      return block_.index();
    }


    template<class M, class VP, class EP>
    inline Graph<M,VP,EP>::EdgeIterator::EdgeIterator(VertexDescriptor source, const typename Edges::iterator edge,
                                                      const ColIterator& block, const ColIterator& blockEnd) :
      edge_(edge), block_(block), blockEnd_(blockEnd), source_(source)
    {
      if(block_!=blockEnd_ && block->index() == source_) {
        // This is the edge from the diagonal to the diagonal. Skip it.
        ++block_;
        ++edge_;
      }
    }

    template<class M, class VP, class EP>
    inline EP & Graph<M,VP,EP>::EdgeIterator::properties() const
    {
      return *edge_;
    }


    template<class M, class VP, class EP>
    inline typename M::block_type& Graph<M,VP,EP>::EdgeIterator::weight() const
    {
      return *block_;
    }

    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::EdgeIterator& Graph<M,VP,EP>::EdgeIterator::operator++()
    {
      ++block_;
      ++edge_;

      if(block_!=blockEnd_ && block_->index() == source_) {
        // This is the edge from the diagonal to the diagonal. Skip it.
        ++block_;
      }
      return *this;
    }

    template<class M, class VP, class EP>
    inline bool Graph<M,VP,EP>::EdgeIterator::operator!=(const Graph<M,VP,EP>::ConstEdgeIterator& other) const
    {
      return block_!=other.block_;
    }

    template<class M, class VP, class EP>
    inline bool Graph<M,VP,EP>::EdgeIterator::operator!=(const Graph<M,VP,EP>::EdgeIterator& other) const
    {
      return block_!=other.block_;
    }
    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::VertexDescriptor Graph<M,VP,EP>::EdgeIterator::target() const
    {
      return block_->index();
    }

    template<class M, class VP, class EP>
    inline void Graph<M,VP,EP>::print(std::ostream& os) const
    {
      for(ConstVertexIterator vertex = begin(); vertex!=end(); ++vertex) {
        const ConstEdgeIterator endEdge = vertex.end();
        os<<"Edges starting from Vertex "<<vertex.index()<<" (weight="<<vertex.weight()<<", properties="<<vertex.properties()<<") to vertices ";

        for(ConstEdgeIterator edge = vertex.begin(); edge != endEdge; ++edge)
          os<<edge.target()<<" (weight="<<edge.weight()<<", properties="<<edge.properties()<<"), ";
        os<<std::endl;
      }
    }

    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::ConstVertexIterator Graph<M,VP,EP>::begin() const
    {
      return ConstVertexIterator(this, 0);
    }

    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::ConstVertexIterator Graph<M,VP,EP>::end() const
    {
      return ConstVertexIterator(this, noVertices_);
    }

    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::VertexIterator Graph<M,VP,EP>::begin()
    {
      return VertexIterator(this, 0);
    }

    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::VertexIterator Graph<M,VP,EP>::end()
    {
      return VertexIterator(this, noVertices_);
    }

    template<class M, class VP, class EP>
    inline Graph<M,VP,EP>::VertexIterator::VertexIterator(Graph<M,VP,EP>* graph, VertexDescriptor current)
      : graph_(graph), current_(current)
    {}

    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::VertexIterator& Graph<M,VP,EP>::VertexIterator::operator++()
    {
      ++current_;
      return *this;
    }

    template<class M, class VP, class EP>
    inline VP & Graph<M,VP,EP>::VertexIterator::properties() const
    {
      return graph_->vertexProperties_[current_];
    }

    template<class M, class VP, class EP>
    inline typename M::block_type& Graph<M,VP,EP>::VertexIterator::weight() const
    {
      return graph_->matrix()[current_][current_];
    }

    template<class M, class VP, class EP>
    inline bool Graph<M,VP,EP>::VertexIterator::operator!=(const VertexIterator& other) const
    {
      return current_!=other.current_;
    }

    template<class M, class VP, class EP>
    inline bool Graph<M,VP,EP>::VertexIterator::operator==(const VertexIterator& other) const
    {
      return current_==other.current_;
    }

    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::VertexDescriptor Graph<M,VP,EP>::VertexIterator::index() const
    {
      return current_;
    }

    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::ConstEdgeIterator Graph<M,VP,EP>::VertexIterator::begin() const
    {
      return ConstEdgeIterator(current_, graph_->edges_[current_],
                               graph_->matrix()[current_].begin(), graph_->matrix()[current_].end());
    }

    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::ConstEdgeIterator Graph<M,VP,EP>::VertexIterator::end() const
    {
      return ConstEdgeIterator(current_, graph_->edges_[current_+1],
                               graph_->matrix()[current_].end(), graph_->matrix()[current_].end());
    }

    template<class M, class VP, class EP>
    inline Graph<M,VP,EP>::ConstVertexIterator::ConstVertexIterator(const Graph<M,VP,EP>* graph, VertexDescriptor current)
      : graph_(graph), current_(current)
    {}

    template<class M, class VP, class EP>
    inline Graph<M,VP,EP>::ConstVertexIterator::ConstVertexIterator(const VertexIterator& other)
      : graph_(other.graph_), current_(other.current_)
    {}

    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::ConstVertexIterator& Graph<M,VP,EP>::ConstVertexIterator::operator++()
    {
      ++current_;
      return *this;
    }

    template<class M, class VP, class EP>
    inline const VP & Graph<M,VP,EP>::ConstVertexIterator::properties() const
    {
      return graph_->vertexProperties_[current_];
    }

    template<class M, class VP, class EP>
    inline const typename M::block_type& Graph<M,VP,EP>::ConstVertexIterator::weight() const
    {
      return graph_->matrix()[current_][current_];
    }

    template<class M, class VP, class EP>
    inline bool Graph<M,VP,EP>::ConstVertexIterator::operator!=(const VertexIterator& other) const
    {
      assert(graph_==other.graph_);
      return current_!=other.current_;
    }

    template<class M, class VP, class EP>
    inline bool Graph<M,VP,EP>::ConstVertexIterator::operator!=(const ConstVertexIterator& other) const
    {
      assert(graph_==other.graph_);
      return current_!=other.current_;
    }
    template<class M, class VP, class EP>
    inline bool Graph<M,VP,EP>::ConstVertexIterator::operator==(const VertexIterator& other) const
    {
      assert(graph_==other.graph_);
      return current_==other.current_;
    }

    template<class M, class VP, class EP>
    inline bool Graph<M,VP,EP>::ConstVertexIterator::operator==(const ConstVertexIterator& other) const
    {
      assert(graph_==other.graph_);
      return current_==other.current_;
    }

    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::VertexDescriptor Graph<M,VP,EP>::ConstVertexIterator::index() const
    {
      return current_;
    }

    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::ConstEdgeIterator Graph<M,VP,EP>::ConstVertexIterator::begin() const
    {
      return ConstEdgeIterator(current_, graph_->edges_[current_],
                               graph_->matrix()[current_].begin(), graph_->matrix()[current_].end());
    }

    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::ConstEdgeIterator Graph<M,VP,EP>::ConstVertexIterator::end() const
    {
      return ConstEdgeIterator(current_, graph_->edges_[current_+1],
                               graph_->matrix()[current_].end(), graph_->matrix()[current_].end());
    }

    template<class M, class VP, class EP>
    inline Graph<M,VP,EP>::Edges::Edges()
      : start_(), edges_(), edgesBuilt_(false), startBuilt_(false), vertices_(-1)
    {}

    template<class M, class VP, class EP>
    inline void Graph<M,VP,EP>::Edges::free()
    {
      if(edgesBuilt_)
        delete[] edges_;
      if(startBuilt_)
        delete[] start_;
      edgesBuilt_ = false;
      startBuilt_ = false;
      vertices_=-1;
    }

    template<class M, class VP, class EP>
    inline void Graph<M,VP,EP>::Edges::reserveVertices(int vertices)
    {
      vertices_=vertices;
      start_ = new int[vertices_+1];
      startBuilt_ = true;
    }

    template<class M, class VP, class EP>
    inline typename Graph<M,VP,EP>::Edges::BuildIterator Graph<M,VP,EP>::Edges::begin()
    {
      return BuildIterator(*this);
    }

    template<class M, class VP, class EP>
    inline EP* Graph<M,VP,EP>::Edges::operator[](int i)
    {
      return edges_+start_[i];
    }

    template<class M, class VP, class EP>
    inline const EP* Graph<M,VP,EP>::Edges::operator[](int i) const
    {
      return edges_+start_[i];
    }

    template<class M, class VP, class EP>
    inline Graph<M,VP,EP>::Edges::~Edges()
    {
      if(startBuilt_)
        delete[] start_;
      if(edgesBuilt_)
        delete[] edges_;
    }
    /** @} */
  }
}
#endif
