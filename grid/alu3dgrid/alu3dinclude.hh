// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DINCLUDE_HH
#define DUNE_ALU3DINCLUDE_HH

#ifndef _ANSI_HEADER
#define _ANSI_HEADER
#endif

// all methods and classes of the ALUGrid are defined in the namespace
#define ALU3DSPACE ALUGridSpace ::

// if this is defined in bsgrid some methods are added which we only need
// for the Dune interface
#define _DUNE_USES_ALU3DGRID_

// if MPI was found include all headers
#ifdef _ALU3DGRID_PARALLEL_
#include <alugrid_parallel.h>
#else
// if not, include only headers for serial version
#include <alugrid_serial.h>
#endif
#undef _DUNE_USES_ALU3DGRID_

namespace ALUGridSpace {

  //#ifndef NDEBUG
  //  static bool global_Geometry_lock = false;
  //#endif

#ifdef _ALU3DGRID_PARALLEL_

  typedef GatherScatter GatherScatterType;



  typedef GitterDunePll GitterType;
  typedef GitterDunePll GitterImplType;

  typedef Hbnd3PllInternal<GitterType::Objects::Hbnd3Default,
      BndsegPllBaseXClosure<GitterType::hbndseg3_GEO>,
      BndsegPllBaseXMacroClosure<GitterType::hbndseg3_GEO> > :: micro_t MicroType;

  // value for boundary to other processes
  static const int ProcessorBoundary_t = GitterImplType::hbndseg_STI::closure;

#else
  typedef GatherScatter GatherScatterType;

  // the header
  typedef Gitter GitterType;
  typedef GitterDuneImpl GitterImplType;
  typedef GitterType::hface_STI PLLFaceType;                     // Interface Face

#endif


  // typedefs of Element types
  typedef GitterType::helement_STI HElementType;               // Interface Element
  typedef GitterType::hface_STI HFaceType;                     // Interface Face
  typedef GitterType::hedge_STI HEdgeType;                     // Interface Edge
  typedef GitterType::vertex_STI VertexType;                   // Interface Vertex
  typedef GitterType::hbndseg_STI HBndSegType;
  typedef GitterType::Geometric::hface3_GEO GEOFace3Type;     // Tetra Face
  typedef GitterType::Geometric::hface4_GEO GEOFace4Type; // Hexa Face
  typedef GitterType::Geometric::hedge1_GEO GEOEdgeT;     // * stays real Face
  typedef GitterType::Geometric::VertexGeo GEOVertexT;     // * stays real Face
  typedef GitterImplType::Objects::tetra_IMPL IMPLTetraElementType; //impl Element
  typedef GitterImplType::Objects::hexa_IMPL IMPLHexaElementType;
  typedef GitterType::Geometric::tetra_GEO GEOTetraElementType;  // real Element
  typedef GitterType::Geometric::hexa_GEO GEOHexaElementType;
  typedef GitterType::Geometric::hasFace3 HasFace3Type;    // has Face with 3 polygons
  typedef GitterType::Geometric::hasFace4 HasFace4Type;
  typedef GitterType::Geometric::Hface3Rule Hface3RuleType;
  typedef GitterType::Geometric::Hface4Rule Hface4RuleType;

  typedef GitterImplType::Objects::Hbnd3Default BNDFace3Type;    // boundary segment
  typedef GitterImplType::Objects::Hbnd4Default BNDFace4Type;
  typedef GitterImplType::Objects::hbndseg3_IMPL ImplBndFace3Type;    // boundary segment
  typedef GitterImplType::Objects::hbndseg4_IMPL ImplBndFace4Type;
} // end namespace ALUGridSpace

namespace Dune {

  enum ALU3dGridElementType { tetra = 4, hexa = 7, mixed, error };

  // i.e. double or float
  typedef double alu3d_ctype;

  //class ALU3dGridError : public Exception {};

  //ALU3dGridElementType convertGeometryType2ALU3dGridElementType(GeometryType);
  //GeometryType convertALU3dGridElementType2GeometryType(ALU3dGridElementType);

  template <ALU3dGridElementType elType>
  struct ALU3dImplTraits {};

  template <>
  struct ALU3dImplTraits<tetra> {
    typedef ALU3DSPACE GEOFace3Type GEOFaceType;
    typedef ALU3DSPACE GEOEdgeT GEOEdgeType;
    typedef ALU3DSPACE GEOVertexT GEOVertexType;
    typedef ALU3DSPACE IMPLTetraElementType IMPLElementType;
    typedef ALU3DSPACE GEOTetraElementType GEOElementType;
    typedef ALU3DSPACE HasFace3Type HasFaceType;
    typedef ALU3DSPACE Hface3RuleType HfaceRuleType;
    typedef ALU3DSPACE BNDFace3Type BNDFaceType;
    typedef ALU3DSPACE ImplBndFace3Type ImplBndFaceType;
    typedef ALU3DSPACE BNDFace3Type PLLBndFaceType;

    // refinement and coarsening enum for tetrahedons
    enum { refine_element_t =
             ALU3DSPACE GitterType::Geometric::TetraRule::iso8 };
    enum { coarse_element_t =
             ALU3DSPACE GitterType::Geometric::TetraRule::crs  };
    enum { nosplit_element_t = ALU3DSPACE GitterType::Geometric::TetraRule::nosplit };

    typedef std::pair<GEOFaceType*, int> NeighbourFaceType;
    typedef std::pair<HasFaceType*, int> NeighbourPairType;
    typedef std::pair<PLLBndFaceType*, int> GhostPairType;

    template <int cdim>
    struct Codim;

  };

  template <>
  struct ALU3dImplTraits<tetra>::Codim<0> {
    typedef ALU3DSPACE GitterType::helement_STI InterfaceType;
    typedef IMPLElementType ImplementationType;
  };

  template <>
  struct ALU3dImplTraits<tetra>::Codim<1> {
    typedef ALU3DSPACE GitterType::hface_STI InterfaceType;
    typedef GEOFaceType ImplementationType;
  };

  template <>
  struct ALU3dImplTraits<tetra>::Codim<2> {
    typedef ALU3DSPACE GitterType::hedge_STI InterfaceType;
    typedef GEOEdgeType ImplementationType;
  };

  template <>
  struct ALU3dImplTraits<tetra>::Codim<3> {
    typedef ALU3DSPACE GitterType::vertex_STI InterfaceType;
    typedef ALU3DSPACE GitterType::Geometric::VertexGeo ImplementationType;
  };



  template <>
  struct ALU3dImplTraits<hexa> {
    typedef ALU3DSPACE GEOFace4Type GEOFaceType;
    typedef ALU3DSPACE GEOEdgeT GEOEdgeType;
    typedef ALU3DSPACE GEOVertexT GEOVertexType;
    typedef ALU3DSPACE IMPLHexaElementType IMPLElementType;
    typedef ALU3DSPACE GEOHexaElementType GEOElementType;
    typedef ALU3DSPACE HasFace4Type HasFaceType;
    typedef ALU3DSPACE Hface4RuleType HfaceRuleType;
    typedef ALU3DSPACE BNDFace4Type BNDFaceType;
    typedef ALU3DSPACE ImplBndFace4Type ImplBndFaceType;
    typedef ALU3DSPACE BNDFace4Type PLLBndFaceType;

    // refinement and coarsening enum for hexahedrons
    enum { refine_element_t  = ALU3DSPACE GitterType::Geometric::HexaRule::iso8 };
    enum { coarse_element_t  = ALU3DSPACE GitterType::Geometric::HexaRule::crs  };
    enum { nosplit_element_t = ALU3DSPACE GitterType::Geometric::HexaRule::nosplit };

    typedef std::pair<GEOFaceType*, int> NeighbourFaceType;
    typedef std::pair<HasFaceType*, int> NeighbourPairType;
    typedef std::pair<PLLBndFaceType*, int> GhostPairType;

    template <int cdim>
    struct Codim;
  };

  template <>
  struct ALU3dImplTraits<hexa>::Codim<0> {
    typedef ALU3DSPACE GitterType::helement_STI InterfaceType;
    typedef IMPLElementType ImplementationType;
  };

  template <>
  struct ALU3dImplTraits<hexa>::Codim<1> {
    typedef ALU3DSPACE GitterType::hface_STI InterfaceType;
    typedef GEOFaceType ImplementationType;
  };

  template <>
  struct ALU3dImplTraits<hexa>::Codim<2> {
    typedef ALU3DSPACE GitterType::hedge_STI InterfaceType;
    typedef GEOEdgeType ImplementationType;
  };

  template <>
  struct ALU3dImplTraits<hexa>::Codim<3> {
    typedef ALU3DSPACE GitterType::vertex_STI InterfaceType;
    typedef ALU3DSPACE GitterType::Geometric::VertexGeo ImplementationType;
  };


#ifdef _ALU3DGRID_PARALLEL_
  static int __MyRank__ = -1;
#endif
} // end namespace Dune

#endif
