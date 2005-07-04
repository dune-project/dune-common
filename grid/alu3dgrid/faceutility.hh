// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGRIDFACEUTILITY_HH
#define DUNE_ALU3DGRIDFACEUTILITY_HH

#include <dune/common/misc.hh>

#include "mappings.hh"
#include "alu3dinclude.hh"
#include "topology.hh"

namespace Dune {
  // * Note 1: change template argument of ALU3dGeometricFaceInfo to elementType as soon as new reference elements are available
  // * Note 2: reconsider lazy evaluation of coordinates

  // Forward declarations
  template <int mydim, int coorddim, class GridImp>
  class ALU3dGridMakeableGeometry;
  template<int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGrid;

  //- class ALU3dGridFaceInfo
  /* \brief Stores face and adjoining elements of the underlying ALU3dGrid
     The class has the same notion of inner and outer element as the
     intersection iterator.
   */
  template <ALU3dGridElementType type>
  class ALU3dGridFaceInfo {
  private:
    //- private typedefs
    typedef typename ALU3dImplTraits<type>::HasFaceType HasFaceType;
  public:
    enum ConformanceState {CONFORMING, REFINED_INNER, REFINED_OUTER};
    //- typedefs
    typedef typename ALU3dImplTraits<type>::GEOFaceType GEOFaceType;
    typedef typename ALU3dImplTraits<type>::GEOElementType GEOElementType;
    typedef typename ALU3dImplTraits<type>::IMPLElementType IMPLElementType;
#ifdef _ALU3DGRID_PARALLEL_
    typedef typename ALU3dImplTraits<type>::PLLBndFaceType BndFaceType;
#else
    typedef typename ALU3dImplTraits<type>::BNDFaceType BndFaceType;
#endif

  public:
    //- constructors and destructors
    //! Construct a connector from a face and the twist seen from the inner
    //! element
    //! \note: The user is responsible for the consistency of the input data
    //! as well as for choosing the appropriate (i.e. most refined) face
    ALU3dGridFaceInfo(const GEOFaceType& face, int innerTwist);
    //! Copy constructor
    ALU3dGridFaceInfo(const ALU3dGridFaceInfo<type>& orig);
    //! Destructor
    ~ALU3dGridFaceInfo();

    //- queries
    //! Lies the face on an outer boundary?
    bool outerBoundary() const;
    //! Lies the face on an internal boundary
    bool boundary() const;

    //! is the neighbour element a ghost elemenet or a ghost face
    //! ic case of face true is returned
    bool ghostBoundary () const;

    //! Returns the ALU3dGrid face
    const GEOFaceType& face() const;
    //! Returns the inner element at that face
    const GEOElementType& innerEntity() const;
    //! Returns the outer element at that face
    //! \note This function is only meaningful in the interior
    const GEOElementType& outerEntity() const;
    //! Returns the boundary (outer) element at that face
    //! \note This function is only meaningful at a boundary
    const BndFaceType& boundaryFace() const;

    //! Twist of the face seen from the inner element
    int innerTwist() const;
    //! Twist of the face seen from the outer element
    int outerTwist() const;

    //! Local number of the face in inner element (ALU3dGrid reference element)
    int innerALUFaceIndex() const;
    //! Local number of the face in outer element (ALU3dGrid reference element)
    int outerALUFaceIndex() const;

    //! Description of conformance on the face
    ConformanceState conformanceState() const;

  private:
    //- forbidden methods
    const ALU3dGridFaceInfo<type>&
    operator=(const ALU3dGridFaceInfo<type>& orig);

  private:
    //- member data
    const GEOFaceType* face_;
    const HasFaceType* innerElement_;
    const HasFaceType* outerElement_;

    int innerFaceNumber_;
    int outerFaceNumber_;

    bool outerBoundary_;
    bool ghostBoundary_;
  };

  template <class GridImp>
  class ALU3dGridGeometricFaceInfo {
  private:
    //- private typedefs
    typedef GridImp GridType;
    typedef typename GridType::template codim<0>::Geometry ElementGeometryType;
    typedef ElementTopologyMapping<GridImp::elementType> ElementTopo;
    typedef FaceTopologyMapping<GridImp::elementType> FaceTopo;
    typedef NonConformingFaceMapping<GridImp::elementType> NonConformingMappingType;
    typedef typename SelectType<
        SameType<Int2Type<tetra>,Int2Type<GridImp::elementType> >::value,
        ALU3DSPACE LinearSurfaceMapping,
        BilinearSurfaceMapping
        >::Type SurfaceMappingType;

    enum SideIdentifier { INNER, OUTER };
    enum { dimworld = GridImp::dimensionworld };
    enum { numVerticesPerFace =
             EntityCount<GridImp::elementType>::numVerticesPerFace };
  public:
    //- public typedefs
    typedef FieldVector<alu3d_ctype, 3> NormalType;
    typedef FieldMatrix<alu3d_ctype,
        numVerticesPerFace,
        dimworld> CoordinateType;
    typedef ALU3dGridMakeableGeometry<2, 3, const ALU3dGrid<3, 3, GridImp::elementType> > FaceGeometryType;

  public:
    typedef ALU3dGridFaceInfo<GridImp::elementType> ConnectorType;

    //- constructors and destructors
    ALU3dGridGeometricFaceInfo(const ConnectorType& ctor);
    ALU3dGridGeometricFaceInfo(const ALU3dGridGeometricFaceInfo& orig);
    ~ALU3dGridGeometricFaceInfo();

    //- functions
    const CoordinateType& intersectionGlobal() const;
    const CoordinateType& intersectionSelfLocal() const;
    const CoordinateType& intersectionNeighborLocal() const;

    NormalType outerNormal(const FieldVector<alu3d_ctype, 2>& local) const;

  private:
    //- forbidden methods
    const ALU3dGridGeometricFaceInfo<GridImp>& operator=(const ALU3dGridGeometricFaceInfo<GridImp>&);

  private:
    //- private methods
    void generateGlobalGeometry() const;
    void generateLocalGeometries() const;

    int referenceElementCornerIndex(int faceIndex,
                                    int faceTwist,
                                    int localVertexIndex) const;

    void referenceElementCoordinatesRefined(SideIdentifier side,
                                            CoordinateType& result) const;
    void referenceElementCoordinatesUnrefined(SideIdentifier side,
                                              CoordinateType& result) const;

    SurfaceMappingType* buildSurfaceMapping(const CoordinateType& coords) const;

    NormalType calculateNormal(const SurfaceMappingType& mapping,
                               const FieldVector<alu3d_ctype, 2>& local) const;

    void convert2CArray(const FieldVector<alu3d_ctype, 3>& in,
                        alu3d_ctype (&out)[3]) const;
    void convert2FieldVector(const alu3d_ctype (&in)[3],
                             FieldVector<alu3d_ctype, 3>& out) const;

  private:
    //- private data
    const ConnectorType& connector_;

    mutable SurfaceMappingType* mappingGlobal_; // needed for calculation of normal
    mutable bool generatedGlobal_;
    mutable bool generatedLocal_;

    mutable CoordinateType coordsGlobal_;
    mutable CoordinateType coordsSelfLocal_;
    mutable CoordinateType coordsNeighborLocal_;
  };

} // end namespace Dune

#include "faceutility_imp.cc"

#endif
