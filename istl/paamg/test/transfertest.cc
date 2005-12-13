// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "config.h"
#include <dune/istl/bvector.hh>
#include <dune/common/fvector.hh>
#include <dune/istl/paamg/aggregates.hh>
#include <dune/istl/paamg/pinfo.hh>
#include <dune/istl/paamg/transfer.hh>

template<class Vertex>
int createAggregates(Dune::Amg::AggregatesMap<Vertex>& aggregates, int size)
{
  int index=0;
  int i=0;

  for(i=1; index<size; ++i)
    for(int j=0; j < i && index < size; ++j, ++index)
      aggregates[index]=i-1;

  return i-1;
}


int main()
{
  const int BS=1;
  typedef Dune::FieldVector<int,BS> VectorBlock;
  typedef Dune::BlockVector<VectorBlock> Vector;

  Vector b(20);
  Dune::Amg::AggregatesMap<int> amap(20);
  int aggregates = createAggregates(amap, 20);

  Vector c(aggregates);
  b=100;
  typedef Dune::Amg::SequentialInformation SequentialInformation;

  SequentialInformation info;

  Dune::Amg::Transfer<int,Vector,SequentialInformation>::restrict (amap, c, b, info);

  Dune::Amg::Transfer<int,Vector,SequentialInformation>::prolongate(amap, c, b, 1);
  return 0;
}
