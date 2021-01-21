#include <dune/common/typetree/compositenode.hh>
#include <dune/common/typetree/leafnode.hh>
#include <dune/common/typetree/powernode.hh>
#include <dune/common/typetree/pairtraversal.hh>
#include <dune/common/typetree/traversal.hh>
#include <dune/common/typetree/treeinfo.hh>

#include <iostream>

struct Counter
{
  Counter()
    : _id(_ids++)
  {
    std::cout << "Constructed id = " << id() << std::endl;
  }

  Counter(const Counter& rhs)
    : _id(_ids++)
  {
    rhs.assert_valid();
    std::cout << "Copy-Constructed id = " << id() << " from id = " << rhs.id() << std::endl;
  }

  Counter(Counter&& rhs)
    : _id(rhs._id)
  {
    rhs.assert_valid();
    rhs._id = -1;
    std::cout << "Move-Constructed id = " << id() << std::endl;
  }

  ~Counter()
  {
    std::cout << "Destructed id = " << _id << std::endl;
  }

  Counter& operator=(const Counter& rhs)
  {
    rhs.assert_valid();
    assert_valid();
    std::cout << "Assigned id = " << id() << " from id = " << rhs.id() << std::endl;
    return *this;
  }

  Counter& operator=(Counter&& rhs)
  {
    assert_valid();
    rhs.assert_valid();
    std::cout << "Move-Assigned id = " << id() << " from id = " << rhs.id() << std::endl;
    rhs._id = -1;
    return *this;
  }

  int id() const
  {
    assert_valid();
    return _id;
  }

  void assert_valid() const
  {
    assert(_id != -1);
  }

  int _id;
  static int _ids;
};

int Counter::_ids = 0;

struct SimpleLeafTag {};

struct SimpleLeaf
  : public Dune::TypeTree::LeafNode
  , public Counter
{

  typedef SimpleLeafTag ImplementationTag;

  static const char* name()
  {
    return "SimpleLeaf";
  }

  SimpleLeaf() {}

  SimpleLeaf(SimpleLeaf&& rhs)
    : Dune::TypeTree::LeafNode(std::move(rhs))
    , Counter(std::move(rhs))
  {
    std::cout << "move ctor" << std::endl;
  }

  SimpleLeaf(const SimpleLeaf& rhs)
    : Dune::TypeTree::LeafNode(rhs)
    , Counter(rhs)
  {
    std::cout << "copy ctor" << std::endl;
  }

};

struct SimpleLeafDerived
  : public SimpleLeaf
{

  static const char* name()
  {
    return "SimpleLeafDerived";
  }

};

struct SimplePowerTag {};

template<typename T, std::size_t k>
struct SimplePower
  : public Dune::TypeTree::PowerNode<T,k>
  , public Counter
{

  typedef SimplePowerTag ImplementationTag;

  static const char* name()
  {
    return "SimplePower";
  }

  typedef Dune::TypeTree::PowerNode<T,k> BaseT;

  SimplePower() {}

  SimplePower(T& c, bool copy)
    : BaseT(c,copy)
  {}

  template<typename C1, typename C2, typename... Children>
  SimplePower(C1&& c1, C2&& c2, Children&&... children)
    : BaseT(std::forward<C1>(c1),std::forward<C2>(c2),std::forward<Children>(children)...)
  {}

};

struct SimpleCompositeTag {};

template<typename... Children>
struct SimpleComposite
  : public Dune::TypeTree::CompositeNode<Children...>
  , public Counter
{

  typedef SimpleCompositeTag ImplementationTag;

  static const char* name()
  {
    return "SimpleComposite";
  }

  typedef Dune::TypeTree::CompositeNode<Children...> BaseT;

  template<typename... Args, typename = typename std::enable_if<(sizeof...(Args) == BaseT::degree())>::type>
  SimpleComposite(Args&&... args)
    : BaseT(std::forward<Args>(args)...)
  {}

};


struct TreePrinter
  : public Dune::TypeTree::TreeVisitor
  , public Dune::TypeTree::DynamicTraversal
{

  template<typename T, typename TreePath>
  void leaf(const T& t, TreePath treePath) const
  {
    pre(t,treePath);
  }

  template<typename T, typename TreePath>
  void pre(const T& t, TreePath treePath) const
  {
    for (std::size_t i = 0; i < treePath.size(); ++i)
      std::cout << "  ";
    std::cout << t.name() << " " << t.id() << std::endl;
  }
};




struct PairPrinter
  : public Dune::TypeTree::TreePairVisitor
  , public Dune::TypeTree::DynamicTraversal
{

  template<typename T1, typename T2, typename TreePath>
  void leaf(const T1& t1, const T2& t2, TreePath treePath) const
  {
    pre(t1,t2,treePath);
  }

  template<typename T1, typename T2, typename TreePath>
  void pre(const T1& t1, const T2& t2, TreePath treePath) const
  {
    for (std::size_t i = 0; i < treePath.size(); ++i)
      std::cout << "  ";
    std::cout << t1.name() << " " << t1.id() << "      " << t2.name() << " " << t2.id() << std::endl;
  }
};
