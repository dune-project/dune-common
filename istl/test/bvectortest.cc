// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <dune/istl/bvector.hh>
#include <dune/common/fvector.hh>
int main()
{
  const int BS=1;

  typedef Dune::FieldVector<int,BS> VectorBlock;
  typedef Dune::BlockVector<VectorBlock> Vector;
  typedef Vector::size_type size_type;

  // empty vector
  Vector v, v1(20), v2(20,100);
  v.reserve(100);
  assert(100==v.capacity());
  assert(20==v1.capacity());
  assert(100==v2.capacity());
  assert(20==v1.N());
  assert(20==v2.N());

  v.resize(25);

  assert(25==v.N());

  for(int i=0; i < v.N(); ++i)
    v[i] = i;

  v.reserve(150);
  assert(150==v.capacity());
  assert(25==v.N());

  // check the entries
  for(int i=0; i < v.N(); ++i)
    assert(v[i] == i);


  // Try to shrink the vector
  v.reserve(v.N());

  assert(v.N()==v.capacity());
  // check the entries
  for(int i=0; i < v.N(); ++i)
    assert(v[i] == i);

}
