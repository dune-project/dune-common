// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "faceutility.hh"
#include "../common/reftopology.hh"

namespace Dune {

  template <>
  ALU3DSPACE LinearSurfaceMapping*
  ALU3dGridGeometricFaceInfo<const ALU3dGrid<3, 3, tetra> >::
  buildSurfaceMapping(const CoordinateType& coords) const {
    alu3d_ctype tmp[3][3];
    convert2CArray(coords[0], tmp[0]);
    convert2CArray(coords[1], tmp[1]);
    convert2CArray(coords[2], tmp[2]);
    return new ALU3DSPACE LinearSurfaceMapping(tmp[0],
                                               tmp[1],
                                               tmp[2]);
  }

  template <>
  BilinearSurfaceMapping*
  ALU3dGridGeometricFaceInfo<const ALU3dGrid<3, 3, hexa> >::
  buildSurfaceMapping(const CoordinateType& coords) const {
    return new BilinearSurfaceMapping(coords[0],
                                      coords[1],
                                      coords[2],
                                      coords[3]);
  }

  template <>
  ALU3dGridGeometricFaceInfo<const ALU3dGrid<3, 3, tetra> >::NormalType
  ALU3dGridGeometricFaceInfo<const ALU3dGrid<3, 3, tetra> >::
  calculateNormal(const SurfaceMappingType& mapping,
                  const FieldVector<alu3d_ctype, 2>& local) const {
    enum { dimworld = 3 };

    alu3d_ctype normal[dimworld];
    mapping.normal(normal);

    NormalType result;
    convert2FieldVector(normal, result);

    return result;
  }

  template <>
  ALU3dGridGeometricFaceInfo<const ALU3dGrid<3, 3, hexa> >::NormalType
  ALU3dGridGeometricFaceInfo<const ALU3dGrid<3, 3, hexa> >::
  calculateNormal(const SurfaceMappingType& mapping,
                  const FieldVector<alu3d_ctype, 2>& local) const {
    NormalType result;
    mapping.normal(local, result);
    return result;
  }

  template <>
  int ALU3dGridGeometricFaceInfo<const ALU3dGrid<3, 3, tetra> >::
  referenceElementCornerIndex(int duneFaceIndex,
                              int faceTwist,
                              int duneFaceVertexIndex) const {
    // get local ALU vertex number on the element's face
    int localALUIndex = FaceTopo::dune2aluVertex(duneFaceVertexIndex,
                                                 faceTwist);

    int localDuneIndex =
      ElementTopo::alu2duneFaceVertex(ElementTopo::dune2aluFace(duneFaceIndex),
                                      localALUIndex);

    // get the structure to compute the global vertex number in the reference
    // element out of a face index and the local vertex number
    int n = -1;
    const int* globalVertexIndicesOnFace = 0;
    ReferenceTopologySet::getSubEntities<1,3>(tetrahedron, duneFaceIndex,
                                              globalVertexIndicesOnFace, n);

    // access the global vertex number
    assert(localALUIndex < n && localALUIndex >= 0);
    return globalVertexIndicesOnFace[localDuneIndex];
  }

  template <>
  int ALU3dGridGeometricFaceInfo<const ALU3dGrid<3, 3, hexa> >::
  referenceElementCornerIndex(int duneFaceIndex,
                              int faceTwist,
                              int duneFaceVertexIndex) const {
    // get local ALU vertex number on the element's face
    int localALUIndex = FaceTopo::dune2aluVertex(duneFaceVertexIndex,
                                                 faceTwist);

    int localDuneIndex =
      ElementTopo::alu2duneFaceVertex(ElementTopo::dune2aluFace(duneFaceIndex),
                                      localALUIndex);

    // get the structure to compute the global vertex number in the reference
    // element out of a face index and the local vertex number
    int n = -1;
    const int* globalVertexIndicesOnFace = 0;
    ReferenceTopologySet::getSubEntities<1,3>(hexahedron, duneFaceIndex,
                                              globalVertexIndicesOnFace, n);

    // access the global vertex number
    assert(localALUIndex < n);
    return globalVertexIndicesOnFace[localDuneIndex];
  }

  template <>
  void ALU3dGridGeometricFaceInfo<const ALU3dGrid<3, 3, tetra> >::
  referenceElementCoordinatesUnrefined(SideIdentifier side,
                                       CoordinateType& result) const {
    enum { numCorners = 3 };
    // barycentric coordinates for local coordinates
    enum { numComponents = 3 };

    typedef FieldVector<alu3d_ctype, numComponents> LocalCoordinateType;

    // get the parent's reference face coordinates
    CoordinateType cornerCoords;
    referenceElementCoordinatesRefined(side, cornerCoords);

    std::auto_ptr<ALU3DSPACE LinearSurfaceMapping>
    referenceElementMapping(buildSurfaceMapping(cornerCoords));

    NonConformingMappingType faceMapper(connector_.face().parentRule(),
                                        connector_.face().nChild());

    // temporary variables
    LocalCoordinateType parentLocal;
    alu3d_ctype parLocalArray[numComponents];
    alu3d_ctype resultArray[numComponents];

    // do the mappings
    for (int i = 0; i < numCorners; ++i) {
      const FieldVector<alu3d_ctype, 2>& childLocal =
        FaceGeometryType::refelem()[i];

      LocalCoordinateType childLocalBary;
      childLocalBary[0] = 1.0 - childLocal[0] - childLocal[1];
      childLocalBary[1] = childLocal[0];
      childLocalBary[2] = childLocal[1];

      faceMapper.child2parent(childLocalBary, parentLocal);
      convert2CArray(parentLocal, parLocalArray);
      referenceElementMapping->map2world(parLocalArray, resultArray);
      convert2FieldVector(resultArray, result[i]);
    }

  }

  template <>
  void ALU3dGridGeometricFaceInfo<const ALU3dGrid<3, 3, hexa> >::
  referenceElementCoordinatesUnrefined(SideIdentifier side,
                                       CoordinateType& result) const {
    enum { numCorners = 4 };
    enum { numComponents = 2 };

    typedef FieldVector<alu3d_ctype, numComponents> LocalCoordinateType;

    // get the parent's reference face coordinates
    CoordinateType cornerCoords;
    referenceElementCoordinatesRefined(side, cornerCoords);

    BilinearSurfaceMapping referenceElementMapping(cornerCoords[0],
                                                   cornerCoords[1],
                                                   cornerCoords[2],
                                                   cornerCoords[3]);

    NonConformingMappingType faceMapper(connector_.face().parentRule(),
                                        connector_.face().nChild());

    // temporary variables
    LocalCoordinateType parentLocal;

    // do the mappings
    for (int i = 0; i < numCorners; ++i) {
      const LocalCoordinateType& childLocal =
        FaceGeometryType::refelem()[i];
      faceMapper.child2parent(childLocal, parentLocal);
      referenceElementMapping.map2world(parentLocal, result[i]);
    }

  }

} // end namespace Dune
