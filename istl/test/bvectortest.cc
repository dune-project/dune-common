// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <dune/istl/bvector.hh>
#include <dune/common/fvector.hh>

template<typename T, int BS>
void assign(Dune::FieldVector<T,BS>& b, const T& i)
{

  for(int j=0; j < BS; j++)
    b[j] = i;
}

template<int BS>
int testVector()
{

  typedef Dune::FieldVector<int,BS> VectorBlock;
  typedef Dune::BlockVector<VectorBlock> Vector;
  typedef typename Vector::size_type size_type;

  // empty vector
  Vector v, w, v1(20), v2(20,100);
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

  for(int i=0; i < v.N(); ++i)
    v2[i] = i*10;
  w = v;


  assert(w.N()==v.N());
  assert(w.capacity()==v.capacity());

  for(int i=0; i < v.N(); ++i)
    assert(v[i] == w[i]);

  w = static_cast<const Dune::block_vector_unmanaged<VectorBlock>&>(v);

  for(int i=0; i < w.N(); ++i)
    assert(v[i] == w[i]);

  Vector z(w);

  assert(w.N()==z.N());
  assert(w.capacity()==z.capacity());

  for(int i=0; i < w.N(); ++i)
    assert(z[i] == w[i]);

  Vector z1(static_cast<const Dune::block_vector_unmanaged<VectorBlock>&>(v2));

  assert(v2.N()==z1.N());
  assert(v2.capacity()==z1.capacity());

  for(int i=1; i < v2.N(); ++i) {
    assert(z1[i] == v2[i]);
  }

  v.reserve(150);
  assert(150==v.capacity());
  assert(25==v.N());

  VectorBlock b;

  // check the entries
  for(int i=0; i < v.N(); ++i) {
    assign(b, i);
    assert(v[i] == b);
  }

  // Try to shrink the vector
  v.reserve(v.N());

  assert(v.N()==v.capacity());

  // check the entries

  for(int i=0; i < v.N(); ++i) {
    assign(b,i);
    assert(v[i] == b);
  }

  return 0;
}


int main()
{
  int ret = testVector<1>();
  return ret + testVector<3>();
}
