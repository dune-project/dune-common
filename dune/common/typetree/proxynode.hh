// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_TYPETREE_PROXYNODE_HH
#define DUNE_COMMON_TYPETREE_PROXYNODE_HH

#include <type_traits>

#include <dune/common/indices.hh>
#include <dune/common/shared_ptr.hh>
#include <dune/common/std/type_traits.hh>
#include <dune/common/typetree/nodebase.hh>
#include <dune/common/typetree/nodeinterface.hh>
#include <dune/common/typetree/nodetags.hh>

namespace Dune {
  namespace TypeTree {

    /** \addtogroup Nodes
     *  \ingroup TypeTree
     *  \{
     */

    template<class Node>
    class ProxyNode;

    //! Mixin class providing methods for child access with compile-time parameter.
    template<class ProxiedNode>
    class StaticChildAccessors
    {
      static const bool proxiedNodeIsConst = std::is_const<std::remove_reference_t<ProxiedNode>>::value;

      typedef ProxyNode<ProxiedNode> Node;

      template<bool enabled = !proxiedNodeIsConst>
      std::enable_if_t<enabled, Node&> node ()
      {
        return static_cast<Node&>(*this);
      }

      const Node& node () const
      {
        return static_cast<const Node&>(*this);
      }

    public:

      //! Access to the type and storage type of the i-th child.
      template<std::size_t k>
      using Child = typename ProxiedNode::template Child<k>;

      //! @name Child Access
      //! @{

      //! Returns the i-th child.
      /**
       * \returns a reference to the i-th child.
       */
      template<std::size_t k, bool enabled = !proxiedNodeIsConst>
      std::enable_if_t<enabled, Child<k>&>
      child (index_constant<k> = {})
      {
        return node().proxiedNode().template child<k>();
      }

      //! Returns the i-th child (const version).
      /**
       * \returns a const reference to the i-th child.
       */
      template<std::size_t k>
      const Child<k>& child (index_constant<k> = {}) const
      {
        return node().proxiedNode().template child<k>();
      }

      //! Sets the i-th child to the passed-in value.
      template<std::size_t k, class C, bool enabled = !proxiedNodeIsConst>
      std::enable_if_t<enabled>
      setChild (C&& child, index_constant<k> = {})
      {
        node().proxiedNode().template setChild<k>(std::forward<C>(child));
      }

      //! Return the storage container for the nodes
      const typename ProxiedNode::NodeStorage& nodeStorage () const
      {
        return node().proxiedNode().nodeStorage();
      }
    };

    //! Mixin class providing methods for child access with run-time parameter.
    /**
     * This class also provides the compile-time parameter based methods, as
     * multiple inheritance from both DynamicChildAccessors and StaticChildAccessors
     * creates ambigous method lookups.
     */
    template<class ProxiedNode>
    class DynamicChildAccessors
      : public StaticChildAccessors<ProxiedNode>
    {

      typedef ProxyNode<ProxiedNode> Node;

      static const bool proxiedNodeIsConst = std::is_const<std::remove_reference_t<ProxiedNode>>::value;

      template<bool enabled = !proxiedNodeIsConst>
      std::enable_if_t<enabled, Node&> node ()
      {
        return static_cast<Node&>(*this);
      }

      const Node& node () const
      {
        return static_cast<const Node&>(*this);
      }

    public:

      using ChildType = typename ProxiedNode::ChildType;

      //! @name Child Access (Dynamic methods)
      //! @{

      //! Returns the i-th child.
      /**
       * \returns a reference to the i-th child.
       */
      template<bool enabled = !proxiedNodeIsConst>
      std::enable_if_t<enabled, ChildType&>
      child (std::size_t i)
      {
        return node().proxiedNode().child(i);
      }

      //! Returns the i-th child (const version).
      /**
       * \returns a const reference to the i-th child.
       */
      const ChildType& child (std::size_t i) const
      {
        return node().proxiedNode().child(i);
      }

      //! Sets the i-th child to the passed-in value.
      template<class C, bool enabled = !proxiedNodeIsConst>
      std::enable_if_t<enabled>
      setChild (std::size_t i, C&& child)
      {
        node().proxiedNode().setChild(i, std::forward<C>(child));
      }

    };

    //! Tag-based dispatch to appropriate base class that provides necessary functionality.
    template<class Node, class NodeTag>
    struct ProxyNodeBase;

    //! ProxyNode base class for LeafNode.
    template<class Node>
    struct ProxyNodeBase<Node,LeafNodeTag>
    {};

    //! ProxyNode base class for CompositeNode.
    template<class Node>
    struct ProxyNodeBase<Node,CompositeNodeTag>
      : public StaticChildAccessors<Node>
    {
      typedef typename Node::NodeStorage NodeStorage;
    };

    //! ProxyNode base class for PowerNode.
    template<class Node>
    struct ProxyNodeBase<Node,PowerNodeTag>
      : public DynamicChildAccessors<Node>
    {
      typedef typename Node::ChildType ChildType;
      typedef typename Node::NodeStorage NodeStorage;
    };


    //! Base class for nodes acting as a proxy for an existing node.
    /**
     * ProxyNode is a utility class for implementing proxy classes
     * that need to provide the TypeTree node functionality of the
     * proxied class. It exactly mirrors the TypeTree node characteristics
     * of the proxied node.
     */
    template<class Node>
    class ProxyNode
      : public NodeBase<ProxyNode<Node>>
      , public ProxyNodeBase<Node,NodeTag<Node>>
    {
      static const bool proxiedNodeIsConst = std::is_const<std::remove_reference_t<Node>>::value;

      template <class N>
      using HasStaticDegree = index_constant<N::degree()>;

      template <class N>
      static constexpr bool hasStaticDegree = Std::is_detected<HasStaticDegree, N>::value;

      // accessor mixins need to be friends for access to proxiedNode()
      friend class StaticChildAccessors<Node>;
      friend class DynamicChildAccessors<Node>;

    public:

      typedef Node ProxiedNode;

      typedef TypeTree::NodeTag<Node> NodeTag;

      template <class N = Node,
        std::enable_if_t<hasStaticDegree<N>, int> = 0>
      static constexpr auto degree ()
      {
        return N::degree();
      }

      template <class N = Node,
        std::enable_if_t<not hasStaticDegree<N>, int> = 0>
      auto degree () const
      {
        return proxiedNode().degree();
      }


    protected:

      //! @name Access to the proxied node
      //! @{

      //! Returns the proxied node.
      template<bool enabled = !proxiedNodeIsConst>
      std::enable_if_t<enabled, Node&>
      proxiedNode ()
      {
        return *node_;
      }

      //! Returns the proxied node (const version).
      const Node& proxiedNode () const
      {
        return *node_;
      }

      //! Returns the storage of the proxied node.
      template<bool enabled = !proxiedNodeIsConst>
      std::enable_if_t<enabled, std::shared_ptr<Node> >
      proxiedNodeStorage ()
      {
        return node_;
      }

      //! Returns the storage of the proxied node (const version).
      std::shared_ptr<const Node>
      proxiedNodeStorage () const
      {
        return node_;
      }

      //! @}

      //! @name Constructors
      //! @{

      ProxyNode (Node& node)
        : node_(stackobject_to_shared_ptr(node))
      {}

      ProxyNode (std::shared_ptr<Node> node)
        : node_(std::move(node))
      {}

      //! @}

    private:

      std::shared_ptr<Node> node_;
    };

    //! \} group Nodes

  } // namespace TypeTree
} //namespace Dune

#endif // DUNE_COMMON_TYPETREE_PROXYNODE_HH
