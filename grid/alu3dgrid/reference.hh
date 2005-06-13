// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALUREFERENCE_HH
#define DUNE_ALUREFERENCE_HH

#include <dune/grid/alu3dgrid.hh>

namespace Dune {

  template <ALU3dGridElementType type>
  struct EntityCount {};

  template <>
  struct EntityCount<tetra> {
    enum {numFaces = 4};
    enum {numVertices = 4};
    enum {numVerticesPerFace = 3};
  };

  template <>
  struct EntityCount<hexa> {
    enum {numFaces = 6};
    enum {numVertices = 8};
    enum {numVerticesPerFace = 4};
  };

  template <ALU3dGridElementType type>
  class ElementTopologyMapping
  {
  public:
    static int dune2aluFace(int index);
    static int alu2duneFace(int index);

    static int dune2aluVertex(int index);
    static int alu2duneVertex(int index);

  private:
    const static int dune2aluFace_[EntityCount<type>::numFaces];
    const static int alu2duneFace_[EntityCount<type>::numFaces];

    const static int dune2aluVertex_[EntityCount<type>::numVertices];
    const static int alu2duneVertex_[EntityCount<type>::numVertices];
  };

  template <ALU3dGridElementType type>
  class FaceTopologyMapping {
  public:
    static int dune2aluVertex(int index);
    static int dune2aluVertex(int index, int twist);
    static int alu2duneVertex(int index);
    static int alu2duneVertex(int index, int twist);

  private:
    static int twist(int faceTwist, int index);
    static int invTwist(int faceTwist, int index);

    const static int dune2aluVertex_[EntityCount<type>::numVerticesPerFace];
    const static int alu2duneVertex_[EntityCount<type>::numVerticesPerFace];
  };

  //- IMPLEMENTATION
  //- class ElementTopologyMapping
  template <ALU3dGridElementType type>
  int ElementTopologyMapping<type>::dune2aluFace(int index) {
    assert(index < EntityCount<type>::numFaces);
    return dune2aluFace_[index];
  }

  template <ALU3dGridElementType type>
  int ElementTopologyMapping<type>::alu2duneFace(int index) {
    assert(index < EntityCount<type>::numFaces);
    return alu2duneFace_[index];
  }

  template <ALU3dGridElementType type>
  int ElementTopologyMapping<type>::dune2aluVertex(int index) {
    assert(index < EntityCount<type>::numVertices);
    return dune2aluVertex_[index];
  }

  template <ALU3dGridElementType type>
  int ElementTopologyMapping<type>::alu2duneVertex(int index) {
    assert(index < EntityCount<type>::numVertices);
    return alu2duneVertex_[index];
  }

  //- class FaceTopologyMapping
  template <ALU3dGridElementType type>
  int FaceTopologyMapping<type>::dune2aluVertex(int index) {
    assert(index < EntityCount<type>::numVertices);
    return dune2aluVertex_[index];
  }

  template <ALU3dGridElementType type>
  int FaceTopologyMapping<type>::dune2aluVertex(int index, int twist) {
    assert(index < EntityCount<type>::numVertices);
    return invTwist(dune2aluVertex_[index], twist);
  }

  template <ALU3dGridElementType type>
  int FaceTopologyMapping<type>::alu2duneVertex(int index) {
    assert(index < EntityCount<type>::numVertices);
    return dune2aluVertex_[index];
  }

  template <ALU3dGridElementType type>
  int FaceTopologyMapping<type>::alu2duneVertex(int index, int twist) {
    assert(index < EntityCount<type>::numVertices);
    return dune2aluVertex_[invTwist(index, twist)];
  }

} // end namespace Dune
#endif
