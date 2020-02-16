// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_TYPETREE_PROXYNODE_HH
#define DUNE_COMMON_TYPETREE_PROXYNODE_HH

#include <type_traits>

#include <dune/common/indices.hh>
#include <dune/common/shared_ptr.hh>
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
      std::enable_if_t<enabled,Node&> node ()
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
      struct Child
        : public ProxiedNode::template Child<k>
      {};

      //! @name Child Access
      //! @{

      //! Returns the i-th child.
      /**
       * \returns a reference to the i-th child.
       */
      template<std::size_t k, bool enabled = !proxiedNodeIsConst>
      std::enable_if_t<enabled,typename Child<k>::Type&>
      child (index_constant<k> = {})
      {
        return node().proxiedNode().template child<k>();
      }

      //! Returns the i-th child (const version).
      /**
       * \returns a const reference to the i-th child.
       */
      template<std::size_t k>
      const typename Child<k>::Type& child (index_constant<k> = {}) const
      {
        return node().proxiedNode().template child<k>();
      }

      //! Returns the storage of the i-th child.
      /**
       * \returns a copy of the object storing the i-th child.
       */
      template<std::size_t k, bool enabled = !proxiedNodeIsConst>
      std::enable_if_t<enabled, typename Child<k>::Storage>
      childStorage ()
      {
        return node().proxiedNode().template childStorage<k>();
      }

      //! Returns the storage of the i-th child (const version).
      /**
       * This method is only important if the child is stored as
       * some kind of pointer, as this allows the pointee type to
       * become const.
       * \returns a copy of the object storing the i-th child.
       */
      template<std::size_t k>
      typename Child<k>::ConstStorage childStorage () const
      {
        return node().proxiedNode().template childStorage<k>();
      }

      //! Sets the i-th child to the passed-in value.
      template<std::size_t k, bool enabled = !proxiedNodeIsConst>
      void setChild (typename Child<k>::type& child, std::enable_if_t<enabled,void*> = nullptr)
      {
        node().proxiedNode().template childStorage<k>() = stackobject_to_shared_ptr(child);
      }

      //! Sets the storage of the i-th child to the passed-in value.
      template<std::size_t k, bool enabled = !proxiedNodeIsConst>
      void setChild (typename Child<k>::storage_type child, std::enable_if_t<enabled,void*> = nullptr)
      {
        node().proxiedNode().template childStorage<k>() = child;
      }

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

      //! @name Child Access (Dynamic methods)
      //! @{

      //! Returns the i-th child.
      /**
       * \returns a reference to the i-th child.
       */
      template<bool enabled = !proxiedNodeIsConst>
      std::enable_if_t<enabled,typename ProxiedNode::ChildType&>
      child (std::size_t i)
      {
        return node().proxiedNode().child(i);
      }

      //! Returns the i-th child (const version).
      /**
       * \returns a const reference to the i-th child.
       */
      const typename ProxiedNode::ChildType& child (std::size_t i) const
      {
        return node().proxiedNode().child(i);
      }

      //! Returns the storage of the i-th child.
      /**
       * \returns a copy of the object storing the i-th child.
       */
      template<bool enabled = !proxiedNodeIsConst>
      std::enable_if_t<enabled, typename ProxiedNode::ChildStorageType>
      childStorage (std::size_t i)
      {
        return node().proxiedNode().childStorage(i);
      }

      //! Returns the storage of the i-th child (const version).
      /**
       * This method is only important if the child is stored as
       * some kind of pointer, as this allows the pointee type to
       * become const.
       * \returns a copy of the object storing the i-th child.
       */
      typename ProxiedNode::ChildConstStorageType childStorage (std::size_t i) const
      {
        return node().proxiedNode().childStorage(i);
      }

      //! Sets the i-th child to the passed-in value.
      template<bool enabled = !proxiedNodeIsConst>
      void setChild (std::size_t i, typename ProxiedNode::ChildType& t, std::enable_if_t<enabled,void*> = nullptr)
      {
        node().proxiedNode().childStorage(i) = stackobject_to_shared_ptr(t);
      }

      //! Sets the stored value representing the i-th child to the passed-in value.
      template<bool enabled = !proxiedNodeIsConst>
      void setChild (std::size_t i, typename ProxiedNode::ChildStorageType st, std::enable_if_t<enabled,void*> = nullptr)
      {
        node().proxiedNode().childStorage(i) = st;
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
      typedef typename Node::ChildTypes ChildTypes;
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
      : public ProxyNodeBase<Node,NodeTag<Node>>
    {

      static const bool proxiedNodeIsConst = std::is_const<std::remove_reference_t<Node>>::value;

      // accessor mixins need to be friends for access to proxiedNode()
      friend class StaticChildAccessors<Node>;
      friend class DynamicChildAccessors<Node>;

    public:

      typedef Node ProxiedNode;

      typedef TypeTree::NodeTag<Node> NodeTag;

      //! Mark this class as non leaf in the \ref TypeTree.
      static const bool isLeaf = Node::isLeaf;

      //! Mark this class as a non power in the \ref TypeTree.
      static const bool isPower = Node::isPower;

      //! Mark this class as a composite in the \ref TypeTree.
      static const bool isComposite = Node::isComposite;

      //! The number of children.
      static const std::size_t CHILDREN = StaticDegree<Node>::value;

      static constexpr std::size_t degree ()
      {
        return StaticDegree<Node>::value;
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
      std::shared_ptr<const Node> proxiedNodeStorage () const
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
