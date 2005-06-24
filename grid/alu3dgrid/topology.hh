// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGRIDTOPOLOGY_HH
#define DUNE_ALU3DGRIDTOPOLOGY_HH

#include "alu3dinclude.hh"

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
    enum { numFaces = EntityCount<type>::numFaces };
    enum { numVertices = EntityCount<type>::numVertices };
    enum { numVerticesPerFace = EntityCount<type>::numVerticesPerFace };

  public:
    static int dune2aluFace(int index);
    static int alu2duneFace(int index);

    static int dune2aluVertex(int index);
    static int alu2duneVertex(int index);

    static int dune2aluFaceVertex(int face, int localVertex);
    static int alu2duneFaceVertex(int face, int localVertex);

  private:
    const static int dune2aluFace_[numFaces];
    const static int alu2duneFace_[numFaces];

    const static int dune2aluVertex_[numVertices];
    const static int alu2duneVertex_[numVertices];

    const static int dune2aluFaceVertex_[numFaces][numVerticesPerFace];
    const static int alu2duneFaceVertex_[numFaces][numVerticesPerFace];
  };

  template <ALU3dGridElementType type>
  class FaceTopologyMapping {
  public:
    static int dune2aluVertex(int index);
    static int dune2aluVertex(int index, int twist);
    static int alu2duneVertex(int index);
    static int alu2duneVertex(int index, int twist);

    //  private:
    static int twist(int index, int faceTwist);
    static int invTwist(int index, int faceTwist);
  private:
    const static int dune2aluVertex_[EntityCount<type>::numVerticesPerFace];
    const static int alu2duneVertex_[EntityCount<type>::numVerticesPerFace];
  };

  //- IMPLEMENTATION
  //- class ElementTopologyMapping
  template <ALU3dGridElementType type>
  int ElementTopologyMapping<type>::dune2aluFace(int index) {
    assert(index >= 0 && index < numFaces);
    return dune2aluFace_[index];
  }

  template <ALU3dGridElementType type>
  int ElementTopologyMapping<type>::alu2duneFace(int index) {
    assert(index >= 0 && index < numFaces);
    return alu2duneFace_[index];
  }

  template <ALU3dGridElementType type>
  int ElementTopologyMapping<type>::dune2aluVertex(int index) {
    assert(index >= 0 && index < numVertices);
    return dune2aluVertex_[index];
  }

  template <ALU3dGridElementType type>
  int ElementTopologyMapping<type>::alu2duneVertex(int index) {
    assert(index >= 0 && index < numVertices);
    return alu2duneVertex_[index];
  }

  template <ALU3dGridElementType type>
  int ElementTopologyMapping<type>::
  dune2aluFaceVertex(int face, int localVertex) {
    assert(face >= 0 && face < numFaces);
    assert(localVertex >= 0 && localVertex < numVerticesPerFace);
    return dune2aluFaceVertex_[face][localVertex];
  }

  template <ALU3dGridElementType type>
  int ElementTopologyMapping<type>::
  alu2duneFaceVertex(int face, int localVertex) {
    assert(face >= 0 && face < numFaces);
    assert(localVertex >= 0 && localVertex < numVerticesPerFace);
    return alu2duneFaceVertex_[face][localVertex];
  }

  //- class FaceTopologyMapping
  template <ALU3dGridElementType type>
  int FaceTopologyMapping<type>::dune2aluVertex(int index) {
    assert(index >= 0 && index < EntityCount<type>::numVerticesPerFace);
    return dune2aluVertex_[index];
  }

  template <ALU3dGridElementType type>
  int FaceTopologyMapping<type>::dune2aluVertex(int index, int twist) {
    assert(index >= 0 && index < EntityCount<type>::numVerticesPerFace);
    return invTwist(dune2aluVertex_[index], twist);
  }

  template <ALU3dGridElementType type>
  int FaceTopologyMapping<type>::alu2duneVertex(int index) {
    assert(index >= 0 && index < EntityCount<type>::numVerticesPerFace);
    return alu2duneVertex_[index];
  }

  template <ALU3dGridElementType type>
  int FaceTopologyMapping<type>::alu2duneVertex(int index, int twist) {
    assert(index >= 0 && index < EntityCount<type>::numVerticesPerFace);
    return alu2duneVertex_[invTwist(index, twist)];
  }

} // end namespace Dune
#endif
