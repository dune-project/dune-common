
#include <config.h>

#include <iostream>
#include <vector>

#include <dune/common/test/testsuite.hh>

#include <dune/common/indices.hh>
#include <dune/common/indexedforeach.hh>

#include <dune/common/fvector.hh>
#include <dune/common/dynvector.hh>
#include <dune/common/tuplevector.hh>

using namespace Dune;

TestSuite testIndexedFlatForEach()
{
  TestSuite t;

  // mix up some types

  FieldVector<double,3> f3;
  FieldVector<double,1> f1;

  DynamicVector<FieldVector<double,3>> d3;

  std::vector<FieldVector<double,1>> v1;

  d3.resize(5);
  v1.resize(5);

  using TBV = TupleVector<DynamicVector<FieldVector<double,3>>,std::vector<FieldVector<double,1>>>;

  TBV v;

  v[Indices::_0] = d3;
  v[Indices::_1] = v1;

  int entries = 0;

  auto countEntres = [&](auto&& index, auto&& entry){
    std::cout << "index = " << index << " entry = " << entry << std::endl;
    entries++;
  };

  FlatVectorView flatView(v);

  auto size = flatView.size();

  indexedForEach(flatView,countEntres);

  t.check( entries == 20 );
  t.check( size == 20 );

  return t;
}



int main(int argc, char** argv)
{
  TestSuite t;

  t.subTest(testIndexedFlatForEach());

  return t.exit();
}
