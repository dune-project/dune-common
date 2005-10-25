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

namespace ALUGridSpace {
  //*************************************************************
  //  definition of original LeafIterators of ALUGrid
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

  // the element level iterator
  template <>
  class ALU3dGridLevelIteratorWrapper<0>
  {
    typedef ALUHElementType<0>::ElementType ElType;
    typedef ALU3DSPACE LevelIterator < ElType > IteratorType;

    // the iterator
    IteratorType it_;
  public:
    typedef IteratorType :: val_t val_t;
    template <class GridImp>
    ALU3dGridLevelIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid() , level ) {}

    int size  ()    { return it_->size(); }
    void next ()    { it_->next();  }
    void first()    { it_->first(); }
    int done ()     { return it_->done(); }
    val_t & item () {
      assert( ! done () );
      return it_->item();
    }
  };

  // the face level iterator
  template <>
  class ALU3dGridLevelIteratorWrapper<1>
  {
    typedef ALUHElementType<1>::ElementType ElType;
    typedef ALU3DSPACE LevelIterator < ElType > IteratorType;

    // the iterator
    IteratorType it_;
  public:
    typedef IteratorType :: val_t val_t;
    template <class GridImp>
    ALU3dGridLevelIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid() , level ) {}

    int size  ()    { return it_->size(); }
    void next ()    { it_->next();  }
    void first()    { it_->first(); }
    int done ()     { return it_->done(); }
    val_t & item () {
      assert( ! done () );
      return it_->item();
    }
  };

  // the edge level iterator
  template <>
  class ALU3dGridLevelIteratorWrapper<2>
  {
    typedef ALUHElementType<2>::ElementType ElType;
    typedef ALU3DSPACE LevelIterator < ElType > IteratorType;

    IteratorType it_;
  public:
    typedef IteratorType :: val_t val_t;
    template <class GridImp>
    ALU3dGridLevelIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid() , level ) {}

    int size  ()    { return it_->size(); }
    void next ()    { it_->next();  }
    void first()    { it_->first(); }
    int done ()     { return it_->done(); }
    val_t & item () {
      assert( ! done () );
      return it_->item();
    }
  };

  // the vertex level iterator, little bit different to the others
  // this implementation uses the vertex leaf iterator and runs over all
  // vertices with level <= the given iteration level
  template <>
  class ALU3dGridLevelIteratorWrapper<3>
  {
    typedef LeafIterator < GitterType::vertex_STI > IteratorType;

    // the underlying iterator
    IteratorType it_;

    // level to walk
    const int level_;
  public:
    typedef IteratorType :: val_t val_t;
    template <class GridImp>
    ALU3dGridLevelIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid()),
        level_(level)  {}

    // returns size of leaf iterator, wrong here, return leaf size
    int size  ()  { return it_->size(); }

    //! if level of item is larger then walk level, go next
    void next ()
    {
      it_->next();
      if(it_->done()) return ;
      // go to next vertex with level <= level_
      while ( it_->item().level() > level_ )
      {
        this->next();
        if(it_->done()) return ;
      }
      return ;
    }

    void first()    { it_->first(); }
    int done () const { return it_->done(); }
    val_t & item () {
      assert( ! done () );
      return it_->item();
    }
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

  // defines the pair of element and boundary
  template <int codim>
  struct IteratorElType
  {
    typedef typename ALUHElementType<codim>::ElementType ElType;
    typedef pair < ElType * , HBndSegType * > val_t;
  };

  template <int codim, PartitionIteratorType pitype> class ALU3dGridLeafIteratorWrapper;
  typedef pair <ALUHElementType<0>::ElementType * , HBndSegType * > LeafValType;
  typedef IteratorWrapperInterface<LeafValType> IteratorWrapperInterfaceType;

  //**********************************************************
  //  LeafIterator Wrapper
  //**********************************************************
  template <PartitionIteratorType pitype>
  class ALU3dGridLeafIteratorWrapper<0,pitype>
    : public IteratorWrapperInterface< typename IteratorElType<0>::val_t >
  {
    // type is helement_STI
    typedef IteratorElType<0>::ElType ElType;
    typedef Insert < AccessIterator < ElType >::Handle,
        TreeIterator < ElType , leaf_or_has_level < ElType > > > IteratorType;

    // the ALU3dGrid Iterator
    IteratorType it_;

  public:
    typedef typename IteratorElType<0>::val_t val_t;
  private:
    val_t elem_;
  public:
    template <class GridImp>
    ALU3dGridLeafIteratorWrapper (const GridImp & grid, int level, const int links )
      : it_(const_cast<GridImp &> (grid).myGrid().container(),level), elem_(0,0) {}

    int size  ()    { return it_.size(); }
    void next ()    { it_.next(); }
    void first()    { it_.first(); }
    int done ()     { return it_.done(); }
    val_t & item ()
    {
      assert( ! done () );
      elem_.first  = & it_.item();
      return elem_;
    }
  };


  template <PartitionIteratorType pitype>
  class ALU3dGridLeafIteratorWrapper<1,pitype>
    : public IteratorWrapperInterface < typename IteratorElType<1>::val_t >
  {
    // type is hface_STI
    typedef IteratorElType<1>::ElType ElType;
    typedef LeafIterator < ElType > IteratorType;

    // the face iterator
    IteratorType it_;

  public:
    typedef IteratorElType<1>::val_t val_t;
  private:
    val_t elem_;
  public:
    template <class GridImp>
    ALU3dGridLeafIteratorWrapper (const GridImp & grid, int level, const int links )
      : it_(const_cast<GridImp &> (grid).myGrid()) , elem_(0,0) {}

    int size  ()    { return it_->size(); }
    void next ()    { it_->next(); }
    void first()    { it_->first(); }
    int done ()     { return it_->done(); }
    val_t & item ()
    {
      assert( ! done () );
      elem_.first  = & it_->item();
      return elem_;
    }
  };

  template <PartitionIteratorType pitype>
  class ALU3dGridLeafIteratorWrapper<2,pitype>
    : public IteratorWrapperInterface < typename IteratorElType<2>::val_t >
  {
    // type of hedge_STI
    typedef IteratorElType<2>::ElType ElType;
    typedef LeafIterator < ElType > IteratorType;

    // the edge iterator
    IteratorType it_;

  public:
    typedef IteratorElType<2>::val_t val_t;
  private:
    val_t elem_;
  public:
    template <class GridImp>
    ALU3dGridLeafIteratorWrapper (const GridImp & grid, int level, const int links )
      : it_(const_cast<GridImp &> (grid).myGrid()), elem_(0,0) {}

    int size  ()    { return it_->size(); }
    void next ()    { it_->next(); }
    void first()    { it_->first(); }
    int done ()     { return it_->done(); }
    val_t & item ()
    {
      assert( ! done () );
      elem_.first  = & it_->item();
      return elem_;
    }
  };

  template <PartitionIteratorType pitype>
  class ALU3dGridLeafIteratorWrapper<3,pitype>
    : public IteratorWrapperInterface < typename IteratorElType<3>::val_t >
  {
    typedef LeafIterator < GitterType::vertex_STI > IteratorType;

    // the vertex iterator
    IteratorType it_;

  public:
    typedef IteratorElType<3>::val_t val_t;
  private:
    val_t elem_;
  public:
    template <class GridImp>
    ALU3dGridLeafIteratorWrapper (const GridImp & grid, int level, const int links )
      : it_(const_cast<GridImp &> (grid).myGrid()), elem_(0,0) {}

    int size  ()    { return it_->size(); }
    void next ()    { it_->next(); }
    void first()    { it_->first(); }
    int done ()     { return it_->done(); }
    val_t & item ()
    {
      assert( ! done () );
      elem_.first  = & it_->item();
      return elem_;
    }
  };

#ifdef _ALU3DGRID_PARALLEL_

  class LeafLevelIteratorTT
  {
    typedef ALUHElementType<1>::ElementType ElType;

    typedef leaf_or_father_of_leaf < ElType > StopRule_t;

    typedef Insert < AccessIteratorTT < ElType > :: InnerHandle,
        TreeIterator < ElType, StopRule_t > > InnerIteratorType;

    typedef Insert < AccessIteratorTT < ElType > :: OuterHandle,
        TreeIterator < ElType, StopRule_t > > OuterIteratorType;

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

  //****************************
  //
  //  --GhostIterator
  //
  //****************************
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


    // number of links
    const int nl_;

    // current link
    int link_;

    const int levelMinusOne_;

  public:
    typedef LeafValType val_t;
  private:
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
        pair < HElementType * , HBndSegType * > elems;
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
        pair < HElementType * , HBndSegType * > elems;
        p.first->getAttachedElement(elems);

        assert( elems.second );
        it_ = &out;
        return ;
      }

      it_ = 0;
    }

    int size  ()
    {
      // if no iterator then size is zero
      // which can happen in the case of parallel grid with 1 processor
      if(!it_)
      {
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
          HBndSegType * face = el.second;
          assert( face );

          if( face->leaf() )
          {
            // if the ghost is not used, go to next ghost
            if(face->ghostLevel() != face->level() )
              next () ;
          }
          else
          {
            HBndSegType * dwn = face->down();
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

    void first()
    {
      link_ = 0;
      createIterator();
      if(it_)
      {
        it_->first();
        checkLevel();
      }
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
      pair < HElementType  * , HBndSegType * > p2;
      p.first->getAttachedElement(p2);
      assert(p2.second);
      elem_.second = p2.second;
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
      : interior_ ( grid, level , nlinks )
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
      if(useInterior_) return 0;
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
  typedef PureElementLeafIterator < GitterType::helement_STI > BSLeafIteratorMaxLevel;

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
  class ALU3dGridHierarchicIterator;
  template<class GridImp>
  class ALU3dGridIntersectionIterator;
  template<int codim, PartitionIteratorType pitype, class GridImp>
  class ALU3dGridLeafIterator;
  template<int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGrid;
  template <ALU3dGridElementType type>
  class ALU3dGridFaceInfo;
  template <ALU3dGridElementType elType>
  class ALU3dGridGeometricFaceInfo;

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

    typedef ALU3dImplTraits<GridImp::elementType> ImplTraits;
    typedef typename ImplTraits::GEOElementType GEOElementType;
    typedef typename ImplTraits::GEOFaceType GEOFaceType;
    typedef typename ImplTraits::NeighbourPairType NeighbourPairType;
    typedef typename ImplTraits::PLLBndFaceType PLLBndFaceType;
    typedef typename ImplTraits::BNDFaceType BNDFaceType;

    typedef ALU3dGridFaceInfo<GridImp::elementType> FaceInfoType;
    typedef typename std::auto_ptr<FaceInfoType> FaceInfoPointer;

    typedef ALU3dGridGeometricFaceInfo<GridImp::elementType> GeometryInfoType;
    typedef ElementTopologyMapping<GridImp::elementType> ElementTopo;
    typedef FaceTopologyMapping<GridImp::elementType> FaceTopo;

    enum { numFaces = EntityCount<GridImp::elementType>::numFaces };
    enum { numVerticesPerFace =
             EntityCount<GridImp::elementType>::numVerticesPerFace };
    enum { numVertices = EntityCount<GridImp::elementType>::numVertices };

    friend class ALU3dGridEntity<0,dim,GridImp>;

  public:
    typedef typename GridImp::template Codim<0>::Entity Entity;
    typedef typename GridImp::template Codim<1>::Geometry Geometry;
    typedef typename GridImp::template Codim<1>::LocalGeometry LocalGeometry;
    typedef ALU3dGridMakeableEntity<0,dim,GridImp> EntityImp;
    typedef ALU3dGridMakeableGeometry<dim-1,dimworld,GridImp> GeometryImp;
    typedef ALU3dGridMakeableGeometry<dim-1,dimworld,GridImp> LocalGeometryImp;
    typedef FieldVector<alu3d_ctype, dimworld> NormalType;
    typedef ALU3dGridEntityPointer<0,GridImp> EntityPointer;

    //! The default Constructor , level tells on which level we want
    //! neighbours
    ALU3dGridIntersectionIterator(const GridImp & grid,
                                  ALU3DSPACE HElementType *el,
                                  int wLevel,bool end=false);

    //! The copy constructor
    ALU3dGridIntersectionIterator(const ALU3dGridIntersectionIterator<GridImp> & org);

    //! The Destructor
    ~ALU3dGridIntersectionIterator();

    //! The copy constructor
    bool equals (const ALU3dGridIntersectionIterator<GridImp> & i) const;

    //! increment iterator
    void increment ();

    //! equality
    //bool equals(const ALU3dGridIntersectionIterator<GridImp> & i) const;

    //! access neighbor
    EntityPointer outside() const;

    //! access entity where iteration started
    EntityPointer inside() const;

    //! return true if intersection is with boundary. \todo connection with
    //! boundary information, processor/outer boundary
    bool boundary () const;

    //! return true if across the edge an neighbor on this level exists
    bool neighbor () const;

    //! return information about the Boundary
    int boundaryId () const;

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

    void initGeometryProvider() const;

    void buildLocalGeometries() const;

    void buildGlobalGeometry() const;

    // get the face corresponding to the index
    const typename ALU3dImplTraits<tetra>::GEOFaceType*
    getFace(const ALU3DSPACE GEOTetraElementType& elem, int index) const;

    const typename ALU3dImplTraits<hexa>::GEOFaceType*
    getFace(const ALU3DSPACE GEOHexaElementType& elem, int index) const;

    //! structure containing the topological and geometrical information about
    //! the face which the iterator points to
    mutable FaceInfoPointer connector_;
    mutable GeometryInfoType* geoProvider_; // need to initialise

    mutable GeometryImp* intersectionGlobal_;
    mutable GeometryImp* intersectionSelfLocal_;
    mutable GeometryImp* intersectionNeighborLocal_;

    //! current element from which we started the intersection iterator
    mutable GEOElementType* item_;

    const int nFaces_;
    const int walkLevel_;
    mutable int index_;

    mutable bool generatedGlobalGeometry_;
    mutable bool generatedLocalGeometries_;
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
    typedef typename GridImp::template Codim<cd>::Entity Entity;

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
    //! do not allow assigment
    ALU3dGridLevelIterator<cd, pitype, GridImp> & operator = (const ALU3dGridLevelIterator<cd, pitype, GridImp> & org)  { return *this; }

    //! return reference to EntityPointers entity_
    EntityImp & myEntity () { return (*(this->entity_)); }

    // element index, -1 for end
    int index_;

    // actual level
    int level_;

    // the wrapper for the original iterator of the ALU3dGrid
    typedef typename ALU3DSPACE ALU3dGridLevelIteratorWrapper<cd> IteratorType;
    ALUGridSpace::AutoPointer< IteratorType > iter_;

    // true if iterator is already a copy
    bool isCopy_;
  };

  //********************************************************************
  //
  //  --ALU3dGridLeafIterator
  //  --LeafIterator
  //
  //********************************************************************
  //! Leaf iterator
  template<int cdim, PartitionIteratorType pitype, class GridImp>
  class ALU3dGridLeafIterator :
    public LeafIteratorDefault<cdim, pitype, GridImp, ALU3dGridLeafIterator>,
    public ALU3dGridEntityPointer<cdim,GridImp>
  {
    enum { dim = GridImp :: dimension };

    friend class ALU3dGridEntity<cdim,dim,GridImp>;
    //friend class ALU3dGrid < dim , dimworld >;
    enum { codim = cdim };

  public:
    typedef typename GridImp::template Codim<cdim>::Entity Entity;
    typedef ALU3dGridMakeableEntity<cdim,dim,GridImp> EntityImp;

    typedef ALU3dGridLeafIterator<cdim, pitype, GridImp> ALU3dGridLeafIteratorType;

    //! Constructor
    ALU3dGridLeafIterator(const GridImp & grid, int level , bool end,
                          const int nlinks);

    //! copy Constructor
    ALU3dGridLeafIterator(const ALU3dGridLeafIterator<cdim, pitype, GridImp> & org);

    //! prefix increment
    void increment ();

  private:
    //! do not allow assigment
    ALU3dGridLeafIterator<cdim, pitype, GridImp> & operator = (const ALU3dGridLeafIterator<cdim, pitype, GridImp> & org)  { return *this; }

    //! return reference to EntityPointers entity_
    EntityImp & myEntity () { return (*(this->entity_)); }

    // element index, -1 for end
    int index_;

    // actual level
    int level_;

    // the wrapper for the original iterator of the ALU3dGrid
    typedef typename ALU3DSPACE ALU3dGridLeafIteratorWrapper<cdim, pitype> IteratorType;

    //typedef typename ALU3DSPACE IteratorType :: val_t val_t;
    typedef typename ALU3DSPACE IteratorElType<cdim>::val_t val_t;
    typedef ALU3DSPACE IteratorWrapperInterface<val_t> IterInterface;
    ALU3DSPACE AutoPointer < IterInterface > iter_;

    // true if iterator is already a copy
    bool isCopy_;
  };

  // - HierarchicIteraor
  template<class GridImp>
  class ALU3dGridHierarchicIterator :
    public ALU3dGridEntityPointer<0,GridImp> ,
    public HierarchicIteratorDefault <GridImp,ALU3dGridHierarchicIterator>
  {
    enum { dim = GridImp::dimension };
  public:
    typedef typename GridImp::template Codim<0>::Entity Entity;
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
