// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGRIDTOPOLOGY_HH
#define DUNE_ALU3DGRIDTOPOLOGY_HH

#include "alu3dinclude.hh"

namespace Dune {

  // types of the elementes, i.e . tetra or hexa, mixed is not implemeneted
  // yet
  //enum ALU3dGridElementType { tetra = 4, hexa = 7, mixed, error };

  template <ALU3dGridElementType type>
  struct EntityCount {};

  template <>
  struct EntityCount<tetra> {
    enum {numFaces = 4};
    enum {numVertices = 4};
    enum {numEdges = 6};
    enum {numVerticesPerFace = 3};
    enum {numEdgesPerFace = 3};
  };

  template <>
  struct EntityCount<hexa> {
    enum {numFaces = 6};
    enum {numVertices = 8};
    enum {numEdges = 12};
    enum {numVerticesPerFace = 4};
    enum {numEdgesPerFace = 4};
  };


  //! Maps indices of the Dune reference element onto the indices of the
  //! ALU3dGrid reference element and vice-versa.
  template <ALU3dGridElementType type>
  class ElementTopologyMapping
  {
  public:
    enum { numFaces = EntityCount<type>::numFaces };
    enum { numVertices = EntityCount<type>::numVertices };
    enum { numEdges = EntityCount<type>::numEdges };
    enum { numVerticesPerFace = EntityCount<type>::numVerticesPerFace };

    //! Maps face index from Dune onto ALU3dGrid reference element
    static int dune2aluFace(int index);
    //! Maps face index from ALU3dGrid onto Dune reference element
    static int alu2duneFace(int index);

    //! Maps edge index from Dune onto ALU3dGrid reference element
    static int dune2aluEdge(int index);
    //! Maps edge index from ALU3dGrid onto Dune reference element
    static int alu2duneEdge(int index);

    //! Maps vertex index from Dune onto ALU3dGrid reference element
    static int dune2aluVertex(int index);
    //! Maps vertex index from ALU3dGrid onto Dune reference element
    static int alu2duneVertex(int index);

    //! Return 1 if faces in ALU3dGrid and Dune reference element
    //! have the same orientation (edge 0->1 is taken as reference as
    //! they are the same in both reference elements), -1 otherwise.
    //! The index is a Dune face index
    static int faceOrientation(int index);

    //! Maps local vertex index of a face onto a global vertex index
    //! (Dune->ALU3dGrid)
    //! \param face Face index (Dune reference element)
    //! \param localVertex Local vertex index on face \face (Dune reference
    //! element)
    //! \return global vertex index in ALU3dGrid reference element
    static int dune2aluFaceVertex(int face, int localVertex);
    //! Maps local vertex index of a face onto a global vertex index
    //! (ALU3dGrid->Dune)
    //! \param face Face index (ALU3dGrid reference element)
    //! \param localVertex Local vertex index on face \face
    //! (ALU3dGrid reference element)
    //! \return global vertex index in Dune reference element
    static int alu2duneFaceVertex(int face, int localVertex);

  private:
    const static int dune2aluFace_[numFaces];
    const static int alu2duneFace_[numFaces];

    const static int dune2aluEdge_[numEdges];
    const static int alu2duneEdge_[numEdges];

    const static int dune2aluVertex_[numVertices];
    const static int alu2duneVertex_[numVertices];

    const static int faceOrientation_[numFaces];

    const static int dune2aluFaceVertex_[numFaces][numVerticesPerFace];
    const static int alu2duneFaceVertex_[numFaces][numVerticesPerFace];

  };

  //! Maps indices of the Dune reference face onto the indices of the
  //! ALU3dGrid reference face and vice-versa.
  template <ALU3dGridElementType type>
  class FaceTopologyMapping {
  public:
    //! Maps vertex index from Dune onto ALU3dGrid reference face
    static int dune2aluVertex(int index);
    //! Maps vertex index from Dune onto ALU3dGrid reference face, where the
    //! face in the ALU3dGrid has the twist \twist compared to the orientation
    //! of the respective face in the reference element
    //! \param index local Dune vertex index on the particular face (i.e. the
    //! face which has a twist \twist compared to the reference element's face
    //! \param twist twist of the face in consideration
    //! \return local ALU3dGrid vertex index on reference element face
    static int dune2aluVertex(int index, int twist);
    //! Maps vertex index from ALU3dGrid onto Dune reference face
    static int alu2duneVertex(int index);
    //! Maps vertex index from ALU3dGrid onto Dune reference face, where the
    //! face in the ALU3dGrid has the twist \twist compared to the orientation
    //! of the respective face in the reference element
    //! \param index local ALU3dGrid vertex index on the particular face (i.e.
    //! the face which has a twist \twist compared to the reference element's
    //! face
    //! \param twist twist of the face in consideration
    //! \return local Dune vertex index on reference element face
    static int alu2duneVertex(int index, int twist);
    //! Maps edge index from Dune onto ALU3dGrid reference face
    static int dune2aluEdge(int index);
    //! Maps edge index from ALU3dGrid onto Dune reference face
    static int alu2duneEdge(int index);
    //  private:
    static int twist(int index, int faceTwist);
    static int invTwist(int index, int faceTwist);
  private:
    const static int dune2aluVertex_[EntityCount<type>::numVerticesPerFace];
    const static int alu2duneVertex_[EntityCount<type>::numVerticesPerFace];

    const static int dune2aluEdge_[EntityCount<type>::numEdgesPerFace];
    const static int alu2duneEdge_[EntityCount<type>::numEdgesPerFace];
  };

  inline const ALU3dImplTraits<tetra>::GEOFaceType*
  getFace(const ALU3DSPACE GEOTetraElementType& elem, int index) {
    assert(index >= 0 && index < 4);
    return elem.myhface3(ElementTopologyMapping<tetra>::dune2aluFace(index));
  }

  inline const ALU3dImplTraits<hexa>::GEOFaceType*
  getFace(const ALU3DSPACE GEOHexaElementType& elem, int index) {
    assert(index >= 0 && index < 6);
    return elem.myhface4(ElementTopologyMapping<hexa>::dune2aluFace(index));
  }

  //- IMPLEMENTATION
  //- class ElementTopologyMapping
  template <ALU3dGridElementType type>
  inline int ElementTopologyMapping<type>::dune2aluFace(int index) {
    assert(index >= 0 && index < numFaces);
    return dune2aluFace_[index];
  }

  template <ALU3dGridElementType type>
  inline int ElementTopologyMapping<type>::alu2duneFace(int index) {
    assert(index >= 0 && index < numFaces);
    return alu2duneFace_[index];
  }

  template <ALU3dGridElementType type>
  inline int ElementTopologyMapping<type>::dune2aluEdge(int index) {
    assert(index >= 0 && index < numEdges);
    return dune2aluEdge_[index];
  }

  template <ALU3dGridElementType type>
  inline int ElementTopologyMapping<type>::alu2duneEdge(int index) {
    assert(index >= 0 && index < numEdges);
    return alu2duneEdge_[index];
  }

  template <>
  inline int ElementTopologyMapping<tetra>::dune2aluVertex(int index) {
    assert(index >= 0 && index < numVertices);
    // at the moment this mapping is represented by the id, if this changes
    // just remove this specialisation
    assert( dune2aluVertex_[index] == index );
    return index;
  }

  template <ALU3dGridElementType type>
  inline int ElementTopologyMapping<type>::dune2aluVertex(int index) {
    assert(index >= 0 && index < numVertices);
    return dune2aluVertex_[index];
  }

  template <ALU3dGridElementType type>
  inline int ElementTopologyMapping<type>::alu2duneVertex(int index) {
    assert(index >= 0 && index < numVertices);
    return alu2duneVertex_[index];
  }

  template <ALU3dGridElementType type>
  inline int ElementTopologyMapping<type>::faceOrientation(int index) {
    assert(index >= 0 && index < numVertices);
    return faceOrientation_[index];
  }

  template <ALU3dGridElementType type>
  inline int ElementTopologyMapping<type>::
  dune2aluFaceVertex(int face, int localVertex) {
    assert(face >= 0 && face < numFaces);
    assert(localVertex >= 0 && localVertex < numVerticesPerFace);
    return dune2aluFaceVertex_[face][localVertex];
  }

  template <ALU3dGridElementType type>
  inline int ElementTopologyMapping<type>::
  alu2duneFaceVertex(int face, int localVertex) {
    assert(face >= 0 && face < numFaces);
    assert(localVertex >= 0 && localVertex < numVerticesPerFace);
    return alu2duneFaceVertex_[face][localVertex];
  }

  //- class FaceTopologyMapping
  template <ALU3dGridElementType type>
  inline int FaceTopologyMapping<type>::dune2aluVertex(int index) {
    assert(index >= 0 && index < EntityCount<type>::numVerticesPerFace);
    return dune2aluVertex_[index];
  }

  template <ALU3dGridElementType type>
  inline int FaceTopologyMapping<type>::dune2aluVertex(int index, int twist) {
    assert(index >= 0 && index < EntityCount<type>::numVerticesPerFace);
    return invTwist(dune2aluVertex_[index], twist);
  }

  template <ALU3dGridElementType type>
  inline int FaceTopologyMapping<type>::alu2duneVertex(int index) {
    assert(index >= 0 && index < EntityCount<type>::numVerticesPerFace);
    return alu2duneVertex_[index];
  }

  template <ALU3dGridElementType type>
  inline int FaceTopologyMapping<type>::alu2duneVertex(int index, int twist) {
    assert(index >= 0 && index < EntityCount<type>::numVerticesPerFace);
    return alu2duneVertex_[invTwist(index, twist)];
  }

  template <ALU3dGridElementType type>
  inline int FaceTopologyMapping<type>::alu2duneEdge(int index) {
    assert(index >= 0 && index < EntityCount<type>::numEdgesPerFace);
    return alu2duneEdge_[index];
  }

  template <ALU3dGridElementType type>
  inline int FaceTopologyMapping<type>::dune2aluEdge(int index) {
    assert(index >= 0 && index < EntityCount<type>::numEdgesPerFace);
    return dune2aluEdge_[index];
  }

} // end namespace Dune
#endif
