// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
//************************************************************************
//
//  implementation of AlbertGrid
//
//  namespace Dune
//
//************************************************************************

namespace Dune
{

  //#ifdef _GNUGCC
#define TEMPPARAM2
  //#endif

  static ALBERT EL_INFO statElInfo[DIM+1];

  // singleton holding reference elements
  template<int dim>
  struct AlbertGridReferenceElement
  {
    enum { dimension = dim };
    AlbertGridElement<dim,dim> refelem;

    AlbertGridReferenceElement () : refelem (true) {};
  };

  // initialize static variable with bool constructor
  // (which makes reference element)
  // this sucks but for gcc we do a lot
  static AlbertGridReferenceElement<3> refelem_3;
  static AlbertGridReferenceElement<2> refelem_2;
  static AlbertGridReferenceElement<1> refelem_1;

  //****************************************************************
  //
  // --AlbertGridElement
  // --Element
  //
  //****************************************************************

  //****************************************************************
  //
  //  specialization of mapVertices
  //
  //****************************************************************
  //
  // default, do nothing
  template< int dim, int dimworld>
  inline int AlbertGridElement<dim,dimworld>::mapVertices (int i) const
  {
    return i;
  }

  // specialication for tetrhedrons
  // the local numbering in Albert is diffrent to Dune
  // see Albert Doc page 12
  static const int mapVerts_3d[4] = {0,3,2,1};
  template <>
  inline int AlbertGridElement<3,3>::mapVertices (int i) const
  {
    return i;
  }

  // specialication for codim == 1, faces
  template <>
  inline int AlbertGridElement<1,2>::mapVertices (int i) const
  {
    //std::cout << face_ << " Kante Map my Vertices!\n";
    int vert = ((face_ + 1 + i) % (N_VERTICES));
    //std::cout << vert << " Map my Vertices!\n";
    return vert;
  }

  template <>
  inline int AlbertGridElement<2,3>::mapVertices (int i) const
  {
    return AlbertHelp::localTetraFaceNumber[face_][i];
  }

  // specialization for codim == 2, edges
  template <>
  inline int AlbertGridElement<1,3>::mapVertices (int i) const
  {
    return ((face_+1)+ (edge_+1) +i)% (N_VERTICES);
  }

  template <>
  inline int AlbertGridElement<0,2>::mapVertices (int i) const
  {
    return ((face_+1)+ (vertex_+1) +i)% (N_VERTICES);
  }

  template <>
  inline int AlbertGridElement<0,3>::mapVertices (int i) const
  {
    return ((face_+1)+ (edge_+1) +(vertex_+1) +i)% (N_VERTICES);
  }

  template< int dim, int dimworld>
  inline AlbertGridElement<dim,dimworld>::
  AlbertGridElement(bool makeRefElement)
  {
    // make empty element
    initGeom();

    // make coords for reference elements, spezial for different dim
    if(makeRefElement)
      makeRefElemCoords();
  }

  template< int dim, int dimworld>
  inline ALBERT EL_INFO * AlbertGridElement<dim,dimworld>::
  makeEmptyElInfo()
  {
    ALBERT EL_INFO * elInfo = &statElInfo[dim];

    elInfo->mesh = NULL;
    elInfo->el = NULL;
    elInfo->parent = NULL;
    elInfo->macro_el = NULL;
    elInfo->level = 0;
#if DIM > 2
    elInfo->orientation = 0;
    elInfo->el_type = 0;
#endif

    for(int i =0; i<dim+1; i++)
    {
      for(int j =0; j< dimworld; j++)
      {
        elInfo->coord[i][j] = 0.0;
        elInfo->opp_coord[i][j] = 0.0;
      }
      elInfo->bound[i] = 0;
    }
    return elInfo;
  }


  template <>
  inline void AlbertGridElement<2,2>::
  makeRefElemCoords()
  {
    // make empty elInfo
    elInfo_ = makeEmptyElInfo();

    //*****************************************************************
    //!
    //!   Dune reference element triangles (2d)
    //!
    //!    (0,1)
    //!     2|\    coordinates and local node numbers
    //!      | \
    // //!      |  \
    //!     1|   \0
    //!      |    \
    // //!      |     \
    // //!     0|______\1
    //!    (0,0) 2  (1,0)
    //
    //*****************************************************************

    // set reference coordinates
    coord_ = 0.0;

    // vertex 1
    coord_(0,1) = 1.0;

    // vertex 2
    coord_(1,2) = 1.0;
  }

  template <>
  inline void AlbertGridElement<3,3>::
  makeRefElemCoords()
  {
    //! make ReferenzElement as default
    elInfo_ = makeEmptyElInfo();
    //*****************************************************************
    //
    //! reference element 3d
    //!        z
    //!        |     y
    //        3|    /
    // (0,0,1) |   /2 (0,1,0)
    //         |  /
    //         | /
    //         |/      (1,0,0)
    //       0 ------------x
    //      (0,0,0)     1
    //!
    //
    //*****************************************************************

    // set coordinates
    coord_ = 0.0;

    // vertex 1
    coord_(0,1) = 1.0;

    // vertex 2
    coord_(1,2) = 1.0;

    // vertex 3
    coord_(2,3) = 1.0;
  }
  template <>
  inline void AlbertGridElement<1,1>::
  makeRefElemCoords()
  {
    //! make  Referenz Element as default
    elInfo_ = makeEmptyElInfo();

    // set reference coordinates
    coord_ = 0.0;

    // vertex 1
    coord_(1) = 1.0;
  }

  template <int dim, int dimworld>
  inline void AlbertGridElement<dim,dimworld>::
  makeRefElemCoords()
  {
    std::cout << "No default implementation for this AlbertGridElement! \n";
    abort();
  }

  template< int dim, int dimworld>
  inline void AlbertGridElement<dim,dimworld>::
  initGeom()
  {
    elInfo_ = NULL;
    face_ = 0;
    edge_ = 0;
    vertex_ = 0;
    builtinverse_ = false;
    builtElMat_   = false;
  }

  // built Geometry
  template< int dim, int dimworld>
  inline bool AlbertGridElement<dim,dimworld>::
  builtGeom(ALBERT EL_INFO *elInfo, int face,
            int edge, int vertex)
  {
    //std::cout << " Built geom !\n";
    elInfo_ = elInfo;
    face_ = face;
    //std::cout << face_ << " Kante \n";
    edge_ = edge;
    vertex_ = vertex;
    elDet_ = 0.0;
    builtinverse_ = false;
    builtElMat_   = false;

    if(elInfo_)
    {
      for(int i=0; i<dim+1; i++)
      {
        (coord_(i)) = static_cast< albertCtype  * >
                      ( elInfo_->coord[mapVertices(i)] );
      }
      // geometry built
      return true;
    }
    // geometry not built
    return false;
  }


  // specialization yields speed up, because vertex_ .. is not copied
  template <>
  inline bool AlbertGridElement<2,2>::
  builtGeom(ALBERT EL_INFO *elInfo, int face,
            int edge, int vertex)
  {
    enum { dim = 2 };
    enum { dimworld = 2 };

    elInfo_ = elInfo;
    elDet_ = 0.0;
    builtinverse_ = false;
    builtElMat_   = false;

    if(elInfo_)
    {
      for(int i=0; i<dim+1; i++)
        (coord_(i)) = static_cast< albertCtype  * >
                      ( elInfo_->coord[mapVertices(i)] );
      // geometry built
      return true;
    }
    // geometry not built
    return false;
  }

  template <>
  inline bool AlbertGridElement<3,3>::
  builtGeom(ALBERT EL_INFO *elInfo, int face,
            int edge, int vertex)
  {
    enum { dim = 3 };
    enum { dimworld = 3 };

    elInfo_ = elInfo;
    elDet_ = 0.0;
    builtinverse_ = false;
    builtElMat_   = false;

    if(elInfo_)
    {
      for(int i=0; i<dim+1; i++)
        (coord_(i)) = static_cast< albertCtype  * >
                      ( elInfo_->coord[mapVertices(i)] );
      // geometry built
      return true;
    }
    // geometry not built
    return false;
  }


  // print the ElementInformation
  template<int dim, int dimworld>
  inline void AlbertGridElement<dim,dimworld>::print (std::ostream& ss, int indent)
  {
    std::cout << "AlbertGridElement<" << dim << "," << dimworld << " = {\n";
    for(int i=0; i<corners(); i++)
    {
      std::cout << " corner " << i;
      ((*this)[i]).print(ss,1); std::cout << "\n";
    }
    std::cout << "} \n";
  }

  template< int dim, int dimworld>
  inline ElementType AlbertGridElement<dim,dimworld>::type()
  {
    switch (dim)
    {
    case 1 : return line;
    case 2 : return triangle;
    case 3 : return tetrahedron;

    default : return unknown;
    }
  }

  template< int dim, int dimworld>
  inline int AlbertGridElement<dim,dimworld>::corners()
  {
    return (dim+1);
  }

  ///////////////////////////////////////////////////////////////////////
  template< int dim, int dimworld>
  inline Vec<dimworld,albertCtype>& AlbertGridElement<dim,dimworld>::
  operator [](int i)
  {
    return coord_(i);
  }

  template<>
  inline AlbertGridElement<3,3>& AlbertGridElement<3,3>::
  refelem()
  {
    return refelem_3.refelem;
  }

  template<>
  inline AlbertGridElement<2,2>& AlbertGridElement<2,3>::
  refelem()
  {
    return refelem_2.refelem;
  }

  template<>
  inline AlbertGridElement<2,2>& AlbertGridElement<2,2>::
  refelem()
  {
    return refelem_2.refelem;
  }

  template<>
  inline AlbertGridElement<1,1>& AlbertGridElement<1,2>::
  refelem()
  {
    return refelem_1.refelem;
  }

  template<>
  inline AlbertGridElement<1,1>& AlbertGridElement<1,1>::
  refelem()
  {
    return refelem_1.refelem;
  }

  template< int dim, int dimworld>
  inline Vec<dimworld,albertCtype> AlbertGridElement<dim,dimworld>::
  global(const Vec<dim>& local)
  {
    // checked, works

    // we calculate interal in barycentric coordinates
    // fake the third local coordinate via localFake
    albertCtype c = local(0);
    albertCtype localFake=1.0-c;

    // the initialize
    // note that we have to swap the j and i
    // in coord(j,i) means coord_(i)(j)
    for(int j=0; j<dimworld; j++)
      globalCoord_(j) = c * coord_(j,1);

    // for all local coords
    for (int i = 1; i < dim; i++)
    {
      c = local(i);
      localFake -= c;
      for(int j=0; j<dimworld; j++)
        globalCoord_(j) += c * coord_(j,i+1);
    }

    // for the last barycentric coord
    for(int j=0; j<dimworld; j++)
      globalCoord_(j) += localFake * coord_(j,0);

    return globalCoord_;
  }

  template <int dim, int dimworld>
  inline void AlbertGridElement<dim,dimworld>::calcElMatrix ()
  {
    builtElMat_ = false;
    std::cout << "AlbertGridElement::calcElMatrix: No default implementation \n";
    abort();
  }
  // calc A for triangles
  template <>
  inline void AlbertGridElement<2,2>::calcElMatrix ()
  {
    if( !builtElMat_ )
    {
      // A = ( P1 - P0 , P2 - P0 )
      for (int i=0; i<2; i++)
      {
        elMat_(i,0) = coord_(i,1) - coord_(i,0);
        elMat_(i,1) = coord_(i,2) - coord_(i,0);
      }
      builtElMat_ = true;
    }
  }

  // calc A for tetrahedra
  template <>
  inline void AlbertGridElement<3,3>::calcElMatrix ()
  {
    enum { dimworld = 3 };
    if( !builtElMat_)
    {
      Vec<dimworld,albertCtype> & coord0 = coord_(0);
      for(int i=0 ; i<dimworld; i++)
      {
        elMat_(i,0) = coord_(i,1) - coord0(i);
        elMat_(i,1) = coord_(i,2) - coord0(i);
        elMat_(i,2) = coord_(i,3) - coord0(i);
      }
      builtElMat_ = true;
    }
  }

  // uses the element matrix, because faster
  template<>
  inline Vec<2,albertCtype> AlbertGridElement<2,2>::
  global(const Vec<2>& local)
  {
    calcElMatrix();
    globalCoord_  = elMat_ * local;
    globalCoord_ += coord_(0);
    return globalCoord_;
  }

  template<>
  inline Vec<3,albertCtype> AlbertGridElement<3,3>::
  global(const Vec<3>& local)
  {
    calcElMatrix();
    globalCoord_  = elMat_ * local;
    globalCoord_ += coord_(0);
    return globalCoord_;
  }


  template< int dim, int dimworld>
  inline Vec<dim> AlbertGridElement<dim,dimworld>::
  local(const Vec<dimworld>& global)
  {
    std::cerr << "local for dim != dimworld not implemented! \n";
    abort();
    return localCoord_;
  }

  template <>
  inline Vec<2> AlbertGridElement<2,2>::
  local(const Vec<2>& global)
  {
    if(!builtinverse_)
      buildJacobianInverse(global);

    localCoord_ = Jinv_ * ( global - coord_(0));
    return localCoord_;
  }

  template <>
  inline Vec<3> AlbertGridElement<3,3>::
  local(const Vec<3>& global)
  {
    if(!builtinverse_)
      buildJacobianInverse(global);

    localCoord_ = Jinv_ * ( global - coord_(0));
    return localCoord_;
  }



  // this method is for (dim==dimworld) = 2 and 3
  template <int dim, int dimworld>
  inline void AlbertGridElement<dim,dimworld>::
  buildJacobianInverse(const Vec<dim,albertCtype>& local)
  {
    //******************************************************
    //
    //  the mapping is:
    //
    //  F(T) = D where T is the reference element
    //  and D the actual element
    //
    //  F(x) = A * x + b    with   A := ( P_0 , P_1 )
    //
    //  A consist of the column vectors P_0 and P_1 and
    //  is calculated by the method calcElMatrix
    //
    //******************************************************

    // calc A and stores it in elMat_
    calcElMatrix();

    // Jinv = A^-1
    assert( builtElMat_ == true );
    elDet_ = ABS( elMat_.invert(Jinv_) );

    assert(elDet_ > 1.0E-25);
    builtinverse_ = true;
    return;
  }

  // calc volume of face of tetrahedron
  template <>
  inline void AlbertGridElement<2,3>::
  buildJacobianInverse(const Vec<2,albertCtype>& local)
  {
    //std::cout << "To be implemented! \n";
    //abort();
    enum { dim = 2 };
    enum { dimworld = 3 };

    // is faster than the lower method
    std::cout << "buildJacobianInverse<2,3> not correctly implemented!\n";
    elDet_ = 0.1;
    builtinverse_ = true;
  }

  template <>
  inline void AlbertGridElement<1,2>::
  buildJacobianInverse(const Vec<1,albertCtype>& local)
  {
    // volume is length of edge
    Vec<2,albertCtype> vec = coord_(0) - coord_(1);
    elDet_ = vec.norm2();

    builtinverse_ = true;
  }
  // default implementation calls ALBERT routine
  template< int dim, int dimworld>
  inline albertCtype AlbertGridElement<dim,dimworld>::elDeterminant ()
  {
    return ALBERT el_det(elInfo_);
  }

  // volume of one Element, here triangle
  template <>
  inline albertCtype AlbertGridElement<2,2>::elDeterminant ()
  {
    calcElMatrix();
    return ABS ( elMat_.determinant () );
  }

  // volume of one Element, here therahedron
  template <>
  inline albertCtype AlbertGridElement<3,3>::elDeterminant ()
  {
    calcElMatrix();
    return ABS ( elMat_.determinant () );
  }

  template< int dim, int dimworld>
  inline albertCtype AlbertGridElement<dim,dimworld>::
  integration_element (const Vec<dim,albertCtype>& local)
  {
    // if inverse was built, volume was calced already
    if(builtinverse_)
      return elDet_;

    elDet_ = elDeterminant();
    return elDet_;
  }

  template <>
  inline Mat<1,1>& AlbertGridElement<1,2>::
  Jacobian_inverse (const Vec<1,albertCtype>& global)
  {
    std::cout << "Jaconbian_inverse for dim=1,dimworld=2 not implemented yet! \n";
    return Jinv_;
  }

  template< int dim, int dimworld>
  inline Mat<dim,dim>& AlbertGridElement<dim,dimworld>::
  Jacobian_inverse (const Vec<dim,albertCtype>& global)
  {
    if(builtinverse_)
      return Jinv_;

    // builds the jacobian inverse and calculates the volume
    buildJacobianInverse(global);
    return Jinv_;
  }

  //************************************************************************
  //  checkMapping and checkInverseMapping are for checks of Jinv_
  //************************************************************************
  template <int dim, int dimworld>
  inline bool AlbertGridElement<dim,dimworld>::checkInverseMapping (int loc)
  {
    std::cout << "AlbertGridElement::checkInverseMapping: no default implemantation! \n";
    abort();
    return false;
  }

  template <>
  inline bool AlbertGridElement<2,2>::checkInverseMapping (int loc)
  {
    // checks if F^-1 (x_i) == xref_i
    enum { dim =2 };

    Vec<dim> & coord    = coord_(loc);
    Vec<dim> & refcoord = refelem()[loc];
    buildJacobianInverse(refcoord);

    Vec<dim> tmp2 = coord - coord_(0);
    tmp2 = Jinv_ * tmp2;

    for(int j=0; j<dim; j++)
      if(tmp2(j) != refcoord(j))
      {
        std::cout << "AlbertGridElement<2,2>::checkInverseMapping: Mapping of coord " << loc << " incorrect! \n";
        return false;
      }
    return true;
  }

  template <>
  inline bool AlbertGridElement<3,3>::checkInverseMapping (int loc)
  {
    // checks if F^-1 (x_i) == xref_i
    enum { dim =3 };

    Vec<dim> & coord    = coord_(loc);
    Vec<dim> & refcoord = refelem()[loc];
    buildJacobianInverse(refcoord);

    Vec<dim> tmp2 = coord - coord_(0);
    tmp2 = Jinv_ * tmp2;

    for(int j=0; j<dim; j++)
      if(tmp2(j) != refcoord(j))
      {
        std::cout << "AlbertGridElement<3,3>::checkInverseMapping: Mapping of coord " << loc << " incorrect! \n";
        return false;
      }
    return true;
  }


  template <int dim, int dimworld>
  inline bool AlbertGridElement<dim,dimworld>::checkMapping (int loc)
  {
    std::cout << "AlbertGridElement::checkMapping: no default implemantation! \n";
    abort();
    return false;
  }

  template <>
  inline bool AlbertGridElement<2,2>::checkMapping (int loc)
  {
    // checks the mapping
    // F = Ax + P_0
    enum { dim =2 };

    calcElMatrix ();

    Vec<dim> & coord    = coord_(loc);
    Vec<dim> & refcoord = refelem()[loc];

    Vec<dim> tmp2 =  elMat_ * refcoord;
    tmp2 += coord_(0);

    for(int j=0; j<dim; j++)
      if(tmp2(j) != coord(j))
      {
        coord.print(std::cout,1); tmp2.print(std::cout,1); std::cout << "\n";
        std::cout << "AlbertGridElement<2,2>::checkMapping: Mapping of coord " << loc << " incorrect! \n";
        return false;
      }
    return true;
  }

  template <>
  inline bool AlbertGridElement<3,3>::checkMapping (int loc)
  {
    // checks the mapping
    // F = Ax + P_0

    enum { dim = 3 };

    calcElMatrix ();

    Vec<dim> & coord    = coord_(loc);
    Vec<dim> & refcoord = refelem()[loc];

    Vec<dim> tmp2 =  elMat_ * refcoord;
    tmp2 += coord_(0);

    for(int j=0; j<dim; j++)
    {
      if(tmp2(j) != coord(j))
      {
        coord.print(std::cout,1); refcoord.print(std::cout,1);
        tmp2.print(std::cout,1); std::cout << "\n";
        std::cout << "AlbertGridElement<3,3>::checkMapping: Mapping of coord " << loc << " incorrect! \n";
        return false;
      }
    }
    return true;
  }


  template<int dim, int dimworld>
  inline bool AlbertGridElement <dim ,dimworld >::
  checkInside(const Vec<dim,albertCtype> &local)
  {
    // only 2d
    albertCtype sum = 0.0;

    for(int i=0; i<dim; i++)
    {
      sum += local(i);
      if(local(i) < 0.0) return false;
    }

    if( sum > 1.0 ) return false;

    return true;
  }

  //*************************************************************************
  //
  //  --AlbertGridEntity
  //  --Entity
  //
  //*************************************************************************
  //
  //  codim > 0
  //
  // The Element is prescribed by the EL_INFO struct of ALBERT MESH
  // the pointer to this struct is set and get by setElInfo and
  // getElInfo.
  //*********************************************************************8
  template<int codim, int dim, int dimworld>
  inline void AlbertGridEntity < codim, dim ,dimworld >::
  makeDescription()
  {
    elInfo_ = NULL;
    builtgeometry_ = false;

    // to slow and not needed
    //geo_.initGeom();
  }

  template<int codim, int dim, int dimworld>
  inline AlbertGridEntity < codim, dim ,dimworld >::
  AlbertGridEntity(AlbertGrid<dim,dimworld> &grid, int level,
                   ALBERT TRAVERSE_STACK * travStack) : grid_(grid)
                                                        , level_ ( level )
                                                        , geo_ (false)
  {
    travStack_ = travStack;
    makeDescription();
  }


  template<int codim, int dim, int dimworld>
  inline void AlbertGridEntity < codim, dim ,dimworld >::
  setTraverseStack(ALBERT TRAVERSE_STACK * travStack)
  {
    travStack_ = travStack;
  }

  template<int codim, int dim, int dimworld>
  inline AlbertGridEntity < codim, dim ,dimworld >::
  AlbertGridEntity(AlbertGrid<dim,dimworld> &grid, int level) :
    grid_(grid)
    , level_ (level)
    , geo_ ( false )
  {
    travStack_ = NULL;
    makeDescription();
  }

  template<int codim, int dim, int dimworld>
  inline ALBERT EL_INFO* AlbertGridEntity < codim, dim ,dimworld >::
  getElInfo() const
  {
    return elInfo_;
  }

  template<int codim, int dim, int dimworld>
  inline void AlbertGridEntity < codim, dim ,dimworld >::
  setElInfo(ALBERT EL_INFO * elInfo, int elNum, int face,
            int edge, int vertex )
  {
    elNum_ = elNum;
    face_ = face;
    edge_ = edge;
    vertex_ = vertex;
    elInfo_ = elInfo;
    builtgeometry_ = geo_.builtGeom(elInfo_,face,edge,vertex);
  }

  template<int codim, int dim, int dimworld>
  inline int AlbertGridEntity < codim, dim ,dimworld >::
  level()
  {
    return level_;
  }

  template<int codim, int dim, int dimworld>
  inline int AlbertGridEntity < codim, dim ,dimworld >::
  index()
  {
    return elNum_;
  }

  template< int codim, int dim, int dimworld>
  inline AlbertGridElement<dim-codim,dimworld>&
  AlbertGridEntity < codim, dim ,dimworld >::geometry()
  {
    return geo_;
  }

  template<int codim, int dim, int dimworld>
  inline Vec<dim,albertCtype>&
  AlbertGridEntity < codim, dim ,dimworld >::local()
  {
    return localFatherCoords_;
  }

  template<int codim, int dim, int dimworld>
  inline AlbertGridLevelIterator<0,dim,dimworld>
  AlbertGridEntity < codim, dim ,dimworld >::father()
  {
    std::cout << "\nAlbertGridEntity<" << codim << "," << dim << "," << dimworld <<">::father not correctly implemented! \n";
    ALBERT TRAVERSE_STACK travStack;
    initTraverseStack(&travStack);

    travStack = (*travStack_);

    travStack.stack_used--;

    AlbertGridLevelIterator <0,dim,dimworld>
    it(grid_,travStack.elinfo_stack+travStack.stack_used,0,0,0,0);
    return it;
  }


  //************************************
  //
  //  --AlbertGridEntity codim = 0
  //  --0Entity codim = 0
  //
  //************************************
  template< int dim, int dimworld>
  inline bool AlbertGridEntity < 0, dim ,dimworld >::
  mark( int refCount )
  {
    if(! hasChildren() )
    {
      // we can not mark for coarsening if already marked for refinement
      if((refCount < 0) && (elInfo_->el->mark > 0))
      {
        return true;
      }

      if( refCount > 0)
      {
        elInfo_->el->mark = 1;
        return true;
      }
      if( refCount < 0)
      {
        grid_.setMark ( true );
        elInfo_->el->mark = -1;
        return true;
      }
    }
    elInfo_->el->mark = 0;
    return false;
  }

  template< int dim, int dimworld>
  inline AdaptationState AlbertGridEntity < 0, dim ,dimworld >::
  state()
  {
    //printf("El State  %d \n", elInfo_->el->mark);
    if( elInfo_->el->mark < 0 )
    {
      //std::cout << " Return Coarsend!\n";
      return COARSEND;
    }

    if( grid_.checkElNew( el_index() ) )
    {
      std::cout << global_index() << " El is new \n";
      return REFINED;
    }
    /*
       if( elInfo_->el->mark < 0 )
       {
        std::cout << " Return Coarsend!\n";
        return COARSEND;
       }
     */
    return NONE;
  }

  template< int dim, int dimworld>
  inline bool AlbertGridEntity < 0, dim ,dimworld >::hasChildren()
  {
    if(elInfo_)
      return (elInfo_->el->child[0] != NULL);

    return false;
  }

  template< int dim, int dimworld>
  inline int AlbertGridEntity < 0, dim ,dimworld >::refDistance()
  {
    return 0;
  }

  //***************************

  template< int dim, int dimworld>
  inline void AlbertGridEntity < 0, dim ,dimworld >::
  makeDescription()
  {
    elInfo_ = NULL;
    builtgeometry_ = false;

    // not fast , and also not needed
    //geo_.initGeom();
  }

  template<int dim, int dimworld>
  inline void AlbertGridEntity < 0, dim ,dimworld >::
  setTraverseStack(ALBERT TRAVERSE_STACK * travStack)
  {
    travStack_ = travStack;
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >::
  AlbertGridEntity(AlbertGrid<dim,dimworld> &grid, int level) : grid_(grid)
                                                                , level_ (level)
                                                                , vxEntity_ ( grid_ , NULL, 0, 0, 0, 0)
                                                                , geo_(false) , travStack_ (NULL) , elInfo_ (NULL)
                                                                , builtgeometry_ (false)
  {}

  //*****************************************************************8
  // count
  template <int dim, int dimworld> template <int cc>
  inline int AlbertGridEntity<0,dim,dimworld>::count ()
  {
    return (dim+1);
  }
  //! specialization only for codim == 2 , edges,
  //! a tetrahedron has always 6 edges
  template <>
#ifdef TEMPPARAM2
  template <>
#endif
  inline int AlbertGridEntity<0,3,3>::count<2> ()
  {
    return 6;
  }

  // subIndex
  template <int dim, int dimworld> template <int cc>
  inline int AlbertGridEntity<0,dim,dimworld>::subIndex ( int i )
  {
    return entity<cc>(i)->index();
  }

  // subIndex
  template <>
#ifdef TEMPPARAM2
  template <>
#endif
  inline int AlbertGridEntity<0,2,2>::subIndex<2> ( int i )
  {
    return grid_.indexOnLevel<2>(elInfo_->el->dof[i][0],level_);
  }

  // subIndex
  template <>
#ifdef TEMPPARAM2
  template <>
#endif
  inline int AlbertGridEntity<0,2,3>::subIndex<2> ( int i )
  {
    return grid_.indexOnLevel<2>(elInfo_->el->dof[i][0],level_);
  }

  // subIndex
  template <>
#ifdef TEMPPARAM2
  template <>
#endif
  inline int AlbertGridEntity<0,3,3>::subIndex<3> ( int i )
  {
    return grid_.indexOnLevel<3>(elInfo_->el->dof[i][0],level_);
  }

  // default is faces
  template <int dim, int dimworld> template <int cc>
  inline AlbertGridLevelIterator<cc,dim,dimworld>
  AlbertGridEntity<0,dim,dimworld>::entity ( int i )
  {
    AlbertGridLevelIterator<cc,dim,dimworld> tmp (grid_,elInfo_,
                                                  grid_. template indexOnLevel<cc>( el_index() ,level_),i,0,0);
    return tmp;
  }

  template <>
#ifdef TEMPPARAM2
  template <>
#endif
  inline AlbertGridLevelIterator<2,3,3>
  AlbertGridEntity<0,3,3>::entity<2> ( int i )
  {
    //enum { cc = 2 };
    int num = grid_.indexOnLevel<2>(el_index() ,level_);
    if(i < 3)
    { // 0,1,2
      AlbertGridLevelIterator<2,3,3> tmp (grid_,level_,elInfo_,num, 0,i,0);
      return tmp;
    }
    else
    { // 3,4,5
      AlbertGridLevelIterator<2,3,3> tmp (grid_,level_,elInfo_,num, i-2,1,0);
      return tmp;
    }
  }

  // specialization for vertices
  template <>
#ifdef TEMPPARAM2
  template <>
#endif
  inline AlbertGridLevelIterator<2,2,2>
  AlbertGridEntity<0,2,2>::entity<2> ( int i )
  {
    std::cout << "entity<2> ,2,2 !\n";
    // we are looking at vertices
    //enum { cc = dimension };
    enum { cc = 2 };
    AlbertGridLevelIterator<cc,2,2>
    tmp (grid_,level_,elInfo_, grid_.indexOnLevel<cc>( elInfo_->el->dof[i][0],level_),
         0,0,i);
    return tmp;
  }
  // specialization for vertices
  template <>
#ifdef TEMPPARAM2
  template <>
#endif
  inline AlbertGridLevelIterator<2,2,3>
  AlbertGridEntity<0,2,3>::entity<2> ( int i )
  {
    // we are looking at vertices
    //enum { cc = dimension };
    enum { cc = 2 };
    AlbertGridLevelIterator<cc,2,3>
    tmp (grid_,level_,elInfo_, grid_.indexOnLevel<cc>( elInfo_->el->dof[i][0],level_),
         0,0,i);
    return tmp;
  }
  // specialization for vertices
  template <>
#ifdef TEMPPARAM2
  template <>
#endif
  inline AlbertGridLevelIterator<3,3,3>
  AlbertGridEntity<0,3,3>::entity<3> ( int i )
  {
    // we are looking at vertices
    enum { cc = 3 };
    //enum { cc = dimension };
    AlbertGridLevelIterator<cc,3,3>
    tmp (grid_,level_,elInfo_, grid_.indexOnLevel<cc>( elInfo_->el->dof[i][0],level_),
         0,0,i);
    return tmp;
  }
  //***************************

  template<int dim, int dimworld>
  inline ALBERT EL_INFO* AlbertGridEntity < 0 , dim ,dimworld >::
  getElInfo() const
  {
    return elInfo_;
  }

  template<int dim, int dimworld>
  inline int AlbertGridEntity < 0, dim ,dimworld >::
  level()
  {
    return level_;
  }

  template<int dim, int dimworld>
  inline int AlbertGridEntity < 0, dim ,dimworld >::
  index()
  {
    return grid_.template indexOnLevel<0>( el_index() , level_ );
  }

  template<int dim, int dimworld>
  inline int AlbertGridEntity < 0, dim ,dimworld >::
  global_index()
  {
    return grid_.globalIndexConsecutive<0>(el_index());
  }

  template< int dim, int dimworld>
  inline void AlbertGridEntity < 0, dim ,dimworld >::
  setLevel(int actLevel)
  {
    level_ = actLevel;
  }

  template< int dim, int dimworld>
  inline void AlbertGridEntity < 0, dim ,dimworld >::
  setElInfo(ALBERT EL_INFO * elInfo, int elNum,  int face,
            int edge, int vertex )
  {
    // in this case the face, edge and vertex information is not used,
    // because we are in the element case
    elInfo_ = elInfo;
    builtgeometry_ = geo_.builtGeom(elInfo_,face,edge,vertex);
  }

  template< int dim, int dimworld>
  inline AlbertGridElement<dim,dimworld>&
  AlbertGridEntity < 0, dim ,dimworld >::geometry()
  {
    assert(builtgeometry_ == true);
    return geo_;
  }


  template< int dim, int dimworld>
  inline AlbertGridLevelIterator<0,dim,dimworld>
  AlbertGridEntity < 0, dim ,dimworld >::father()
  {
    ALBERT EL_INFO * fatherInfo = NULL;
    int fatherLevel = level_-1;
    // if this level > 0 return father = elInfoStack -1,
    // else return father = this
    if(!travStack_)
    {
      std::cout << "No traverse stack in father! \n";
      abort();
    }
    if(level_ > 0)
      fatherInfo = & (travStack_->elinfo_stack)[travStack_->stack_used-1];
    else
    {
      std::cout << "No father on macro level! \n";
      fatherInfo = elInfo_;
      fatherLevel = 0;
    }

    int fatherIndex = grid_.template indexOnLevel<0>(fatherInfo->el->index,fatherLevel);
    // new LevelIterator with EL_INFO one level above
    AlbertGridLevelIterator <0,dim,dimworld> it(grid_,fatherLevel,fatherInfo,fatherIndex,0,0,0);
    return it;
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >
  AlbertGridEntity < 0, dim ,dimworld >::
  newEntity()
  {
    AlbertGridEntity < 0, dim ,dimworld > tmp ( grid_ , level_ );
    return tmp;
  }

  template< int dim, int dimworld>
  inline void
  AlbertGridEntity < 0, dim ,dimworld >::
  father(AlbertGridEntity < 0, dim ,dimworld >& vati )
  {
    ALBERT EL_INFO * fatherInfo = NULL;
    int fatherLevel = level_-1;
    // if this level > 0 return father = elInfoStack -1,
    // else return father = this
    if(!travStack_)
    {
      std::cout << "No traverse stack in father! \n";
      abort();
    }
    if(level_ > 0)
      fatherInfo = & (travStack_->elinfo_stack)[travStack_->stack_used-1];
    else
    {
      std::cout << "No father on macro level! \n";
      fatherInfo = elInfo_;
      fatherLevel = 0;
    }

    vati.setElInfo( fatherInfo );
    vati.setLevel ( fatherLevel );
  }

  template< int dim, int dimworld>
  inline AlbertGridElement<dim,dim>&
  AlbertGridEntity < 0, dim ,dimworld >::father_relative_local()
  {
    //AlbertGridLevelIterator<0,dim,dimworld> daddy = father();
    AlbertGridElement<dim,dimworld> daddy = (*father()).geometry();

    fatherReLocal_.initGeom();
    // compute the local coordinates in father refelem
    for(int i=0; i<fatherReLocal_.corners(); i++)
      fatherReLocal_[i] = daddy.local(geometry()[i]);

    std::cout << "\nfather_realtive_local not tested yet! \n";
    return fatherReLocal_;
  }

  // end AlbertGridEntity

  //***************************************************************
  //
  //  --AlbertGridHierarchicIterator
  //  --HierarchicIterator
  //
  //***************************************************************
  template< int dim, int dimworld>
  inline void AlbertGridHierarchicIterator<dim,dimworld>::
  makeIterator()
  {
    virtualEntity_.setTraverseStack(NULL);
    virtualEntity_.setElInfo(NULL,0,0,0,0);
  }

  template< int dim, int dimworld>
  inline AlbertGridHierarchicIterator<dim,dimworld>::
  AlbertGridHierarchicIterator(AlbertGrid<dim,dimworld> &grid,int actLevel,
                               int maxLevel) : grid_(grid), level_ (actLevel)
                                               , maxlevel_ (maxLevel) , virtualEntity_(grid,level_)
  {
    makeIterator();
  }

  template< int dim, int dimworld>
  inline AlbertGridHierarchicIterator<dim,dimworld>::
  AlbertGridHierarchicIterator(AlbertGrid<dim,dimworld> &grid,
                               ALBERT TRAVERSE_STACK *travStack,int actLevel, int maxLevel) :
    grid_(grid), level_ (actLevel),
    maxlevel_ ( maxLevel), virtualEntity_(grid,level_)
  {
    if(travStack)
    {
      // get new ALBERT TRAVERSE STACK
      manageStack_.makeItNew(true);

      ALBERT TRAVERSE_STACK *stack = manageStack_.getStack();

      // cut old traverse stack, kepp only actual element
      cutHierarchicStack(stack, travStack);

      // set new traverse level
      if(maxlevel_ < 0)
      {
        std::cout << "WARNING: maxlevel < 0 in AlbertGridHierarchicIterator! \n";
        // this means, we go until leaf level
        stack->traverse_fill_flag = CALL_LEAF_EL | stack->traverse_fill_flag;
        // exact here has to stand Grid->maxlevel, but is ok anyway
        maxlevel_ = grid_.maxlevel(); //123456789;
      }
      // set new traverse level
      stack->traverse_level = maxlevel_;

      virtualEntity_.setTraverseStack(stack);
      // Hier kann ein beliebiges Element uebergeben werden,
      // da jedes AlbertElement einen Zeiger auf das Macroelement
      // enthaelt.
      virtualEntity_.setElInfo(recursiveTraverse(stack));

      // set new level
      virtualEntity_.setLevel(level_);
    }
    else
    {
      std::cout << "Warning: travStack == NULL in HierarchicIterator(travStack,travLevel) \n";
      makeIterator();
    }
  }

  template< int dim, int dimworld>
  inline AlbertGridHierarchicIterator<dim,dimworld>&
  AlbertGridHierarchicIterator< dim,dimworld >::operator ++()
  {
    virtualEntity_.setElInfo(
      recursiveTraverse(manageStack_.getStack())
      );
    // set new actual level, calculated by recursiveTraverse
    virtualEntity_.setLevel(level_);
    return (*this);
  }

  template< int dim, int dimworld>
  inline AlbertGridHierarchicIterator<dim,dimworld>&
  AlbertGridHierarchicIterator<dim,dimworld>::
  operator ++(int steps)
  {
    for(int i=0; i<steps; i++)
      ++(*this);
    return (*this);
  }

  template< int dim, int dimworld>
  inline bool AlbertGridHierarchicIterator<dim,dimworld>::
  operator ==(const AlbertGridHierarchicIterator& I) const
  {
    return (virtualEntity_.getElInfo() == I.virtualEntity_.getElInfo());
  }

  template< int dim, int dimworld>
  inline bool AlbertGridHierarchicIterator<dim,dimworld>::
  operator !=(const AlbertGridHierarchicIterator& I) const
  {
    return !((*this) == I);
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >&
  AlbertGridHierarchicIterator<dim,dimworld>::
  operator *()
  {
    return virtualEntity_;
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >*
  AlbertGridHierarchicIterator<dim,dimworld>::
  operator ->()
  {
    return &virtualEntity_;
  }

  template< int dim, int dimworld>
  inline ALBERT EL_INFO *
  AlbertGridHierarchicIterator<dim,dimworld>::
  recursiveTraverse(ALBERT TRAVERSE_STACK * stack)
  {
    // see function
    // static EL_INFO *traverse_leaf_el(TRAVERSE_STACK *stack)
    // Common/traverse_nr_common.cc, line 392
    ALBERT EL * el=NULL;

    if(!stack->elinfo_stack)
    {
      /* somethin' wrong */
      return NULL;
    }
    else
    {
      // go up until we can go down again
      el = stack->elinfo_stack[stack->stack_used].el;

      // stack->stack_used is actual element in stack
      // stack->info_stack[stack->stack_used] >= 2
      //    means the two children has been visited
      while((stack->stack_used > 0) &&
            ((stack->info_stack[stack->stack_used] >= 2)
             || (el->child[0]==NULL)
             || ( stack->traverse_level <=
                  (stack->elinfo_stack+stack->stack_used)->level)) )
      {
        stack->stack_used--;
        el = stack->elinfo_stack[stack->stack_used].el;
        level_ = stack->elinfo_stack[stack->stack_used].level;
      }

      // goto next father is done by other iterator and not our problem
      if(stack->stack_used < 1)
      {
        return NULL;
      }
    }

    // go down next child
    if(el->child[0] && (stack->traverse_level >
                        (stack->elinfo_stack+stack->stack_used)->level) )
    {
      if(stack->stack_used >= stack->stack_size - 1)
        ALBERT enlargeTraverseStack(stack);

      int i = stack->info_stack[stack->stack_used];
      el = el->child[i];
      stack->info_stack[stack->stack_used]++;

      // new: go down maxlevel, but fake the elements
      level_++;
      grid_.fillElInfo(i, level_, stack->elinfo_stack+stack->stack_used,
                       stack->elinfo_stack+stack->stack_used+1 ,true);
      //ALBERT fill_elinfo(i, stack->elinfo_stack + stack->stack_used,
      //  stack->elinfo_stack + (stack->stack_used + 1));

      stack->stack_used++;
      stack->info_stack[stack->stack_used] = 0;
    }
    else
    {
      return NULL;
    }

    return (stack->elinfo_stack + stack->stack_used);
  } // recursive traverse over all childs

  // end AlbertGridHierarchicIterator

  //***************************************************************
  //
  //  --AlbertGridIntersectionIterator
  //  --IntersectionIterator
  //
  //***************************************************************

  // these object should be generated with new by Entity, because
  // for a LevelIterator we only need one virtualNeighbour Entity, which is
  // given to the Neighbour Iterator, we need a list of Neighbor Entitys
  template< int dim, int dimworld>
  inline void AlbertGridIntersectionIterator<dim,dimworld>::freeObjects ()
  {
    if(manageObj_)
      manageObj_ = grid_->entityProvider_.freeObjectEntity(manageObj_);

    if(manageInterEl_)
      manageInterEl_ = grid_->interSelfProvider_.freeObjectEntity(manageInterEl_);

    if(manageNeighEl_)
      manageNeighEl_ = grid_->interNeighProvider_.freeObjectEntity(manageNeighEl_);

    if(boundaryEntity_)
    {
      delete boundaryEntity_;
      boundaryEntity_ = NULL;
    }

    if(manageNeighInfo_)
      manageNeighInfo_ = elinfoProvider.freeObjectEntity(manageNeighInfo_);
  }

  template< int dim, int dimworld>
  inline AlbertGridIntersectionIterator<dim,dimworld>::~AlbertGridIntersectionIterator ()
  {
    freeObjects();
  }

  template< int dim, int dimworld>
  inline AlbertGridIntersectionIterator<dim,dimworld>::
  AlbertGridIntersectionIterator(AlbertGrid<dim,dimworld> &grid, int level) :
    grid_( &grid ), level_ (level) , neighborCount_ (dim+1), virtualEntity_ (NULL)
    , fakeNeigh_ (NULL)
    , neighGlob_ (NULL) , elInfo_ (NULL)
    , manageObj_ (NULL)
    , manageInterEl_ (NULL)
    , manageNeighEl_ (NULL)
    , boundaryEntity_ (NULL)
    , manageNeighInfo_ (NULL) , neighElInfo_ (NULL) {}


  template< int dim, int dimworld>
  inline AlbertGridIntersectionIterator<dim,dimworld>::AlbertGridIntersectionIterator
    (AlbertGrid<dim,dimworld> & grid, int level, ALBERT EL_INFO *elInfo ) :
    grid_( &grid ) , level_ (level), neighborCount_ (0), elInfo_ ( elInfo )
    , fakeNeigh_ (NULL) , neighGlob_ (NULL)
    , virtualEntity_ (NULL)
    , builtNeigh_ (false)
    , manageObj_ (NULL)
    , manageInterEl_ (NULL)
    , manageNeighEl_ (NULL)
    , boundaryEntity_ (NULL)
  {
    manageNeighInfo_ = elinfoProvider.getNewObjectEntity();
    neighElInfo_ = manageNeighInfo_->item;
  }

  // empty constructor
  template< int dim, int dimworld>
  inline AlbertGridIntersectionIterator<dim,dimworld>::
  AlbertGridIntersectionIterator ( ) :
    grid_( NULL ) , level_ ( -1 ), neighborCount_ ( -1 ), elInfo_ ( NULL )
    , fakeNeigh_ (NULL) , neighGlob_ (NULL)
    , virtualEntity_ (NULL)
    , builtNeigh_ (false)
    , manageObj_ (NULL)
    , manageInterEl_ (NULL)
    , manageNeighEl_ (NULL)
    , boundaryEntity_ (NULL) {}

  template< int dim, int dimworld>
  inline void AlbertGridIntersectionIterator<dim,dimworld>::makeBegin
    (AlbertGrid<dim,dimworld> &grid, int level, ALBERT EL_INFO *elInfo )
  {
    grid_ = &grid;
    level_ = level;
    elInfo_ = elInfo;
    neighborCount_ = 0;
    builtNeigh_ = false;

    // remove old objects
    freeObjects();

    manageNeighInfo_ = elinfoProvider.getNewObjectEntity();
    neighElInfo_ = manageNeighInfo_->item;
  }

  template< int dim, int dimworld>
  inline void AlbertGridIntersectionIterator<dim,dimworld>::makeEnd
    (AlbertGrid<dim,dimworld> &grid, int level )
  {
    grid_ = &grid;
    level_ = level;
    elInfo_ = NULL;
    neighborCount_ = dim+1;
    builtNeigh_ = false;

    // remove old objects
    freeObjects();
  }

  template< int dim, int dimworld>
  inline AlbertGridIntersectionIterator<dim,dimworld>&
  AlbertGridIntersectionIterator<dim,dimworld>::
  operator ++()
  {
    builtNeigh_ = false;
    // is like go to the next neighbour
    neighborCount_++;
    return (*this);
  }

  template< int dim, int dimworld>
  inline AlbertGridIntersectionIterator<dim,dimworld>&
  AlbertGridIntersectionIterator<dim,dimworld>::operator ++(int steps)
  {
    neighborCount_ += steps;
    if(neighborCount_ > dim+1) neighborCount_ = dim+1;
    builtNeigh_ = false;

    return (*this);
  }

  template< int dim, int dimworld>
  inline bool AlbertGridIntersectionIterator<dim,dimworld>::operator ==
    (const AlbertGridIntersectionIterator& I) const
  {
    return (neighborCount_ == I.neighborCount_);
  }

  template< int dim, int dimworld>
  inline bool AlbertGridIntersectionIterator<dim,dimworld>::
  operator !=(const AlbertGridIntersectionIterator& I) const
  {
    return (neighborCount_ != I.neighborCount_);
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >&
  AlbertGridIntersectionIterator<dim,dimworld>::
  operator *()
  {
    if(!builtNeigh_)
    {
      if(!manageObj_)
      {
        manageObj_ = grid_->entityProvider_.getNewObjectEntity( *grid_ ,level_);
        virtualEntity_ = manageObj_->item;
        virtualEntity_->setLevel(level_);
        memcpy(neighElInfo_,elInfo_,sizeof(ALBERT EL_INFO));
      }

      setupVirtEn();
    }
    return (*virtualEntity_);
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >*
  AlbertGridIntersectionIterator<dim,dimworld>::
  operator ->()
  {
    if(!builtNeigh_)
    {
      if(!manageObj_)
      {
        manageObj_ = grid_->entityProvider_.getNewObjectEntity( *grid_ ,level_);
        virtualEntity_ = manageObj_->item;
        virtualEntity_->setLevel(level_);
        memcpy(neighElInfo_,elInfo_,sizeof(ALBERT EL_INFO));
      }

      setupVirtEn();
    }
    return virtualEntity_;
  }

  template< int dim, int dimworld>
  inline AlbertGridBoundaryEntity<dim,dimworld>&
  AlbertGridIntersectionIterator<dim,dimworld>::boundaryEntity ()
  {
    if(!boundaryEntity_)
    {
      boundaryEntity_ = new AlbertGridBoundaryEntity<dim,dimworld> ();
    }
    boundaryEntity_->setElInfo(elInfo_,neighborCount_);
    return (*boundaryEntity_);
  }

  template< int dim, int dimworld>
  inline bool AlbertGridIntersectionIterator<dim,dimworld>::boundary()
  {
    return (elInfo_->boundary[neighborCount_] != NULL);
  }

  template< int dim, int dimworld>
  inline bool AlbertGridIntersectionIterator<dim,dimworld>::neighbor()
  {
    return (elInfo_->neigh[neighborCount_] != NULL);
  }

  template< int dim, int dimworld>
  inline Vec<dimworld,albertCtype>& AlbertGridIntersectionIterator<dim,dimworld>::
  unit_outer_normal(Vec<dim-1,albertCtype>& local)
  {
    // calculates the outer_normal
    Vec<dimworld,albertCtype>& tmp = outer_normal(local);

    double norm_1 = (1.0/tmp.norm2());
    assert(norm_1 > 0.0);
    outNormal_ *= norm_1;

    return outNormal_;
  }

  template< int dim, int dimworld>
  inline Vec<dimworld,albertCtype>& AlbertGridIntersectionIterator<dim,dimworld>::
  unit_outer_normal()
  {
    // calculates the outer_normal
    Vec<dimworld,albertCtype>& tmp = outer_normal();

    double norm_1 = (1.0/tmp.norm2());
    assert(norm_1 > 0.0);
    outNormal_ *= norm_1;

    return outNormal_;
  }

  template< int dim, int dimworld>
  inline Vec<dimworld,albertCtype>& AlbertGridIntersectionIterator<dim,dimworld>::
  outer_normal(Vec<dim-1,albertCtype>& local)
  {
    // we dont have curved boundary
    // therefore return outer_normal
    return outer_normal();
  }

  template< int dim, int dimworld>
  inline Vec<dimworld,albertCtype>& AlbertGridIntersectionIterator<dim,dimworld>::
  outer_normal()
  {
    std::cout << "outer_normal() not correctly implemented yet! \n";
    for(int i=0; i<dimworld; i++)
      outNormal_(i) = 0.0;

    return outNormal_;
  }

  template <>
  inline Vec<2,albertCtype>& AlbertGridIntersectionIterator<2,2>::
  outer_normal()
  {
    // seems to work
    ALBERT REAL_D *coord = elInfo_->coord;

    outNormal_(0) = -(coord[(neighborCount_+1)%3][1] - coord[(neighborCount_+2)%3][1]);
    outNormal_(1) =   coord[(neighborCount_+1)%3][0] - coord[(neighborCount_+2)%3][0];

    return outNormal_;
  }

  template <>
  inline Vec<3,albertCtype>& AlbertGridIntersectionIterator<3,3>::
  outer_normal()
  {
    enum { dim = 3 };
    // rechne Kreuzprodukt der Vectoren aus
    ALBERT REAL_D *coord = elInfo_->coord;
    Vec<3,albertCtype> v;
    Vec<3,albertCtype> u;
    const albertCtype val = 0.5;

    // neighborCount_ is the local face number
    const int * localFaces = AlbertHelp::localTetraFaceNumber[neighborCount_];
    for(int i=0; i<dim; i++)
    {
      v(i) =   coord[localFaces[1]][i] - coord[localFaces[0]][i];
      u(i) =   coord[localFaces[2]][i] - coord[localFaces[1]][i];

      //v(i) = coord[(neighborCount_+2)%4][i] - coord[(neighborCount_+1)%4][i];
      //u(i) = coord[(neighborCount_+3)%4][i] - coord[(neighborCount_+2)%4][i];
    }

    // outNormal_ has length 3
    for(int i=0; i<dim; i++)
      outNormal_(i) = u((i+1)%dim)*v((i+2)%dim) - u((i+2)%dim)*v((i+1)%dim);

    outNormal_ *= val;
    return outNormal_;
  }

  template< int dim, int dimworld>
  inline AlbertGridElement< dim-1, dim >&
  AlbertGridIntersectionIterator<dim,dimworld>::
  intersection_self_local()
  {
    std::cout << "\nintersection_self_local not checked until now! \n";
    if(!manageInterEl_)
    {
      manageInterEl_ = grid_->interSelfProvider_.getNewObjectEntity();
      fakeNeigh_ = manageInterEl_->item;
    }

    fakeNeigh_->builtGeom(elInfo_,neighborCount_,0,0);
    return (*fakeNeigh_);
  }

  template< int dim, int dimworld>
  inline AlbertGridElement< dim-1, dimworld >&
  AlbertGridIntersectionIterator<dim,dimworld>::
  intersection_self_global()
  {
    if(!manageNeighEl_)
    {
      manageNeighEl_ = grid_->interNeighProvider_.getNewObjectEntity();
      neighGlob_ = manageNeighEl_->item;
    }

    if(neighGlob_->builtGeom(elInfo_,neighborCount_,0,0))
      return (*neighGlob_);
    else
      abort();
    return (*neighGlob_);
  }

  template< int dim, int dimworld>
  inline AlbertGridElement< dim-1, dim >&
  AlbertGridIntersectionIterator<dim,dimworld>::
  intersection_neighbor_local()
  {
    std::cout << "intersection_neighbor_local not check until now! \n";
    if(!manageInterEl_)
    {
      manageInterEl_ = grid_->interSelfProvider_.getNewObjectEntity();
      fakeNeigh_ = manageInterEl_->item;
    }

    if(!builtNeigh_)
    {
      setupVirtEn();
    }

    fakeNeigh_->builtGeom(neighElInfo_,neighborCount_,0,0);
    return (*fakeNeigh_);
  }

  template< int dim, int dimworld>
  inline AlbertGridElement< dim-1, dimworld >&
  AlbertGridIntersectionIterator<dim,dimworld>::
  intersection_neighbor_global()
  {
    std::cout << "intersection_neighbor_global not check until now! \n";
    if(!manageNeighEl_)
    {
      manageNeighEl_ = grid_->interNeighProvider_.getNewObjectEntity();
      neighGlob_ = manageNeighEl_->item;
    }

    // built neighGlob_ first
    if(!builtNeigh_)
    {
      setupVirtEn();
    }
    neighGlob_->builtGeom(elInfo_,neighborCount_,0,0);
    return (*neighGlob_);
  }

  template< int dim, int dimworld>
  inline int AlbertGridIntersectionIterator<dim,dimworld>::
  number_in_self ()
  {
    return neighborCount_;
  }

  template< int dim, int dimworld>
  inline int AlbertGridIntersectionIterator<dim,dimworld>::
  number_in_neighbor ()
  {
    return elInfo_->opp_vertex[neighborCount_];
  }

  // setup neighbor element with the information of elInfo_
  template< int dim, int dimworld>
  inline void AlbertGridIntersectionIterator<dim,dimworld>::setupVirtEn()
  {

    // set the neighbor element as element
    neighElInfo_->el = elInfo_->neigh[neighborCount_];

    int vx = elInfo_->opp_vertex[neighborCount_];

    /* now it's ok */
    memcpy(&(neighElInfo_->coord[vx]), &(elInfo_->opp_coord[neighborCount_]),
           dimworld*sizeof(ALBERT REAL));

    for(int i=1; i<dim+1; i++)
    {
      int nb = (((neighborCount_-i)%(dim+1)) +dim+1)%(dim+1);
      memcpy(&neighElInfo_->coord[(vx+i)%(dim+1)], &elInfo_->coord[nb],
             dimworld*sizeof(ALBERT REAL));
    }
    /* works, tested many times */

    virtualEntity_->setElInfo(neighElInfo_);
    builtNeigh_ = true;
  }

  // end IntersectionIterator


  //*******************************************************
  //
  // --AlbertGridLevelIterator
  // --LevelIterator
  //
  //*******************************************************

  //***********************************************************
  //  some template specialization of goNextEntity
  //***********************************************************
  // default implementation, go next elInfo
  template<int codim, int dim, int dimworld>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<codim,dim,dimworld >::
  goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextElInfo(stack,elinfo_old);
  }

  // specializations for codim 1, go next face
  template <>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<1,2,2>::
  goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextFace(stack,elinfo_old);
  }
  template <>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<1,2,3>::
  goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextFace(stack,elinfo_old);
  }
  template <>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<1,3,3>::
  goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextFace(stack,elinfo_old);
  }

  // specialization for codim 2, if dim > 2, go next edge,
  // only if dim == dimworld == 3
  template <>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<2,3,3>::
  goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextEdge(stack,elinfo_old);
  }

  // specialization for codim == dim , go next vertex
  template <>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<2,2,2>::
  goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextVertex(stack,elinfo_old);
  }
  template <>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<2,2,3>::
  goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextVertex(stack,elinfo_old);
  }
  template <>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<3,3,3>::
  goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextVertex(stack,elinfo_old);
  }
  // end specialization of goNextEntity
  //***************************************

  template<int codim, int dim, int dimworld>
  inline void AlbertGridLevelIterator<codim,dim,dimworld >::
  makeIterator()
  {
    level_ = 0;
    vertex_ = 0;
    face_ = 0;
    edge_ = 0;
    vertexMarker_ = NULL;

    virtualEntity_.setTraverseStack(NULL);
    virtualEntity_.setElInfo(NULL,0,0,0,0);
  }

  // Make LevelIterator with point to element from previous iterations
  template<int codim, int dim, int dimworld>
  inline AlbertGridLevelIterator<codim,dim,dimworld >::
  AlbertGridLevelIterator(AlbertGrid<dim,dimworld> &grid, int travLevel, bool leafIt ) :
    grid_(grid), level_ (travLevel) ,  virtualEntity_(grid,0) ,leafIt_(leafIt)
  {
    makeIterator();
  }

  // Make LevelIterator with point to element from previous iterations
  template<int codim, int dim, int dimworld>
  inline AlbertGridLevelIterator<codim,dim,dimworld >::
  AlbertGridLevelIterator(AlbertGrid<dim,dimworld> &grid, int level,
                          ALBERT EL_INFO *elInfo,int elNum,int face,int edge,int vertex) :
    grid_(grid), level_ (level) , virtualEntity_(grid,level) , elNum_ ( elNum ) , face_ ( face ) ,
    edge_ ( edge ), vertex_ ( vertex ) , leafIt_(false)
  {
    vertexMarker_ = NULL;
    virtualEntity_.setTraverseStack(NULL);

    if(elInfo)
    {
      // diese Methode muss neu geschrieben werden, da man
      // die ParentElement explizit speichern moechte.
      virtualEntity_.setElInfo(elInfo,elNum_,face_,edge_,vertex_);
    }
  }

  template<int codim, int dim, int dimworld>
  inline AlbertGridLevelIterator<codim,dim,dimworld >::
  AlbertGridLevelIterator(AlbertGrid<dim,dimworld> &grid,
                          AlbertMarkerVector * vertexMark,
                          int travLevel,bool leafIt) : grid_(grid) , level_ (travLevel), leafIt_(leafIt)
                                                       , virtualEntity_(grid,travLevel)
  {
    ALBERT MESH * mesh = grid_.getMesh();

    if(mesh)
    {
      elNum_ = 0;
      vertex_ = 0;
      face_ = 0;
      edge_ = 0;

      vertexMarker_ = vertexMark;

      ALBERT FLAGS travFlags = FILL_ANY; //FILL_COORDS | FILL_NEIGH;

      if((travLevel < 0) || (travLevel > grid_.maxlevel()))
      {
        printf("AlbertGridLevelIterator<%d,%d,%d>: Wrong Level (%d) in Contructor, grid.maxlevel() = %d ! \n",
               codim,dim,dimworld,travLevel, grid_.maxlevel());
        abort();
      }

      // CALL_LEAF_EL is not used anymore
      travFlags = travFlags | CALL_LEAF_EL_LEVEL;

      // get traverse_stack
      manageStack_.makeItNew(true);

      virtualEntity_.setTraverseStack(manageStack_.getStack());

      // diese Methode muss neu geschrieben werden, da man
      // die ParentElement explizit speichern moechte.
      ALBERT EL_INFO* elInfo =
        goFirstElement(manageStack_.getStack(), mesh, travLevel,travFlags);

      virtualEntity_.setElInfo(elInfo,elNum_,face_,edge_,vertex_);
    }
    else
      makeIterator();

  };

  template<int codim, int dim, int dimworld>
  inline bool AlbertGridLevelIterator<codim,dim,dimworld >::
  operator ==(const AlbertGridLevelIterator<codim,dim,dimworld > &I) const
  {
    return (virtualEntity_.getElInfo() == I.virtualEntity_.getElInfo());
  }

  template<int codim, int dim, int dimworld>
  inline bool AlbertGridLevelIterator < codim,dim,dimworld >::
  operator !=(const AlbertGridLevelIterator< codim,dim,dimworld > & I) const
  {
    return (virtualEntity_.getElInfo() != I.virtualEntity_.getElInfo() );
  }


  // gehe zum naechsten Element, wie auch immer
  template<int codim, int dim, int dimworld>
  inline AlbertGridLevelIterator < codim,dim,dimworld >&
  AlbertGridLevelIterator < codim,dim,dimworld >::operator ++()
  {
    elNum_++;
    virtualEntity_.setElInfo(
      goNextEntity(manageStack_.getStack(),virtualEntity_.getElInfo()),
      elNum_,face_,edge_,vertex_);

    return (*this);
  }

  template<int codim, int dim, int dimworld>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<codim,dim,dimworld >::
  goNextFace(ALBERT TRAVERSE_STACK *stack, ALBERT EL_INFO *elInfo)
  {
    // go next Element, if face_ > numberOfVertices, then go to next elInfo
    face_++;
    if(face_ >= (dim+1)) // dim+1 Faces
    {
      elInfo = goNextElInfo(stack, elInfo);
      face_ = 0;
    }

    if(!elInfo)
      return elInfo; // if no more Faces, return

    if( (elInfo->neigh[face_]) &&
        (elInfo->el->index > elInfo->neigh[face_]->index))
    {
      // if reachedFace before, go next
      elInfo = goNextFace(stack,elInfo);
    }

    return elInfo;
  }

  template<int codim, int dim, int dimworld>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<codim,dim,dimworld >::
  goNextEdge(ALBERT TRAVERSE_STACK *stack, ALBERT EL_INFO *elInfo)
  {
    std::cout << "EdgeIterator not implemented for 3d!\n";
    return NULL;
  }

  template<int codim, int dim, int dimworld>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<codim,dim,dimworld >::
  goNextVertex(ALBERT TRAVERSE_STACK *stack, ALBERT EL_INFO *elInfo)
  {
    // go next Element, Vertex 0
    // treat Vertices like Faces
    vertex_++;
    if(vertex_ >= (dim+1)) // dim+1 Vertices
    {
      elInfo = goNextElInfo(stack, elInfo);
      vertex_ = 0;
    }

    if(!elInfo)
      return elInfo; // if no more Vertices, return

    // go next, if Vertex is not treated on this Element
    if(vertexMarker_->notOnThisElement(elInfo->el,level_,vertex_))
      elInfo = goNextVertex(stack,elInfo);

    return elInfo;
  }


  // gehe zum i Schritte weiter , wie auch immer
  template<int codim, int dim, int dimworld>
  inline AlbertGridLevelIterator < codim,dim,dimworld >&
  AlbertGridLevelIterator < codim,dim,dimworld >::operator++(int steps)
  {
    // die 0 ist wichtig, weil Face 0, heist hier jetzt Element
    ALBERT EL_INFO *elInfo =
      goNextEntity(manageStack_.getStack(), virtualEntity_.getElInfo());
    for(int i=1; i<= steps; i++)
      elInfo = goNextEntity(manageStack_.getStack(),virtualEntity_.getElInfo());

    virtualEntity_.setElInfo(elInfo,face_,edge_,vertex_);

    return (*this);
  }

  template<int codim, int dim, int dimworld>
  inline AlbertGridEntity<codim, dim, dimworld> &
  AlbertGridLevelIterator< codim,dim,dimworld >::operator *()
  {
    return virtualEntity_;
  }

  template<int codim, int dim, int dimworld>
  inline AlbertGridEntity< codim,dim,dimworld >*
  AlbertGridLevelIterator< codim,dim,dimworld >::operator ->()
  {
    return &virtualEntity_;
  }


  template<int codim, int dim, int dimworld>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<codim,dim,dimworld >::
  goFirstElement(ALBERT TRAVERSE_STACK *stack,ALBERT MESH *mesh, int level,
                 ALBERT FLAGS fill_flag)
  {
    FUNCNAME("goFirstElement");

    if (!stack)
    {
      ALBERT_ERROR("no traverse stack\n");
      return(nil);
    }

    stack->traverse_mesh      = mesh;
    stack->traverse_level     = level;
    stack->traverse_fill_flag = fill_flag;

    if (stack->stack_size < 1)
      enlargeTraverseStack(stack);

    for (int i=0; i<stack->stack_size; i++)
      stack->elinfo_stack[i].fill_flag = fill_flag & FILL_ANY;

    stack->elinfo_stack[0].mesh = stack->elinfo_stack[1].mesh = mesh;

    if (fill_flag & CALL_LEAF_EL_LEVEL)
    {
      ALBERT_TEST_EXIT(level >= 0) ("invalid level: %d\n",level);
    }

    stack->traverse_mel = NULL;
    stack->stack_used   = 0;
    stack->el_count     = 0;

    // go to first enInfo, therefore goNextElInfo for all codims
    return(goNextElInfo(stack,NULL));
  }


  template<int codim, int dim, int dimworld>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<codim,dim,dimworld >::
  goNextElInfo(ALBERT TRAVERSE_STACK *stack, ALBERT EL_INFO *elinfo_old)
  {
    FUNCNAME("goNextElInfo");
    ALBERT EL_INFO       *elinfo=NULL;

    if (stack->traverse_fill_flag & CALL_LEAF_EL_LEVEL)
    {
      // this is done in traverse_next
      if (stack->stack_used)
      {
        ALBERT_TEST_EXIT(elinfo_old == stack->elinfo_stack+stack->stack_used)
          ("invalid old elinfo\n");
      }
      else
      {
        ALBERT_TEST_EXIT(elinfo_old == nil) ("invalid old elinfo != nil\n");
      }

      // overloaded traverse_leaf_el_level, is not implemened in ALBERT yet
      elinfo = traverseElLevel(stack);

      // if leafIt_ == false go to elements only on desired level
      if((elinfo) && (!leafIt_))
      {
        if(elinfo->level == stack->traverse_level)
          okReturn_ = true;

        while(!okReturn_)
        {
          elinfo = traverseElLevel(stack);
          if(!elinfo) okReturn_ = true;
        }
        stack->el_count++;
      }
    }
    else
    {
      std::cout << "Warning: dont use traverse_next becasue we overloaded fill_elinfo\n";
      // the original ALBERT traverse_next, goes to next elinfo,
      // depending on the flags choosen
      elinfo = ALBERT traverse_next(stack,elinfo_old);
    }
    return(elinfo);
  }


  template<int codim, int dim, int dimworld>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<codim,dim,dimworld >::
  traverseElLevel(ALBERT TRAVERSE_STACK *stack)
  {
    FUNCNAME("traverseElLevel");
    ALBERT EL *el;
    int i;
    okReturn_ = false;

    if (stack->stack_used == 0) /* first call */
    {
      stack->traverse_mel = stack->traverse_mesh->first_macro_el;
      if (stack->traverse_mel == nil) return(nil);

      stack->stack_used = 1;
      fill_macro_info(stack->traverse_mel,
                      stack->elinfo_stack+stack->stack_used);
      stack->info_stack[stack->stack_used] = 0;

      el = stack->elinfo_stack[stack->stack_used].el;
      if ((el == nil) || (el->child[0] == nil)) {
        return(stack->elinfo_stack+stack->stack_used);
      }
    }
    else
    {
      el = stack->elinfo_stack[stack->stack_used].el;

      /* go up in tree until we can go down again */
      while((stack->stack_used > 0) &&
            ((stack->info_stack[stack->stack_used] >= 2) || (el->child[0]==nil)
             || ( stack->traverse_level <=
                  (stack->elinfo_stack+stack->stack_used)->level)) )
      // Aenderung hier
      {
        stack->stack_used--;
        el = stack->elinfo_stack[stack->stack_used].el;
      }
      /* goto next macro element */
      if (stack->stack_used < 1) {

        stack->traverse_mel = stack->traverse_mel->next;
        if (stack->traverse_mel == nil) return(nil);

        stack->stack_used = 1;
        fill_macro_info(stack->traverse_mel,
                        stack->elinfo_stack+stack->stack_used);
        stack->info_stack[stack->stack_used] = 0;

        el = stack->elinfo_stack[stack->stack_used].el;
        if ((el == nil) || (el->child[0] == nil))
        {
          return(stack->elinfo_stack+stack->stack_used);
        }
      }
    }

    /* go down tree until leaf oder level*/
    while (el->child[0] &&
           ( stack->traverse_level > (stack->elinfo_stack+stack->stack_used)->level))
    // Aenderung hier
    {
      if(stack->stack_used >= stack->stack_size-1)
        enlargeTraverseStack(stack);

      i = stack->info_stack[stack->stack_used];
      el = el->child[i];
      stack->info_stack[stack->stack_used]++;

      //ALBERT fill_elinfo(i, stack->elinfo_stack+stack->stack_used,
      //               stack->elinfo_stack+stack->stack_used+1);
      grid_.fillElInfo(i, level_, stack->elinfo_stack+stack->stack_used,
                       stack->elinfo_stack+stack->stack_used+1, false);

      stack->stack_used++;

      ALBERT_TEST_EXIT(stack->stack_used < stack->stack_size)
        ("stack_size=%d too small, level=(%d,%d)\n",
        stack->stack_size, stack->elinfo_stack[stack->stack_used].level);

      stack->info_stack[stack->stack_used] = 0;

      if(stack->traverse_level == (stack->elinfo_stack+stack->stack_used)->level)
      {
        okReturn_ = true;
      }
    }

    return(stack->elinfo_stack+stack->stack_used);
  }

  template<int codim, int dim, int dimworld>
  inline int AlbertGridLevelIterator<codim,dim,dimworld >::level()
  {
    return (manageStack_.getStack())->stack_used;
  }


  template< int dim, int dimworld>
  inline AlbertGridHierarchicIterator<dim,dimworld>
  AlbertGridEntity < 0, dim ,dimworld >::hbegin(int maxlevel)
  {
    // Kopiere alle Eintraege des stack, da man im Stack weiterlaeuft und
    // sich deshalb die Werte anedern koennen, der elinfo_stack bleibt jedoch
    // der gleiche, deshalb kann man auch nur nach unten, d.h. zu den Kindern
    // laufen
    AlbertGridHierarchicIterator<dim,dimworld>
    it(grid_,travStack_,level(),maxlevel);
    return it;
  }

  template< int dim, int dimworld>
  inline AlbertGridHierarchicIterator<dim,dimworld>
  AlbertGridEntity < 0, dim ,dimworld >::hend(int maxlevel)
  {
    AlbertGridHierarchicIterator<dim,dimworld> it(grid_,level(),maxlevel);
    return it;
  }

  template< int dim, int dimworld>
  inline AlbertGridIntersectionIterator<dim,dimworld>
  AlbertGridEntity < 0, dim ,dimworld >::ibegin()
  {
    AlbertGridIntersectionIterator<dim,dimworld> it(grid_,level(),elInfo_);
    return it;
  }

  template< int dim, int dimworld>
  inline AlbertGridIntersectionIterator<dim,dimworld>
  AlbertGridEntity < 0, dim ,dimworld >::iend()
  {
    AlbertGridIntersectionIterator<dim,dimworld> it(grid_,level());
    return it;
  }

  template< int dim, int dimworld>
  inline void AlbertGridEntity < 0, dim ,dimworld >::
  ibegin(AlbertGridIntersectionIterator<dim,dimworld> &it)
  {
    it.makeBegin( grid_ , level() , elInfo_ );
  }

  template< int dim, int dimworld>
  inline void AlbertGridEntity < 0, dim ,dimworld >::
  iend(AlbertGridIntersectionIterator<dim,dimworld> &it)
  {
    it.makeEnd( grid_ , level() );
  }
  //*********************************************************************
  //
  //  AlbertMarkerVertex
  //
  //*********************************************************************
  inline bool AlbertMarkerVector::
  notOnThisElement(ALBERT EL * el, int level, int localNum)
  {
    return (vec_[ numVertex_ * level + el->dof[localNum][0]] != el->index);
  }

  template <class GridType>
  inline void AlbertMarkerVector::markNewVertices(GridType &grid)
  {
    enum { dim      = GridType::dimension };
    enum { dimworld = GridType::dimensionworld };

    ALBERT MESH *mesh_ = grid.getMesh();

    int nvx = mesh_->n_vertices;
    // remember the number of vertices of the mesh
    numVertex_ = nvx;

    int maxlevel = grid.maxlevel();

    int number = (maxlevel+1) * nvx;
    if(vec_.size() < number) vec_.resize( 2 * number );
    for(int i=0; i<vec_.size(); i++) vec_[i] = -1;

    for(int level=0; level <= maxlevel; level++)
    {
      typedef typename GridType::Traits<0>::LevelIterator LevelIteratorType;
      LevelIteratorType endit = grid.template lend<0> (level);
      for(LevelIteratorType it = grid.template lbegin<0> (level); it != endit; ++it)
      {
        for(int local=0; local<dim+1; local++)
        {
          int num = it->getElInfo()->el->dof[local][0];
          if( vec_[level * nvx + num] == -1 )
            vec_[level * nvx + num] = it->el_index();
        }
      }
      // remember the number of entity on level and codim = 0
    }
  }

  inline void AlbertMarkerVector::print()
  {
    printf("\nEntries %d \n",vec_.size());
    for(int i=0; i<vec_.size(); i++)
      printf("Konten %d visited on Element %d \n",i,vec_[i]);
  }

  //***********************************************************************
  //
  // --AlbertGrid
  // --Grid
  //
  //***********************************************************************
  template < int dim, int dimworld >
  inline AlbertGrid < dim, dimworld >::AlbertGrid() :
    mesh_ (NULL), maxlevel_ (0) , wasChanged_ (false), time_ (0.0)
    , isMarked_ (false)
  {
    vertexMarker_ = new AlbertMarkerVector ();
  }

  template < int dim, int dimworld >
  inline AlbertGrid < dim, dimworld >::AlbertGrid(const char *MacroTriangFilename) :
    mesh_ (NULL), maxlevel_ (0) , wasChanged_ (false), time_ (0.0)
    , isMarked_ (false)
  {
    assert(dimworld == DIM_OF_WORLD);
    assert(dim      == DIM);

    bool makeNew = true;
    {
      std::fstream file (MacroTriangFilename,std::ios::in);
      std::basic_string <char> str,str1;
      file >> str1; str = str1.assign(str1,0,3);
      // With that Albert MacroTriang starts DIM or DIM_OF_WORLD
      if (str != "DIM") makeNew = false;
      file.close();
    }

    vertexMarker_ = new AlbertMarkerVector ();
    if(makeNew)
    {
      mesh_ = ALBERT get_mesh("AlbertGrid", ALBERT AlbertHelp::initDofAdmin, ALBERT AlbertHelp::initLeafData);
      ALBERT read_macro(mesh_, MacroTriangFilename, ALBERT AlbertHelp::initBoundary);

      numberOfEntitys_[0]     = mesh_->n_hier_elements;
      numberOfEntitys_[1]     = 0;
      numberOfEntitys_[dim-1] = 0;
      numberOfEntitys_[dim]   = mesh_->n_vertices;

      // we have at least one level, level 0
      maxlevel_ = 0;
      maxHierIndex_ = mesh_->n_hier_elements;
      neighOnLevel_.resize( maxHierIndex_ );

      vertexMarker_->markNewVertices( *this );

      markNew();
      wasChanged_ = true;
      isMarked_ = false;
    }
    else
    {
      read (MacroTriangFilename,time_,0);
    }
  }


  template < int dim, int dimworld >
  inline AlbertGrid < dim, dimworld >::~AlbertGrid()
  {
    if(vertexMarker_) delete vertexMarker_;
    ALBERT free_mesh(mesh_);
  };

  template < int dim, int dimworld > template<int codim>
  inline AlbertGridLevelIterator<codim,dim,dimworld>
  AlbertGrid < dim, dimworld >::lbegin (int level)
  {
    AlbertGridLevelIterator<codim,dim,dimworld> it(*this,vertexMarker_,level);
    return it;
  }

  template < int dim, int dimworld > template<int codim>
  inline AlbertGridLevelIterator<codim,dim,dimworld>
  AlbertGrid < dim, dimworld >::lend (int level)
  {
    AlbertGridLevelIterator<codim,dim,dimworld> it((*this),level);
    return it;
  }

  //*****************************************************************
  template < int dim, int dimworld >
  inline AlbertGridLevelIterator<0,dim,dimworld>
  AlbertGrid < dim, dimworld >::leafbegin (int level)
  {
    bool leaf = true;
    AlbertGridLevelIterator<0,dim,dimworld> it(*this,vertexMarker_,level,leaf);
    return it;
  }

  template < int dim, int dimworld >
  inline AlbertGridLevelIterator<0,dim,dimworld>
  AlbertGrid < dim, dimworld >::leafend (int level)
  {
    bool leaf = true;
    AlbertGridLevelIterator<0,dim,dimworld> it((*this),level,leaf);
    return it;
  }


  //**************************************
  //  refine and coarsen methods
  //**************************************
  template < int dim, int dimworld >
  inline bool AlbertGrid < dim, dimworld >::
  globalRefine(int refCount)
  {
    unsigned char flag;
    typedef LeafIterator LeafIt;
    LeafIt endit = leafend(maxlevel());
    for(LeafIt it = leafbegin(maxlevel()); it != endit; ++it)
    {
      (*it).mark(refCount);
    }

    flag = ALBERT AlbertRefine ( mesh_ );
    wasChanged_ = (flag == 0) ? false : true;
    if(wasChanged_)
    {
      calcExtras();
    }

    postAdapt();
    return wasChanged_;
  }

  template < int dim, int dimworld >
  inline bool AlbertGrid < dim, dimworld >::preAdapt()
  {
    return isMarked_;
  }

  template < int dim, int dimworld >
  inline bool AlbertGrid < dim, dimworld >::postAdapt()
  {
    isMarked_ = false;
    return wasChanged_;
  }

  template < int dim, int dimworld >
  inline void AlbertGrid < dim, dimworld >::setMark (bool isMarked)
  {
    isMarked_ = isMarked;
  }

  template < int dim, int dimworld >
  inline bool AlbertGrid < dim, dimworld >::checkElNew (int num) const
  {
    return gIndex_.isNew(num);
  }

  template < int dim, int dimworld >
  inline bool AlbertGrid < dim, dimworld >::adapt()
  {
    unsigned char flag;
    bool refined = false;
    wasChanged_ = false;

    flag = ALBERT AlbertRefine ( mesh_ );
    refined = (flag == 0) ? false : true;

    if(isMarked_) // true if a least on element is marked for coarseing
      flag = ALBERT AlbertCoarsen( mesh_ );

    if(!refined)
    {
      wasChanged_ = (flag == 0) ? false : true;
    }
    else
      wasChanged_ = true;

    if(wasChanged_)
    {
      calcExtras();
      isMarked_ = false;
    }

    return refined;
  }

  template < int dim, int dimworld >
  inline int AlbertGrid < dim, dimworld >::maxlevel() const
  {
    return maxlevel_;
  }

  template < int dim, int dimworld >
  inline int AlbertGrid < dim, dimworld >::size (int level, int codim) const
  {
    return const_cast<AlbertGrid<dim,dimworld> *> (this)->size(level,codim);
  }

  template < int dim, int dimworld >
  inline int AlbertGrid < dim, dimworld >::size (int level, int codim)
  {
    enum { numCodim = dim+1 };
    int ind = (level * numCodim) + codim;

    if(size_[ind] == -1)
    {
      int numberOfElements = 0;

      if(codim == 0)
      {
        AlbertGridLevelIterator<0,dim,dimworld> endit = lend<0>(level);
        for(AlbertGridLevelIterator<0,dim,dimworld> it = lbegin<0>(level);
            it != endit; ++it)
          numberOfElements++;
      }
      if(codim == 1)
      {
        AlbertGridLevelIterator<1,dim,dimworld> endit = lend<1>(level);
        for(AlbertGridLevelIterator<1,dim,dimworld> it = lbegin<1>(level);
            it != endit; ++it)
          numberOfElements++;
      }
      if(codim == 2)
      {
        AlbertGridLevelIterator<2,dim,dimworld> endit = lend<2>(level);
        for(AlbertGridLevelIterator<2,dim,dimworld> it = lbegin<2>(level);
            it != endit; ++it)
          numberOfElements++;
      }

      if(codim == 3)
      {
        AlbertGridLevelIterator<3,dim,dimworld> endit = lend<3>(level);
        for(AlbertGridLevelIterator<3,dim,dimworld> it = lbegin<3>(level);
            it != endit; ++it)
          numberOfElements++;
      }

      size_[ind] = numberOfElements;
      return numberOfElements;
    }
    else
    {
      return size_[ind];
    }
  }

  template < int dim, int dimworld >
  inline int AlbertGrid < dim, dimworld >::hierSize () const
  {
    return gIndex_.size();
  }

  template < int dim, int dimworld >
  inline void AlbertGrid < dim, dimworld >::calcExtras ()
  {
    // save number of old entities
    for(int i=0; i<dim+1; i++)
      oldNumberOfEntities_[i] = numberOfEntitys_[i];

    // calc new number of entities
    numberOfEntitys_[0]     = mesh_->n_hier_elements; // elements
    numberOfEntitys_[1]     = 1;                    // faces
    numberOfEntitys_[dim-1] = 1;                    // edges
    numberOfEntitys_[dim]   = mesh_->n_vertices;    // vertices

    // determine new maxlevel and mark neighbours
    maxlevel_ = ALBERT AlbertHelp::calcMaxLevelAndMarkNeighbours( mesh_, neighOnLevel_ , maxHierIndex_ );

    // mark vertices on elements
    vertexMarker_->markNewVertices(*this);

    // map the indices
    markNew();

    // we have a new grid
    wasChanged_ = true;
  }

  template < int dim, int dimworld >  template <FileFormatType ftype>
  inline bool AlbertGrid < dim, dimworld >::writeGrid (const char * filename, albertCtype time )
  {
    switch (ftype)
    {
    case xdr :
    { return writeGridXdr (filename , time ); }
    case USPM : {
      //std::cout << "Write USPM \n";
      return writeGridUSPM ( filename , time , maxlevel_ );
    };
    }
    std::cerr << "Only xdr Format support in writeGrid!\n";
    abort();
    return false;
  }

  template < int dim, int dimworld >  template <FileFormatType ftype>
  inline bool AlbertGrid < dim, dimworld >::readGrid (const char * filename, albertCtype &time )
  {
    if(ftype != xdr)
    {
      std::cerr << "Only xdr Format support in readGrid!\n";
      abort();
    }
    return readGridXdr (filename , time );
  }

  template < int dim, int dimworld >
  inline bool AlbertGrid < dim, dimworld >::writeGridXdr (const char * filename, albertCtype time )
  {
    // use write_mesh_xdr, but works mot correctly
    //return static_cast<bool> (ALBERT write_mesh (mesh_ , filename, time) );
    return static_cast<bool>
           (ALBERT AlbertWrite::new_write_mesh_xdr (mesh_ , filename, time, gIndex_ ) );
  }

  template < int dim, int dimworld >
  inline bool AlbertGrid < dim, dimworld >::readGridXdr (const char * filename, albertCtype & time )
  {
    // use read_mesh_xdr, but works not correctly
#if 0
    mesh_ = (ALBERT read_mesh (filename, &time , ALBERT AlbertHelp::initLeafData ,
                               ALBERT AlbertHelp::initBoundary) );
#else
    mesh_ = (ALBERT AlbertRead::new_read_mesh_xdr
               (filename, &time , ALBERT AlbertHelp::initLeafData ,
               ALBERT AlbertHelp::initBoundary ,
               gIndex_ ) );
#endif

    // calc maxlevel and indexOnLevel and so on
    calcExtras();

    return true;
  }

  template < int dim, int dimworld >
  inline bool AlbertGrid < dim, dimworld >::
  writeGridUSPM ( const char * filename, double time , int level)
  {
    printf("Not implemented for dim=%d , dimworld=%d \n",dim,dimworld);
    abort();
    return false;
  }

  template <>
  inline bool AlbertGrid <2,2>::
  writeGridUSPM ( const char * filename, double time , int level)
  {
    std::cout << "\nStarting USPM Grid write! \n";
    // USPM 2d

    enum {dim = 2}; enum {dimworld = 2};
    typedef LeafIterator LEVit;

    LEVit endit = leafend (level);

    int nvx = size(level,dim);
    int noe = size(level,0);

    double **coord = new double *[nvx];
    for (int i = 0; i < nvx; i++)
      coord[i] = new double[dimworld];

    int **nb = new int *[noe];
    for (int i = 0; i < noe; i++)
      nb[i] = new int[dim + 1];

    int **vertex = new int *[noe];
    for (int i = 0; i < noe; i++)
      vertex[i] = new int[dim + 1];

    // setup the USPM Mesh
    for (LEVit it = leafbegin (level); it != endit; ++it)
    {
      int elNum = it->index();

      typedef AlbertGridIntersectionIterator<dim,dimworld> Neighit;
      Neighit nit = it->ibegin();

      for (int i = 0; i < dim+1; i++)
      {
        int k = it->subIndex<dim>(i);
        //      std::cout << k << " K " << nvx << " Nop\n";
        vertex[elNum][i] = k;

        nb[elNum][i] = nit->index();
        //std::cout << nb[elNum][i] << " Neigh \n";

        Vec<dimworld>& vec = (it->geometry())[i];
        for (int j = 0; j < dimworld; j++)
          coord[k][j] = vec(j);

        ++nit;
      }
      //std::cout << "------------------------------------\n";
    }

    // write the USPM Mesh
    FILE *file = fopen(filename, "w");
    if(!file)
    {
      std::cout << "Couldnt open grid.uspm \n";
      abort();
    }
    fprintf(file, "USPM 2\n");
    fprintf(file, "%d %d \n", dim+1, (dim+1) * noe);
    fprintf(file, "%d %d 0\n", noe , nvx);

    for (int i = 0; i < nvx; i++)
    {
      fprintf(file, "%d ", i);
      for (int j = 0; j < dimworld; j++)
        fprintf(file, "%le ", coord[i][j]);
      fprintf(file, "\n");
    }

    for (int i = 0; i < noe; i++)
    {
      fprintf(file, "%d ", i);
      for (int j = 0; j < dim + 1; j++)
        fprintf(file, "%d ", vertex[i][j]);
      for (int j = 0; j < dim + 1; j++)
        fprintf(file, "%d ", nb[i][j]);
      fprintf(file, "\n");
    }

    fclose(file);
    std::cout << "\nUSPM grid `" << filename << "' written !\n\n";

    for (int i = 0; i < nvx; i++)
      delete [] coord[i];
    delete [] coord;

    for (int i = 0; i < noe; i++)
      delete [] nb[i];
    delete [] nb;

    for (int i = 0; i < noe; i++)
      delete [] vertex[i];
    delete [] vertex;

    return true;
  }

  template <>
  inline bool AlbertGrid<3,3>::
  writeGridUSPM ( const char * filename, double time , int level)
  {
    std::cout << "\nStarting 3d Grid write\n";

    enum {dim = 3}; enum {dimworld = 3};
    typedef AlbertGridLevelIterator<0,dim,dimworld> LEVit;

    double **coord = new double *[mesh_->n_vertices];
    for (int i = 0; i < mesh_->n_vertices; i++)
      coord[i] = new double[dimworld];

    LEVit endit = lend<0>(level);

    int **vertex = new int *[mesh_->n_elements];
    for (int i = 0; i < mesh_->n_elements; i++)
      vertex[i] = new int[dim + 1];

    // / setup the Wesenber 3d Grid
    for (LEVit it = lbegin<0>(level); it != endit; ++it)
    {
      int elNum = (*it).index();

      for (int i = 0; i < dim + 1; i++)
      {
        ALBERT EL_INFO * elInfo = it->getElInfo();
        int k = elInfo->el->dof[i][0];

        vertex[elNum][i] = k;

        Vec<dimworld> vec = (it->geometry())[i];
        for (int j = 0; j < dimworld; j++)
          coord[k][j] = vec(j);
      }
    }

    // / write the Wesenber 3d grid Mesh
    FILE *file = fopen(filename, "w");
    if(!file)
    {
      std::cout << "Couldnt open `" << filename <<"' \n";
      abort();
    }
    // die Zeit
    fprintf(file, "0.0 \n");
    fprintf(file, "%d \n", mesh_->n_vertices);

    for (int i = 0; i < mesh_->n_vertices; i++)
    {
      for (int j = 0; j < dimworld; j++)
        fprintf(file, "%le ", coord[i][j]);
      fprintf(file, "\n");
    }

    fprintf(file, "%d \n", mesh_->n_elements);

    for (int i = 0; i < mesh_->n_elements; i++)
    {
      for (int j = 0; j < dim + 1; j++)
        fprintf(file, "%d ", vertex[i][j]);
      double a = (double) i;
      fprintf(file, "%f \n", a);
    }

    fclose(file);
    for (int i = 0; i < mesh_->n_vertices; i++)
      delete [] coord[i];
    delete [] coord;

    for (int i = 0; i < mesh_->n_elements; i++)
      delete [] vertex[i];
    delete [] vertex;

    //system("gzip -fq grid3d.0");
    std::cout << "3d Grid written! \n";
    return true;
  }

  //! Index Mapping
  template < int dim, int dimworld >
  inline void AlbertGrid < dim, dimworld >::
  makeNewSize(Array<int> &a, int newNumberOfEntries)
  {
    a.resize(newNumberOfEntries);
    for(Array<int>::Iterator it = a.begin(); it != a.end(); ++it)
      (*it) = -1;
  }

  template < int dim, int dimworld > template <int codim>
  inline int AlbertGrid < dim, dimworld >::
  globalIndexConsecutive(int num)
  {
    assert(codim == 0);
    assert(num >= 0);
    return gIndex_[num];
  }

  template < int dim, int dimworld > template <int codim>
  inline int AlbertGrid < dim, dimworld >::
  indexOnLevel(int globalIndex, int level)
  {
    // level = 0 is not interesting for this implementation
    // +1, because if Entity is Boundary then globalIndex == -1
    // an therefore we add 1 and get Entry 0, which schould be -1
    if (globalIndex < 0)
      return globalIndex;
    else
      return levelIndex_[codim][level][globalIndex];
  }

  template < int dim, int dimworld > template <int codim>
  inline int AlbertGrid < dim, dimworld >::
  oldIndexOnLevel(int globalIndex, int level)
  {
    assert(codim == 0);
    if (globalIndex < 0)
      return globalIndex;
    else
    {
      if(globalIndex >= oldLevelIndex_[codim][level].size())
        return -1;

      int ind = oldLevelIndex_[codim][level][globalIndex];
      return ind;
    }
  }

  // create lookup table for indices of the elements
  template < int dim, int dimworld >
  inline void AlbertGrid < dim, dimworld >::markNew()
  {
    // only for gcc, means notin'
    //typedef AlbertGrid < dim ,dimworld > GridType;
    for(int i=0; i<dim+1; i++)
    {
      for(int l=0; l<=maxlevel_; l++)
        levelIndex_[i][l].swap ( oldLevelIndex_[i][l] );
    }

    gIndex_.resize ( maxHierIndex_ );

    int nElements = maxHierIndex_;
    int nVertices = mesh_->n_vertices;

    for(int l=0; l<=maxlevel_; l++)
    {
      if(nElements > levelIndex_[0][l].size())
        makeNewSize(levelIndex_[0][l], nElements);
    }

    // make new size and set all levels to -1 ==> new calc
    if((maxlevel_+1)*(numCodim) > size_.size())
      makeNewSize(size_, 2*((maxlevel_+1)*numCodim));

    // the easiest way, in Albert all elements have unique global element
    // numbers, therefore we make one big array from which we get with the
    // global unique number the local level number
    for(int level=0; level <= maxlevel_; level++)
    {
      typedef AlbertGridLevelIterator<0,dim,dimworld> LevelIterator;
      int num = 0;
      LevelIterator endit = lend<0>(level);
      //std::cout << levelIndex_[0][level].size() << " Size \n";
      for(LevelIterator it = lbegin<0> (level); it != endit; ++it)
      {
        int no = it->el_index();
        gIndex_.insert( no );

        levelIndex_[0][level][no] = num;
        num++;
      }
      // remember the number of entity on level and codim = 0
      size_[level*numCodim /* +0 */] = num;
    };

    gIndex_.finish();

    for(int l=0; l<=maxlevel_; l++)
    {
      if(nVertices > levelIndex_[dim][l].size())
      {
        makeNewSize(levelIndex_[dim][l], nVertices);
      }
    }

    for(int level=0; level <= maxlevel_; level++)
    {
      //std::cout << level << " " << maxlevel_ << "\n";
      typedef AlbertGridLevelIterator<dim,dim,dimworld> LevelIterator;
      int num = 0;
      LevelIterator endit = lend<dim> (level);
      //std::cout << levelIndex_[dim][level].size() << " Size \n";;
      for(LevelIterator it = lbegin<dim> (level); it != endit; ++it)
      {
        int no = it->el_index();
        //std::cout << no << " " << level << " Glob Num\n";
        levelIndex_[dim][level][no] = num;
        num++;
      }
      //   std::cout << "Done LevelIt \n";
      // remember the number of entity on level and codim = 0
      size_[level*numCodim + dim] = num;
    };

  }

  // if defined some debugging test were made that reduce the performance
  // so they were switch off normaly

  //#define DEBUG_FILLELINFO
  //*********************************************************************
  //  fillElInfo 2D
  //*********************************************************************
  template<int dim, int dimworld>
  inline void AlbertGrid<dim,dimworld >::
  fillElInfo(int ichild, int actLevel , const ALBERT EL_INFO *elinfo_old, ALBERT EL_INFO *elinfo, bool hierarchical) const
  {

#if 0
    ALBERT fill_elinfo(ichild,elinfo_old,elinfo);
#else

    ALBERT EL      *nb=NULL;
    ALBERT EL      *el = elinfo_old->el;
    ALBERT FLAGS fill_flag = elinfo_old->fill_flag;
    ALBERT FLAGS fill_opp_coords;

    //ALBERT_TEST_EXIT(el->child[0])("no children?\n");
    // in this implementation we can go down without children
    if(el->child[0])
    {
#ifdef DEBUG_FILLELINFO
      printf("Called fillElInfo with El %d \n",el->index);
#endif

      ALBERT_TEST_EXIT((elinfo->el = el->child[ichild])) ("missing child %d?\n", ichild);

      elinfo->macro_el  = elinfo_old->macro_el;
      elinfo->fill_flag = fill_flag;
      elinfo->mesh      = elinfo_old->mesh;
      elinfo->parent    = el;
      elinfo->level     = elinfo_old->level + 1;

      if (fill_flag & FILL_COORDS)
      {
        if (el->new_coord)
        {
          for (int j = 0; j < dimworld; j++)
            elinfo->coord[2][j] = el->new_coord[j];
        }
        else
        {
          for (int j = 0; j < dimworld; j++)
            elinfo->coord[2][j] =
              0.5 * (elinfo_old->coord[0][j] + elinfo_old->coord[1][j]);
        }

        if (ichild==0)
        {
          for (int j = 0; j < dimworld; j++)
          {
            elinfo->coord[0][j] = elinfo_old->coord[2][j];
            elinfo->coord[1][j] = elinfo_old->coord[0][j];
          }
        }
        else
        {
          for (int j = 0; j < dimworld; j++)
          {
            elinfo->coord[0][j] = elinfo_old->coord[1][j];
            elinfo->coord[1][j] = elinfo_old->coord[2][j];
          }
        }
      }

      /* ! NEIGH_IN_EL */
      // make the neighbour relations

      if (fill_flag & (FILL_NEIGH | FILL_OPP_COORDS))
      {
        fill_opp_coords = (fill_flag & FILL_OPP_COORDS);

        // first child
        if (ichild==0)
        {
          elinfo->opp_vertex[2] = elinfo_old->opp_vertex[1];
          if ((elinfo->neigh[2] = elinfo_old->neigh[1]))
          {
            if (fill_opp_coords)
            {
              for (int j=0; j<dimworld; j++)
                elinfo->opp_coord[2][j] = elinfo_old->opp_coord[1][j];
            }

            if(hierarchical)
            {
              // this is new
              ALBERT EL * nextNb = elinfo->neigh[2]->child[0];
              if(nextNb)
              {
#ifdef DEBUG_FILLELINFO
                ALBERT_TEST_EXIT(elinfo_old->opp_vertex[2] != 2) ("invalid neighbour\n");
#endif
                int oppV = elinfo->opp_vertex[2];
                // if oppV == 0 neighbour must be the child 1
                if( oppV == 0)
                  nextNb = elinfo->neigh[2]->child[1];

                if(neighOnLevel_[nextNb->index] <= actLevel )
                  elinfo->neigh[2] = nextNb;
                // if we go down the opposite vertex now must be 2
                elinfo->opp_vertex[2] = 2;

                if (fill_opp_coords)
                {
                  if(oppV == 0)
                  {
                    for (int k=0; k < dimworld ; k++)
                      elinfo->opp_coord[2][k] = 0.5 *
                                                (elinfo_old->opp_coord[oppV][k] +
                                                 elinfo_old->coord[1][k]);
                  }
                  else
                  {
                    for (int k=0; k < dimworld ; k++)
                      elinfo->opp_coord[2][k] = 0.5 *
                                                (elinfo_old->opp_coord[oppV][k] +
                                                 elinfo_old->coord[0][k]);
                  }
                }
              }
            } // end hierarchical

          } // end new code

          if (el->child[0])
          {
            bool goDownNextChi = false;
            ALBERT EL *chi1 = el->child[1];

            if(chi1->child[0])
            {
              if(neighOnLevel_[chi1->child[1]->index] <= actLevel )
                goDownNextChi = true;
            }

            if(goDownNextChi)
            {
              // set neighbour
              ALBERT_TEST_EXIT((elinfo->neigh[1] = chi1->child[1]))
                ("el->child[1]->child[0]!=nil, but el->child[1]->child[1]=nil\n");

              elinfo->opp_vertex[1] = 2;
              if (fill_opp_coords)
              {
                if (chi1->new_coord)
                {
                  for (int j=0; j<dimworld; j++)
                    elinfo->opp_coord[1][j] = chi1->new_coord[j];
                }
                else
                {
                  for (int j=0; j<dimworld; j++)
                    elinfo->opp_coord[1][j] =
                      0.5 * (elinfo_old->coord[1][j] + elinfo_old->coord[2][j]);
                }
              }
            }
            else
            {
              // set neighbour
              ALBERT_TEST_EXIT((elinfo->neigh[1] = chi1))
                ("el->child[0] != nil, but el->child[1] = nil\n");

              elinfo->opp_vertex[1] = 0;
              if (fill_opp_coords)
              {
                for (int j=0; j<dimworld; j++)
                  elinfo->opp_coord[1][j] = elinfo_old->coord[1][j];
              }
            }
          }
#ifdef DEBUG_FILLELINFO
          else
          {
            ALBERT_TEST_EXIT((el->child[0])) ("No Child\n");
          }
#endif

          if ((nb = elinfo_old->neigh[2]))
          {
            // the neighbour across the refinement edge
#ifdef DEBUG_FILLELINFO
            printf("El  %d , Neigh %d \n",el->index,nb->index);
            printf("El  %d , Neigh %d \n",el->child[0]->index,nb->index);
            //printf("OppVx %d \n",elinfo_old->opp_vertex[2]);
            //printf("El  %d , Neigh %d \n",el->index,nb->index);
            ALBERT_TEST_EXIT(elinfo_old->opp_vertex[2] == 2) ("invalid neighbour\n");
            //ALBERT_TEST_EXIT((nb->child[0]))("missing child[0] of nb2 ?\n");
#endif
            if(nb->child[0])
            {
              ALBERT_TEST_EXIT((nb = nb->child[1])) ("missing child[1]?\n");
            }
            else
            {
              printf("El  %d , Neigh %d \n",el->child[0]->index,nb->index);
              //nb = nb->child[0];
            }

            if (nb->child[0])
            {
              bool goDownNextChi = false;
              if(neighOnLevel_[nb->child[0]->index] <= actLevel )
                goDownNextChi = true;

              if(goDownNextChi)
              {

                elinfo->opp_vertex[0] = 2;
                if (fill_opp_coords)
                {
                  if (nb->new_coord)
                  {
                    for (int j=0; j<dimworld; j++)
                      elinfo->opp_coord[0][j] = nb->new_coord[j];
                  }
                  else
                  {
                    for (int j=0; j<dimworld; j++)
                    {
                      elinfo->opp_coord[0][j] = 0.5*
                                                (elinfo_old->opp_coord[2][j] + elinfo_old->coord[0][j]);
                    }
                  }
                }
                nb = nb->child[0];
              }
              else
              {
                elinfo->opp_vertex[0] = 1;
                if (fill_opp_coords)
                {
                  for (int j=0; j<dimworld; j++)
                    elinfo->opp_coord[0][j] = elinfo_old->opp_coord[2][j];
                }
              }
            }
          }
          elinfo->neigh[0] = nb;
        }
        else /* ichild==1 , second child */
        {
          elinfo->opp_vertex[2] = elinfo_old->opp_vertex[0];
          if ((elinfo->neigh[2] = elinfo_old->neigh[0]))
          {
            if (fill_opp_coords)
            {
              for (int j=0; j<dimworld; j++)
                elinfo->opp_coord[2][j] = elinfo_old->opp_coord[0][j];
            }

            if(hierarchical)
            {
              ALBERT EL * nextNb = elinfo->neigh[2]->child[0];
              if(nextNb)
              {
#ifdef DEBUG_FILLELINFO
                ALBERT_TEST_EXIT(elinfo_old->opp_vertex[2] != 2) ("invalid neighbour\n");
#endif
                int oppV = elinfo->opp_vertex[2];
                // if oppV == 0 neighbour must be the child 1
                if( oppV == 0 )
                  nextNb = elinfo->neigh[2]->child[1];

                if(neighOnLevel_[nextNb->index] <= actLevel )
                  elinfo->neigh[2] = nextNb;
                // if we go down the opposite vertex now must be 2
                elinfo->opp_vertex[2] = 2;

                if (fill_opp_coords)
                {
                  // add new coord here
                  if(oppV == 0)
                  {
                    for (int k=0; k < dimworld ; k++)
                      elinfo->opp_coord[2][k] = 0.5 *
                                                (elinfo_old->opp_coord[oppV][k] +
                                                 elinfo_old->coord[1][k]);
                  }
                  else
                  {
                    for (int k=0; k < dimworld ; k++)
                      elinfo->opp_coord[2][k] = 0.5 *
                                                (elinfo_old->opp_coord[oppV][k] +
                                                 elinfo_old->coord[0][k]);
                  }
                }
              }
            } // end hierarchical
          }

          { // begin chi0
            ALBERT EL *chi0=el->child[0];
            bool goDownChild = false;

            if (chi0->child[0])
            {
              if(neighOnLevel_[chi0->child[0]->index] <= actLevel )
                goDownChild = true;
            }

            if(goDownChild)
            {
              elinfo->neigh[0] = chi0->child[0];
              elinfo->opp_vertex[0] = 2;
              if (fill_opp_coords)
              {
                if (chi0->new_coord)
                {
                  for (int j=0; j<dimworld; j++)
                    elinfo->opp_coord[0][j] = chi0->new_coord[j];
                }
                else
                {
                  for (int j=0; j<dimworld; j++)
                    elinfo->opp_coord[0][j] = 0.5 *
                                              (elinfo_old->coord[0][j] + elinfo_old->coord[2][j]);
                }
              }
            }
            else
            {
              elinfo->neigh[0] = chi0;
              elinfo->opp_vertex[0] = 1;
              if (fill_opp_coords)
              {
                for (int j=0; j<dimworld; j++)
                  elinfo->opp_coord[0][j] = elinfo_old->coord[0][j];
              }
            }
          } // end chi0

          if ((nb = elinfo_old->neigh[2]))
          {
            // the neighbour across the refinement edge
#ifdef DEBUG_FILLELINFO
            //printf("OppVx %d \n",elinfo_old->opp_vertex[2]);
            ALBERT_TEST_EXIT(elinfo_old->opp_vertex[2] == 2) ("invalid neighbour\n");
#endif
            if(nb->child[0])
              ALBERT_TEST_EXIT((nb = nb->child[0])) ("missing child?\n");

            bool goDownChild = false;
            if (nb->child[0])
            {
              if(neighOnLevel_[nb->child[1]->index] <= actLevel )
                goDownChild = true;
            }

            if(goDownChild)
            {
              // we go down, calc new coords
              elinfo->opp_vertex[1] = 2;
              if (fill_opp_coords)
              {
                if (nb->new_coord)
                  for (int j=0; j<dimworld; j++)
                    elinfo->opp_coord[1][j] = nb->new_coord[j];
                else
                  for (int j=0; j<dimworld; j++)
                    elinfo->opp_coord[1][j] = 0.5 *
                                              (elinfo_old->opp_coord[2][j] + elinfo_old->coord[1][j]);
              }
              nb = nb->child[1];
            }
            else
            {
              // we are nto going down, so copy the coords
              elinfo->opp_vertex[1] = 0;
              if (fill_opp_coords)
              {
                for (int j=0; j<dimworld; j++)
                  elinfo->opp_coord[1][j] = elinfo_old->opp_coord[2][j];
              }
            }
          }
          elinfo->neigh[1] = nb;
        }
      }

      if (fill_flag & FILL_BOUND)
      {
        if (elinfo_old->boundary[2])
          elinfo->bound[2] = elinfo_old->boundary[2]->bound;
        else
          elinfo->bound[2] = INTERIOR;

        if (ichild==0)
        {
          elinfo->bound[0] = elinfo_old->bound[2];
          elinfo->bound[1] = elinfo_old->bound[0];
          elinfo->boundary[0] = elinfo_old->boundary[2];
          elinfo->boundary[1] = nil;
          elinfo->boundary[2] = elinfo_old->boundary[1];
        }
        else
        {
          elinfo->bound[0] = elinfo_old->bound[1];
          elinfo->bound[1] = elinfo_old->bound[2];
          elinfo->boundary[0] = nil;
          elinfo->boundary[1] = elinfo_old->boundary[2];
          elinfo->boundary[2] = elinfo_old->boundary[0];
        }
      }

    }
    // no child exists, but go down maxlevel
    // means neighbour may be changed but element itself not
    else
    {
      memcpy(elinfo,elinfo_old,sizeof(ALBERT EL_INFO));
      elinfo->level = (unsigned  char) (elinfo_old->level + 1);

      if (fill_flag & (FILL_NEIGH | FILL_OPP_COORDS))
      {
        fill_opp_coords = (fill_flag & FILL_OPP_COORDS);

        // we use triangles here
        enum { numberOfNeighbors = 3 };
        enum { numOfVx = 3 };
        for(int i=0; i<numberOfNeighbors; i++)
        {
          if(elinfo_old->neigh[i])
          {
            // if children of neighbour
            if(elinfo_old->neigh[i]->child[0])
            {
#ifdef DEBUG_FILLELINFO
              ALBERT_TEST_EXIT(elinfo_old->opp_vertex[2] != 2) ("invalid neighbour\n");
#endif
              int oppV = elinfo_old->opp_vertex[i];
              if(oppV == 0)
              {
                elinfo->neigh[i] = elinfo_old->neigh[i]->child[1];
                elinfo->opp_vertex[i] = 2;

                if( fill_opp_coords )
                  for (int k=0; k < dimworld ; k++)
                    elinfo->opp_coord[i][k] = 0.5 *
                                              (elinfo_old->opp_coord[oppV][k] +
                                               elinfo_old->coord[(i-1)%numOfVx][k]);
              }
              else
              {
                elinfo->neigh[i] = elinfo_old->neigh[i]->child[0];
                elinfo->opp_vertex[i] = 2;
                if( fill_opp_coords )
                  for (int k=0; k < dimworld ; k++)
                    elinfo->opp_coord[i][k] = 0.5 *
                                              (elinfo_old->opp_coord[oppV][k] +
                                               elinfo_old->coord[(i+1)%numOfVx][k]);
              }
            }
          }
        }
      } // end if (fill_flag & (FILL_NEIGH | FILL_OPP_COORDS))
    } //end else

#endif

  } // end Grid::fillElInfo 2D


  //***********************************************************************
  // fillElInfo 3D
  //***********************************************************************
#if DIM == 3
  template <>
  inline void AlbertGrid<3,3>::
  fillElInfo(int ichild, int actLevel , const ALBERT EL_INFO *elinfo_old,
             ALBERT EL_INFO *elinfo, bool hierarchical) const
  {
    enum { dim = 3 };
    enum { dimworld = 3 };

#if 0
    ALBERT fill_elinfo(ichild,elinfo_old,elinfo);
#else
    static S_CHAR child_orientation[3][2] = {{1,1}, {1,-1}, {1,-1}};

    int k;
    int el_type=0;                                   /* el_type in {0,1,2} */
    int ochild=0;                       /* index of other child = 1-ichild */
    int     *cv=nil;                   /* cv = child_vertex[el_type][ichild] */
    int     (*cvg)[4]=nil;                    /* cvg = child_vertex[el_type] */
    int     *ce;                     /* ce = child_edge[el_type][ichild] */
    int iedge;
    EL      *nb, *nbk, **neigh_old;
    EL      *el_old = elinfo_old->el;
    FLAGS fill_flag = elinfo_old->fill_flag;
    DOF    *dof;
#if !NEIGH_IN_EL
    int ov;
    EL      **neigh;
    FLAGS fill_opp_coords;
    U_CHAR  *opp_vertex;
#endif

    TEST_EXIT(el_old->child[0]) ("missing child?\n"); /* Kuni 22.08.96 */

    elinfo->el        = el_old->child[ichild];
    elinfo->macro_el  = elinfo_old->macro_el;
    elinfo->fill_flag = fill_flag;
    elinfo->mesh      = elinfo_old->mesh;
    elinfo->parent    = el_old;
    elinfo->level     = elinfo_old->level + 1;
#if !NEIGH_IN_EL
    elinfo->el_type   = (elinfo_old->el_type + 1) % 3;
#endif

    REAL_D * opp_coord = elinfo->opp_coord;
    REAL_D * coord = elinfo->coord;

    const REAL_D * old_coord = elinfo_old->coord;
    const REAL_D * oldopp_coord = elinfo_old->opp_coord;


    TEST_EXIT(elinfo->el) ("missing child %d?\n", ichild);

    if (fill_flag) {
      el_type = EL_TYPE(elinfo_old->el, elinfo_old);
      cvg = child_vertex[el_type];
      cv = cvg[ichild];
      ochild = 1-ichild;
    }

    if (fill_flag & FILL_COORDS)
    {
      for (int i=0; i<3; i++) {
        for (int j = 0; j < dimworld; j++) {
          coord[i][j] = old_coord[cv[i]][j];
        }
      }
      if (el_old->new_coord)
        for (int j = 0; j < dimworld; j++)
          coord[3][j] = el_old->new_coord[j];
      else
        for (int j = 0; j < dimworld; j++)
          coord[3][j] = 0.5*
                        (old_coord[0][j] + old_coord[1][j]);
    }


#if NEIGH_IN_EL
    if (fill_flag & FILL_OPP_COORDS)
    {
      neigh_old = el_old->neigh;

      /*----- nb[0] is other child -------------------------------------------*/

      /*    if (nb = el_old->child[ochild]) {        old version    */
      if (el_old->child[0]  &&  (nb = el_old->child[ochild])) /* Kuni 22.08.96*/
      {
        if (nb->child[0]) {   /* go down one level for direct neighbour */
          k = cvg[ochild][1];
          if (nb->new_coord)
            for (int j = 0; j < dimworld; j++)
              opp_coord[0][j] = nb->new_coord[j];
          else
            for (int j = 0; j < dimworld; j++)
              opp_coord[0][j] = 0.5*
                                (old_coord[ochild][j] + old_coord[k][j]);
        }
        else {
          for (int j = 0; j < dimworld; j++) {
            opp_coord[0][j] = old_coord[ochild][j];
          }
        }
      }
      else {
        ERROR_EXIT("no other child");
      }


      /*----- nb[1],nb[2] are childs of old neighbours nb_old[cv[i]] ----------*/

      for (int i=1; i<3; i++)
      {
        if (nb = neigh_old[cv[i]])
        {
          TEST_EXIT(nb->child[0]) ("nonconforming triangulation\n");

          for (k=0; k<2; k++) { /* look at both children of old neighbour */

            nbk = nb->child[k];
            if (nbk->dof[0] == el_old->dof[ichild]) {
              dof = nb->dof[el_old->opp_vertex[cv[i]]]; /* opp. vertex */
              if (dof == nbk->dof[1]) {
                if (nbk->child[0]) {
                  if (nbk->new_coord)
                    for (int j = 0; j < dimworld; j++)
                      opp_coord[i][j] = nbk->new_coord[j];
                  else
                    for (int j = 0; j < dimworld; j++)
                      opp_coord[i][j] = 0.5*
                                        (oldopp_coord[cv[i]][j] + old_coord[ichild][j]);
                  break;
                }
              }
              else {
                TEST_EXIT(dof == nbk->dof[2]) ("opp_vertex not found\n");
              }

              for (int j = 0; j < dimworld; j++) {
                opp_coord[i][j] = oldopp_coord[cv[i]][j];
              }
              break;
            }

          } /* end for k */
          TEST_EXIT(k<2) ("child not found with vertex\n");

        }
      } /* end for i */


      /*----- nb[3] is old neighbour neigh_old[ochild] ------------------------*/

      if (neigh_old[ochild]) {
        for (int j = 0; j < dimworld; j++) {
          opp_coord[3][j] = oldopp_coord[ochild][j];
        }
      }

    }

#else  /* ! NEIGH_IN_EL */

    if (fill_flag & (FILL_NEIGH | FILL_OPP_COORDS))
    {
      neigh      = elinfo->neigh;
      neigh_old  = (EL **) elinfo_old->neigh;
      opp_vertex = (U_CHAR *) &(elinfo->opp_vertex);
      fill_opp_coords = (fill_flag & FILL_OPP_COORDS);

      /*----- nb[0] is other child --------------------------------------------*/

      /*    if (nb = el_old->child[ochild])   old version  */
      if (el_old->child[0]  &&  (nb = el_old->child[ochild])) /*Kuni 22.08.96*/
      {
        if (nb->child[0])
        {   /* go down one level for direct neighbour */
          if (fill_opp_coords)
          {
            if (nb->new_coord)
            {
              for (int j = 0; j < dimworld; j++)
                opp_coord[0][j] = nb->new_coord[j];
            }
            else
            {
              k = cvg[ochild][1];
              for (int j = 0; j < dimworld; j++)
                opp_coord[0][j] = 0.5*
                                  (old_coord[ochild][j] + old_coord[k][j]);
            }
          }
          neigh[0]      = nb->child[1];
          opp_vertex[0] = 3;
        }
        else {
          if (fill_opp_coords) {
            for (int j = 0; j < dimworld; j++) {
              opp_coord[0][j] = old_coord[ochild][j];
            }
          }
          neigh[0]      = nb;
          opp_vertex[0] = 0;
        }
      }
      else {
        ERROR_EXIT("no other child");
        neigh[0] = nil;
      }


      /*----- nb[1],nb[2] are childs of old neighbours nb_old[cv[i]] ----------*/

      for (int i=1; i<3; i++)
      {
        if ((nb = neigh_old[cv[i]]))
        {
          TEST_EXIT(nb->child[0]) ("nonconforming triangulation\n");

          for (k=0; k<2; k++) /* look at both childs of old neighbour */
          {
            nbk = nb->child[k];
            if (nbk->dof[0] == el_old->dof[ichild]) {
              dof = nb->dof[elinfo_old->opp_vertex[cv[i]]]; /* opp. vertex */
              if (dof == nbk->dof[1])
              {
                ov = 1;
                if (nbk->child[0])
                {
                  if (fill_opp_coords)
                  {
                    if (nbk->new_coord)
                      for (int j = 0; j < dimworld; j++)
                        opp_coord[i][j] = nbk->new_coord[j];
                    else
                      for (int j = 0; j < dimworld; j++)
                        opp_coord[i][j] = 0.5*
                                          (oldopp_coord[cv[i]][j]
                                           + old_coord[ichild][j]);
                  }
                  neigh[i]      = nbk->child[0];
                  opp_vertex[i] = 3;
                  break;
                }
              }
              else
              {
                TEST_EXIT(dof == nbk->dof[2]) ("opp_vertex not found\n");
                ov = 2;
              }

              if (fill_opp_coords)
              {
                for (int j = 0; j < dimworld; j++)
                {
                  opp_coord[i][j] = oldopp_coord[cv[i]][j];
                }
              }
              neigh[i]      = nbk;
              opp_vertex[i] = ov;
              break;
            }

          } /* end for k */
          TEST_EXIT(k<2) ("child not found with vertex\n");
        }
        else
        {
          neigh[i] = nil;
        }
      } /* end for i */


      /*----- nb[3] is old neighbour neigh_old[ochild] ------------------------*/

      if ((neigh[3] = neigh_old[ochild]))
      {
        opp_vertex[3] = elinfo_old->opp_vertex[ochild];
        if (fill_opp_coords) {
          for (int j = 0; j < dimworld; j++) {
            opp_coord[3][j] = oldopp_coord[ochild][j];
          }
        }
      }
    }
#endif

    if (fill_flag & FILL_BOUND)
    {
      for (int i = 0; i < 3; i++)
      {
        elinfo->bound[i] = elinfo_old->bound[cv[i]];
      }
      elinfo->bound[3] = GET_BOUND(elinfo_old->boundary[N_FACES+0]);

      elinfo->boundary[0] = nil;
      elinfo->boundary[1] = elinfo_old->boundary[cv[1]];
      elinfo->boundary[2] = elinfo_old->boundary[cv[2]];
      elinfo->boundary[3] = elinfo_old->boundary[ochild];

      ce = child_edge[el_type][ichild];
      for (iedge=0; iedge<4; iedge++) {
        elinfo->boundary[N_FACES+iedge] =
          elinfo_old->boundary[N_FACES+ce[iedge]];
      }
      for (iedge=4; iedge<6; iedge++) {
        int i = 5 - cv[iedge-3];              /* old vertex opposite new edge */
        elinfo->boundary[N_FACES+iedge] = elinfo_old->boundary[i];
      }
    }


    if (elinfo->fill_flag & FILL_ORIENTATION) {
      elinfo->orientation =
        elinfo_old->orientation * child_orientation[el_type][ichild];
    }
#endif
  }
#endif

} // namespace Albert
