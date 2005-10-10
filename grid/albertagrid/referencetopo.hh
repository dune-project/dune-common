// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALBERTAGRID_REFERENCETOPO_HH
#define DUNE_ALBERTAGRID_REFERENCETOPO_HH

template <int dim>
class AlbertaGridReferenceTopology
{
  const int (&edgemap_)[6];
  int em_[6];
public:
  //! create reference topology
  AlbertaGridReferenceTopology() : edgemap_ (em_)
  {
    // the edgemap
    const int edgemap[6] = {0, 3 , 1 , 2 , 4, 5};
    for(int i=0; i<6; i++) em_[i] = edgemap[i];
  }

  //! dune to alberta edge mapping
  //! this is the id
  int dune2albertaVertex( int i ) const
  {
    return i;
  }

  //! dune to alberta edge mapping
  int dune2albertaEdge( int i ) const
  {
    assert( i >= 0 );
    assert( i < 6 );
    return edgemap_[i];
  }
};


#endif
