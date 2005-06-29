// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGRIDFACEUTILITY_HH
#define DUNE_ALU3DGRIDFACEUTILITY_HH

#include <dune/common/misc.hh>

#include "mappings.hh"
#include "alu3dinclude.hh"
#include "topology.hh"

namespace Dune {
  // * Note 1: idea for refactoring: make ALU3dGridFaceGeometryInfo return only the coordinates of the respective geometries, so that the inclusion of grid.hh and geometry.hh can be removed. The intersectionIterator then needs to manage the geometries, which is more natural anyway

  // * Note 2: generate the geometries only when necessary (also ALU3dGridFaceGeometryInfo)

  // * Note 3: change that silly name...

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
    enum RefinementState {UNREFINED, REFINED_INNER, REFINED_OUTER};
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
    bool internalBoundary() const;

    //! is the neighbour element a ghost elemenet or a ghost face
    //! ic case of face true is returned
    bool isGhostBnd () const;

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
    RefinementState refinementState() const;

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
    bool internalBnd_;
    bool isGhostBnd_;
  };

  template <class GridImp>
  class ALU3dGridFaceGeometryInfo {
  private:
    //- private typedefs
    typedef GridImp GridType;
    typedef ALU3dImplTraits<GridImp::elementType> ImplTraitsType;
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
    typedef typename GridType::template codim<1>::Geometry FaceGeometryType;
    typedef ALU3dGridMakeableGeometry<2, 3, GridImp> FaceGeometryImp;

  public:
    typedef ALU3dGridFaceInfo<GridImp::elementType> ConnectorType;

    //- constructors and destructors
    ALU3dGridFaceGeometryInfo(const ConnectorType& ctor,
                              const GridImp& grid,
                              int wLevel);
    ALU3dGridFaceGeometryInfo(const ALU3dGridFaceGeometryInfo& orig);
    ~ALU3dGridFaceGeometryInfo();

    //- functions
    const FaceGeometryType& intersectionGlobal() const;
    const FaceGeometryType& intersectionSelfLocal() const;
    const FaceGeometryType& intersectionNeighborLocal() const;

    NormalType outerNormal(const FieldVector<alu3d_ctype, 2>& local) const;

  private:
    //- forbidden methods
    const ALU3dGridFaceGeometryInfo<GridImp>& operator=(const ALU3dGridFaceGeometryInfo<GridImp>&);

  private:
    //- private methods
    void generateGlobalGeometry();
    void generateLocalGeometries();

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
    const GridImp& grid_;

    mutable SurfaceMappingType* mappingGlobal_; // needed for calculation of normal

    int wLevel_;

    FaceGeometryImp* intersectionSelfLocal_;
    FaceGeometryImp* intersectionNeighborLocal_;
    FaceGeometryImp* intersectionGlobal_;
  };

} // end namespace Dune

#include "faceutility_imp.cc"

#endif
