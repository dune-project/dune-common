// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_BSINCLUDE_ORIGINAL_HH__
#define __DUNE_BSINCLUDE_ORIGINAL_HH__

#define _ANSI_HEADER

// includes for bsgrid_src
#include "systemincludes.hh"

// all methods and classes of the sbgrid are defined in the namespace
// BernhardSchuppGrid
#define BSSPACE BernhardSchuppGrid ::

// if this is defined in bsgrid some methods are added which we only need
// for the Dune interface
#define _DUNE_USES_BSGRID_

#include <dune/grid/common/indexstack.hh>

namespace BernhardSchuppGrid {

  // definition of indexstack
  enum { lengthOfFiniteStack = 10000 };
  typedef Dune::IndexStack<int,lengthOfFiniteStack> IndexManagerType;

  // type for outer normal of intersection iterator
  typedef Dune::FieldVector <double,3> BSGridVecType;

  typedef enum {hexa,tetra} grid_t;

  // bsgrid original sources
#include "bsgrid_src/gitter_impl.hh"
#include "bsgrid_src/walk.hh"

#include "bsgrid_src/gitter_geo.cc"
#include "bsgrid_src/gitter_mgb.cc"
#include "bsgrid_src/gitter_sti.cc"
#include "bsgrid_src/mapp_cube_3d.cc"
#include "bsgrid_src/mapp_tetra_3d.cc"
#include "bsgrid_src/myalloc.cc"

  // rule for Dune LeafIterator
#include "leafwalk.hh"

  // typedefs of Element types
  typedef Gitter::helement_STI HElementType;           // Interface Element
  typedef Gitter::hface_STI HFaceType;                 // Interface Element
  typedef Gitter::Geometric::hface3_GEO GEOFaceType;   // real Element
  typedef GitterBasis::Objects::tetra_IMPL IMPLElementType; // real Element
  typedef Gitter::Geometric::tetra_GEO GEOElementType; // real Element

  // refinement and coarsening enum for tetrahedons
  enum { refine_element_t = Gitter::Geometric::TetraRule::iso8 };
  enum { coarse_element_t = Gitter::Geometric::TetraRule::crs  };

  typedef pair < GEOFaceType * , int > NeighbourPairType;

  //*************************************************************
  //  definition of original LeafIterators of BSGrid
  //
  // default is element (codim = 0)
  template <int codim>
  struct BSMacroIterator
  {
    typedef AccessIterator < Gitter::helement_STI > :: Handle IteratorType;
  };

  //******************************************************************
  //  LevelIterators
  //******************************************************************
  template <int codim>
  struct BSLevelIterator
  {
    typedef Insert < AccessIterator < Gitter::helement_STI>::Handle,
        TreeIterator <Gitter::helement_STI,
            any_has_level <Gitter::helement_STI> > > IteratorType;
  };
  template <>
  struct BSLevelIterator<1>
  {
    typedef Insert <AccessIterator <Gitter::hface_STI>::Handle,
        TreeIterator <Gitter::hface_STI ,
            any_has_level <Gitter::hface_STI > > > IteratorType;
  };

  template <>
  struct BSLevelIterator<2>
  {
    typedef Insert <AccessIterator <Gitter::hedge_STI>::Handle,
        TreeIterator <Gitter::hedge_STI ,
            any_has_level <Gitter::hedge_STI > > > IteratorType;
  };

  template <>
  struct BSLevelIterator<3>
  {
    /*
       typedef Insert <AccessIterator <Gitter::vertex_STI>::Handle,
       TreeIterator <Gitter::vertex_STI ,
       any_has_level <Gitter::vertex_STI > > > IteratorType;
     */
    typedef LeafIterator < Gitter :: vertex_STI > IteratorType;
  };

  template <int codim>
  struct BSLeafIterator
  {
    typedef Insert <AccessIterator <Gitter::helement_STI>::Handle,
        TreeIterator <Gitter::helement_STI,
            leaf_or_has_level <Gitter::helement_STI> > > IteratorType;
  };

  /*
     template <>
     struct BSLeafIterator<1>
     {
     typedef LeafIterator < Gitter::hface_STI > IteratorType;
     };

     template <>
     struct BSLeafIterator<2>
     {
     typedef LeafIterator < Gitter::hedge_STI > IteratorType;
     };

     template <>
     struct BSLeafIterator<3>
     {
     typedef LeafIterator < Gitter::vertex_STI > IteratorType;
     };
   */
  //*************************************************************
  typedef LeafIterator < Gitter::helement_STI > BSLeafIteratorMaxLevel;

} //end namespace BernhardSchuppGrid

#endif
