// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <sstream>
#include <dune/common/parametertree.hh>
#include <dune/common/parametertreeparser.hh>

template<class P>
void testparam(const P & p)
{
  // try accessing key
  std::cout << p.template get<int>("x1") << std::endl;
  std::cout << p.template get<double>("x1") << std::endl;
  std::cout << p.template get<std::string>("x2") << std::endl;
  std::cout << p.template get<bool>("x3") << std::endl;
  // try reading array like structures
  std::vector<unsigned int>
  array1 = p.template get< std::vector<unsigned int> >("array");
#ifndef NDEBUG
  Dune::array<unsigned int, 8>
  array2 = p.template get< Dune::array<unsigned int, 8> >("array");
  Dune::FieldVector<unsigned int, 8>
  array3 = p.template get< Dune::FieldVector<unsigned int, 8> >("array");
#endif
  assert(array1.size() == 8);
  std::cout << "array =";
  for (unsigned int i=0; i<8; i++)
  {
    assert(array1[i] == i+1);
    assert(array2[i] == i+1);
    assert(array3[i] == i+1);
    std::cout << "\t" << array1[i];
  }
  std::cout << std::endl;
  // try accessing subtree
  p.sub("Foo");
  p.sub("Foo").template get<std::string>("peng");
  // check hasSub and hasKey
  assert(p.hasSub("Foo"));
  assert(!p.hasSub("x1"));
  assert(p.hasKey("x1"));
  assert(!p.hasKey("Foo"));
  // try accessing inexistent key
  try {
    p.template get<int>("bar");
    DUNE_THROW(Dune::Exception, "failed to detect missing key");
  }
  catch (Dune::RangeError & r) {}
  // try accessing inexistent subtree
  try {
    p.sub("bar");
    DUNE_THROW(Dune::Exception, "failed to detect missing subtree");
  }
  catch (Dune::RangeError & r) {}
  // try accessing key as subtree
  try {
    p.sub("x1");
    DUNE_THROW(Dune::Exception, "succeeded to access key as subtree");
  }
  catch (Dune::RangeError & r) {}
  // try accessing subtree as key
  try {
    p.template get<double>("Foo");
    DUNE_THROW(Dune::Exception, "succeeded to access subtree as key");
  }
  catch (Dune::RangeError & r) {}
}

template<class P>
void testmodify(P parameterSet)
{
  parameterSet["testDouble"] = "3.14";
  parameterSet["testInt"]    = "42";
  parameterSet["testString"] = "Hallo Welt!";
  parameterSet["testVector"] = "2 3 5 7 11";
  parameterSet.sub("Foo")["bar"] = "2";

  double testDouble      = parameterSet.template get<double>("testDouble");
  int testInt            = parameterSet.template get<int>("testInt");
  ++testDouble;
  ++testInt;
  std::string testString = parameterSet.template get<std::string>("testString");
  typedef Dune::FieldVector<unsigned, 5> FVector;
  FVector testFVector    = parameterSet.template get<FVector>("testVector");
  typedef std::vector<unsigned> SVector;
  SVector testSVector    = parameterSet.template get<SVector>("testVector");
  if(testSVector.size() != 5)
    DUNE_THROW(Dune::Exception, "Testing std::vector<unsigned>: expected "
               "size()==5, got size()==" << testSVector.size());
  for(unsigned i = 0; i < 5; ++i)
    if(testFVector[i] != testSVector[i])
      DUNE_THROW(Dune::Exception,
                 "testFVector[" << i << "]==" << testFVector[i] << " but "
                 "testSVector[" << i << "]==" << testSVector[i]);
  if (parameterSet.template get<std::string>("Foo.bar") != "2")
    DUNE_THROW(Dune::Exception, "Failed to write subtree entry");
  if (parameterSet.sub("Foo").template get<std::string>("bar") != "2")
    DUNE_THROW(Dune::Exception, "Failed to write subtree entry");
}

int main()
{
  try {
    // read config
    std::stringstream s;
    s << "x1 = 1 # comment\n"
      << "x2 = hallo\n"
      << "x3 = no\n"
      << "array = 1   2 3 4 5\t6 7 8\n"
      << "\n"
      << "[Foo]\n"
      << "peng = ligapokal\n";

    Dune::ParameterTree c;
    Dune::ParameterTreeParser::readINITree(s, c);

    // test modifying and reading
    testmodify<Dune::ParameterTree>(c);
    try {
      c.get<int>("testInt");
      DUNE_THROW(Dune::Exception, "unexpected shallow copy of ParameterTree");
    }
    catch (Dune::RangeError & r) {}

    // more const tests
    testparam<Dune::ParameterTree>(c);
  }
  catch (Dune::Exception & e)
  {
    std::cout << e << std::endl;
    return 1;
  }
  return (0);
}
