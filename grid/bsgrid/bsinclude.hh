// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_BSINCLUDE_ORIGINAL_HH__
#define __DUNE_BSINCLUDE_ORIGINAL_HH__

#ifndef _ANSI_HEADER
#define _ANSI_HEADER
#endif

#define _BSGRID_PARALLEL_

// includes for bsgrid_src
#include "systemincludes.hh"

// all methods and classes of the sbgrid are defined in the namespace
// BernhardSchuppGrid
#define BSSPACE BernhardSchuppGrid ::
//#define BSSPACE

// if this is defined in bsgrid some methods are added which we only need
// for the Dune interface
#define _DUNE_USES_BSGRID_

#include <dune/grid/common/indexstack.hh>
#include <dune/common/matvec.hh>

namespace BernhardSchuppGrid {

  // definition of indexstack
  enum { lengthOfFiniteStack = 10000 };
  typedef Dune::IndexStack<int,lengthOfFiniteStack> IndexManagerType;

  // type for outer normal of intersection iterator
  typedef Dune::FieldVector <double,3> BSGridVecType;


  typedef enum {hexa,tetra} grid_t;

  // bsgrid original sources

#ifdef _BSGRID_PARALLEL_

  struct GatherScatter;
  typedef GatherScatter GatherScatterType;

#include "bsgrid_src/serialize.h"
#include "bsgrid_src/gitter_dune_pll_impl.h"

  typedef GitterPll::helement_STI HElemType;   // Interface Element
  typedef GitterPll::hbndseg HGhostType;

  struct GatherScatter
  {
    virtual void inlineData ( ObjectStream & str , HElemType & elem ) = 0;
    virtual void xtractData ( ObjectStream & str , HElemType & elem ) = 0;

    virtual void sendData ( ObjectStream & str , const HElemType  & elem ) = 0;
    virtual void recvData ( ObjectStream & str , HGhostType & elem ) = 0;
  };
  typedef GatherScatter GatherScatterType;

  // all header files of the BSGrid
#include "allheaders.h"

  //#include "bsgrid_src/gitter_pll_sti.cc"
#include "bsgrid_src/gitter_pll_impl.cc"
  //#include "bsgrid_src/gitter_pll_ldb.cc"
  //#include "bsgrid_src/gitter_pll_mgb.cc"
  //#include "bsgrid_src/gitter_pll_idn.cc"
  //#include "bsgrid_src/mpAccess.cc"
  //#include "bsgrid_src/mpAccess_MPI.cc"

  typedef GitterDunePll GitterType;
  typedef GitterDunePll BSGitterType;
  typedef GitterDunePll BSGitterImplType;

  //typedef BndsegPllBaseXClosure < BSGitterType :: hbndseg3_GEO >   PLLFaceType;       // ParallelInterface Face

  typedef Hbnd3PllInternal < GitterType :: Objects :: Hbnd3Default,
      BndsegPllBaseXClosure < GitterType :: hbndseg3_GEO > ,
      BndsegPllBaseXMacroClosure < GitterType :: hbndseg3_GEO > > :: micro_t MicroType;

  enum { ProcessorBoundary_t = BSGitterType:: hbndseg_STI :: closure };
#else
  struct GatherScatter;
  typedef GatherScatter GatherScatterType;
#include "bsgrid_src/serialize.h"
#include "bsgrid_src/gitter_dune_impl.h"
  //#include "bsgrid_src/gitter_impl.h"

  typedef Gitter::helement_STI HElemType;   // Interface Element

  struct GatherScatter
  {
    virtual void inlineData ( ObjectStream & str , HElemType & elem ) = 0;
    virtual void xtractData ( ObjectStream & str , HElemType & elem ) = 0;

    virtual void sendData ( ObjectStream & str , HElemType & elem ) = 0;
    virtual void recvData ( ObjectStream & str , HElemType & elem ) = 0;
  };
  typedef GatherScatter GatherScatterType;

  // the header
#include "allheaders.h"
  typedef Gitter GitterType;
  typedef GitterDuneImpl BSGitterType;
  typedef GitterDuneImpl BSGitterImplType;
  typedef GitterType::hface_STI PLLFaceType;                   // Interface Face

#endif

  /*
     // the serial sources
     #include "bsgrid_src/gitter_geo.cc"
     #include "bsgrid_src/gitter_mgb.cc"
     //#include "bsgrid_src/gitter_sti_inline.h"
     //#include "bsgrid_src/gitter_sti.cc"
     #include "bsgrid_src/mapp_cube_3d.cc"
     #include "bsgrid_src/mapp_tetra_3d.cc"
     #include "bsgrid_src/myalloc.cc"
   */


  // rule for Dune LeafIterator
#include "leafwalk.hh"

  // typedefs of Element types
  typedef GitterType::helement_STI HElementType;             // Interface Element
  typedef GitterType::hface_STI HFaceType;                   // Interface Face
  typedef GitterType::Geometric::hface3_GEO GEOFaceType;     // real Face
  typedef BSGitterType::Objects::tetra_IMPL IMPLElementType;   // impl Element
  typedef GitterType::Geometric::tetra_GEO GEOElementType;   // real Element
  typedef GitterType::Geometric::hasFace3 HasFace3Type;      // has Face with 3 polygons

  typedef GitterType::Objects::Hbnd3Default BNDFaceType;   // boundary segment
  typedef BSGitterType::Objects::hbndseg3_IMPL ImplBndFaceType;  // boundary segment
  typedef BNDFaceType PLLBndFaceType;

  // refinement and coarsening enum for tetrahedons
  enum { refine_element_t = GitterType::Geometric::TetraRule::iso8 };
  enum { coarse_element_t = GitterType::Geometric::TetraRule::crs  };

  typedef pair < GEOFaceType * , int > NeighbourFaceType;
  typedef pair < HasFace3Type* , int > NeighbourPairType;

  //*************************************************************
  //  definition of original LeafIterators of BSGrid
  //
  // default is element (codim = 0)
  template <int codim>
  struct BSMacroIterator
  {
    typedef AccessIterator < GitterType::helement_STI > :: Handle IteratorType;
  };

  //******************************************************************
  //  LevelIterators
  //******************************************************************
  template <int codim>
  struct BSLevelIterator
  {
    typedef Insert < AccessIterator < GitterType::helement_STI>::Handle,
        TreeIterator <GitterType::helement_STI,
            any_has_level <GitterType::helement_STI> > > IteratorType;
  };
  template <>
  struct BSLevelIterator<1>
  {
    typedef Insert <AccessIterator <GitterType::hface_STI>::Handle,
        TreeIterator <GitterType::hface_STI ,
            any_has_level <GitterType::hface_STI > > > IteratorType;
  };

  template <>
  struct BSLevelIterator<2>
  {
    typedef Insert <AccessIterator <GitterType::hedge_STI>::Handle,
        TreeIterator <GitterType::hedge_STI ,
            any_has_level <GitterType::hedge_STI > > > IteratorType;
  };

  template <>
  struct BSLevelIterator<3>
  {
    /*
       typedef Insert <AccessIterator <GitterType::vertex_STI>::Handle,
       TreeIterator <GitterType::vertex_STI ,
       any_has_level <GitterType::vertex_STI > > > IteratorType;
     */
    typedef LeafIterator < GitterType :: vertex_STI > IteratorType;
  };

  template <int codim>
  struct BSLeafIterator
  {
    typedef Insert <AccessIterator <GitterType::helement_STI>::Handle,
        TreeIterator <GitterType::helement_STI,
            leaf_or_has_level <GitterType::helement_STI> > > IteratorType;
  };

  /*
     template <>
     struct BSLeafIterator<1>
     {
     typedef LeafIterator < GitterType::hface_STI > IteratorType;
     };

     template <>
     struct BSLeafIterator<2>
     {
     typedef LeafIterator < GitterType::hedge_STI > IteratorType;
     };

     template <>
     struct BSLeafIterator<3>
     {
     typedef LeafIterator < GitterType::vertex_STI > IteratorType;
     };
   */
  //*************************************************************
  typedef LeafIterator < GitterType::helement_STI > BSLeafIteratorMaxLevel;


} //end namespace BernhardSchuppGrid

#endif
