// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_AMG_GRAPH_HH
#define DUNE_AMG_GRAPH_HH

#include "link.hh"
#include "node.hh"
#include <iostream>
namespace Dune
{
  namespace amg
  {
    /**
     * @defgroup ISTL_PAAMG Parallel Algebraic Multigrid
     * @ingroup ISTL
     * @brief A Parallel Algebraic Multigrid based on Agglomeration
     *
     * @addtogroup ISTL_PAAMG
     *
     * @{
     * @file
     * @author Markus Blatt
     * @brief Provides classes for building the matrix graph.
     *
     * During the coarsening process in AMG the matrix graph together
     * with the dependencies, what connections in the graph are considered
     * strong or weak, what nodes are isolated, etc., have to build.
     * This information will be contained in the MatrixGraph class.
     */

    /**
     * @brief The matrix graph.
     *
     * This class contains information about the graph of a matrix.
     * It holds information about the nodes and links in the graph.
     */
    template<class M>
    class Graph
    {
    public:
      /**
       * @brief The type of the matrix we are a graph for.
       */
      typedef M Matrix;

      class LinkIterator;

      /**
       * @brief Const iterator over the links starting at specific node of the graph.
       */
      class ConstLinkIterator
      {
        typedef typename Matrix::row_type::ConstIterator RowIterator;
        friend class LinkIterator;

      public:
        /**
         * @brief Constructor an iterator over all links starting from a specific source node.
         *
         * @param source The index of the source node.
         * @param link The link the iterator should point to.
         * @param block The corresponding matrix row block.
         * @param blockEnd Pointer to the end of the matrix row.
         */
        ConstLinkIterator(int source, const typename Links::const_iterator link,
                          const RowIterator& block, const RowIterator& blockEnd);

        const Link& operator*() const;

        const Link* operator->() const;

        /** @brief preincrement operator. */
        ConstLinkIterator& operator++();

        /** @brief Inequality operator. */
        bool operator!=(const LinkIterator& other) const;

        /** @brief Inequality operator. */
        bool operator!=(const ConstLinkIterator& other) const;

        /** @brief The index of the destination node of the current link. */
        int destination() const;

      private:
        /** The current link. */
        typename Links::const_iterator link_;
        /** The matrix row iterator. */
        RowIterator block_;
        /** End of the matrix row. */
        RowIterator blockEnd_;
        /** @brief The source index of the links. */
        int source_;
      };

      /**
       * @brief Iterator over the links starting at specific node of the graph.
       */
      class LinkIterator
      {
        typedef typename Matrix::row_type::Iterator RowIterator;
        friend class ConstLinkIterator;

      public:
        /**
         * @brief Constructor an iterator over all links starting from a specific source node.
         *
         * @param source The index of the source node.
         * @param link The link the iterator should point to.
         * @param block The corresponding matrix row block.
         * @param blockEnd Pointer to the end of the matrix row.
         */
        LinkIterator(int source, const typename Links::iterator link, const RowIterator& block,
                     const RowIterator& blockEnd);

        Link& operator*() const;

        Link* operator->() const;

        /** @brief preincrement operator. */
        LinkIterator& operator++();

        /** @brief Inequality operator. */
        bool operator!=(const LinkIterator& other) const;

        /** @brief Inequality operator. */
        bool operator!=(const ConstLinkIterator& other) const;

        /** @brief The index of the destination node of the current link. */
        int destination() const;

      private:
        /** The current link. */
        typename Links::iterator link_;
        /** The matrix row iterator. */
        RowIterator block_;
        /** End of the matrix row. */
        RowIterator blockEnd_;
        /** @brief The source index of the links. */
        int source_;
      };

      /**
       * @brief Iterator over the matrix graph nodes.
       *
       * Provides access to the node indices and iterators
       * over the links starting at each node.
       */
      class NodeIterator
      {
      public:
        /**
         * @brief Constructor.
         *
         * @param graph Reference to surrounding graph.
         * @param currentNode The current node index.
         */
        NodeIterator(const Graph<M>& graph, int currentNode);

        /** @brief Preincrement operator. */
        NodeIterator& operator++();

        /** @brief Inequality operator. */
        bool operator!=(const NodeIterator& other) const;

        /** @brief Equality operator. */
        bool operator==(const NodeIterator& other) const;

        /**
         * @brief Get the index of the current node.
         * @return The index of the currently referenced node.
         */
        int index() const;

        /**
         * @brief Get an iterator over all links starting at the
         * current node.
         * @return Iterator position on the first link to another node.
         */
        ConstLinkIterator begin() const;

        /**
         * @brief Get an iterator over all links starting at the
         * current node.
         * @return Iterator position on the first link to another node.
         */
        ConstLinkIterator end() const;

      private:
        /** @brief Reference to the outer graph. */
        const Graph<M>& graph_;
        /** @brief Current node index. */
        int current_;
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
       * @brief Get an iterator over the links of the node positioned at the first link.
       * @param row The index of the matrix row whose links we want.
       */
      ConstLinkIterator beginLinks(int row) const;

      /**
       * @brief Get an iterator over the links of the node positioned at the first link.
       * @param row The index of the matrix row whose links we want.
       */
      LinkIterator beginLinks(int row);

      /**
       * @brief Get an end iterator over the links of the node.
       * @param row The index of the matrix row whose links we want.
       */
      ConstLinkIterator endLinks(int row) const;

      /**
       * @brief Get an end iterator over the links of the node.
       * @param row The index of the matrix row whose links we want.
       */
      LinkIterator endLinks(int row);

      /**
       * @brief Random access to links in the graph.
       * @param source The index of the source node.
       * @param destination The index of the destination node.
       * @return The link pointing from source to destination.
       */
      Link& operator()(int source, int destination);

      /**
       * @brief Random access to links in the graph.
       * @param source The index of the source node.
       * @param destination The index of the destination node.
       * @return The link pointing from source to destination.
       */
      const Link& operator()(int source, int destination) const;

      /**
       * @brief Random access to the nodes of the graph.
       * @param index The index of the node.
       * @return The node corresponding to that index.
       */
      Node& operator()(int index);

      /**
       * @brief Random access to the nodes of the graph.
       * @param index The index of the node.
       * @return The node corresponding to that index.
       */
      const Node& operator()(int index) const;
      /**
       * @brief Get end iterator over the nodes.
       * @param A iterator over the nodes positioned at the
       * first node.
       */
      NodeIterator begin() const;

      /**
       * @brief Get end iterator over the nodes.
       * @param A iterator over the nodes positioned behind the
       * last node.
       */
      NodeIterator end() const;

      /**
       * @brief Print the matrix graph.
       * @param os The output stream to use.
       */
      void print(std::ostream& os) const;

    private:
      /** @brief the matrix we are a graph for. */
      const Matrix* matrix_;
      /** @brief The nodes of the graph. */
      Node* nodes_;
      /** @brief The number of nodes of the graph. */
      int noNodes_;
      /** @brief  The links of the graph. */
      Links links_;
      /** @brief True if the graph is built. */
      bool built_;
    };

    template<class M>
    Graph<M>::Graph()
      : matrix_(0), nodes_(0), noNodes_(0), built_(false)
    {}

    template<class M>
    inline void Graph<M>::build(const Matrix& matrix)
    {
      // Setup the links for each node.
      typedef typename Matrix::ConstRowIterator RowIterator;
      noNodes_ = matrix.N();
      nodes_ = new Node[matrix.N()];
      links_.reserveNodes(matrix.N());
      const RowIterator end = matrix.end();
      typename Links::BuildIterator linkBuilder = links_.begin();

      for(RowIterator row = matrix.begin(); row != end; ++row,++linkBuilder) {
        linkBuilder.setNoLinks(row->size()-1);
      }

      matrix_ = &matrix;
    }

    template<class M>
    inline void Graph<M>::free()
    {
      delete[] nodes_;
      links_.free();
      matrix_ = 0;
      noNodes_ = 0;
      built_=false;
    }

    template<class M>
    Graph<M>::~Graph()
    {
      if(built_)
        free();
    }

    template<class M>
    inline const M& Graph<M>::matrix() const
    {
      return *matrix_;
    }

    template<class M>
    inline typename Graph<M>::ConstLinkIterator Graph<M>::beginLinks(int row) const
    {
      return ConstLinkIterator(row, links_[row], matrix_[row].begin(), matrix_[row].end());
    }

    template<class M>
    inline typename Graph<M>::ConstLinkIterator Graph<M>::endLinks(int row) const
    {
      return ConstLinkIterator(row, links_[row+1], matrix_[row].end(), matrix_[row].end());
    }

    template<class M>
    inline typename Graph<M>::LinkIterator Graph<M>::beginLinks(int row)
    {
      return LinkIterator(row, links_[row], matrix_[row].begin(), matrix_[row].end());
    }

    template<class M>
    inline typename Graph<M>::LinkIterator Graph<M>::endLinks(int row)
    {
      return LinkIterator(row, links_[row+1], matrix_[row].end(), matrix_[row].end());
    }

    template<class M>
    inline Link& Graph<M>::operator()(int source, int destination)
    {
      return links_[source] + matrix[source].find(destination)-matrix[source].begin();
    }


    template<class M>
    inline const Link& Graph<M>::operator()(int source, int destination) const
    {
      return links_[source] + matrix[source].find(destination)-matrix[source].begin();
    }

    template<class M>
    inline Graph<M>::ConstLinkIterator::ConstLinkIterator(int source, const typename Links::const_iterator link,
                                                          const RowIterator& block, const RowIterator& blockEnd) :
      link_(link), block_(block), blockEnd_(blockEnd), source_(source)
    {
      if(block_!=blockEnd_ && block.index() == source_) {
        ++block_;
      }else{
        int i=source_;
        i+=5;
      }
    }

    template<class M>
    inline const Link& Graph<M>::ConstLinkIterator::operator*() const
    {
      return *link_;
    }


    template<class M>
    inline const Link* Graph<M>::ConstLinkIterator::operator->() const
    {
      return link_;
    }

    template<class M>
    inline typename Graph<M>::ConstLinkIterator& Graph<M>::ConstLinkIterator::operator++()
    {
      ++block_;
      ++link_;

      if(block_!=blockEnd_ && block_.index() == source_) {
        ++block_;
      }

      return *this;
    }

    template<class M>
    inline bool Graph<M>::ConstLinkIterator::operator!=(const Graph<M>::LinkIterator& other) const
    {
      return block_!=other.block_;
    }

    template<class M>
    inline bool Graph<M>::ConstLinkIterator::operator!=(const Graph<M>::ConstLinkIterator& other) const
    {
      return block_!=other.block_;
    }

    template<class M>
    inline int Graph<M>::ConstLinkIterator::destination() const
    {
      return block_.index();
    }


    template<class M>
    inline Graph<M>::LinkIterator::LinkIterator(int source, const typename Links::iterator link,
                                                const RowIterator& block, const RowIterator& blockEnd) :
      link_(link), block_(block), blockEnd_(blockEnd), source_(source)
    {
      if(block_!=blockEnd_ && block->index() == source_) {
        ++block_;
      }
    }

    template<class M>
    inline Link& Graph<M>::LinkIterator::operator*() const
    {
      return *link_;
    }


    template<class M>
    inline Link* Graph<M>::LinkIterator::operator->() const
    {
      return link_;
    }

    template<class M>
    inline typename Graph<M>::LinkIterator& Graph<M>::LinkIterator::operator++()
    {
      ++block_;
      ++link_;

      if(block_!=blockEnd_ && block_->index() == source_) {
        ++block_;
      }
      return *this;
    }

    template<class M>
    inline bool Graph<M>::LinkIterator::operator!=(const Graph<M>::ConstLinkIterator& other) const
    {
      return block_!=other.block_;
    }

    template<class M>
    inline bool Graph<M>::LinkIterator::operator!=(const Graph<M>::LinkIterator& other) const
    {
      return block_!=other.block_;
    }
    template<class M>
    inline int Graph<M>::LinkIterator::destination() const
    {
      return block_->index();
    }

    template<class M>
    inline void Graph<M>::print(std::ostream& os) const
    {
      for(NodeIterator node = begin(); node!=end(); ++node) {
        const ConstLinkIterator endLink = node.end();
        os<<"Links starting from Node "<<node.index()<<" to nodes ";

        for(ConstLinkIterator link = node.begin(); link != endLink; ++link)
          os<<link.destination()<<" ";
        os<<std::endl;
      }
    }

    template<class M>
    inline typename Graph<M>::NodeIterator Graph<M>::begin() const
    {
      return NodeIterator(*this, 0);
    }

    template<class M>
    inline typename Graph<M>::NodeIterator Graph<M>::end() const
    {
      return NodeIterator(*this, noNodes_);
    }

    template<class M>
    inline Graph<M>::NodeIterator::NodeIterator(const Graph<M>& graph, int current)
      : graph_(graph), current_(current)
    {}

    template<class M>
    inline typename Graph<M>::NodeIterator& Graph<M>::NodeIterator::operator++()
    {
      ++current_;
      return *this;
    }

    template<class M>
    inline bool Graph<M>::NodeIterator::operator!=(const NodeIterator& other) const
    {
      return current_!=other.current_;
    }

    template<class M>
    inline bool Graph<M>::NodeIterator::operator==(const NodeIterator& other) const
    {
      return current_==other.current_;
    }

    template<class M>
    inline int Graph<M>::NodeIterator::index() const
    {
      return current_;
    }

    template<class M>
    inline typename Graph<M>::ConstLinkIterator Graph<M>::NodeIterator::begin() const
    {
      return ConstLinkIterator(current_, graph_.links_[current_],
                               graph_.matrix()[current_].begin(), graph_.matrix()[current_].end());
    }

    template<class M>
    inline typename Graph<M>::ConstLinkIterator Graph<M>::NodeIterator::end() const
    {
      return ConstLinkIterator(current_, graph_.links_[current_+1],
                               graph_.matrix()[current_].end(), graph_.matrix()[current_].end());
    }


    /** @} */
  }
}
#endif
