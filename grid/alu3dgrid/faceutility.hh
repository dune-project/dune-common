// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGRIDFACEUTILITY_HH
#define DUNE_ALU3DGRIDFACEUTILITY_HH

#include <dune/common/misc.hh>
#include <dune/grid/common/referenceelements.hh>

#include "mappings.hh"
#include "alu3dinclude.hh"
#include "topology.hh"

namespace Dune {


  // * Note: reconsider lazy evaluation of coordinates

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
    //! constructor creating empty face info
    ALU3dGridFaceInfo();
    void updateFaceInfo(const GEOFaceType& face, int innerTwist);

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

  //! Helper class which provides geometric face information for the
  //! ALU3dGridIntersectionIterator
  template <ALU3dGridElementType type>
  class ALU3dGridGeometricFaceInfoBase {
  public:
    //- private typedefs
    typedef ElementTopologyMapping<type> ElementTopo;
    typedef FaceTopologyMapping<type> FaceTopo;
    typedef NonConformingFaceMapping<type> NonConformingMappingType;
    typedef typename SelectType<
        SameType<Int2Type<tetra>,Int2Type<type> >::value,
        ALU3DSPACE LinearSurfaceMapping,
        BilinearSurfaceMapping
        >::Type SurfaceMappingType;

    typedef typename SelectType<
        SameType<Int2Type<tetra>, Int2Type<type> >::value,
        ReferenceSimplex<alu3d_ctype, 3>,
        ReferenceCube<alu3d_ctype, 3>
        >::Type ReferenceElementType;

    typedef typename SelectType<
        SameType<Int2Type<tetra>, Int2Type<type> >::value,
        ReferenceSimplex<alu3d_ctype, 2>,
        ReferenceCube<alu3d_ctype, 2>
        >::Type ReferenceFaceType;


    enum SideIdentifier { INNER, OUTER };
    enum { dimworld = 3 }; // ALU is a pure 3d grid
    enum { numVerticesPerFace =
             EntityCount<type>::numVerticesPerFace };
  public:
    //- public typedefs
    typedef FieldVector<alu3d_ctype, 3> NormalType;
    typedef FieldMatrix<alu3d_ctype,
        numVerticesPerFace,
        dimworld> CoordinateType;

    typedef typename ALU3dGridFaceInfo<type>::GEOFaceType GEOFaceType;
  public:
    typedef ALU3dGridFaceInfo<type> ConnectorType;

    //- constructors and destructors
    ALU3dGridGeometricFaceInfoBase(const ConnectorType& ctor);
    ALU3dGridGeometricFaceInfoBase(const ALU3dGridGeometricFaceInfoBase<type> & orig);

    //! reset status of faceGeomInfo
    void resetFaceGeom();

    //- functions
    const CoordinateType& intersectionGlobal() const;
    const CoordinateType& intersectionSelfLocal() const;
    const CoordinateType& intersectionNeighborLocal() const;

    //NormalType & outerNormal(const FieldVector<alu3d_ctype, 2>& local) const;

  private:
    //- forbidden methods
    const ALU3dGridGeometricFaceInfoBase<type>& operator=(const ALU3dGridGeometricFaceInfoBase<type>&);

  private:
    //- private methods
    void generateGlobalGeometry() const;
    void generateLocalGeometries() const;

    int globalVertexIndex(int duneFaceIndex,
                          int faceTwist,
                          int duneFaceVertexIndex) const;

    void referenceElementCoordinatesRefined(SideIdentifier side,
                                            CoordinateType& result) const;
    void referenceElementCoordinatesUnrefined(SideIdentifier side,
                                              CoordinateType& result) const;

    // old method , copies values for tetra twice
    SurfaceMappingType* buildSurfaceMapping(const CoordinateType& coords) const;

    // get face and doesnt copy values twice
    SurfaceMappingType* buildSurfaceMapping(const GEOFaceType & face) const;

    void convert2CArray(const FieldVector<alu3d_ctype, 3>& in,
                        alu3d_ctype (&out)[3]) const;
    void convert2FieldVector(const alu3d_ctype (&in)[3],
                             FieldVector<alu3d_ctype, 3>& out) const;
  protected:
    //- private data
    const ConnectorType& connector_;

    mutable bool generatedGlobal_;
    mutable bool generatedLocal_;

    mutable CoordinateType coordsGlobal_;
    mutable CoordinateType coordsSelfLocal_;
    mutable CoordinateType coordsNeighborLocal_;

    static ReferenceElementType refElem_;
    static ReferenceFaceType refFace_;

  private:
    // convert FieldVectors to alu3dtypes
    // only used for calculating the normals because the method of the
    // mapping classes want double (&)[3] and we have FieldVectors which store an
    // double [3] this is why we can cast here
    // plz say notin' Adrian
    template <int dim>
    alu3d_ctype (& fieldVector2alu3d_ctype ( FieldVector <alu3d_ctype,dim> & val ) const )[dim]
    {
      return ((alu3d_ctype (&)[dim])(*( &(val[0])) ));
    }

    // convert const FieldVectors to const alu3dtypes
    template <int dim>
    const alu3d_ctype (& fieldVector2alu3d_ctype ( const FieldVector <alu3d_ctype,dim> & val ) const )[dim]
    {
      return ((const alu3d_ctype (&)[dim])(*( &(val[0])) ) );
    }
  };

  //! Helper class which provides geometric face information for the
  //! ALU3dGridIntersectionIterator
  class ALU3dGridGeometricFaceInfoTetra : public ALU3dGridGeometricFaceInfoBase<tetra>
  {
  public:
    //- public typedefs
    typedef FieldVector<alu3d_ctype, 3> NormalType;
    typedef FieldMatrix<alu3d_ctype,
        numVerticesPerFace,
        dimworld> CoordinateType;

    typedef ALU3dGridFaceInfo<tetra>::GEOFaceType GEOFaceType;
  public:
    typedef ALU3dGridFaceInfo<tetra> ConnectorType;

    //- constructors and destructors
    ALU3dGridGeometricFaceInfoTetra(const ConnectorType& ctor);
    ALU3dGridGeometricFaceInfoTetra(const ALU3dGridGeometricFaceInfoTetra & orig);

    NormalType & outerNormal(const FieldVector<alu3d_ctype, 2>& local) const;

    //! reset status of faceGeomInfo
    void resetFaceGeom();

  private:
    //- forbidden methods
    const ALU3dGridGeometricFaceInfoTetra & operator=(const ALU3dGridGeometricFaceInfoTetra &);

  private:
    //- private data
    mutable NormalType outerNormal_;

    // false if surface mapping needs a update
    mutable bool normalUp2Date_;
  };

  //! Helper class which provides geometric face information for the
  //! ALU3dGridIntersectionIterator
  class ALU3dGridGeometricFaceInfoHexa : public ALU3dGridGeometricFaceInfoBase<hexa>
  {
  public:
    //- public typedefs
    typedef FieldVector<alu3d_ctype, 3> NormalType;
    typedef FieldMatrix<alu3d_ctype,
        numVerticesPerFace,
        dimworld> CoordinateType;

    typedef ALU3dGridFaceInfo<hexa>::GEOFaceType GEOFaceType;
    typedef BilinearSurfaceMapping SurfaceMappingType;
  public:
    typedef ALU3dGridFaceInfo<hexa> ConnectorType;

    //- constructors and destructors
    ALU3dGridGeometricFaceInfoHexa(const ConnectorType& ctor);
    ALU3dGridGeometricFaceInfoHexa(const ALU3dGridGeometricFaceInfoHexa & orig);

    NormalType & outerNormal(const FieldVector<alu3d_ctype, 2>& local) const;

    //! reset status of faceGeomInfo
    void resetFaceGeom();

  private:
    //- forbidden methods
    const ALU3dGridGeometricFaceInfoHexa & operator=(const ALU3dGridGeometricFaceInfoHexa &);

  private:
    //- private data
    mutable NormalType outerNormal_;

    // surface mapping for calculating the outer normal
    mutable SurfaceMappingType mappingGlobal_;
    // false if surface mapping needs a update
    mutable bool mappingGlobalUp2Date_;
  };

} // end namespace Dune

#include "faceutility_imp.cc"

#endif
