// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALBERTAGRID_REFERENCETOPO_HH
#define DUNE_ALBERTAGRID_REFERENCETOPO_HH

#ifdef __ALBERTApp__
namespace Albert {
#endif

namespace AlbertHelp {

  // NOTE: Vertex numbering in ALBERTA is the same as in Dune
  // therefore no map is provided for that

  // faces in 2d (i.e. triangle edges )
  // which vertices belong to which face
  static const int localTriangleFaceNumber [3][2] = { {1,2} , {2,0} , {0,1} };

  // edges in 3d
  // local numbers of vertices belonging to one edge
  // according to Alberta reference element which is for edges different to
  // the Dune reference simplex in 3d ,see Alberta doc page 105
  static const int localEdgeNumber [6][2] =
  {
    {0,1} , {0,2} , {0,3} , // first three vertices like in 2d for faces(edges)
    {1,2} , {1,3} , {2,3} // then all with the last vertex
  };

  // see Albert Doc page 12 for reference element
  // if we look from outside, then face numbering must be clockwise
  // see below that the vertex numbers for each face are the same
  // but in Dune reference element not clockwise ,but this we need for
  // calculation the outer normal, see calcOuterNormal in albertagrid.cc
  static const int tetraFace_0[3] = {3,2,1};
  static const int tetraFace_1[3] = {2,3,0};
  static const int tetraFace_2[3] = {0,3,1};
  static const int tetraFace_3[3] = {0,1,2};

  // local vertex numbering of faces in the Dune refrence simplex in 3d
  static const int localDuneTetraFaceNumber[4][3] =
  { {1,2,3}, // face 0
    {0,3,2}, // face 1
    {0,1,3}, // face 2
    {0,2,1} // face 3
  };

  static const int * localAlbertaFaceNumber[4] = {tetraFace_0, tetraFace_1,
                                                  tetraFace_2 , tetraFace_3 };

  static const int dune2AlbertaEdgeMap[6] = {0, 3, 1, 2, 4, 5};

  template <int dim>
  class AlbertaGridReferenceTopology
  {
    // the real edge map
    int em_[6];
    const int (&edgemap_)[6];

    const int (&localTriangleFaceNumber_)[3][2];
    int lTFN[3][2];

  public:
    //! create reference topology
    AlbertaGridReferenceTopology()
      : edgemap_ (em_) , localTriangleFaceNumber_(lTFN)
    {
      // the edgemap , see ALBERTA docu
      for(int i=0; i<6; i++) em_[i] = dune2AlbertaEdgeMap[i];
      for(int i=0; i<3; i++)
      {
        for(int j=0; j<2; j++) lTFN[i][j] = localTriangleFaceNumber[i][j];
      }
    }

    //static int mapVertices (int i, int face, int edge, int vertex)

    //! dune to alberta edge mapping
    //! this is the id
    int dune2albertaVertex( int i ) const { return i; }

    //! dune to alberta edge mapping
    int dune2albertaEdge( int i ) const
    {
      assert( i >= 0 );
      assert( i < 6 );
      return edgemap_[i];
    }
  };

  //****************************************************************
  //
  //  specialization of mapVertices
  //  see referencetopo.hh
  //
  //****************************************************************
  template <int md, int cd>
  struct MapVertices
  {
    static int mapVertices (int i, int face, int edge, int vertex)
    {
      return i;
    }
  };

  // faces in 2d
  template <>
  struct MapVertices<1,2>
  {
    static int mapVertices (int i, int face, int edge, int vertex)
    {
      assert( i >= 0);
      assert( i < 2 );
      assert( face >= 0 );
      assert( face <  3 );
      return ALBERTA AlbertHelp::localTriangleFaceNumber[face][i];
    }
  };

  // faces in 3d
  template <>
  struct MapVertices<2,3>
  {
    static int mapVertices (int i, int face, int edge, int vertex)
    {
      // we have 3 vertices each of the 4 faces
      assert( i >= 0);
      assert( i < 3 );
      assert( face >= 0 );
      assert( face <  4 );
      return ALBERTA AlbertHelp::localDuneTetraFaceNumber[face][i];
    }
  };

  // edges in 3d
  template <>
  struct MapVertices<1,3>
  {
    static int mapVertices (int i, int face, int edge, int vertex)
    {
      assert( i >= 0 );
      assert( i < 2  );
      assert( edge >= 0 );
      assert( edge < 6 );
      return ALBERTA AlbertHelp :: localEdgeNumber[
               ALBERTA AlbertHelp :: dune2AlbertaEdgeMap [edge] ][i];
    }
  };

  // vertices in 2d and 3d
  template <int cd>
  struct MapVertices<0,cd>
  {
    static int mapVertices (int i, int face, int edge, int vertex)
    {
      // just return number of vertex
      return vertex;
    }
  };


} // end namespace AlbertHelp

#ifdef __ALBERTApp__
} // end namespace Albert
#endif

#endif
