// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
namespace Dune {

  template <ALU3dGridElementType type>
  ALU3dGridFaceInfo<type>::ALU3dGridFaceInfo(const GEOFaceType& face,
                                             int innerTwist) :
    face_(&face),
    innerElement_(0),
    outerElement_(0),
    innerFaceNumber_(-1),
    outerFaceNumber_(-1),
    outerBoundary_  ( false ),
    ghostBoundary_  ( false )
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

    outerBoundary_ = outerElement_->isboundary();
#ifdef _ALU3DGRID_PARALLEL_
    // check for ghosts
    // this check is only need in the parallel case
    if( outerBoundary_ )
    {
      const BndFaceType * bnd = dynamic_cast<const BndFaceType *> (outerElement_);
      if(bnd->bndtype() ==  ALU3DSPACE ProcessorBoundary_t)
      {
        // NOTE: this changes if ghost elements are implemented
        // at the moment there is no difference between internalBoundary
        // and ghostBoundary
        ghostBoundary_  = true;

        // this dosen't count as outer boundary
        outerBoundary_ = false;
      }
    }
#endif
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
    outerFaceNumber_(orig.outerFaceNumber_),
    outerBoundary_(orig.outerBoundary_),
    ghostBoundary_(orig.ghostBoundary_) {}

  template <ALU3dGridElementType type>
  inline bool ALU3dGridFaceInfo<type>::outerBoundary() const {
    return outerBoundary_;
  }

  template <ALU3dGridElementType type>
  inline bool ALU3dGridFaceInfo<type>::boundary() const {
    return outerBoundary_ || ghostBoundary_;
  }

  template <ALU3dGridElementType type>
  inline bool ALU3dGridFaceInfo<type>::ghostBoundary () const {
    return ghostBoundary_;
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
    if (!boundary() ) {
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
  typename ALU3dGridFaceInfo<type>::ConformanceState
  ALU3dGridFaceInfo<type>::conformanceState() const {
    ConformanceState result = CONFORMING;

    // A boundary is always unrefined
    int levelDifference = 0 ;
    if ( boundary() )
      levelDifference = innerEntity().level() - boundaryFace().level();
    else
      levelDifference = innerEntity().level() - outerEntity().level();

    if (levelDifference < 0) {
      result = REFINED_OUTER;
    } else if (levelDifference > 0) {
      result = REFINED_INNER;
    }

    return result;
  }

  //- class ALU3dGridFaceGeometryCoordinates
  template <ALU3dGridElementType type>
  ALU3dGridGeometricFaceInfo<type>::
  ALU3dGridGeometricFaceInfo(const ConnectorType& connector) :
    connector_(connector),
    refElem_(),
    refFace_(),
    mappingGlobal_(0),
    generatedGlobal_(false),
    generatedLocal_(false),
    coordsGlobal_(-1.0), // dummy value
    coordsSelfLocal_(-1.0),
    coordsNeighborLocal_(-1.0)
  {}

  template <ALU3dGridElementType type>
  ALU3dGridGeometricFaceInfo<type>::
  ALU3dGridGeometricFaceInfo(const ALU3dGridGeometricFaceInfo<type>& orig) :
    connector_(orig.connector_),
    refElem_(),
    refFace_(),
    mappingGlobal_(orig.mappingGlobal_),
    generatedGlobal_(orig.generatedGlobal_),
    generatedLocal_(orig.generatedLocal_),
    coordsGlobal_(orig.coordsGlobal_),
    coordsSelfLocal_(orig.coordsSelfLocal_),
    coordsNeighborLocal_(orig.coordsNeighborLocal_)
  {}

  template <ALU3dGridElementType type>
  ALU3dGridGeometricFaceInfo<type>::
  ~ALU3dGridGeometricFaceInfo() {
    if (mappingGlobal_) {
      delete mappingGlobal_;
      mappingGlobal_ = 0;
    }
  }

  template <ALU3dGridElementType type>
  const typename ALU3dGridGeometricFaceInfo<type>::CoordinateType&
  ALU3dGridGeometricFaceInfo<type>::intersectionGlobal() const {
    generateGlobalGeometry();
    assert(generatedGlobal_);
    return coordsGlobal_;
  }

  template <ALU3dGridElementType type>
  const typename ALU3dGridGeometricFaceInfo<type>::CoordinateType&
  ALU3dGridGeometricFaceInfo<type>::intersectionSelfLocal() const {
    generateLocalGeometries();
    assert(generatedLocal_);
    return coordsSelfLocal_;
  }

  template <ALU3dGridElementType type>
  const typename ALU3dGridGeometricFaceInfo<type>::CoordinateType&
  ALU3dGridGeometricFaceInfo<type>::intersectionNeighborLocal() const {
    assert(!connector_.outerBoundary());
    generateLocalGeometries();
    assert(generatedLocal_);
    return coordsNeighborLocal_;
  }

  template <ALU3dGridElementType type>
  FieldVector<alu3d_ctype, 3>
  ALU3dGridGeometricFaceInfo<type>::
  outerNormal(const FieldVector<alu3d_ctype, 2>& local) const {
    // construct a mapping (either a linear or a bilinear one)
    if (!mappingGlobal_) {
      CoordinateType coords;
      for (int i = 0; i < numVerticesPerFace; ++i) {
        const double (&p)[3] =
          connector_.face().myvertex(FaceTopo::dune2aluVertex(i))->Point();
        convert2FieldVector(p, coords[i] );
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

  template <ALU3dGridElementType type>
  inline void ALU3dGridGeometricFaceInfo<type>::
  generateGlobalGeometry() const
  {
    if (!generatedGlobal_) {
      for (int i = 0; i < numVerticesPerFace; ++i) {
        const double (&p)[3] =
          connector_.face().myvertex(FaceTopo::dune2aluVertex(i))->Point();
        for (int j = 0; j < 3; ++j) {
          coordsGlobal_[i][j] = p[j];
        }
      }

      generatedGlobal_ = true;
    } // end if
  }

  template <ALU3dGridElementType type>
  inline void ALU3dGridGeometricFaceInfo<type>::
  generateLocalGeometries() const
  {
    if (!generatedLocal_) {
      // Get the coordinates of the face in the reference element of the
      // adjoining inner and outer elements and initialise the respective
      // geometries
      switch (connector_.conformanceState()) {
      case (ConnectorType::CONFORMING) :
        referenceElementCoordinatesRefined(INNER, coordsSelfLocal_);
        // generate outer local geometry only when not at boundary
        // * in the parallel case, this needs to be altered for the ghost cells
        if (!connector_.outerBoundary()) {
          referenceElementCoordinatesRefined(OUTER, coordsNeighborLocal_);
        } // end if
        break;
      case (ConnectorType::REFINED_INNER) :
        referenceElementCoordinatesRefined(INNER, coordsSelfLocal_);
        referenceElementCoordinatesUnrefined(OUTER, coordsNeighborLocal_);
        break;
      case (ConnectorType::REFINED_OUTER) :
        referenceElementCoordinatesUnrefined(INNER, coordsSelfLocal_);
        referenceElementCoordinatesRefined(OUTER, coordsNeighborLocal_);
        break;
      default :
        assert(false);
      } // end switch

      generatedLocal_ = true;
    } // end if
  }

  template <ALU3dGridElementType type>
  int ALU3dGridGeometricFaceInfo<type>::
  globalVertexIndex(int duneFaceIndex,
                    int faceTwist,
                    int duneFaceVertexIndex) const {
    // get local ALU vertex number on the element's face
    int localALUIndex = FaceTopo::dune2aluVertex(duneFaceVertexIndex,
                                                 faceTwist);
    int localDuneIndex =
      ElementTopo::alu2duneFaceVertex(ElementTopo::dune2aluFace(duneFaceIndex),
                                      localALUIndex);
    return refElem_.subentity(duneFaceIndex, 1, localDuneIndex, 3);
  }

  template <ALU3dGridElementType type>
  void ALU3dGridGeometricFaceInfo<type>::
  referenceElementCoordinatesRefined(SideIdentifier side,
                                     CoordinateType& result) const
  {
    // this is a dune face index
    int faceIndex =
      (side == INNER ?
       ElementTopo::alu2duneFace(connector_.innerALUFaceIndex()) :
       ElementTopo::alu2duneFace(connector_.outerALUFaceIndex()));
    int faceTwist =
      (side == INNER ?
       connector_.innerTwist() :
       connector_.outerTwist());

    for (int i = 0; i < numVerticesPerFace; ++i) {
      int duneVertexIndex = globalVertexIndex(faceIndex, faceTwist, i);
      result[i] = refElem_.position(duneVertexIndex, 3);
    }
  }

  template <ALU3dGridElementType type>
  void ALU3dGridGeometricFaceInfo<type>::
  convert2CArray(const FieldVector<alu3d_ctype, 3>& in,
                 alu3d_ctype (&out)[3]) const {
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
  }

  template <ALU3dGridElementType type>
  void ALU3dGridGeometricFaceInfo<type>::
  convert2FieldVector(const alu3d_ctype (&in)[3],
                      FieldVector<alu3d_ctype, 3>& out) const {
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
  }

} //end namespace Dune
