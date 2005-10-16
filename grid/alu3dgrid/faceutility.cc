// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>
#include "faceutility.hh"

namespace Dune {

  // just for simplicity
  //typedef BilinearSurfaceMapping BilinearSurfaceMappingType;
  //typedef ALU3DSPACE BilinearSurfaceMapping BilinearSurfaceMappingType;
  typedef BilinearSurfaceMapping BilinearSurfaceMappingType;

  //- class ALU3dGridGeometricFaceInfo
  template <>
  ALU3DSPACE LinearSurfaceMapping*
  ALU3dGridGeometricFaceInfo<tetra>::
  buildSurfaceMapping(const CoordinateType& coords) const {

    return new ALU3DSPACE LinearSurfaceMapping(
             fieldVector2alu3d_ctype ( coords[0] ),
             fieldVector2alu3d_ctype ( coords[1] ),
             fieldVector2alu3d_ctype ( coords[2] ) );
  }

  template <>
  BilinearSurfaceMappingType*
  ALU3dGridGeometricFaceInfo<hexa>::
  buildSurfaceMapping(const CoordinateType& coords) const {
    return new BilinearSurfaceMappingType(coords[0],
                                          coords[1],
                                          coords[2],
                                          coords[3]);
  }

  // new methods
  template <>
  ALU3DSPACE LinearSurfaceMapping*
  ALU3dGridGeometricFaceInfo<tetra>::
  buildSurfaceMapping(const GEOFaceType & face) const
  {
    // this is the original ALUGrid LinearSurfaceMapping,
    // see mapp_tetra_3d.* in ALUGrid code
    return new ALU3DSPACE LinearSurfaceMapping(face.myvertex(0)->Point(),
                                               face.myvertex(1)->Point(),
                                               face.myvertex(2)->Point());
  }

  template <>
  BilinearSurfaceMappingType*
  ALU3dGridGeometricFaceInfo<hexa>::
  buildSurfaceMapping(const GEOFaceType & face) const
  {
    // this is dangerous
    CoordinateType coords;
    for (int i = 0; i < numVerticesPerFace; ++i) {
      const double (&p)[3] = face.myvertex(FaceTopo::dune2aluVertex(i))->Point();
      convert2FieldVector(p, coords[i] );
    } // end for

    // this is the new implementation using FieldVector
    // see mappings.hh
    return new BilinearSurfaceMappingType(coords[0],
                                          coords[1],
                                          coords[2],
                                          coords[3]);
  }

  template <>
  ALU3dGridGeometricFaceInfo<tetra>::NormalType
  ALU3dGridGeometricFaceInfo<tetra>::
  calculateNormal(const SurfaceMappingType& mapping,
                  const FieldVector<alu3d_ctype, 2>& local) const
  {
    NormalType result;
    mapping.normal( fieldVector2alu3d_ctype ( result ) );
    return result;
  }

  template <>
  ALU3dGridGeometricFaceInfo<hexa>::NormalType
  ALU3dGridGeometricFaceInfo<hexa>::
  calculateNormal(const SurfaceMappingType& mapping,
                  const FieldVector<alu3d_ctype, 2>& local) const
  {
    NormalType result;
    mapping.normal(local,result);
    return result;
  }

  template <>
  void ALU3dGridGeometricFaceInfo<tetra>::
  referenceElementCoordinatesUnrefined(SideIdentifier side,
                                       CoordinateType& result) const {
    enum { numCorners = 3 };
    // barycentric coordinates for local coordinates
    enum { numComponents = 3 };

    typedef FieldVector<alu3d_ctype, numComponents> LocalCoordinateType;

    // get the parent's face coordinates on the reference element
    // (Dune reference element)
    CoordinateType cornerCoords;
    referenceElementCoordinatesRefined(side, cornerCoords);

    std::auto_ptr<ALU3DSPACE LinearSurfaceMapping>
    referenceElementMapping(buildSurfaceMapping(cornerCoords));

    NonConformingMappingType faceMapper(connector_.face().parentRule(),
                                        connector_.face().nChild());

    // temporary variables
    LocalCoordinateType parentLocal;

    //alu3d_ctype parLocalArray[numComponents];
    //alu3d_ctype resultArray[numComponents];

    // do the mappings
    for (int i = 0; i < numCorners; ++i)
    {
      const FieldVector<alu3d_ctype, 2>& childLocal =
        refFace_.position(i, 2);

      // transform from coordinates of R^2 to barycentric coordinates
      // for barycentric coordinates see the child2parent method in
      // mappings.cc
      LocalCoordinateType childLocalBary;
      childLocalBary[0] = 1.0 - childLocal[0] - childLocal[1];
      childLocalBary[1] = childLocal[0];
      childLocalBary[2] = childLocal[1];

      faceMapper.child2parent(childLocalBary, parentLocal);
      referenceElementMapping->map2world(
        fieldVector2alu3d_ctype ( parentLocal ) ,
        fieldVector2alu3d_ctype ( result[i]   ) );

      //convert2CArray(parentLocal, parLocalArray);

      //referenceElementMapping->map2world(parLocalArray, resultArray);
      //convert2FieldVector(resultArray, result[i]);
    }

  }

  template <>
  void ALU3dGridGeometricFaceInfo<hexa>::
  referenceElementCoordinatesUnrefined(SideIdentifier side,
                                       CoordinateType& result) const {
    enum { numCorners = 4 };
    enum { numComponents = 2 };

    typedef FieldVector<alu3d_ctype, numComponents> LocalCoordinateType;

    // get the parent's reference face coordinates
    CoordinateType cornerCoords;
    referenceElementCoordinatesRefined(side, cornerCoords);

    BilinearSurfaceMapping
    referenceElementMapping(cornerCoords[0],
                            cornerCoords[1],
                            cornerCoords[2],
                            cornerCoords[3]);

    NonConformingMappingType faceMapper(connector_.face().parentRule(),
                                        connector_.face().nChild());

    // temporary variables
    LocalCoordinateType parentLocal;

    // do the mappings
    for (int i = 0; i < numCorners; ++i) {
      const LocalCoordinateType& childLocal = refFace_.position(i, 2);

      faceMapper.child2parent(childLocal, parentLocal);
      referenceElementMapping.map2world( parentLocal , result[i] );
    }
  }

} // end namespace Dune
