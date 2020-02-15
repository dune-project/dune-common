#include "config.h"

#include "typetreetestswitch.hh"

#if TEST_TYPETREE_INVALID

int main()
{
  return 0;
}

#else

#include "typetreetestutility.hh"


// struct NodeCountingFunctor
// {

//   typedef std::size_t result_type;

//   template<typename Node, typename TreePath>
//   struct doVisit
//   {
//     static const bool value = true;
//   };

//   template<typename Node, typename TreePath>
//   struct visit
//   {
//     static const result_type result = 1;
//   };

// };

// struct LeafCountingFunctor
// {

//   typedef std::size_t result_type;

//   template<typename Node, typename TreePath>
//   struct doVisit
//   {
//     static const bool value = Node::isLeaf;
//   };

//   template<typename Node, typename TreePath>
//   struct visit
//   {
//     static const result_type result = 1;
//   };

// };


// struct DepthFunctor
// {

//   typedef std::size_t result_type;

//   template<typename Node, typename TreePath>
//   struct doVisit
//   {
//     static const bool value = Node::isLeaf;
//   };

//   template<typename Node, typename TreePath>
//   struct visit
//   {
//     // the TreePath is always one entry shorter than the actual depth of the tree
//     static const result_type result = Dune::TypeTree::TreePathSize<TreePath>::value + 1;
//   };

// };


int main(int argc, char** argv)
{

  // basic tests

  // leaf node
  TreePrinter treePrinter;
  SimpleLeaf sl1;

  Dune::TypeTree::applyToTree(sl1,treePrinter);

  typedef SimplePower<SimpleLeaf,3> SP1;
  SP1 sp1_1;
  sp1_1.setChild(0,sl1);
  sp1_1.setChild(1,sl1);
  sp1_1.setChild(2,sl1);

  Dune::TypeTree::applyToTree(sp1_1,TreePrinter());

  SimpleLeaf sl2;
  SP1 sp1_2(sl2,false);

  Dune::TypeTree::applyToTree(sp1_2,TreePrinter());

  SP1 sp1_2a(sl2,true);

  Dune::TypeTree::applyToTree(sp1_2a,TreePrinter());


  typedef SimpleComposite<SimpleLeaf,SP1,SimpleLeaf> SC1;
  SC1 sc1_1(sl1,sp1_2,sl2);
  Dune::TypeTree::applyToTree(const_cast<const SC1&>(sc1_1),treePrinter);

  typedef SimpleComposite<SimpleLeaf,SimpleLeaf,SimpleLeaf> SC2;
  SC2 sc2(sl1,sl1,sl1);

  typedef SimpleComposite<SimpleLeaf,SP1,SimpleLeaf,SC1> SVC1;
  SVC1 svc1_1(sl1,sp1_1,sl2,sc1_1);
  Dune::TypeTree::applyToTree(svc1_1,treePrinter);

  SP1 sp1_3(SimpleLeaf(),SimpleLeaf(),sl1);
  Dune::TypeTree::applyToTree(sp1_3,TreePrinter());

  SVC1 svc1_2(SimpleLeaf(),SP1(sp1_2),sl2,const_cast<const SC1&>(sc1_1));
  Dune::TypeTree::applyToTree(svc1_2,TreePrinter());

  typedef SimpleComposite<SimpleLeaf,SC2,SimpleLeaf,SC1> SVC2;
  SVC2 svc2_1(sl1,sc2,sl2,sc1_1);

  Dune::TypeTree::applyToTreePair(svc1_2,svc2_1,PairPrinter());

  typedef SimplePower<SC2,2> SPC2;
  SPC2 spc2(sc2, true);

  typedef Dune::TypeTree::TreeInfo<SVC2> TI;

  // test TreeInfo
  static_assert(TI::depth == 4 && TI::nodeCount == 14 && TI::leafCount == 10,
                "TreeInfo yields wrong information");

  std::cout << "depth: " << TI::depth << std::endl
            << "nodes: " << TI::nodeCount << std::endl
            << "leafs: " << TI::leafCount << std::endl;

  // static_assert((Dune::TypeTree::AccumulateValue<
  //                SVC2,
  //                NodeCountingFunctor,
  //                Dune::TypeTree::plus<std::size_t>,
  //                0>::result == TI::nodeCount),
  //               "Error in AccumulateValue");

  // static_assert((Dune::TypeTree::AccumulateValue<
  //                SVC2,
  //                LeafCountingFunctor,
  //                Dune::TypeTree::plus<std::size_t>,
  //                0>::result == TI::leafCount),
  //               "Error in AccumulateValue");

  // static_assert((Dune::TypeTree::AccumulateValue<
  //                SVC2,
  //                DepthFunctor,
  //                Dune::TypeTree::max<std::size_t>,
  //                0>::result == TI::depth),
  //               "Error in AccumulateValue");

  // Test valid and invalid child access. Invalid access should be caught at compile time
  auto const _0 = Dune::index_constant<0>();
  auto const _1 = Dune::index_constant<1>();
  auto const _2 = Dune::index_constant<2>();

  // 1: valid access
  auto x1 = child(sp1_1, _0);
#ifdef FAILURE2
  // 2: invalid access (too few children)
  {
    auto const _3 = Dune::index_constant<3>();
    auto x2 = child(sp1_1, _3);
  }
#endif
#ifdef FAILURE3
  // 3: invalid access (child has no children)
  auto x3 = child(sp1_1, _0, _0);
#endif

  // 4: valid access
  auto x4 = child(sc1_1, _1, 2);
#ifdef FAILURE5
  // 5: invalid access (too few children)
  {
    auto const _3 = Dune::index_constant<3>();
    auto x5 = child(sc1_1, _3);
  }
#endif
#ifdef FAILURE6
  // 6: invalid access (child has no children)
  auto x6 = child(sc1_1, _0, _0);
#endif

  // 7: valid access
  auto x7 = child(spc2, 0, _1);

  return 0;
}

#endif
