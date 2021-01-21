struct TargetLeaf
  : public Dune::TypeTree::LeafNode
{

  template<typename Transformation>
  TargetLeaf(const SimpleLeaf& sl, const Transformation& t)
    : s(Dune::stackobject_to_shared_ptr(sl))
  {}

  template<typename Transformation>
  TargetLeaf(std::shared_ptr<const SimpleLeaf> sl, const Transformation& t)
    : s(sl)
  {}

  std::shared_ptr<const SimpleLeaf> s;

  const char* name() const
  {
    return "TargetLeaf";
  }

  int id() const
  {
    return s->id();
  }

};

template<typename S, typename T, std::size_t k>
struct TargetPower
  : public Dune::TypeTree::PowerNode<T,k>
{

  template<typename Transformation>
  TargetPower(const S& sc, const Transformation& t, const std::array<std::shared_ptr<T>,k>& children)
    : Dune::TypeTree::PowerNode<T,k>(children)
    , s(Dune::stackobject_to_shared_ptr(sc))
  {}

  template<typename Transformation>
  TargetPower(std::shared_ptr<const S> sc, const Transformation& t, const std::array<std::shared_ptr<T>,k>& children)
    : Dune::TypeTree::PowerNode<T,k>(children)
    , s(sc)
  {}

  std::shared_ptr<const S> s;

  const char* name() const
  {
    return "TargetPower";
  }

  int id() const
  {
    return s->id();
  }


};

template<typename S, typename... Children>
struct TargetComposite
  : public Dune::TypeTree::CompositeNode<Children...>
{

  template<typename Transformation>
  TargetComposite(const S& sc, const Transformation& t, std::shared_ptr<Children>... children)
    : Dune::TypeTree::CompositeNode<Children...>(children...)
    , s(Dune::stackobject_to_shared_ptr(sc))
  {}

  template<typename Transformation>
  TargetComposite(std::shared_ptr<const S> sc, const Transformation& t, std::shared_ptr<Children>... children)
    : Dune::TypeTree::CompositeNode<Children...>(children...)
    , s(sc)
  {}

  std::shared_ptr<const S> s;

  const char* name() const
  {
    return "TargetComposite";
  }

  int id() const
  {
    return s->id();
  }


};


struct TestTransformation {};

// register leaf node
template<typename SL>
Dune::TypeTree::GenericLeafNodeTransformation<SimpleLeaf,TestTransformation,TargetLeaf>
registerNodeTransformation(SL* sl, TestTransformation* t, SimpleLeafTag* tag);

template<typename SP>
Dune::TypeTree::GenericPowerNodeTransformation<SP,TestTransformation,TargetPower>
registerNodeTransformation(SP* sp, TestTransformation* t, SimplePowerTag* tag);

template<typename SC>
Dune::TypeTree::GenericCompositeNodeTransformation<SC,TestTransformation,TargetComposite>
registerNodeTransformation(SC* sc, TestTransformation* t, SimpleCompositeTag* tag);
