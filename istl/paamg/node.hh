// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_AMG_NODE_HH
#define DUNE_AMG_NODE_HH

namespace Dune
{
  namespace amg
  {
    /**
     * @addtogroup ISTL_PAAMG
     *
     * @{
     * @file
     * @author Markus Blatt
     * @brief Provides class representing the attributes of
     * a matrix graph node.
     */

    /**
     * @brief Class representing a node in the matrix graph.
     *
     * Contains methods for getting and setting node attributes.
     */
    class Node {
    private:
      /** @brief The attribute flags. */
      char flags_;
    public:
      /** @brief Constructor. */
      Node();

      /**
       * @brief Marks that node as being isolated.
       *
       * A node is isolated if it ha not got any strong connections to other nodes
       * in the matrix graph.
       */
      void setIsolated();

      /**
       * @brief Checks wether the node is isolated.
       */
      bool isIsolated();

      /**
       * @brief Resets the isolated flag.
       */
      void resetIsolated();

      /**
       * @brief Mark the node as already visited.
       */
      void setVisited();

      /**
       * @brief Checks wether the node is marked as visited.
       */
      bool isVisited();

      /**
       * @brief Resets the visited flag.
       */
      void resetVisited();

      /**
       * @brief Marks the node as belonging to the current clusters front.
       */
      void setFrontNode();

      /**
       * @brief Checks wether the node is marked as a front node.
       */
      bool isFrontNode();

      /**
       *  @brief Resets the front node flag.
       */
      void resetFrontNode();
    };

    inline Node::Node()
      : flags_(0)
    {}

    inline void Node::setIsolated()
    {
      (flags_) = (flags_) | (0x1);
    }

    inline bool Node::isIsolated()
    {
      return (flags_) & (0x1);
    }

    inline void Node::resetIsolated()
    {
      (flags_) = (flags_) & (~(0x1));
    }

    inline void Node::setVisited()
    {
      (flags_) = (flags_) | (0x80);
    }

    inline bool Node::isVisited()
    {
      return (flags_) & (0x80);
    }

    inline void Node::resetVisited()
    {
      (flags_) = (flags_) & (0x7F);
    }

    inline void Node::setFrontNode()
    {
      (flags_) = (flags_) |(0x40);
    }

    inline bool Node::isFrontNode()
    {
      return (flags_) & (0x40);
    }

    inline void Node::resetFrontNode()
    {
      (flags_) = (flags_) & (0xBF);
    }

    /** @} */
  }

}
#endif
