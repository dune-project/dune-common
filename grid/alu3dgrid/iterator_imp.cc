// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "geometry.hh"
#include "entity.hh"
#include "grid.hh"
#include "faceutility.hh"

#include <dune/common/misc.hh>

namespace Dune {

  /************************************************************************************
  ###
  #     #    #   #####  ######  #####    ####   ######   ####      #     #####
  #     ##   #     #    #       #    #  #       #       #    #     #       #
  #     # #  #     #    #####   #    #   ####   #####   #          #       #
  #     #  # #     #    #       #####        #  #       #          #       #
  #     #   ##     #    #       #   #   #    #  #       #    #     #       #
  ###    #    #     #    ######  #    #   ####   ######   ####      #       #
  ************************************************************************************/

  // --IntersectionIterator
  template<class GridImp>
  inline ALU3dGridIntersectionIterator<GridImp> ::
  ALU3dGridIntersectionIterator(const GridImp & grid,
                                int wLevel) :
    geoProvider_(connector_),
    grid_(grid),
    item_(0),
    nFaces_(0),
    walkLevel_(wLevel),
    index_(0),
    generatedGlobalGeometry_(false),
    generatedLocalGeometries_(false),
    intersectionGlobal_(GeometryImp()),
    intersectionGlobalImp_(grid_.getRealImplementation(intersectionGlobal_)),
    intersectionSelfLocal_(GeometryImp()),
    intersectionSelfLocalImp_(grid_.getRealImplementation(intersectionSelfLocal_)),
    intersectionNeighborLocal_(GeometryImp()),
    intersectionNeighborLocalImp_(grid_.getRealImplementation(intersectionNeighborLocal_)),
    done_(true)
  {}

  // --IntersectionIterator
  template<class GridImp>
  inline ALU3dGridIntersectionIterator<GridImp> ::
  ALU3dGridIntersectionIterator(const GridImp & grid,
                                ALU3DSPACE HElementType *el,
                                int wLevel,bool end) :
    connector_(),
    geoProvider_(connector_),
    grid_(grid),
    item_(0),
    nFaces_(el ? el->nFaces() : 0),
    walkLevel_(wLevel),
    index_(0),
    generatedGlobalGeometry_(false),
    generatedLocalGeometries_(false),
    intersectionGlobal_(GeometryImp()),
    intersectionGlobalImp_(grid_.getRealImplementation(intersectionGlobal_)),
    intersectionSelfLocal_(GeometryImp()),
    intersectionSelfLocalImp_(grid_.getRealImplementation(intersectionSelfLocal_)),
    intersectionNeighborLocal_(GeometryImp()),
    intersectionNeighborLocalImp_(grid_.getRealImplementation(intersectionNeighborLocal_)),
    done_(end)
  {
    if (!end)
    {
      setFirstItem(*el,wLevel);
    }
    else
    {
      done();
    }
  }

  template<class GridImp>
  inline void
  ALU3dGridIntersectionIterator<GridImp> :: done ()
  {
    done_ = true;
    item_ = 0;
  }

  template<class GridImp>
  inline void ALU3dGridIntersectionIterator<GridImp> ::
  setFirstItem (const ALU3DSPACE HElementType & elem, int wLevel)
  {
    item_      = static_cast<const IMPLElementType *> (&elem);
    walkLevel_ = wLevel;

    // Get first face
    const GEOFaceType* firstFace = getFace(*item_, index_);
    if (canGoDown(*firstFace)) {
      firstFace = firstFace->down();
    }

    // Store the face in the connector
    setNewFace(*firstFace);
  }

  template<class GridImp>
  template <class EntityType>
  inline void ALU3dGridIntersectionIterator<GridImp> ::
  first (const EntityType & en, int wLevel)
  {
    done_   = false;
    nFaces_ = en.getItem().nFaces();
    index_  = 0;
    generatedGlobalGeometry_ = false;
    generatedLocalGeometries_= false;
    setFirstItem(en.getItem(),wLevel);
  }

  // copy constructor
  template<class GridImp>
  inline ALU3dGridIntersectionIterator<GridImp> ::
  ALU3dGridIntersectionIterator(const ALU3dGridIntersectionIterator<GridImp> & org) :
    connector_(org.connector_),
    geoProvider_(connector_),
    grid_(org.grid_),
    item_(org.item_),
    nFaces_(org.nFaces_),
    walkLevel_(org.walkLevel_),
    generatedGlobalGeometry_(false),
    generatedLocalGeometries_(false),
    intersectionGlobal_(GeometryImp()),
    intersectionGlobalImp_(grid_.getRealImplementation(intersectionGlobal_)),
    intersectionSelfLocal_(GeometryImp()),
    intersectionSelfLocalImp_(grid_.getRealImplementation(intersectionSelfLocal_)),
    intersectionNeighborLocal_(GeometryImp()),
    intersectionNeighborLocalImp_(grid_.getRealImplementation(intersectionNeighborLocal_)),
    done_(org.done_)
  {
    if(org.item_) { // else it's a end iterator
      item_           = org.item_;
      index_          = org.index_;
    } else {
      done();
    }
  }

  // copy constructor
  template<class GridImp>
  inline void
  ALU3dGridIntersectionIterator<GridImp> ::
  assign(const ALU3dGridIntersectionIterator<GridImp> & org)
  {
    if(org.item_)
    {
      // else it's a end iterator
      nFaces_    = org.nFaces_;
      walkLevel_ = org.walkLevel_;
      generatedGlobalGeometry_ =false;
      generatedLocalGeometries_=false;
      item_      = org.item_;
      index_     = org.index_;
      connector_.updateFaceInfo(org.connector_.face(),item_->twist(ElementTopo::dune2aluFace(index_)));
      geoProvider_.resetFaceGeom();
    }
    else {
      done();
    }
  }

  // check whether entities are the same or whether iterator is done
  template<class GridImp>
  inline bool ALU3dGridIntersectionIterator<GridImp> ::
  equals (const ALU3dGridIntersectionIterator<GridImp> & i ) const
  {
    // this method is only to check equality of real iterators and end
    // iterators
    return ((item_ == i.item_) &&
            (done_ == i.done_)
            // && (&(connector_.outerEntity()) == &(i.connector_.outerEntity()) )
            );
  }

  template<class GridImp>
  inline void ALU3dGridIntersectionIterator<GridImp> :: increment ()
  {
    assert(item_);

    const GEOFaceType * nextFace = 0;

    // the dune interface gives neighbour on the same level
    // there dont go down anymore, but keep the code
    /*
       // When neighbour element is refined, try to get the next child on the face
       if (connector_.conformanceState() == FaceInfoType::REFINED_OUTER) {
       nextFace = connector_.face().next();

       // There was a next child face...
       if (nextFace) {
        setNewFace(*nextFace);
        return; // we found what we were looking for...
       }
       } // end if
     */

    // Next face number of starting element
    ++index_;

    // When the face number is larger than the number of faces an element
    // can have, we've reached the end...
    if (index_ >= nFaces_) {
      this->done();
      return;
    }

    // ... else we can take the next face
    nextFace = getFace(connector_.innerEntity(), index_);
    assert(nextFace);

    // Check whether we need to go down first
    //if (nextFace has children which need to be visited)
    /*
       if (canGoDown(*nextFace)) {
       nextFace = nextFace->down();
       assert(nextFace);
       }
     */

    setNewFace(*nextFace);
    return;
  }

  template<class GridImp>
  inline typename ALU3dGridIntersectionIterator<GridImp>::EntityPointer
  ALU3dGridIntersectionIterator<GridImp>::outside () const
  {
#ifdef _ALU3DGRID_PARALLEL_
    if(connector_.ghostBoundary())
    {
      const BNDFaceType * ghost = &connector_.boundaryFace();

      // if nonconformity occurs then go up one level
      if( ghost->level () != ghost->ghostLevel() )
        ghost = static_cast<const BNDFaceType *>(ghost->up());

      // set bnd face as ghost
      return EntityPointer(this->grid_, *ghost );
    }
#endif

    assert( &connector_.outerEntity() );
    return EntityPointer(this->grid_, connector_.outerEntity() );
  }

  template<class GridImp>
  inline typename ALU3dGridIntersectionIterator<GridImp>::EntityPointer
  ALU3dGridIntersectionIterator<GridImp>::inside () const {
    return EntityPointer(this->grid_, connector_.innerEntity() );
  }

  template<class GridImp>
  inline bool ALU3dGridIntersectionIterator<GridImp> :: boundary () const
  {
    return (connector_.outerBoundary());
  }

  template<class GridImp>
  inline bool ALU3dGridIntersectionIterator<GridImp>::neighbor () const
  {
    return !(this->boundary());
  }

  template<class GridImp>
  inline int ALU3dGridIntersectionIterator<GridImp>::numberInSelf () const
  {
    assert(ElementTopo::dune2aluFace(index_) ==
           connector_.innerALUFaceIndex());
    return index_;
  }


  template <class GridImp>
  inline const typename ALU3dGridIntersectionIterator<GridImp>::LocalGeometry &
  ALU3dGridIntersectionIterator<GridImp>::intersectionSelfLocal() const {
    buildLocalGeometries();
    return intersectionSelfLocal_;
  }

  template<class GridImp>
  inline int ALU3dGridIntersectionIterator<GridImp>::numberInNeighbor () const
  {
    return ElementTopo::alu2duneFace(connector_.outerALUFaceIndex());
  }

  template<class GridImp>
  inline int ALU3dGridIntersectionIterator<GridImp>::twistInSelf () const
  {
    int aluTwist = connector_.innerTwist();
    int mappedZero =
      FaceTopo::twist(ElementTopo::dune2aluFaceVertex(numberInSelf(), 0),
                      aluTwist);
    return
      (ElementTopo::faceOrientation(numberInSelf())*sign(aluTwist) >= 0 ?
       -mappedZero-1 : mappedZero);
  }

  template<class GridImp>
  inline int ALU3dGridIntersectionIterator<GridImp>::twistInNeighbor () const
  {
    int aluTwist = connector_.innerTwist();
    int mappedZero =
      FaceTopo::twist(ElementTopo::dune2aluFaceVertex(numberInNeighbor(), 0),
                      aluTwist);

    return
      (ElementTopo::faceOrientation(numberInNeighbor())*sign(aluTwist) >= 0 ?
       -mappedZero-1 : mappedZero);
  }

  template <class GridImp>
  inline const typename ALU3dGridIntersectionIterator<GridImp>::LocalGeometry &
  ALU3dGridIntersectionIterator<GridImp>::intersectionNeighborLocal() const {
    assert(neighbor());
    buildLocalGeometries();
    return intersectionNeighborLocal_;
  }

  template<class GridImp>
  inline typename ALU3dGridIntersectionIterator<GridImp>::NormalType &
  ALU3dGridIntersectionIterator<GridImp>::
  integrationOuterNormal(const FieldVector<alu3d_ctype, dim-1>& local) const
  {
    return this->outerNormal(local);
  }

  template<class GridImp>
  inline typename ALU3dGridIntersectionIterator<GridImp>::NormalType &
  ALU3dGridIntersectionIterator<GridImp>::
  outerNormal(const FieldVector<alu3d_ctype, dim-1>& local) const
  {
    assert(item_ != 0);
    return geoProvider_.outerNormal(local);
  }

  template<class GridImp>
  inline typename ALU3dGridIntersectionIterator<GridImp>::NormalType &
  ALU3dGridIntersectionIterator<GridImp>::
  unitOuterNormal(const FieldVector<alu3d_ctype, dim-1>& local) const
  {
    unitOuterNormal_ = this->outerNormal(local);
    unitOuterNormal_ *= (1.0/unitOuterNormal_.two_norm());
    return unitOuterNormal_;
  }

  template<class GridImp>
  inline const typename ALU3dGridIntersectionIterator<GridImp>::Geometry &
  ALU3dGridIntersectionIterator<GridImp>::intersectionGlobal () const
  {
    buildGlobalGeometry();
    return intersectionGlobal_;
  }

  template<class GridImp>
  inline int
  ALU3dGridIntersectionIterator<GridImp>::boundaryId () const
  {
    assert(item_);
    return (boundary() ? connector_.boundaryFace().bndtype() : 0);
  }

  template <class GridImp>
  void ALU3dGridIntersectionIterator<GridImp>::buildGlobalGeometry() const {
    intersectionGlobalImp_.buildGeom(geoProvider_.intersectionGlobal());
    generatedGlobalGeometry_ = true;
  }

  template <class GridImp>
  void ALU3dGridIntersectionIterator<GridImp>::buildLocalGeometries() const {
    intersectionSelfLocalImp_.buildGeom(geoProvider_.intersectionSelfLocal());
    if (!connector_.outerBoundary()) {
      intersectionNeighborLocalImp_.buildGeom(geoProvider_.intersectionNeighborLocal());
    }
    generatedLocalGeometries_ = true;
  }

  template <class GridImp>
  inline const ALU3dImplTraits<tetra>::GEOFaceType*
  ALU3dGridIntersectionIterator<GridImp>::
  getFace(const ALU3DSPACE GEOTetraElementType& elem, int index) const {
    assert(index >= 0 && index < numFaces);
    return elem.myhface3(ElementTopo::dune2aluFace(index));
  }

  template <class GridImp>
  inline const ALU3dImplTraits<hexa>::GEOFaceType*
  ALU3dGridIntersectionIterator<GridImp>::
  getFace(const ALU3DSPACE GEOHexaElementType& elem, int index) const {
    assert(index >= 0 && index < numFaces);
    return elem.myhface4(ElementTopo::dune2aluFace(index));
  }

  template <class GridImp>
  void ALU3dGridIntersectionIterator<GridImp>::
  setNewFace(const GEOFaceType& newFace) {
    connector_.updateFaceInfo(newFace,item_->twist(ElementTopo::dune2aluFace(index_)));
    geoProvider_.resetFaceGeom();
  }

  template <class GridImp>
  bool ALU3dGridIntersectionIterator<GridImp>::
  canGoDown(const GEOFaceType& nextFace) const {
    return (item_->level() < walkLevel_ && item_->leaf() && nextFace.down());
  }

  template <class GridImp>
  void ALU3dGridIntersectionIterator<GridImp>::outputElementInfo() const {
    std::cout << "Starting outputElementInfo\n";
    // output element corner coordinates
    std::cout << "Element corner coordinates" << std::endl;
    for (int i = 0; i < numVertices; ++i) {
      printToScreen(ElementTopo::alu2duneVertex(i), i,
                    convert2FV(item_->myvertex(i)->Point()));
    }

    // output midpoint of faces
    /* too specific for hexa
       std::cout << "Midpoint of faces" << std::endl;
       FieldVector<alu3d_ctype, 2> mid(0.5);
       for (int i = 0; i < numFaces; ++i) {
       NormalType c0 =
        convert2FV(item_->myvertex(i, FaceTopo::dune2aluVertex(0))->Point());
       NormalType c1 =
        convert2FV(item_->myvertex(i, FaceTopo::dune2aluVertex(1))->Point());
       NormalType c2 =
        convert2FV(item_->myvertex(i, FaceTopo::dune2aluVertex(2))->Point());
       NormalType c3 =
        convert2FV(item_->myvertex(i, FaceTopo::dune2aluVertex(3))->Point());

       BilinearSurfaceMapping biMap(c0, c1, c2, c3);
       NormalType result;
       biMap.map2world(mid, result);

       printToScreen(ElementTopo::alu2duneFace(i), i, result);
       } // end for

       // output element reference faces - as calculated in Dune
       std::cout << "Corner indices (from ReferenceTopologySet)" << std::endl;
       for (int i = 0; i < numFaces; ++i) {
       std::cout << "-- Face " << i << "--\n";
       const int* result;
       int n;
       ReferenceTopologySet::getSubEntities< 1, 3 >(hexahedron,
                                                   i,
                                                   result,
                                                   n);
       for (int j = 0; j < n; ++j) {
        printToScreen(result[j], ElementTopo::dune2aluVertex(result[j]), j);
       } // end for
       } // end for
     */
    // output element reference faces - compared with data from Alu
    std::cout << "Corner indices (from ALU)" << std::endl;
    for (int i = 0; i < numFaces; ++i) {
      std::cout << "-- Face " << i << "--\n";
      for (int j = 0; j < numVerticesPerFace; ++j) {
        int aluIdx = GEOElementType::prototype[i][j];
        printToScreen(ElementTopo::alu2duneVertex(aluIdx), aluIdx, j);
      }
    }

    std::cout << "Ending outputElementInfo\n" << std::endl;
  }

  template <class GridImp>
  void ALU3dGridIntersectionIterator<GridImp>::outputFaceInfo() const {
    std::cout << "Starting outputFaceInfo\n";
    // output face index (inner, outer)
    std::cout << "Inner twist" << std::endl;
    printToScreen(index_,
                  connector_.innerALUFaceIndex(),
                  connector_.innerTwist());

    assert(index_ == ElementTopo::alu2duneFace(connector_.innerALUFaceIndex()));

    std::cout << "Outer twist" << std::endl;
    printToScreen(-1,
                  connector_.outerALUFaceIndex(),
                  connector_.outerTwist());

    // output corner coordinates
    std::cout << "Face corner coordinates (ALU face)" << std::endl;
    const GEOFaceType& face = connector_.face();
    for (int i = 0; i < numVerticesPerFace; ++i) {
      printToScreen(FaceTopo::alu2duneVertex(i), i,
                    convert2FV(face.myvertex(i)->Point()));
    }

    std::cout << "Face corner coordinates (intersectionGlobal)" << std::endl;
    const Geometry& interGlobal = intersectionGlobal();
    for (int i = 0; i < numVerticesPerFace; ++i) {
      printToScreen(i, FaceTopo::dune2aluVertex(i), interGlobal[i]);
    }

    std::cout << "Ending outputFaceInfo\n" << std::endl;
  }

  template <class GridImp>
  void ALU3dGridIntersectionIterator<GridImp>::
  printToScreen(int duneIdx, int aluIdx) const {
    printToScreen(duneIdx, aluIdx, "-");
  }

  template <class GridImp>
  template <typename T>
  void ALU3dGridIntersectionIterator<GridImp>::
  printToScreen(int duneIdx, int aluIdx, const T& info) const {
    std::cout << duneIdx << ", " << aluIdx << ": " << info << "\n";
  }

  template <class GridImp>
  typename ALU3dGridIntersectionIterator<GridImp>::NormalType
  ALU3dGridIntersectionIterator<GridImp>::
  convert2FV(const alu3d_ctype (&p)[3]) const {
    NormalType result;
    result[0] = p[0];
    result[1] = p[1];
    result[2] = p[2];
    return result;
  }

  template <class GridImp>
  inline int
  ALU3dGridIntersectionIterator<GridImp>::
  level() const {
    assert( item_ );
    return item_->level();
  }

  /*************************************************************************
  #       ######  #    #  ######  #          #     #####  ######  #####
  #       #       #    #  #       #          #       #    #       #    #
  #       #####   #    #  #####   #          #       #    #####   #    #
  #       #       #    #  #       #          #       #    #       #####
  #       #        #  #   #       #          #       #    #       #   #
  ######  ######    ##    ######  ######     #       #    ######  #    #
  *************************************************************************/
  //--LevelIterator
  // Constructor for begin iterator
  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline ALU3dGridLevelIterator<codim,pitype,GridImp> ::
  ALU3dGridLevelIterator(const GridImp & grid,
                         VertexListType & vxList , int level)
    : ALU3dGridEntityPointer<codim,GridImp> (grid,level)
      , endIter_(false)
      , level_(level)
      , isCopy_ (0)
  {
    IteratorType * it = new IteratorType ( this->grid_ , vxList , level_ );
    iter_.store( it );

    (*iter_).first();
    if(!(*iter_).done())
    {
      assert((*iter_).size() > 0);
      this->updateEntityPointer( & (*iter_).item() );
    }
  }

  // Constructor for end iterator
  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline ALU3dGridLevelIterator<codim,pitype,GridImp> ::
  ALU3dGridLevelIterator(const GridImp & grid, int level)
    : ALU3dGridEntityPointer<codim,GridImp> (grid ,level)
      , endIter_(true)
      , level_(level)
      , isCopy_ (0)
  {
    this->done();
  }

  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline ALU3dGridLevelIterator<codim,pitype,GridImp> ::
  ALU3dGridLevelIterator(const ALU3dGridLevelIterator<codim,pitype,GridImp> & org )
    : ALU3dGridEntityPointer<codim,GridImp> ( org )
      , endIter_( org.endIter_ )
      , level_( org.level_ )
      , iter_ ( org.iter_ )
      , isCopy_(org.isCopy_+1)
  {
    // don't copy a copy of a copy of a copy of a copy
    assert( org.isCopy_ < 3 );
  }

  template<int codim, PartitionIteratorType pitype, class GridImp >
  inline void ALU3dGridLevelIterator<codim,pitype,GridImp> :: increment ()
  {
    // if assertion is thrown then end iterator was forgotten or didnt stop
    assert(!endIter_);

    (*iter_).next();
    if ((*iter_).done())
    {
      endIter_ = true;
      this->done();
      return ;
    }

    this->updateEntityPointer( & (*iter_).item() );
    return ;
  }

  template<int cdim, PartitionIteratorType pitype, class GridImp>
  inline typename ALU3dGridLevelIterator<cdim, pitype, GridImp> :: Entity &
  ALU3dGridLevelIterator<cdim, pitype, GridImp> :: dereference () const
  {
    // don't dereference empty entity pointer
    assert( this->item_ );
    assert( this->entity_ );
    assert( this->item_ == & (*this->entity_).getItem() );
    return (*this->entity_);
  }

  //*******************************************************************
  //
  //  LEAFITERATOR
  //
  //--LeafIterator
  //*******************************************************************
  template<int codim, PartitionIteratorType pitype, class GridImp>
  inline ALU3dGridLeafIterator<codim, pitype, GridImp> ::
  ALU3dGridLeafIterator(const GridImp &grid, int level,
                        bool end, const int nlinks)
    : ALU3dGridEntityPointer <codim,GridImp> ( grid,level)
      , endIter_(end)
      , level_(level)
      , isCopy_ (0)
  {
    if(!end)
    {
      // create interior iterator
      IteratorType * it = new IteratorType ( this->grid_ , level_, nlinks );
      iter_.store( it );

      (*iter_).first();
      if((!(*iter_).done()) && grid.hierSetSize(0) > 0) // else iterator empty
      {
        assert((*iter_).size() > 0);
        val_t & item = (*iter_).item();
        if( item.first )
          this->updateEntityPointer( item.first );
        else
          this->updateGhostPointer( *item.second );
      }
    }
    else
    {
      endIter_ = true;
      this->done();
    }
  }

  template<int cdim, PartitionIteratorType pitype, class GridImp>
  inline ALU3dGridLeafIterator<cdim, pitype, GridImp> ::
  ALU3dGridLeafIterator(const ALU3dGridLeafIterator<cdim, pitype, GridImp> &org)
    : ALU3dGridEntityPointer <cdim,GridImp> ( org )
      , endIter_(org.endIter_)
      , level_(org.level_)
      , iter_ ( org.iter_ )
      , isCopy_ (org.isCopy_+1)
  {
    // dont copy a copy of a copy of a copy of a copy
    assert( org.isCopy_  < 3 );
  }

  template<int cdim, PartitionIteratorType pitype, class GridImp>
  inline void ALU3dGridLeafIterator<cdim, pitype, GridImp> :: increment ()
  {
    // if assertion is thrown then end iterator was forgotten or didnt stop
    assert( !endIter_ );

    (*iter_).next();

    if((*iter_).done())
    {
      endIter_ = true;
      this->done();
      return ;
    }

    val_t & item = (*iter_).item();
    if( item.first )
      this->updateEntityPointer( item.first );
    else
      this->updateGhostPointer( *item.second );

    return ;
  }

  template<int cdim, PartitionIteratorType pitype, class GridImp>
  inline typename ALU3dGridLeafIterator<cdim, pitype, GridImp> :: Entity &
  ALU3dGridLeafIterator<cdim, pitype, GridImp> :: dereference () const
  {
    // don't dereference empty entity pointer
    assert( this->item_ );
    assert( this->entity_ );
    assert( this->item_ == & (*this->entity_).getItem() );
    return (*this->entity_);
  }


  /************************************************************************************
  #     #
  #     #     #    ######  #####      #     #####  ######  #####
  #     #     #    #       #    #     #       #    #       #    #
  #######     #    #####   #    #     #       #    #####   #    #
  #     #     #    #       #####      #       #    #       #####
  #     #     #    #       #   #      #       #    #       #   #
  #     #     #    ######  #    #     #       #    ######  #    #
  ************************************************************************************/
  // --HierarchicIterator
  template <class GridImp>
  inline ALU3dGridHierarchicIterator<GridImp> ::
  ALU3dGridHierarchicIterator(const GridImp & grid ,
                              const ALU3DSPACE HElementType & elem, int maxlevel ,bool end)
    : ALU3dGridEntityPointer<0,GridImp> ( grid, maxlevel )
      , elem_(elem)
      , maxlevel_(maxlevel)
  {
    if (!end)
    {
      ALU3DSPACE HElementType * item =
        const_cast<ALU3DSPACE HElementType *> (elem_.down());
      if(item)
      {
        // we have children and they lie in the disired level range
        if(item->level() <= maxlevel_)
        {
          this->updateEntityPointer( item );
        }
        else
        { // otherwise do nothing
          this->done();
        }
      }
      else
      {
        this->done();
      }
    }
  }

  template <class GridImp>
  inline ALU3dGridHierarchicIterator<GridImp> ::
  ALU3dGridHierarchicIterator(const ALU3dGridHierarchicIterator<GridImp> & org)
    : ALU3dGridEntityPointer<0,GridImp> (org)
      , elem_ (org.elem_)
      , maxlevel_(org.maxlevel_)
  {}

  template <class GridImp>
  inline ALU3DSPACE HElementType * ALU3dGridHierarchicIterator<GridImp>::
  goNextElement(ALU3DSPACE HElementType * oldelem )
  {
    // strategy is:
    // - go down as far as possible and then over all children
    // - then go to father and next and down again

    ALU3DSPACE HElementType * nextelem = oldelem->down();
    if(nextelem)
    {
      if(nextelem->level() <= maxlevel_)
        return nextelem;
    }

    nextelem = oldelem->next();
    if(nextelem)
    {
      if(nextelem->level() <= maxlevel_)
        return nextelem;
    }

    nextelem = oldelem->up();
    if(nextelem == &elem_) return 0;

    while( !nextelem->next() )
    {
      nextelem = nextelem->up();
      if(nextelem == &elem_) return 0;
    }

    if(nextelem) nextelem = nextelem->next();

    return nextelem;
  }

  template <class GridImp>
  inline void ALU3dGridHierarchicIterator<GridImp> :: increment ()
  {
    assert(this->item_ != 0);

    ALU3DSPACE HElementType * nextItem = goNextElement( this->item_ );
    if(!nextItem)
    {
      this->done();
      return ;
    }

    this->updateEntityPointer(nextItem);
    return ;
  }

  template <class GridImp>
  inline typename ALU3dGridHierarchicIterator<GridImp> :: Entity &
  ALU3dGridHierarchicIterator<GridImp> :: dereference () const
  {
    // don't dereference empty entity pointer
    assert( this->item_ );
    assert( this->entity_ );
    assert( this->item_ == & (*this->entity_).getItem() );
    return (*this->entity_);
  }




} // end namespace Dune
