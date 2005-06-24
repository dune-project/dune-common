// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "topology.hh"

namespace Dune {

  //- class ElementTopologyMapping
  template <>
  const int ElementTopologyMapping<tetra>::
  dune2aluFace_[EntityCount<tetra>::numFaces] = {0, 1, 2, 3};

  template <>
  const int ElementTopologyMapping<hexa>::
  dune2aluFace_[EntityCount<hexa>::numFaces] = {4, 2, 5, 3, 0, 1};

  template <>
  const int ElementTopologyMapping<tetra>::
  alu2duneFace_[EntityCount<tetra>::numFaces] = {0, 1, 2, 3};

  template <>
  const int ElementTopologyMapping<hexa>::
  alu2duneFace_[EntityCount<hexa>::numFaces] = {4, 5, 1, 3, 0, 2};

  template <>
  const int ElementTopologyMapping<tetra>::
  dune2aluVertex_[EntityCount<tetra>::numVertices] = {0, 1, 2, 3};

  template <>
  const int ElementTopologyMapping<hexa>::
  dune2aluVertex_[EntityCount<hexa>::numVertices] = {3, 0, 2, 1, 7, 4, 6, 5};

  template <>
  const int ElementTopologyMapping<tetra>::
  alu2duneVertex_[EntityCount<tetra>::numVertices] = {0, 1, 2, 3};

  template <>
  const int ElementTopologyMapping<hexa>::
  alu2duneVertex_[EntityCount<hexa>::numVertices] = {1, 3, 2, 0, 5, 7, 6, 4};

  template <>
  const int ElementTopologyMapping<tetra>::
  dune2aluFaceVertex_[numFaces][numVerticesPerFace] = {{0, 1, 2},
                                                       {0, 1, 2},
                                                       {0, 1, 2},
                                                       {0, 1, 2}};

  template <>
  const int ElementTopologyMapping<hexa>::
  dune2aluFaceVertex_[numFaces][numVerticesPerFace] = {{1, 0, 2, 3},
                                                       {0, 1, 3, 2},
                                                       {3, 0, 2, 1},
                                                       {1, 0, 2, 3},
                                                       {1, 0, 2, 3},
                                                       {3, 0, 2, 1}};

  template <>
  const int ElementTopologyMapping<tetra>::
  alu2duneFaceVertex_[numFaces][numVerticesPerFace] = {{0, 1, 2},
                                                       {0, 1, 2},
                                                       {0, 1, 2},
                                                       {0, 1, 2}};

  template <>
  const int ElementTopologyMapping<hexa>::
  alu2duneFaceVertex_[numFaces][numVerticesPerFace] = {{1, 0, 2, 3},
                                                       {1, 3, 2, 0},
                                                       {0, 1, 3, 2},
                                                       {1, 0, 2, 3},
                                                       {1, 0, 2, 3},
                                                       {1, 3, 2, 0}};

  //- class FaceTopologyMapping
  template <>
  int FaceTopologyMapping<tetra>::
  twist(int index, int faceTwist) {
    return (faceTwist < 0) ?
           (7 - index + faceTwist)%3 : (faceTwist + index)%3 ;
  }

  template <>
  int FaceTopologyMapping<hexa>::
  twist(int index, int faceTwist) {
    return (faceTwist < 0) ?
           (9 - index + faceTwist)%4 : (faceTwist + index)%4 ;
  }

  template <>
  int FaceTopologyMapping<tetra>::
  invTwist(int index, int faceTwist) {
    return (faceTwist < 0) ?
           (7 - index + faceTwist)%3 : (3 + index - faceTwist)%3;
  }

  template <>
  int FaceTopologyMapping<hexa>::
  invTwist(int index, int faceTwist) {
    return (faceTwist < 0) ?
           (9 - index + faceTwist)%4 : (4 + index - faceTwist)%4;
  }

  template <>
  const int FaceTopologyMapping<tetra>::
  dune2aluVertex_[EntityCount<tetra>::numVerticesPerFace] = {0, 1, 2};

  template <>
  const int FaceTopologyMapping<hexa>::
  dune2aluVertex_[EntityCount<hexa>::numVerticesPerFace] = {0, 3, 1, 2};

  template <>
  const int FaceTopologyMapping<tetra>::
  alu2duneVertex_[EntityCount<tetra>::numVerticesPerFace] = {0, 1, 2};

  template <>
  const int FaceTopologyMapping<hexa>::
  alu2duneVertex_[EntityCount<hexa>::numVerticesPerFace] = {0, 2, 3, 1};


} // end namespace Dune
