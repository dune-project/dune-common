// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "grid.hh"
#include "geometry.hh"

namespace Dune {

  template <ALU3dGridElementType type>
  ALU3dGridFaceInfo<type>::ALU3dGridFaceInfo(const GEOFaceType& face,
                                             int innerTwist) :
    face_(&face),
    innerElement_(0),
    outerElement_(0),
    innerFaceNumber_(-1),
    outerFaceNumber_(-1)
  {
    // points face from inner element away?
    if (innerTwist < 0) {
      innerElement_ = face.nb.rear().first;
      innerFaceNumber_ = face.nb.rear().second;
      outerElement_ = face.nb.front().first;
      outerFaceNumber_ = face.nb.front().second;
    } else {
      innerElement_ = face.nb.front().first;
      innerFaceNumber_ = face.nb.front().second;
      outerElement_ = face.nb.rear().first;
      outerFaceNumber_ = face.nb.rear().second;
    } // end if

    assert(innerTwist == innerEntity().twist(innerFaceNumber_));
  }

  template <ALU3dGridElementType type>
  ALU3dGridFaceInfo<type>::~ALU3dGridFaceInfo() {}

  template <ALU3dGridElementType type>
  ALU3dGridFaceInfo<type>::
  ALU3dGridFaceInfo(const ALU3dGridFaceInfo<type>& orig) :
    face_(orig.face_),
    innerElement_(orig.innerElement_),
    outerElement_(orig.outerElement_),
    innerFaceNumber_(orig.innerFaceNumber_),
    outerFaceNumber_(orig.outerFaceNumber_) {}

  template <ALU3dGridElementType type>
  bool ALU3dGridFaceInfo<type>::boundary() const {
    return outerElement_->isboundary();
  }

  template <ALU3dGridElementType type>
  bool ALU3dGridFaceInfo<type>::internalBoundary() const {
    assert(false); // not implemented yet
    return false;
  }

  template <ALU3dGridElementType type>
  const typename ALU3dGridFaceInfo<type>::GEOFaceType&
  ALU3dGridFaceInfo<type>::face() const {
    return *face_;
  }

  template <ALU3dGridElementType type>
  const typename ALU3dGridFaceInfo<type>::GEOElementType&
  ALU3dGridFaceInfo<type>::innerEntity() const {
    assert(!innerElement_->isboundary());
    return static_cast<const GEOElementType&>(*innerElement_);
  }

  template <ALU3dGridElementType type>
  const typename ALU3dGridFaceInfo<type>::GEOElementType&
  ALU3dGridFaceInfo<type>::outerEntity() const {
    assert(!boundary());
    return static_cast<const GEOElementType&>(*outerElement_);
  }

  template <ALU3dGridElementType type>
  const typename ALU3dGridFaceInfo<type>::BndFaceType&
  ALU3dGridFaceInfo<type>::boundaryFace() const {
    assert(boundary());
    return static_cast<const BndFaceType&>(*outerElement_);
  }

  template <ALU3dGridElementType type>
  int ALU3dGridFaceInfo<type>::innerTwist() const {
    return innerEntity().twist(innerALUFaceIndex());
  }

  template <ALU3dGridElementType type>
  int ALU3dGridFaceInfo<type>::outerTwist() const {
    if (!boundary()) {
      return outerEntity().twist(outerALUFaceIndex());
    } else {
      return boundaryFace().twist(outerALUFaceIndex());
    }
  }

  template <ALU3dGridElementType type>
  int ALU3dGridFaceInfo<type>::innerALUFaceIndex() const {
    return innerFaceNumber_;
  }

  template <ALU3dGridElementType type>
  int ALU3dGridFaceInfo<type>::outerALUFaceIndex() const {
    return outerFaceNumber_;
  }

  template <ALU3dGridElementType type>
  ALU3dGridFaceInfo<type>::RefinementState
  ALU3dGridFaceInfo<type>::refinementState() const {
    RefinementState result = UNREFINED;

    // A boundary is always unrefined
    if (!boundary()) {
      int levelDifference = innerEntity().level() - outerEntity().level();
      if (levelDifference < 0) {
        result = REFINED_OUTER;
      } else if (levelDifference > 0) {
        result = REFINED_INNER;
      }
    }
    return result;
  }

  //- class ALU3dGridFaceGeometryInfo
  template <class GridImp>
  ALU3dGridFaceGeometryInfo<GridImp>::
  ALU3dGridFaceGeometryInfo(const ConnectorType& connector,
                            const GridImp& grid,
                            int wLevel) :
    connector_(connector),
    grid_(grid),
    mappingGlobal_(0),
    wLevel_(wLevel)
  {
    intersectionGlobal_ =
      grid_.geometryProvider_.getNewObjectEntity(grid_ ,wLevel_);
    intersectionSelfLocal_ =
      grid_.geometryProvider_.getNewObjectEntity(grid_, wLevel_);
    intersectionNeighborLocal_ =
      grid_.geometryProvider_.getNewObjectEntity(grid_, wLevel_);

    generateGlobalGeometry();
    generateLocalGeometries();
  }

  template <class GridImp>
  ALU3dGridFaceGeometryInfo<GridImp>::
  ALU3dGridFaceGeometryInfo(const ALU3dGridFaceGeometryInfo<GridImp>& orig) :
    connector_(orig.connector_),
    grid_(orig.grid_),
    mappingGlobal_(orig.mappingGlobal_),
    wLevel_(orig.wLevel_)
  {
    intersectionGlobal_ =
      grid_.geometryProvider_.getNewObjectEntity(grid_ ,wLevel_);
    intersectionSelfLocal_ =
      grid_.geometryProvider_.getNewObjectEntity(grid_, wLevel_);
    intersectionNeighborLocal_ =
      grid_.geometryProvider_.getNewObjectEntity(grid_, wLevel_);

    generateGlobalGeometry();
    generateLocalGeometries();
  }

  template <class GridImp>
  ALU3dGridFaceGeometryInfo<GridImp>::
  ~ALU3dGridFaceGeometryInfo() {
    if (mappingGlobal_) {
      delete mappingGlobal_;
      mappingGlobal_ = 0;
    }

    if (intersectionGlobal_) {
      grid_.geometryProvider_.freeObjectEntity(intersectionGlobal_);
      intersectionGlobal_ = 0;
    }

    if (intersectionSelfLocal_) {
      grid_.geometryProvider_.freeObjectEntity(intersectionSelfLocal_);
      intersectionSelfLocal_ = 0;
    }

    if (intersectionNeighborLocal_) {
      grid_.geometryProvider_.freeObjectEntity(intersectionNeighborLocal_);
      intersectionNeighborLocal_ = 0;
    }
  }

  template <class GridImp>
  const ALU3dGridFaceGeometryInfo<GridImp>::FaceGeometryType&
  ALU3dGridFaceGeometryInfo<GridImp>::
  intersectionGlobal() const {
    assert(intersectionGlobal_);
    return *intersectionGlobal_;
  }

  template <class GridImp>
  const ALU3dGridFaceGeometryInfo<GridImp>::FaceGeometryType&
  ALU3dGridFaceGeometryInfo<GridImp>::
  intersectionSelfLocal() const {
    assert(intersectionSelfLocal_);
    return *intersectionSelfLocal_;
  }

  template <class GridImp>
  const ALU3dGridFaceGeometryInfo<GridImp>::FaceGeometryType&
  ALU3dGridFaceGeometryInfo<GridImp>::
  intersectionNeighborLocal() const {
    assert(intersectionNeighborLocal_);
    assert(!connector_.boundary());
    return *intersectionNeighborLocal_;
  }

  template <class GridImp>
  FieldVector<alu3d_ctype, 3>
  ALU3dGridFaceGeometryInfo<GridImp>::
  outerNormal(const FieldVector<alu3d_ctype, 2>& local) const {

    // construct a mapping (either a linear or a bilinear one)
    if (!mappingGlobal_) {
      CoordinateType coords;
      for (int i = 0; i < numVerticesPerFace; ++i) {
        const double (&p)[3] =
          connector_.face().myvertex(FaceTopo::dune2aluVertex(i))->Point();
        FieldVector<alu3d_ctype, 3> tmp;
        convert2FieldVector(p, tmp);

        coords[i] = tmp;
      } // end for

      mappingGlobal_ = buildSurfaceMapping(coords);
    } // end if

    // calculate the normal
    NormalType result = calculateNormal(*mappingGlobal_, local);

    // change sign if face normal points into inner element
    if (connector_.innerTwist() < 0) {
      result *= -1.0;
    } // end if

    return result;
  }

  template <class GridImp>
  void ALU3dGridFaceGeometryInfo<GridImp>::generateGlobalGeometry() {
    intersectionGlobal_->buildGeom(connector_.face());
  }

  template <class GridImp>
  void ALU3dGridFaceGeometryInfo<GridImp>::generateLocalGeometries() {
    // Get the coordinates of the face in the reference element of the adjoining
    // inner and outer elements and initialise the respective geometries

    // if this is a conforming face...
    if (connector_.refinementState() == ConnectorType::UNREFINED) {
      CoordinateType coords;
      referenceElementCoordinatesRefined(INNER, coords);
      intersectionSelfLocal_->buildGeom(coords);
      // generate outer local geometry only when not at boundary
      // * in the parallel case, this needs to be altered for the ghost cells
      if (!connector_.boundary()) {
        referenceElementCoordinatesRefined(OUTER, coords);
        intersectionNeighborLocal_->buildGeom(coords);
      } // end if

    }
    else {
      assert(!connector_.boundary());

      FaceGeometryImp* refinedGeometry = 0;
      FaceGeometryImp* unrefinedGeometry = 0;
      SideIdentifier refinedSide = INNER;
      SideIdentifier unrefinedSide = INNER;

      if (connector_.refinementState() == ConnectorType::REFINED_INNER) {
        refinedSide = INNER;
        unrefinedSide = OUTER;
        refinedGeometry = intersectionSelfLocal_;
        unrefinedGeometry = intersectionNeighborLocal_;
      }
      else {
        refinedSide = OUTER;
        unrefinedSide = INNER;
        refinedGeometry = intersectionNeighborLocal_;
        unrefinedGeometry = intersectionSelfLocal_;
      }
      // refined element
      // get local corner coordinates of refined element and generate geometry
      CoordinateType coords;
      referenceElementCoordinatesRefined(refinedSide, coords);
      refinedGeometry->buildGeom(coords);

      // unrefined element
      referenceElementCoordinatesUnrefined(unrefinedSide, coords);
      unrefinedGeometry->buildGeom(coords);
    } // end if
  }

  template <class GridImp>
  void ALU3dGridFaceGeometryInfo<GridImp>::
  referenceElementCoordinatesRefined(SideIdentifier side,
                                     CoordinateType& result) const
  {
    const static ElementGeometryType& referenceGeometry =
      ElementGeometryType::refelem();

    // this is a dune face index
    int faceIndex = (side == INNER ?
                     ElementTopo::alu2duneFace(connector_.innerALUFaceIndex()) :
                     ElementTopo::alu2duneFace(connector_.outerALUFaceIndex()));
    int faceTwist = (side == INNER ?
                     connector_.innerTwist() :
                     connector_.outerTwist());

    for (int i = 0; i < numVerticesPerFace; ++i) {
      result[i] =
        referenceGeometry[referenceElementCornerIndex(faceIndex, faceTwist, i)];
    }
  }

  template <class GridImp>
  void ALU3dGridFaceGeometryInfo<GridImp>::
  convert2CArray(const FieldVector<alu3d_ctype, 3>& in,
                 alu3d_ctype (&out)[3]) const {
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
  }

  template <class GridImp>
  void ALU3dGridFaceGeometryInfo<GridImp>::
  convert2FieldVector(const alu3d_ctype (&in)[3],
                      FieldVector<alu3d_ctype, 3>& out) const {
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
  }

} //end namespace Dune
