// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DINCLUDE_HH
#define DUNE_ALU3DINCLUDE_HH

#ifndef _ANSI_HEADER
#define _ANSI_HEADER
#endif

// all methods and classes of the ALU3d-Grid are defined in the namespace
#define ALU3DSPACE ALU3dGridSpace ::

// if this is defined in bsgrid some methods are added which we only need
// for the Dune interface
#define _DUNE_USES_ALU3DGRID_

// if MPI was found include all headers
#ifdef _ALU3DGRID_PARALLEL_
#include <mpi.h>
#include <alu3dgrid_parallel.h>
#else
// if not, include only headers for serial version
#include <alu3dgrid_serial.h>
#endif
#undef _DUNE_USES_ALU3DGRID_

namespace ALU3dGridSpace {

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

  // rule for Dune LeafIterator
#include "leafwalk.hh"

  // typedefs of Element types
  typedef GitterType::helement_STI HElementType;               // Interface Element
  typedef GitterType::hface_STI HFaceType;                     // Interface Face
  typedef GitterType::hedge_STI HEdgeType;                     // Interface Edge
  typedef GitterType::vertex_STI VertexType;                   // Interface Vertex
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

  // * end new

  //*************************************************************
  //  definition of original LeafIterators of ALU3dGrid
  //
  // default is element (codim = 0)
  template <int codim>
  struct BSMacroIterator
  {
    typedef AccessIterator<GitterType::helement_STI>::Handle IteratorType;
  };

  //******************************************************************
  //  LevelIterators
  //******************************************************************
  template <int codim> struct ALUHElementType
  {
    typedef GitterType :: helement_STI ElementType;
  };

  template <> struct ALUHElementType<0> {
    typedef GitterType :: helement_STI ElementType;
  };
  template <> struct ALUHElementType<1> {
    typedef GitterType :: hface_STI ElementType;
  };
  template <> struct ALUHElementType<2> {
    typedef GitterType :: hedge_STI ElementType;
  };
  template <> struct ALUHElementType<3> {
    typedef GitterType :: vertex_STI ElementType;
  };

  template <int codim> struct BSIMPLElementType
  {
    typedef GitterImplType::Objects::tetra_IMPL ElementType; // impl Element
  };

  template <> struct BSIMPLElementType<0> {
    typedef GitterImplType::Objects::tetra_IMPL ElementType; // impl Element
  };
  template <> struct BSIMPLElementType<1> {
    typedef GitterImplType::Objects::hface3_IMPL ElementType; // impl Element
  };
  template <> struct BSIMPLElementType<2> {
    typedef GitterImplType::Objects::hedge1_IMPL ElementType; // impl Element
  };

  template <> struct BSIMPLElementType<3> {
    typedef GitterType::Geometric::VertexGeo ElementType;
  };

  //*********************************************************
  //  LevelIterator Wrapper
  //*********************************************************
  template <int codim> class ALU3dGridLevelIteratorWrapper;
  template <>
  class ALU3dGridLevelIteratorWrapper<0>
  {
    typedef ALUHElementType<0>::ElementType ElType;
    typedef Insert < AccessIterator < ElType >::Handle ,
        TreeIterator  < ElType ,  any_has_level < ElType > > > IteratorType;

    IteratorType it_;
    typedef IteratorType :: val_t val_t;
  public:
    template <class GridImp>
    ALU3dGridLevelIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid().container(),level) {}

    int size  ()    { return it_.size(); }
    void next ()    { it_.next(); }
    void first()    { it_.first(); }
    int done ()     { return it_.done(); }
    val_t & item () { return it_.item(); }

  };

  template <>
  class ALU3dGridLevelIteratorWrapper<1>
  {
    typedef ALUHElementType<1>::ElementType ElType;
    typedef Insert < AccessIterator < ElType >::Handle ,
        TreeIterator  < ElType ,  any_has_level < ElType > > > IteratorType;

    IteratorType it_;
    typedef IteratorType :: val_t val_t;
  public:
    template <class GridImp>
    ALU3dGridLevelIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid().container(),level) {}

    int size  ()    { return it_.size(); }
    void next ()    { it_.next(); }
    void first()    { it_.first(); }
    int done ()     { return it_.done(); }
    val_t & item () { return it_.item(); }

  };

  template <>
  class ALU3dGridLevelIteratorWrapper<2>
  {
    typedef ALUHElementType<2>::ElementType ElType;
    typedef Insert < AccessIterator < ElType >::Handle ,
        TreeIterator  < ElType ,  any_has_level < ElType > > > IteratorType;

    IteratorType it_;
    typedef IteratorType :: val_t val_t;
  public:
    template <class GridImp>
    ALU3dGridLevelIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid().container(),level) {}

    int size  ()    { return it_.size(); }
    void next ()    { it_.next(); }
    void first()    { it_.first(); }
    int done ()     { return it_.done(); }
    val_t & item () { return it_.item(); }

  };

  template <>
  class ALU3dGridLevelIteratorWrapper<3>
  {
    typedef LeafIterator < GitterType::vertex_STI > IteratorType;

    IteratorType it_;
    typedef IteratorType :: val_t val_t;

    // level to walk
    int level_;
  public:
    template <class GridImp>
    ALU3dGridLevelIteratorWrapper (const GridImp & grid, int level )
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


  template <class val_t>
  class IteratorWrapperInterface
  {
  public:
    virtual ~IteratorWrapperInterface () {}

    virtual int size  () = 0;
    virtual void next () = 0;
    virtual void first() = 0;
    virtual int done  () = 0;
    virtual val_t & item () = 0;
  };

  typedef Dune::PartitionIteratorType PartitionIteratorType;

  template <int codim, PartitionIteratorType pitype> class ALU3dGridLeafIteratorWrapper;
  typedef pair <ALUHElementType<0>::ElementType * , BNDFace3Type * > LeafValType;
  typedef IteratorWrapperInterface<LeafValType> IteratorWrapperInterfaceType;
  //**********************************************************
  //  LeafIterator Wrapper
  //**********************************************************
  template <PartitionIteratorType pitype>
  class ALU3dGridLeafIteratorWrapper<0,pitype>
    : public IteratorWrapperInterface< LeafValType >
  {
    typedef ALUHElementType<0>::ElementType ElType;
    typedef Insert < AccessIterator < ElType >::Handle,
        TreeIterator < ElType , leaf_or_has_level < ElType > > > IteratorType;

    IteratorType it_;
    //typedef IteratorType :: val_t val_t;

    typedef LeafValType val_t;
    val_t elem_;
  public:
    template <class GridImp>
    ALU3dGridLeafIteratorWrapper (const GridImp & grid, int level)
      : it_(const_cast<GridImp &> (grid).myGrid().container(),level)
    {
      elem_.first  = 0;
      elem_.second = 0;
    }

    int size  ()    { return it_.size(); }
    void next ()    { it_.next(); }
    void first()    { it_.first(); }
    int done ()     { return it_.done(); }
    val_t & item ()
    {
      elem_.first  = & it_.item();
      return elem_;
    }
  };

  /*
     template <>
     class ALU3dGridLeafIteratorWrapper<1> : public IteratorWrapperInterface
     {
     typedef ALUHElementType<1>::ElementType ElType;
     typedef Insert < AccessIterator < ElType >::Handle,
                     TreeIterator < ElType , leaf_or_has_level < ElType > > > IteratorType;

     IteratorType it_;
     typedef IteratorType :: val_t val_t;
     public:
     template <class GridImp>
     ALU3dGridLeafIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid().container(),level) {}

     int size  ()    { return it_.size(); }
     void next ()    { it_.next(); }
     void first()    { it_.first(); }
     int done ()     { return it_.done(); }
     val_t & item () { return it_.item(); }

     };

     template <>
     class ALU3dGridLeafIteratorWrapper<2> : public IteratorWrapperInterface
     {
     typedef ALUHElementType<2>::ElementType ElType;
     typedef Insert < AccessIterator < ElType >::Handle,
                     TreeIterator < ElType , leaf_or_has_level < ElType > > > IteratorType;

     IteratorType it_;
     typedef IteratorType :: val_t val_t;
     public:
     template <class GridImp>
     ALU3dGridLeafIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid().container(),level) {}

     int size  ()    { return it_.size(); }
     void next ()    { it_.next(); }
     void first()    { it_.first(); }
     int done ()     { return it_.done(); }
     val_t & item () { return it_.item(); }

     };

     template <>
     class ALU3dGridLeafIteratorWrapper<3> : public IteratorWrapperInterface
     {
     typedef LeafIterator < GitterType::vertex_STI > IteratorType;
     IteratorType it_;
     typedef IteratorType :: val_t val_t;
     public:
     template <class GridImp>
     ALU3dGridLeafIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid()) {}

     int size  ()    { return it_->size(); }
     void next ()    { it_->next(); }
     void first()    { it_->first(); }
     int done ()     { return it_->done(); }
     val_t & item () { return it_->item(); }

     };
   */

#ifdef _ALU3DGRID_PARALLEL_

  class LeafLevelIteratorTT
  {
    typedef ALUHElementType<1>::ElementType ElType;

    typedef Insert < AccessIteratorTT < ElType > :: InnerHandle,
        TreeIterator < ElType, leaf_minus_one < ElType > > > InnerIteratorType;

    typedef Insert < AccessIteratorTT < ElType > :: OuterHandle,
        TreeIterator < ElType, leaf_minus_one < ElType > > > OuterIteratorType;

    typedef IteratorSTI < ElType > IteratorType;

    AccessIteratorTT < ElType > :: InnerHandle mif_;
    AccessIteratorTT < ElType > :: OuterHandle mof_;

    InnerIteratorType wi_;
    OuterIteratorType wo_;
  public:

    LeafLevelIteratorTT( GitterImplType & gitter , int link , int level )
      : mif_ (gitter.containerPll (),link) , mof_ (gitter.containerPll (),link)
        , wi_ (mif_)
        , wo_ (mof_)
    {}

    IteratorType & inner () { return wi_; }
    IteratorType & outer () { return wo_; }
  };

  template <>
  class ALU3dGridLeafIteratorWrapper<0,Dune::Ghost_Partition>
    : public IteratorWrapperInterface< LeafValType >
  {
    GitterImplType & gitter_;

    typedef ALUHElementType<1>::ElementType ElType;
  private:


    //typedef LeafIteratorTT < ElType > IteratorType;
    typedef LeafLevelIteratorTT IteratorType;
    IteratorType * iterTT_;

    typedef IteratorSTI < ElType > InnerIteratorType;
    InnerIteratorType * it_;

    //typedef InnerIteratorType :: val_t val_t;

    typedef LeafValType val_t;

    // number of links
    const int nl_;

    // current link
    int link_;

    const int levelMinusOne_;

    // the pair of elementand boundary face
    val_t elem_;
  public:
    typedef ElementPllXIF_t ItemType;

    template <class GridImp>
    ALU3dGridLeafIteratorWrapper (const GridImp & grid, int level , const int nlinks )
      : gitter_(const_cast<GridImp &> (grid).myGrid()) , iterTT_(0) , it_(0), nl_(nlinks) , link_(0) , levelMinusOne_ ( (level > 0) ? (level-1) : 0)
    {
      elem_.first  = 0;
      elem_.second = 0;
      createIterator();
    }

    ~ALU3dGridLeafIteratorWrapper ()
    {
      if(iterTT_) delete iterTT_;
    }

    void createIterator()
    {
      if(link_ < nl_)
      {
        if(iterTT_) delete iterTT_;iterTT_ = 0;
        iterTT_ = new IteratorType ( gitter_, link_, levelMinusOne_ );
        assert(iterTT_);
        checkInnerOuter();
      }
      else
      {
        if(iterTT_) delete iterTT_;
        iterTT_ = 0;
        it_ = 0;
      }
    }

    void checkInnerOuter()
    {
      assert(iterTT_);
      it_ = &( iterTT_->inner() );
      InnerIteratorType & it = iterTT_->inner();
      it.first();

      if(!it.done())
      {
        pair < ElementPllXIF_t *, int > p = it.item ().accessPllX ().accessOuterPllX () ;
        pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > elems;
        p.first->getAttachedElement(elems);

        assert( elems.first || elems.second );

        if(elems.second)
        {
          return;
        }
      }

      InnerIteratorType & out = iterTT_->outer();
      out.first();
      if(!out.done())
      {
        pair < ElementPllXIF_t *, int > p = out.item ().accessPllX ().accessOuterPllX () ;
        pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > elems;
        p.first->getAttachedElement(elems);

        assert( elems.second );
        it_ = &out;
        return ;
      }

      assert(false);
      it_ = 0;
    }

    int size  ()
    {
      if(!it_)
      {
        assert(false);
        return 0;
      }
      return it_->size();
    }

    void checkLevel ()
    {
      if(it_)
      {
        if(!it_->done())
        {
          val_t & el = item();
          BNDFace3Type * face = el.second;
          assert( face );

          if( face->leaf() )
          {
            // if the ghost is not used, go to next ghost
            if(face->ghostLevel() != face->level() )
              next () ;
          }
          else
          {
            BNDFace3Type * dwn = dynamic_cast<BNDFace3Type * > (face->down());
            assert( dwn );
            // if owr child is ok then we go to the children
            if(dwn->ghostLevel() == dwn->level())
              next();
          }
        }
      }
    }

    // go next ghost
    void next ()
    {
      if(it_)
      {
        it_->next();
      }
      if(it_->done())
      {
        link_++;
        createIterator();
      }

      checkLevel();
      // if we still have iterator
    }

    void first() {
      link_ = 0;
      createIterator();
      if(it_) it_->first();
      checkLevel();
    }

    int done ()
    {
      if(link_ >= nl_ ) return 1;
      return ((it_) ? it_->done() : 1);
    }

    val_t & item ()
    {
      assert(it_);
      pair < ElementPllXIF_t *, int > p = it_->item ().accessPllX ().accessOuterPllX () ;
      pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > p2;
      p.first->getAttachedElement(p2);
      assert(p2.second);
      elem_.second = dynamic_cast< BNDFace3Type * > (p2.second);
      return elem_;
    }
  };

  // the all partition iterator
  template <>
  class ALU3dGridLeafIteratorWrapper<0,Dune::All_Partition>
    : public IteratorWrapperInterface< LeafValType >
  {
    ALU3dGridLeafIteratorWrapper<0,Dune::InteriorBorder_Partition> interior_;
    ALU3dGridLeafIteratorWrapper<0,Dune::Ghost_Partition> ghosts_;

    typedef LeafValType val_t;

    bool useInterior_;

  public:
    typedef ElementPllXIF_t ItemType;

    template <class GridImp>
    ALU3dGridLeafIteratorWrapper (const GridImp & grid, int level , const int nlinks )
      : interior_ ( grid, level )
        , ghosts_ ( grid, level, nlinks ) , useInterior_(true) {}

    int size  ()
    {
      return interior_.size() + ghosts_.size();
    }

    void next ()
    {
      if(useInterior_)
      {
        interior_.next();
        if(interior_.done())
        {
          useInterior_ = false;
          ghosts_.first();
        }
      }
      else
      {
        ghosts_.next();
      }
    }

    void first() {
      useInterior_ = true;
      interior_.first();
    }

    int done ()
    {
      if( useInterior_) return 0;
      return ghosts_.done();
    }

    val_t & item ()
    {
      if(useInterior_)
        return interior_.item();
      else
        return ghosts_.item();
    }
  };

#endif

  //*************************************************************
  typedef LeafIterator < GitterType::helement_STI > BSLeafIteratorMaxLevel;

} //end namespace BernhardSchuppGrid

#endif
