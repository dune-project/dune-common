// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_BSINCLUDE_ORIGINAL_HH__
#define __DUNE_BSINCLUDE_ORIGINAL_HH__

#ifndef _ANSI_HEADER
#define _ANSI_HEADER
#endif

//#define _BSGRID_PARALLEL_

// all methods and classes of the BSgrid are defined in the namespace
// BernhardSchuppGrid
#define BSSPACE BernhardSchuppGrid ::

// if this is defined in bsgrid some methods are added which we only need
// for the Dune interface
#define _DUNE_USES_BSGRID_

#include <dune/common/fvector.hh>

// if MPI was found include all headers
#ifdef _BSGRID_PARALLEL_
#include <alu3dgrid_parallel.h>
#else
// if not, include only headers for serial version
#include <alu3dgrid_serial.h>
#endif

namespace BernhardSchuppGrid {

  // definition of AutoPointer class
#include "myautoptr.hh"

#ifdef _BSGRID_PARALLEL_

  struct GatherScatter;
  typedef GatherScatter GatherScatterType;

  typedef GitterDunePll GitterType;
  typedef GitterDunePll BSGitterType;
  typedef GitterDunePll BSGitterImplType;

  typedef Hbnd3PllInternal < GitterType :: Objects :: Hbnd3Default,
      BndsegPllBaseXClosure < GitterType :: hbndseg3_GEO > ,
      BndsegPllBaseXMacroClosure < GitterType :: hbndseg3_GEO > > :: micro_t MicroType;

  enum { ProcessorBoundary_t = BSGitterType:: hbndseg_STI :: closure };

#else
  struct GatherScatter;
  typedef GatherScatter GatherScatterType;

  // the header
  typedef Gitter GitterType;
  typedef GitterDuneImpl BSGitterType;
  typedef GitterDuneImpl BSGitterImplType;
  typedef GitterType::hface_STI PLLFaceType;                   // Interface Face

#endif

  // rule for Dune LeafIterator
#include "leafwalk.hh"

  // typedefs of Element types
  typedef GitterType::helement_STI HElementType;             // Interface Element
  typedef GitterType::hface_STI HFaceType;                   // Interface Face
  typedef GitterType::hedge_STI HEdgeType;                   // Interface Edge
  typedef GitterType::vertex_STI VertexType;                 // Interface Vertex
  typedef GitterType::Geometric::hface3_GEO GEOFaceType;     // real Face
  typedef GitterType::Geometric::hedge1_GEO GEOEdgeType;     // real Face
  typedef GitterType::Geometric::VertexGeo GEOVertexType;      // real Face
  typedef BSGitterType::Objects::tetra_IMPL IMPLElementType; // impl Element
  typedef GitterType::Geometric::tetra_GEO GEOElementType;   // real Element
  typedef GitterType::Geometric::hasFace3 HasFace3Type;      // has Face with 3 polygons

  typedef BSGitterType::Objects::Hbnd3Default BNDFaceType;   // boundary segment
  typedef BSGitterType::Objects::hbndseg3_IMPL ImplBndFaceType;  // boundary segment
  typedef BNDFaceType PLLBndFaceType;

  // refinement and coarsening enum for tetrahedons
  enum { refine_element_t = GitterType::Geometric::TetraRule::iso8 };
  enum { coarse_element_t = GitterType::Geometric::TetraRule::crs  };

  typedef pair < GEOFaceType *    , int > NeighbourFaceType;
  typedef pair < HasFace3Type*    , int > NeighbourPairType;
  typedef pair < PLLBndFaceType * , int > GhostPairType;

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
  template <int codim> struct BSHElementType
  {
    typedef GitterType :: helement_STI ElementType;
  };

  template <> struct BSHElementType<0> {
    typedef GitterType :: helement_STI ElementType;
  };
  template <> struct BSHElementType<1> {
    typedef GitterType :: hface_STI ElementType;
  };
  template <> struct BSHElementType<2> {
    typedef GitterType :: hedge_STI ElementType;
  };
  template <> struct BSHElementType<3> {
    typedef GitterType :: vertex_STI ElementType;
  };

  template <int codim> struct BSIMPLElementType
  {
    typedef BSGitterType::Objects::tetra_IMPL ElementType; // impl Element
  };

  template <> struct BSIMPLElementType<0> {
    typedef BSGitterType::Objects::tetra_IMPL ElementType; // impl Element
  };
  template <> struct BSIMPLElementType<1> {
    typedef BSGitterType::Objects::hface3_IMPL ElementType; // impl Element
  };
  template <> struct BSIMPLElementType<2> {
    typedef BSGitterType::Objects::hedge1_IMPL ElementType; // impl Element
  };

  template <> struct BSIMPLElementType<3> {
    //typedef GitterType :: vertex_STI ElementType;
    typedef GitterType::Geometric::VertexGeo ElementType;
  };

  //*********************************************************
  //  LevelIterator Wrapper
  //*********************************************************
  template <int codim> class BSGridLevelIteratorWrapper;
  template <>
  class BSGridLevelIteratorWrapper<0>
  {
    typedef BSHElementType<0>::ElementType ElType;
    typedef Insert < AccessIterator < ElType >::Handle ,
        TreeIterator  < ElType ,  any_has_level < ElType > > > IteratorType;

    IteratorType it_;
    typedef IteratorType :: val_t val_t;
  public:
    template <class GridImp>
    BSGridLevelIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid().container(),level) {}

    int size  ()    { return it_.size(); }
    void next ()    { it_.next(); }
    void first()    { it_.first(); }
    int done ()     { return it_.done(); }
    val_t & item () { return it_.item(); }

  };

  template <>
  class BSGridLevelIteratorWrapper<1>
  {
    typedef BSHElementType<1>::ElementType ElType;
    typedef Insert < AccessIterator < ElType >::Handle ,
        TreeIterator  < ElType ,  any_has_level < ElType > > > IteratorType;

    IteratorType it_;
    typedef IteratorType :: val_t val_t;
  public:
    template <class GridImp>
    BSGridLevelIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid().container(),level) {}

    int size  ()    { return it_.size(); }
    void next ()    { it_.next(); }
    void first()    { it_.first(); }
    int done ()     { return it_.done(); }
    val_t & item () { return it_.item(); }

  };

  template <>
  class BSGridLevelIteratorWrapper<2>
  {
    typedef BSHElementType<2>::ElementType ElType;
    typedef Insert < AccessIterator < ElType >::Handle ,
        TreeIterator  < ElType ,  any_has_level < ElType > > > IteratorType;

    IteratorType it_;
    typedef IteratorType :: val_t val_t;
  public:
    template <class GridImp>
    BSGridLevelIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid().container(),level) {}

    int size  ()    { return it_.size(); }
    void next ()    { it_.next(); }
    void first()    { it_.first(); }
    int done ()     { return it_.done(); }
    val_t & item () { return it_.item(); }

  };

  template <>
  class BSGridLevelIteratorWrapper<3>
  {
    typedef LeafIterator < GitterType::vertex_STI > IteratorType;

    IteratorType it_;
    typedef IteratorType :: val_t val_t;

    // level to walk
    int level_;
  public:
    template <class GridImp>
    BSGridLevelIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid()),
        level_(level)  {}

    int size  ()  { return it_->size(); }

    //! if level of item is larger then walk level, go next
    void next ()
    {
      it_->next();
      if(it_->done()) return ;
      if(it_->item().level() > level_) this->next();
      return ;
    }
    void first()    { it_->first(); }
    int done () const { return it_->done(); }
    val_t & item () { return it_->item(); }

  };

  template <int codim> class BSGridLeafIteratorWrapper;
  //**********************************************************
  //  LeafIterator Wrapper
  //**********************************************************
  template <>
  class BSGridLeafIteratorWrapper<0>
  {
    typedef BSHElementType<0>::ElementType ElType;
    typedef Insert < AccessIterator < ElType >::Handle,
        TreeIterator < ElType , leaf_or_has_level < ElType > > > IteratorType;

    IteratorType it_;
    typedef IteratorType :: val_t val_t;
  public:
    template <class GridImp>
    BSGridLeafIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid().container(),level) {}

    int size  ()    { return it_.size(); }
    void next ()    { it_.next(); }
    void first()    { it_.first(); }
    int done ()     { return it_.done(); }
    val_t & item () { return it_.item(); }

  };

  template <>
  class BSGridLeafIteratorWrapper<1>
  {
    typedef BSHElementType<1>::ElementType ElType;
    typedef Insert < AccessIterator < ElType >::Handle,
        TreeIterator < ElType , leaf_or_has_level < ElType > > > IteratorType;

    IteratorType it_;
    typedef IteratorType :: val_t val_t;
  public:
    template <class GridImp>
    BSGridLeafIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid().container(),level) {}

    int size  ()    { return it_.size(); }
    void next ()    { it_.next(); }
    void first()    { it_.first(); }
    int done ()     { return it_.done(); }
    val_t & item () { return it_.item(); }

  };

  template <>
  class BSGridLeafIteratorWrapper<2>
  {
    typedef BSHElementType<2>::ElementType ElType;
    typedef Insert < AccessIterator < ElType >::Handle,
        TreeIterator < ElType , leaf_or_has_level < ElType > > > IteratorType;

    IteratorType it_;
    typedef IteratorType :: val_t val_t;
  public:
    template <class GridImp>
    BSGridLeafIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid().container(),level) {}

    int size  ()    { return it_.size(); }
    void next ()    { it_.next(); }
    void first()    { it_.first(); }
    int done ()     { return it_.done(); }
    val_t & item () { return it_.item(); }

  };

  template <>
  class BSGridLeafIteratorWrapper<3>
  {
    typedef LeafIterator < GitterType::vertex_STI > IteratorType;
    IteratorType it_;
    typedef IteratorType :: val_t val_t;
  public:
    template <class GridImp>
    BSGridLeafIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid()) {}

    int size  ()    { return it_->size(); }
    void next ()    { it_->next(); }
    void first()    { it_->first(); }
    int done ()     { return it_->done(); }
    val_t & item () { return it_->item(); }

  };

  //*************************************************************
  typedef LeafIterator < GitterType::helement_STI > BSLeafIteratorMaxLevel;

} //end namespace BernhardSchuppGrid

#endif
