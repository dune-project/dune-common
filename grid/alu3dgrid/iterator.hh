// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGRIDITERATOR_HH
#define DUNE_ALU3DGRIDITERATOR_HH

// System includes

// Dune includes
#include "../common/grid.hh"

// Local includes
#include "alu3dinclude.hh"
#include "topology.hh"
#include "myautoptr.hh"

// using directives

namespace ALU3dGridSpace {
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
  //  typedef pair <ALUHElementType<0>::ElementType * , BNDFace3Type * > LeafValType;
  typedef pair <ALUHElementType<0>::ElementType * , HBndSegType * > LeafValType;
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

  typedef LeafIterator < GitterType::helement_STI > BSLeafIteratorMaxLevel;

} //end namespace ALU3dGrid


namespace Dune {
  // Forward declarations
  template<int cd, int dim, class GridImp>
  class ALU3dGridEntity;
  template<int cd, PartitionIteratorType pitype, class GridImp >
  class ALU3dGridLevelIterator;
  template<int cd, class GridImp >
  class ALU3dGridEntityPointer;
  template<int mydim, int coorddim, class GridImp>
  class ALU3dGridGeometry;
  template<class GridImp>
  class ALU3dGridBoundaryEntity;
  template<class GridImp>
  class ALU3dGridHierarchicIterator;
  template<class GridImp>
  class ALU3dGridIntersectionIterator;
  template<class GridImp>
  class ALU3dGridLeafIterator;
  template<int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGrid;
  template <ALU3dGridElementType type>
  class ALU3dGridFaceInfo;
  template <class GridImp>
  class ALU3dGridFaceGeometryInfo;

  //**********************************************************************
  //
  // --ALU3dGridIntersectionIterator
  // --IntersectionIterator
  /*!
     Mesh entities of codimension 0 ("elements") allow to visit all neighbors, wh
     a neighbor is an entity of codimension 0 which has a common entity of codimens
     These neighbors are accessed via a IntersectionIterator. This allows the implement
     non-matching meshes. The number of neigbors may be different from the number o
     of an element!
   */
  template<class GridImp>
  class ALU3dGridIntersectionIterator :
    public ALU3dGridEntityPointer <0,GridImp> ,
    public IntersectionIteratorDefault <GridImp,ALU3dGridIntersectionIterator>
  {
    enum { dim       = GridImp::dimension };
    enum { dimworld  = GridImp::dimensionworld };

    typedef typename ALU3dImplTraits<GridImp::elementType> ImplTraits;
    typedef typename ImplTraits::GEOElementType GEOElementType;
    typedef typename ImplTraits::GEOFaceType GEOFaceType;
    typedef typename ImplTraits::NeighbourPairType NeighbourPairType;
    typedef typename ImplTraits::PLLBndFaceType PLLBndFaceType;
    typedef typename ImplTraits::BNDFaceType BNDFaceType;

    typedef typename ALU3dGridFaceInfo<GridImp::elementType> FaceInfoType;
    typedef typename std::auto_ptr<FaceInfoType> FaceInfoPointer;

    typedef typename ALU3dGridFaceGeometryInfo<GridImp> GeometryInfoType;
    typedef typename ElementTopologyMapping<GridImp::elementType> ElementTopo;
    typedef typename FaceTopologyMapping<GridImp::elementType> FaceTopo;

    enum { numFaces = EntityCount<GridImp::elementType>::numFaces };
    enum { numVerticesPerFace =
             EntityCount<GridImp::elementType>::numVerticesPerFace };
    enum { numVertices = EntityCount<GridImp::elementType>::numVertices };

    friend class ALU3dGridEntity<0,dim,GridImp>;

  public:
    typedef typename GridImp::template codim<0>::Entity Entity;
    typedef typename GridImp::template codim<0>::BoundaryEntity BoundaryEntity;
    typedef ALU3dGridMakeableBoundaryEntity<GridImp> MakeableBndEntityImp;
    typedef typename GridImp::template codim<1>::Geometry Geometry;
    typedef typename GridImp::template codim<1>::LocalGeometry LocalGeometry;
    typedef ALU3dGridMakeableEntity<0,dim,GridImp> EntityImp;
    typedef ALU3dGridMakeableGeometry<dim-1,dimworld,GridImp> GeometryImp;
    typedef ALU3dGridMakeableGeometry<dim-1,dimworld,GridImp> LocalGeometryImp;
    typedef FieldVector<alu3d_ctype, dimworld> NormalType;

    //! The default Constructor , level tells on which level we want
    //! neighbours
    ALU3dGridIntersectionIterator(const GridImp & grid,
                                  ALU3DSPACE HElementType *el,
                                  int wLevel,bool end=false);

    //! The copy constructor
    ALU3dGridIntersectionIterator(const ALU3dGridIntersectionIterator<GridImp> & org);

    //! The Destructor
    ~ALU3dGridIntersectionIterator();

    //! increment iterator
    void increment ();

    //! equality
    bool equals(const ALU3dGridIntersectionIterator<GridImp> & i) const;

    //! access neighbor, dereferencing
    Entity & dereference () const;

    //! return true if intersection is with boundary. \todo connection with
    //! boundary information, processor/outer boundary
    bool boundary () const;

    //! return true if across the edge an neighbor on this level exists
    bool neighbor () const;

    //! return information about the Boundary
    const BoundaryEntity & boundaryEntity () const;

    //! intersection of codimension 1 of this neighbor with element where
    //! iteration started.
    //! Here returned element is in LOCAL coordinates of the element
    //! where iteration started.
    const LocalGeometry & intersectionSelfLocal () const;

    //! intersection of codimension 1 of this neighbor with element where
    //!  iteration started.
    //! Here returned element is in GLOBAL coordinates of the element where
    //! iteration started.
    const Geometry & intersectionGlobal () const;

    //! local number of codim 1 entity in self where intersection is contained
    //!  in
    int numberInSelf () const;

    //! intersection of codimension 1 of this neighbor with element where
    //! iteration started.
    //! Here returned element is in LOCAL coordinates of neighbor
    const LocalGeometry & intersectionNeighborLocal () const;

    //! local number of codim 1 entity in neighbor where intersection is
    //! contained
    int numberInNeighbor () const;

    //! return unit outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    NormalType unitOuterNormal (const FieldVector<alu3d_ctype, dim-1>& local) const ;

    //! return outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    NormalType outerNormal (const FieldVector<alu3d_ctype, dim-1>& local) const;

    //! return outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    NormalType integrationOuterNormal (const FieldVector<alu3d_ctype, dim-1>& local) const;

  private:
    void outputElementInfo() const;

    void outputFaceInfo() const;

    template <typename T>
    void printToScreen(int duneIdx, int aluIdx,
                       const T& info) const;

    void printToScreen(int duneIdx, int aluIdx) const;

    NormalType convert2FV(const alu3d_ctype (&p)[3]) const;

    // reset IntersectionIterator to first neighbour
    void first(ALU3DSPACE HElementType & elem, int wLevel);

    // set new face
    void setNewFace(const GEOFaceType& newFace);

    // is there a refined element at the outer side of the face which needs to be considered when incrementing the iterator?
    bool canGoDown(const GEOFaceType& nextFace) const;

    // init the geometry provider
    void initGeometryProvider() const;

    // get the face corresponding to the index
    const typename ALU3dImplTraits<tetra>::GEOFaceType*
    getFace(const ALU3DSPACE GEOTetraElementType& elem, int index) const;

    const typename ALU3dImplTraits<hexa>::GEOFaceType*
    getFace(const ALU3DSPACE GEOHexaElementType& elem, int index) const;

    //! structure containing the topological and geometrical information about
    //! the face which the iterator points to
    mutable FaceInfoPointer connector_;
    mutable GeometryInfoType* geoProvider_; // need to initialise

    //! current element from which we started the intersection iterator
    mutable GEOElementType* item_;
    mutable MakeableBndEntityImp* bndEntity_;

    const int nFaces_;
    const int walkLevel_;
    mutable int index_;
  };


  //**********************************************************************
  //
  // --ALU3dGridLevelIterator
  // --LevelIterator
  /*!
     Enables iteration over all entities of a given codimension and level of a grid.
   */
  template<int cd, PartitionIteratorType pitype, class GridImp>
  class ALU3dGridLevelIterator :
    public ALU3dGridEntityPointer <cd,GridImp> ,
    public LevelIteratorDefault <cd,pitype,GridImp,ALU3dGridLevelIterator>
  {
    enum { dim       = GridImp::dimension };
    enum { dimworld  = GridImp::dimensionworld };

    friend class ALU3dGridEntity<3,dim,GridImp>;
    friend class ALU3dGridEntity<2,dim,GridImp>;
    friend class ALU3dGridEntity<1,dim,GridImp>;
    friend class ALU3dGridEntity<0,dim,GridImp>;
    friend class ALU3dGrid < dim , dimworld, GridImp::elementType >;

  public:
    typedef typename GridImp::template codim<cd>::Entity Entity;

    typedef ALU3dGridMakeableEntity<cd,dim,GridImp> EntityImp;

    //! typedef of my type
    typedef ALU3dGridLevelIterator<cd,pitype,GridImp> ALU3dGridLevelIteratorType;

    //! Constructor
    ALU3dGridLevelIterator(const GridImp & grid, int level , bool end=false);

    //! Constructor
    ALU3dGridLevelIterator(const ALU3dGridLevelIterator<cd,pitype,GridImp> & org);

    //! prefix increment
    void increment ();

  private:
    //! return reference to EntityPointers entity_
    EntityImp & myEntity () { return (*(this->entity_)); }

    // element index, -1 for end
    int index_;

    // actual level
    int level_;

    // the wrapper for the original iterator of the ALU3dGrid
    typedef typename ALU3DSPACE ALU3dGridLevelIteratorWrapper<cd> IteratorType;
    ALU3dGridSpace::AutoPointer< IteratorType > iter_;
  };

  //********************************************************************
  //
  //  --ALU3dGridLeafIterator
  //  --LeafIterator
  //
  //********************************************************************
  template<class GridImp>
  class ALU3dGridLeafIterator :
    public ALU3dGridEntityPointer<0,GridImp>
  {
    enum { dim = GridImp :: dimension };

    friend class ALU3dGridEntity<0,dim,GridImp>;
    //friend class ALU3dGrid < dim , dimworld >;
    enum { codim = 0 };

  public:
    typedef typename GridImp::template codim<0>::Entity Entity;
    typedef ALU3dGridMakeableEntity<0,dim,GridImp> EntityImp;

    typedef ALU3dGridLeafIterator<GridImp> ALU3dGridLeafIteratorType;

    //! Constructor
    ALU3dGridLeafIterator(const GridImp & grid, int level , bool end,
                          const int nlinks , PartitionIteratorType pitype );

    //! copy Constructor
    ALU3dGridLeafIterator(const ALU3dGridLeafIterator<GridImp> & org);

    //! prefix increment
    void increment ();

  private:
    //! return reference to EntityPointers entity_
    EntityImp & myEntity () { return (*(this->entity_)); }

    // element index, -1 for end
    int index_;

    // actual level
    int level_;

    // the wrapper for the original iterator of the ALU3dGrid
    typedef typename ALU3DSPACE ALU3dGridLeafIteratorWrapper<codim,InteriorBorder_Partition> IteratorType;

    typedef ALU3DSPACE IteratorWrapperInterface<ALU3DSPACE LeafValType> IterInterface;
    ALU3DSPACE AutoPointer < IterInterface > iter_;

    //! my partition tpye
    const PartitionIteratorType pitype_;
  };

  // - HierarchicIteraor
  template<class GridImp>
  class ALU3dGridHierarchicIterator :
    public ALU3dGridEntityPointer<0,GridImp> ,
    public HierarchicIteratorDefault <GridImp,ALU3dGridHierarchicIterator>
  {
    enum { dim = GridImp::dimension };
  public:
    typedef typename GridImp::template codim<0>::Entity Entity;
    typedef typename GridImp::ctype ctype;
    typedef ALU3dGridMakeableEntity<0,dim,GridImp> EntityImp;

    //! the normal Constructor
    ALU3dGridHierarchicIterator(const GridImp &grid,
                                const ALU3DSPACE HElementType & elem, int maxlevel, bool end=false);

    //! the normal Constructor
    ALU3dGridHierarchicIterator(const ALU3dGridHierarchicIterator<GridImp> &org);

    //! the Destructor
    //~ALU3dGridHierarchicIterator();

    //! increment
    void increment();

    /*
       //! equality
       bool equals (const ALU3dGridHierarchicIterator<GridImp>& i) const;

       //! dereferencing
       Entity & dereference() const;
     */

  private:
    //! return reference to EntityPointers entity_
    EntityImp & myEntity () { return (*(this->entity_)); }

    // go to next valid element
    ALU3DSPACE HElementType * goNextElement (ALU3DSPACE HElementType * oldEl);

    //! element from where we started
    const ALU3DSPACE HElementType & elem_;

    //! the actual element of this iterator
    ALU3DSPACE HElementType * item_;

    //! maximal level to go down
    int maxlevel_;
  };


} // end namespace Dune

#include "iterator_imp.cc"

#endif
