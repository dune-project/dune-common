// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
//************************************************************************
//
//  Implementation von AlbertGrid
//
//  namespace Dune
//
//************************************************************************

namespace Dune
{

  static ALBERT EL_INFO statElInfo[DIM+1];

  // singleton holding reference elements
  template<int dim> struct AlbertGridReferenceElement
  {
    enum { dimension = dim };
    enum { type = unknown };

    static AlbertGridElement<dim,dim> refelem;
    static ALBERT EL_INFO elInfo_;
  };

  struct AlbertGridReferenceElement<2>
  {
    enum { dimension = 2 };
    enum { type = triangle };

    enum { make = true };

    static AlbertGridElement<dimension,dimension> refelem;
    static ALBERT EL_INFO elInfo_;
  };

  // singleton holding reference elements
  template<>
  struct AlbertGridReferenceElement<3>
  {
    enum { dimension = 3 };
    enum { type = tetrahedron };

    static AlbertGridElement<dimension,dimension> refelem;
    static ALBERT EL_INFO elInfo_;
  };

  // initialize static variable with bool constructor
  // (which makes reference element)
  template<int dim>
  AlbertGridElement<dim,dim> AlbertGridReferenceElement<dim>::refelem(true);

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

  // specialication for codim == 1, faces
  template <>
  inline int AlbertGridElement<1,2>::mapVertices (int i) const
  {
    // N_VERTICES (from ALBERT) = dim +1
    return ((face_ + 1 + i) % (dimension+1));
  }

  template <>
  inline int AlbertGridElement<2,3>::mapVertices (int i) const
  {
    // N_VERTICES (from ALBERT) = dim +1
    return ((face_ + 1 + i) % (dimension+1));
  }

  // specialization for codim == 2, edges
  template <>
  inline int AlbertGridElement<1,3>::mapVertices (int i) const
  {
    // N_VERTICES (from ALBERT) = dim +1 = 4
    return ((face_+1)+ (edge_+1) +i)% (dimension+1);
  }

  template <>
  inline int AlbertGridElement<0,2>::mapVertices (int i) const
  {
    // N_VERTICES (from ALBERT) = dim +1 = 3
    return ((face_+1)+ (vertex_+1) +i)% (dimension+1);
  }

  template <>
  inline int AlbertGridElement<0,3>::mapVertices (int i) const
  {
    // N_VERTICES (from ALBERT) = dim +1 = 4
    return ((face_+1)+ (edge_+1) +(vertex_+1) +i)% (dimension+1);
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


  inline void AlbertGridElement<3,3>::
  makeRefElemCoords()
  {
    //! make ReferenzElement as default
    elInfo_ = makeEmptyElInfo();

    int i = 0;
    // point 0
    elInfo_->coord[i][0] = 0.0;
    elInfo_->coord[i][1] = 0.0;
#if DIMOFWORLD > 2
    elInfo_->coord[i][2] = 0.0;
#endif

    i = 1;
    // point 1
    elInfo_->coord[i][0] = 1.0;
    elInfo_->coord[i][1] = 1.0;
#if DIMOFWORLD > 2
    elInfo_->coord[i][2] = 1.0;
#endif

    i = 2;
    // point 2
    elInfo_->coord[i][0] = 1.0;
    elInfo_->coord[i][1] = 1.0;
#if DIMOFWORLD > 2
    elInfo_->coord[i][2] = 0.0;
#endif

#if DIM > 2
    i = 3;
    // point 3
    elInfo_->coord[i][0] = 1.0;
    elInfo_->coord[i][1] = 0.0;
    elInfo_->coord[i][2] = 0.0;
#endif
  }

  inline void AlbertGridElement<2,2>::
  makeRefElemCoords()
  {
    //! make ReferenzElement as default
    elInfo_ = makeEmptyElInfo();

    int i = 0;
    // point 0
    elInfo_->coord[i][0] = 1.0;
    elInfo_->coord[i][1] = 0.0;

    i = 1;
    // point 1
    elInfo_->coord[i][0] = 0.0;
    elInfo_->coord[i][1] = 1.0;

    i = 2;
    // point 2
    elInfo_->coord[i][0] = 0.0;
    elInfo_->coord[i][1] = 0.0;

  }

  inline void AlbertGridElement<1,1>::
  makeRefElemCoords()
  {
    //! make  Referenz Element as default
    elInfo_ = makeEmptyElInfo();

    int i = 0;
    // point 0
    elInfo_->coord[i][0] = 0.0;

    i = 1;
    // point 1
    elInfo_->coord[i][0] = 1.0;

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
  }

  // built Geometry
  template< int dim, int dimworld>
  inline bool AlbertGridElement<dim,dimworld>::
  builtGeom(ALBERT EL_INFO *elInfo, unsigned char face,
            unsigned char edge, unsigned char vertex)
  {
    elInfo_ = elInfo;
    face_ = face;
    edge_ = edge;
    vertex_ = vertex;
    volume_ = 0.0;
    builtinverse_ = false;

    if(elInfo_)
    {
      for(int i=0; i<dim+1; i++)
      {
        (coord_(i)) = static_cast< albertCtype  * >
                      ( elInfo_->coord[mapVertices(i)] );
        //  ( elInfo_->coord[mapVertices<dimworld-dim>(i)] );
      }
      // geometry built
      return true;
    }
    // geometry not built
    return false;
  }


  // specialization yields speed up, because vertex_ .. is not copied
  inline bool AlbertGridElement<2,2>::
  builtGeom(ALBERT EL_INFO *elInfo, unsigned char face,
            unsigned char edge, unsigned char vertex)
  {
    enum { dim = 2 };
    enum { dimworld = 2 };

    elInfo_ = elInfo;
    volume_ = 0.0;
    builtinverse_ = false;
    if(elInfo_)
    {
      for(int i=0; i<dim+1; i++)
        (coord_(i)) = static_cast< albertCtype  * >
                      ( elInfo_->coord[mapVertices(i)] );
      //( elInfo_->coord[mapVertices<dimworld-dim>(i)] );
      // geometry built
      return true;
    }
    // geometry not built
    return false;
  }

  inline bool AlbertGridElement<3,3>::
  builtGeom(ALBERT EL_INFO *elInfo, unsigned char face,
            unsigned char edge, unsigned char vertex)
  {
    enum { dim = 3 };
    enum { dimworld = 3 };

    elInfo_ = elInfo;
    volume_ = 0.0;
    builtinverse_ = false;
    if(elInfo_)
    {
      for(int i=0; i<dim+1; i++)
        (coord_(i)) = static_cast< albertCtype  * >
                      ( elInfo_->coord[mapVertices(i)] );
      //( elInfo_->coord[mapVertices<dimworld-dim>(i)] );
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
    for(int i=0; i<corners(); i++)
      ((*this)[i]).print(ss,dimworld);
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

  template< int dim, int dimworld>
  inline AlbertGridElement<dim,dim>& AlbertGridElement<dim,dimworld>::
  refelem()
  {
    return AlbertGridReferenceElement<dim>::refelem;
  }

  template< int dim, int dimworld>
  inline Vec<dimworld,albertCtype>& AlbertGridElement<dim,dimworld>::
  global(const Vec<dim>& local)
  {
    // only the dim first components are needed, because we don't use
    // barycentric coordinates
    Vec<dim+1,albertCtype> tmp;
    for(int i=0; i<dim; i++)
      tmp(i) = local.read(i);

    tmp(dim) = 1.0;
    for(int i=0; i<dim; i++)
      tmp(dim) -= local.read(i);

    // globale Koordinaten ausrechnen
    globalCoord_ = globalBary(tmp);

    return globalCoord_;
  }

  template< int dim, int dimworld>
  inline Vec<dimworld> AlbertGridElement<dim,dimworld>::
  globalBary(const Vec<dim+1>& local)
  {
    ALBERT REAL *v = NULL;
    ALBERT REAL c;
    Vec<dimworld> ret(0.0);

    v = static_cast<ALBERT REAL *> (elInfo_->coord[0]);
    c = local.read(0);
    for (int j = 0; j < dimworld; j++)
      ret(j) = c * v[j];

    for (int i = 1; i < dim+1; i++)
    {
      v = static_cast<ALBERT REAL *> (elInfo_->coord[i]);
      c = local.read(i);
      for (int j = 0; j < dimworld; j++)
        ret(j) += c * v[j];
    }

    return ret;
  }

  template< int dim, int dimworld>
  inline Vec<dim>& AlbertGridElement<dim,dimworld>::
  local(const Vec<dimworld>& global)
  {
    ALBERT REAL lambda[dim+1];

    Vec<dim+1,albertCtype> tmp = localBary(global);

    // Umrechnen von baryzentrischen localen Koordinaten nach
    // localen Koordinaten,
    for(int i=0; i<dim; i++)
      localCoord_(i) = tmp(i);

    return localCoord_;
  }

  template <int dim, int dimworld>
  inline Vec<dim+1> AlbertGridElement<dim,dimworld>::
  localBary(const Vec<dimworld>& global)
  {
    std::cout << "localBary for dim != dimworld not implemented yet!";
    Vec<dim+1> tmp (0.0);
    return tmp;
  }

  inline Vec<3> AlbertGridElement<2,2>::
  localBary(const Vec<2>& global)
  {
    enum { dim = 2};
    enum { dimworld = 2};

    ALBERT REAL edge[dim][dimworld], x[dimworld];
    ALBERT REAL x0, det, det0, det1, lmin;
    int j, k;
    Vec<dim+1,albertCtype> lambda;
    ALBERT REAL *v = NULL;

    /*
     * we got to solve the problem :
     */
    /*
     * ( q1x q2x ) (lambda1) = (qx)
     */
    /*
     * ( q1y q2y ) (lambda2) = (qy)
     */
    /*
     * with qi=pi-p3, q=global-p3
     */

    v = static_cast<ALBERT REAL *> (elInfo_->coord[0]);
    for (int j = 0; j < dimworld; j++)
    {
      x0 = elInfo_->coord[dim][j];
      x[j] = global.read(j) - x0;
      for (int i = 0; i < dim; i++)
        edge[i][j] = elInfo_->coord[i][j] - x0;
    }

    det = edge[0][0] * edge[1][1] - edge[0][1] * edge[1][0];

    det0 = x[0] * edge[1][1] - x[1] * edge[1][0];
    det1 = edge[0][0] * x[1] - edge[0][1] * x[0];

    if(ABS(det) < 1.E-20)
    {
      printf("det = %e; abort\n", det);
      abort();
    }

    // lambda is initialized here
    lambda(0) = det0 / det;
    lambda(1) = det1 / det;
    lambda(2) = 1.0 - lambda(0) - lambda(1);

    return lambda;
  }

  //template< int dim, int dimworld>
  inline Vec<4> AlbertGridElement<3,3>::
  localBary(const Vec<3>& global)
  {
    enum { dim = 3};
    enum { dimworld = 3};

    ALBERT REAL edge[dim][dimworld], x[dimworld];
    ALBERT REAL x0, det, det0, det1, det2, lmin;
    Vec<dim+1,albertCtype> lambda;
    int j, k;

    //! we got to solve the problem :
    //! ( q1x q2x q3x) (lambda1) (qx)
    //! ( q1y q2y q3y) (lambda2) = (qy)
    //! ( q1z q2z q3z) (lambda3) (qz)
    //! with qi=pi-p3, q=xy-p3

    for (int j = 0; j < dimworld; j++)
    {
      x0 = elInfo_->coord[dim][j];
      x[j] = global.read(j) - x0;
      for (int i = 0; i < dim; i++)
        edge[i][j] = elInfo_->coord[i][j] - x0;
    }

    det = edge[0][0] * edge[1][1] * edge[2][2]
          + edge[0][1] * edge[1][2] * edge[2][0]
          + edge[0][2] * edge[1][0] * edge[2][1]
          - edge[0][2] * edge[1][1] * edge[2][0]
          - edge[0][0] * edge[1][2] * edge[2][1]
          - edge[0][1] * edge[1][0] * edge[2][2];
    det0 = x[0] * edge[1][1] * edge[2][2]
           + x[1] * edge[1][2] * edge[2][0]
           + x[2] * edge[1][0] * edge[2][1]
           - x[2] * edge[1][1] * edge[2][0]
           - x[0] * edge[1][2] * edge[2][1] - x[1] * edge[1][0] * edge[2][2];
    det1 = edge[0][0] * x[1] * edge[2][2]
           + edge[0][1] * x[2] * edge[2][0]
           + edge[0][2] * x[0] * edge[2][1]
           - edge[0][2] * x[1] * edge[2][0]
           - edge[0][0] * x[2] * edge[2][1] - edge[0][1] * x[0] * edge[2][2];
    det2 = edge[0][0] * edge[1][1] * x[2]
           + edge[0][1] * edge[1][2] * x[0]
           + edge[0][2] * edge[1][0] * x[1]
           - edge[0][2] * edge[1][1] * x[0]
           - edge[0][0] * edge[1][2] * x[1] - edge[0][1] * edge[1][0] * x[2];
    if(ABS(det) < 1.E-20)
    {
      printf("det = %e; abort\n", det);
      abort();
      return (-2);
    }

    // lambda is initialized here
    lambda(0) = det0 / det;
    lambda(1) = det1 / det;
    lambda(2) = det2 / det;
    lambda(3) = 1.0 - lambda(0) - lambda(1) - lambda(2);

    return lambda;
  }

  template< int dim, int dimworld>
  inline albertCtype AlbertGridElement<dim,dimworld>::
  integration_element (const Vec<dim,albertCtype>& local)
  {
    if(builtinverse_)
      return volume_;

    // builds the jacobian inverse and calculates the volume
    builtJacobianInverse(local);
    return volume_;
  }
  inline Mat<1,1>& AlbertGridElement<1,2>::
  Jacobian_inverse (const Vec<1,albertCtype>& local)
  {
    std::cout << "Jaconbian_inverse for dim=1,dimworld=2 not implemented yet! \n";
    return Jinv_;
  }

  template< int dim, int dimworld>
  inline Mat<dim,dim>& AlbertGridElement<dim,dimworld>::
  Jacobian_inverse (const Vec<dim,albertCtype>& local)
  {
    if(builtinverse_)
      return Jinv_;

    // builds the jacobian inverse and calculates the volume
    builtJacobianInverse(local);
    return Jinv_;
  }

  template< int dim, int dimworld>
  inline void AlbertGridElement<dim,dimworld>::
  builtJacobianInverse(const Vec<dim,albertCtype>& local)
  {
    enum { div = (dim < 3) ? 1 : 2 };
    // volFactor should be 0.5 for dim==2 and (1.0/6.0) for dim==3
    static const albertCtype volFactor = static_cast<albertCtype> (0.5/div);

    ALBERT REAL lambda[dim+1][dimworld];

    // is faster than the lower method
    volume_ = volFactor * ALBERT el_grd_lambda(elInfo_,lambda);

    for(int i=0; i<dim; i++)
    {
      for(int j=0; j<dimworld; j++)
        Jinv_(i,j) = lambda[i][j];
    }
    builtinverse_ = true;
  }

  inline void AlbertGridElement<1,2>::
  builtJacobianInverse(const Vec<1,albertCtype>& local)
  {
    // volume is length of edge
    Vec<2,albertCtype> vec = coord_(0) - coord_(1);
    volume_ = sqrt( vec*vec );

    builtinverse_ = true;
  }

  template<int dim, int dimworld>
  inline bool AlbertGridElement <dim ,dimworld >::
  checkInside(const Vec<dimworld> &global)
  {
    Vec<dim+1> localCoords = localBary(global);

    // return true if point is inside element
    bool ret=true;

    // if one of the barycentric coordinates is negativ
    // then the point must be outside of the element
    for(int i=0; i<dim+1; i++)
      if(localCoords(i) < 0.0) ret = false;

    return ret;
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
  setElInfo(ALBERT EL_INFO * elInfo, int elNum, unsigned char face,
            unsigned char edge, unsigned char vertex )
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
    std::cout << "father not correctly implemented! \n";
    ALBERT TRAVERSE_STACK travStack;
    initTraverseStack(&travStack);

    travStack = (*travStack_);

    travStack.stack_used--;

    AlbertGridLevelIterator <0,dim,dimworld>
    it(travStack.elinfo_stack+travStack.stack_used);
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
      elInfo_->el->mark = refCount;
      return true;
    }

    std::cout << "Element not marked!\n";
    return false;
  }

  template< int dim, int dimworld>
  inline bool AlbertGridEntity < 0, dim ,dimworld >::hasChildren()
  {
    //return (elInfo_->el->child[0] != NULL);
    return ( level_ != grid_.maxlevel() );
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
  inline int AlbertGridEntity<0,3,3>::count<2> ()
  {
    return 6;
  }

  // subIndex
  template <int dim, int dimworld> template <int cc>
  inline int AlbertGridEntity<0,dim,dimworld>::subIndex ( int i )
  {
    assert(cc == dim);
    return grid_.template indexOnLevel<dim>(elInfo_->el->dof[i][0],level_);
  }

  // default is faces
  template <int dim, int dimworld> template <int cc>
  inline AlbertGridLevelIterator<cc,dim,dimworld>
  AlbertGridEntity<0,dim,dimworld>::entity ( int i )
  {
    AlbertGridLevelIterator<cc,dim,dimworld> tmp (grid_,elInfo_,
                                                  grid_. template indexOnLevel<cc>( globalIndex() ,level_),i,0,0);
    return tmp;
  }

  template <>
  inline AlbertGridLevelIterator<2,3,3>
  AlbertGridEntity<0,3,3>::entity<2> ( int i )
  {
    enum { cc = 2 };
    int num = grid_.indexOnLevel<cc>(globalIndex() ,level_);
    if(i < 3)
    { // 0,1,2
      AlbertGridLevelIterator<cc,3,3> tmp (grid_,elInfo_,num, 0,i,0);
      return tmp;
    }
    else
    { // 3,4,5
      AlbertGridLevelIterator<cc,3,3> tmp (grid_,elInfo_,num, i-2,1,0);
      return tmp;
    }
  }

  // specialization for vertices
  template <>
  inline AlbertGridLevelIterator<2,2,2>
  AlbertGridEntity<0,2,2>::entity<2> ( int i )
  {
    std::cout << "entity<2> ,2,2 !\n";
    // we are looking at vertices
    enum { cc = dimension };
    AlbertGridLevelIterator<cc,dimension,dimensionworld>
    tmp (grid_,elInfo_, grid_.indexOnLevel<cc>( elInfo_->el->dof[i][0],level_),
         0,0,i);
    return tmp;
  }
  // specialization for vertices
  template <>
  inline AlbertGridLevelIterator<2,2,3>
  AlbertGridEntity<0,2,3>::entity<2> ( int i )
  {
    // we are looking at vertices
    enum { cc = dimension };
    AlbertGridLevelIterator<cc,dimension,dimensionworld>
    tmp (grid_,elInfo_, grid_.indexOnLevel<cc>( elInfo_->el->dof[i][0],level_),
         0,0,i);
    return tmp;
  }
  // specialization for vertices
  template <>
  inline AlbertGridLevelIterator<3,3,3>
  AlbertGridEntity<0,3,3>::entity<3> ( int i )
  {
    // we are looking at vertices
    enum { cc = dimension };
    AlbertGridLevelIterator<cc,dimension,dimensionworld>
    tmp (grid_,elInfo_, grid_.indexOnLevel<cc>( elInfo_->el->dof[i][0],level_),
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
    //return elInfo_->level;
  }

  template<int dim, int dimworld>
  inline int AlbertGridEntity < 0, dim ,dimworld >::
  index()
  {
    return grid_.template indexOnLevel<0>( globalIndex() , level_ );
  }

  template< int dim, int dimworld>
  inline void AlbertGridEntity < 0, dim ,dimworld >::
  setLevel(int actLevel)
  {
    level_ = actLevel;
  }

  template< int dim, int dimworld>
  inline void AlbertGridEntity < 0, dim ,dimworld >::
  setElInfo(ALBERT EL_INFO * elInfo, int elNum,  unsigned char face,
            unsigned char edge, unsigned char vertex )
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
    if(!builtgeometry_)
      std::cout << "AlbertGridEntity<0,dim,dimworld>::geometry(): Warning, geometry has not been built! \n";
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
    if(level_ > 0)
      fatherInfo = &travStack_->elinfo_stack[travStack_->stack_used-1];
    else
    {
      fatherInfo = elInfo_;
      fatherLevel = 0;
    }

    // new LevelIterator with EL_INFO one level above
    AlbertGridLevelIterator <0,dim,dimworld>
    it(grid_,fatherInfo,grid_.template indexOnLevel<0>(fatherInfo->el->index,fatherLevel),0,0,0);
    return it;
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
    manageStack_.init();
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
    virtualEntity_.setElInfo(recursiveTraverse(manageStack_.getStack()));
    // set new actual level
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

      while((stack->stack_used > 0) &&
            ((stack->info_stack[stack->stack_used] >= 2)
             || ((el->child[0]==NULL) && level_ >= maxlevel_ )
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
    // the case if we have no child but level_ < maxlevel_
    // then we want to fake the next maxlevel_ - level_ elements
    else if(level_ < maxlevel_)
    {
      // new: go down until maxlevel, but fake the not existant elements
      if(stack->stack_used >= stack->stack_size - 1)
        ALBERT enlargeTraverseStack(stack);

      el = el;

      // means all elements visited
      stack->info_stack[stack->stack_used] = 2;

      // new: go down maxlevel, but fake the elements
      level_++;
      grid_.fillElInfo(0, level_, stack->elinfo_stack+stack->stack_used,stack->elinfo_stack+stack->stack_used+1, true);
      //ALBERT fill_elinfo(i, stack->elinfo_stack + stack->stack_used,
      //  stack->elinfo_stack + (stack->stack_used + 1));
      stack->stack_used++;

      stack->info_stack[stack->stack_used] = 0;
    }

    return (stack->elinfo_stack + stack->stack_used);
  } // recursive traverse over all childs

  // end AlbertGridHierarchicIterator

  //***************************************************************
  //
  //  --AlbertGridNeighborIterator
  //  --NeighborIterator
  //
  //***************************************************************

  //static ALBERT EL_INFO neighElInfo_;

  // these object should be generated with new by Entity, because
  // for a LevelIterator we only need one virtualNeighbour Entity, which is
  // given to the Neighbour Iterator, we need a list of Neighbor Entitys
  template< int dim, int dimworld>
  inline AlbertGridNeighborIterator<dim,dimworld>::~AlbertGridNeighborIterator ()
  {
    if(manageObj_)
      grid_.entityProvider_.freeObjectEntity(manageObj_);

    if(manageInterEl_)
      grid_.interSelfProvider_.freeObjectEntity(manageInterEl_);

    if(manageNeighEl_)
      grid_.interNeighProvider_.freeObjectEntity(manageNeighEl_);

    if(boundaryEntity_) delete boundaryEntity_;

    if(manageNeighInfo_) elinfoProvider.freeObjectEntity(manageNeighInfo_);
  }

  template< int dim, int dimworld>
  inline AlbertGridNeighborIterator<dim,dimworld>::
  AlbertGridNeighborIterator(AlbertGrid<dim,dimworld> &grid, int level) :
    grid_(grid), level_ (level) , virtualEntity_ (NULL)
    , fakeNeigh_ (NULL)
    , neighGlob_ (NULL) , elInfo_ (NULL) , neighborCount_ (dim+1)
    , manageObj_ (NULL)
    , manageInterEl_ (NULL)
    , manageNeighEl_ (NULL)
    , boundaryEntity_ (NULL)
    , manageNeighInfo_ (NULL) , neighElInfo_ (NULL) {}

  template< int dim, int dimworld>
  inline AlbertGridNeighborIterator<dim,dimworld>::AlbertGridNeighborIterator
    (AlbertGrid<dim,dimworld> &grid, int level, ALBERT EL_INFO *elInfo ) :
    grid_(grid) , level_ (level), neighborCount_ (0), elInfo_ ( elInfo )
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

  template< int dim, int dimworld>
  inline AlbertGridNeighborIterator<dim,dimworld>&
  AlbertGridNeighborIterator<dim,dimworld>::
  operator ++()
  {
    builtNeigh_ = false;
    // is like go to the next neighbour
    neighborCount_++;
    return (*this);
  }

  template< int dim, int dimworld>
  inline AlbertGridNeighborIterator<dim,dimworld>&
  AlbertGridNeighborIterator<dim,dimworld>::operator ++(int steps)
  {
    neighborCount_ += steps;
    if(neighborCount_ > dim+1) neighborCount_ = dim+1;
    builtNeigh_ = false;

    return (*this);
  }

  template< int dim, int dimworld>
  inline bool AlbertGridNeighborIterator<dim,dimworld>::operator ==
    (const AlbertGridNeighborIterator& I) const
  {
    return (neighborCount_ == I.neighborCount_);
  }

  template< int dim, int dimworld>
  inline bool AlbertGridNeighborIterator<dim,dimworld>::
  operator !=(const AlbertGridNeighborIterator& I) const
  {
    return (neighborCount_ != I.neighborCount_);
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >&
  AlbertGridNeighborIterator<dim,dimworld>::
  operator *()
  {
    if(!builtNeigh_)
    {
      if(!manageObj_)
      {
        manageObj_ = grid_.entityProvider_.getNewObjectEntity(grid_,level_);
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
  AlbertGridNeighborIterator<dim,dimworld>::
  operator ->()
  {
    if(!builtNeigh_)
    {
      if(!manageObj_)
      {
        manageObj_ = grid_.entityProvider_.getNewObjectEntity(grid_,level_);
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
  AlbertGridNeighborIterator<dim,dimworld>::boundaryEntity ()
  {
    if(!boundaryEntity_)
    {
      boundaryEntity_ = new AlbertGridBoundaryEntity<dim,dimworld> ();
    }
    boundaryEntity_->setElInfo(elInfo_,neighborCount_);
    return (*boundaryEntity_);
  }

  template< int dim, int dimworld>
  inline bool AlbertGridNeighborIterator<dim,dimworld>::boundary()
  {
    return (elInfo_->boundary[neighborCount_] != NULL);
  }

  template< int dim, int dimworld>
  inline bool AlbertGridNeighborIterator<dim,dimworld>::neighbor()
  {
    return (elInfo_->neigh[neighborCount_] != NULL);
  }

  template< int dim, int dimworld>
  inline Vec<dimworld,albertCtype>& AlbertGridNeighborIterator<dim,dimworld>::
  unit_outer_normal(Vec<dim-1,albertCtype>& local)
  {
    Vec<dimworld,albertCtype> tmp = outer_normal(local);

    double norm = tmp.norm2();
    if(!(norm > 0.0)) norm = 1.0;

    for(int i=0; i<dimworld; i++)
      outerNormal_(i) = tmp(i)/norm;

    return outerNormal_;
  }

  template< int dim, int dimworld>
  inline Vec<dimworld,albertCtype>& AlbertGridNeighborIterator<dim,dimworld>::
  unit_outer_normal()
  {
    Vec<dimworld,albertCtype> tmp = outer_normal();

    double norm = tmp.norm2();

    for(int i=0; i<dimworld; i++)
      outerNormal_(i) = tmp(i)/norm;

    return outerNormal_;
  }

  template< int dim, int dimworld>
  inline Vec<dimworld,albertCtype>& AlbertGridNeighborIterator<dim,dimworld>::
  outer_normal(Vec<dim-1,albertCtype>& local)
  {
    std::cout << "outer_normal() not correctly implemented yet! \n";
    for(int i=0; i<dimworld; i++)
      outerNormal_(i) = 0.0;

    return outerNormal_;
  }

  template< int dim, int dimworld>
  inline Vec<dimworld,albertCtype>& AlbertGridNeighborIterator<dim,dimworld>::
  outer_normal()
  {
    std::cout << "outer_normal() not correctly implemented yet! \n";
    for(int i=0; i<dimworld; i++)
      outerNormal_(i) = 0.0;

    return outerNormal_;
  }

  inline Vec<2,albertCtype>& AlbertGridNeighborIterator<2,2>::
  outer_normal()
  {
    // seems to work
    ALBERT REAL_D *coord = elInfo_->coord;

    outerNormal_(0) = -(coord[(neighborCount_+1)%3][1] - coord[(neighborCount_+2)%3][1]);
    outerNormal_(1) =   coord[(neighborCount_+1)%3][0] - coord[(neighborCount_+2)%3][0];

    return outerNormal_;
  }

  inline Vec<3,albertCtype>& AlbertGridNeighborIterator<3,3>::
  outer_normal()
  {
    // rechne Kreuzprodukt der Vectoren aus
    ALBERT REAL_D *coord = elInfo_->coord;
    Vec<3,albertCtype> v(0.0);
    Vec<3,albertCtype> u(0.0);

    for(int i=0; i<3; i++)
    {
      v(i) = coord[(neighborCount_+2)%4][i] - coord[(neighborCount_+1)%4][i];
      u(i) = coord[(neighborCount_+3)%4][i] - coord[(neighborCount_+2)%4][i];
    }

    for(int i=0; i<3; i++)
      outerNormal_(i) = u((i+1)%3)*v((i+2)%3) - u((i+2)%3)*v((i+1)%3);

    return outerNormal_;
  }

  template< int dim, int dimworld>
  inline AlbertGridElement< dim-1, dim >&
  AlbertGridNeighborIterator<dim,dimworld>::
  intersection_self_local()
  {
    std::cout << "intersection_self_local not check until now! \n";
    if(!manageInterEl_)
    {
      manageInterEl_ = grid_.interSelfProvider_.getNewObjectEntity();
      fakeNeigh_ = manageInterEl_->item;
    }

    fakeNeigh_->builtGeom(elInfo_,neighborCount_,0,0);
    return (*fakeNeigh_);
  }

  template< int dim, int dimworld>
  inline AlbertGridElement< dim-1, dimworld >&
  AlbertGridNeighborIterator<dim,dimworld>::
  intersection_self_global()
  {
    if(!manageNeighEl_)
    {
      manageNeighEl_ = grid_.interNeighProvider_.getNewObjectEntity();
      neighGlob_ = manageNeighEl_->item;
    }
    // built neighGlob_ first
    neighGlob_->builtGeom(elInfo_,neighborCount_,0,0);
    return (*neighGlob_);
  }

  template< int dim, int dimworld>
  inline AlbertGridElement< dim-1, dim >&
  AlbertGridNeighborIterator<dim,dimworld>::
  intersection_neighbor_local()
  {
    std::cout << "intersection_neighbor_local not check until now! \n";
    if(!manageInterEl_)
    {
      manageInterEl_ = grid_.interSelfProvider_.getNewObjectEntity();
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
  AlbertGridNeighborIterator<dim,dimworld>::
  intersection_neighbor_global()
  {
    std::cout << "intersection_neighbor_global not check until now! \n";
    if(!manageNeighEl_)
    {
      manageNeighEl_ = grid_.interNeighProvider_.getNewObjectEntity();
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
  inline int AlbertGridNeighborIterator<dim,dimworld>::
  number_in_self ()
  {
    return neighborCount_;
  }

  template< int dim, int dimworld>
  inline int AlbertGridNeighborIterator<dim,dimworld>::
  number_in_neighbor ()
  {
    return elInfo_->opp_vertex[neighborCount_];
  }

  // setup neighbor element with the information of elInfo_
  template< int dim, int dimworld>
  inline void AlbertGridNeighborIterator<dim,dimworld>::setupVirtEn()
  {

#if DIM > 2
    std::cout << "dim Formel not checked in setupVirtEn !\n ";
    abort();
#endif
    // set the neighbor element as element
    neighElInfo_->el = elInfo_->neigh[neighborCount_];

    int vx = elInfo_->opp_vertex[neighborCount_];

    memcpy(&neighElInfo_->coord[vx], &elInfo_->coord[neighborCount_],
           dimworld*sizeof(ALBERT REAL));

    for(int i=1; i<dim+1; i++)
    {
      int nb = (((neighborCount_-i)%(dim+1)) +dim+1)%(dim+1);
      memcpy(&neighElInfo_->coord[(vx+i)%(dim+1)], &elInfo_->coord[nb],
             dimworld*sizeof(ALBERT REAL));
    }

    /*
       ALBERT REAL_D *elcoord = (REAL_D *) &elInfo_->coord;
       ALBERT REAL_D *nbcoord = (REAL_D *) &neighElInfo_->coord;

       for(int j=0; j<dimworld; j++)
        nbcoord[vx][j] = elcoord[neighborCount_][j];

       for(int i=1; i<dim+1; i++)
       {
        int nb = (neighborCount_-i+dim+1)%(dim+1);
        for(int j=0; j<dimworld; j++)
          nbcoord[(vx+i)%(dim+1)][j] = elcoord[nb][j];
       }
     */
    virtualEntity_->setElInfo(neighElInfo_);
    builtNeigh_ = true;
  }

  // end NeighborIterator


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
  inline ALBERT EL_INFO * AlbertGridLevelIterator<1,2,2>::
  goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextFace(stack,elinfo_old);
  }
  inline ALBERT EL_INFO * AlbertGridLevelIterator<1,2,3>::
  goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextFace(stack,elinfo_old);
  }
  inline ALBERT EL_INFO * AlbertGridLevelIterator<1,3,3>::
  goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextFace(stack,elinfo_old);
  }

  // specialization for codim 2, if dim > 2, go next edge,
  // only if dim == dimworld == 3
  inline ALBERT EL_INFO * AlbertGridLevelIterator<2,3,3>::
  goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextEdge(stack,elinfo_old);
  }

  // specialization for codim == dim , go next vertex
  inline ALBERT EL_INFO * AlbertGridLevelIterator<2,2,2>::
  goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextVertex(stack,elinfo_old);
  }
  inline ALBERT EL_INFO * AlbertGridLevelIterator<2,2,3>::
  goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextVertex(stack,elinfo_old);
  }
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

    manageStack_.init();

    virtualEntity_.setTraverseStack(NULL);
    virtualEntity_.setElInfo(NULL,0,0,0,0);
  }

  // Make LevelIterator with point to element from previous iterations
  template<int codim, int dim, int dimworld>
  inline AlbertGridLevelIterator<codim,dim,dimworld >::
  AlbertGridLevelIterator(AlbertGrid<dim,dimworld> &grid, int travLevel) :
    grid_(grid), level_ (travLevel) ,  virtualEntity_(grid,0)
  {
    makeIterator();
  }

  // Make LevelIterator with point to element from previous iterations
  template<int codim, int dim, int dimworld>
  inline AlbertGridLevelIterator<codim,dim,dimworld >::
  AlbertGridLevelIterator(AlbertGrid<dim,dimworld> &grid,
                          ALBERT EL_INFO *elInfo,int elNum,int face,int edge,int vertex) :
    grid_(grid), virtualEntity_(grid,0) , elNum_ ( elNum ) , face_ ( face ) ,
    edge_ ( edge ), vertex_ ( vertex )
  {
    vertexMarker_ = NULL;
    manageStack_.init();
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
                          int travLevel) : grid_(grid) , level_ (travLevel)
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
      elinfo = traverseLeafElLevel(stack);
      if (elinfo)
        stack->el_count++;
      //else {
      //  /* MSG("total element count:%d\n",stack->el_count); */
      // }
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
  traverseLeafElLevel(ALBERT TRAVERSE_STACK *stack)
  {
    // 28.02.2003 robertk, zwei Unterschiede zu
    // traverse_leaf_el, naemlich Abbruch bei Level > ...
    FUNCNAME("traverseLeafElLevel");
    ALBERT EL *el;
    int i;

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
           (stack->traverse_level > (stack->elinfo_stack+stack->stack_used)->level))
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
    }

    //printElInfo(stack->elinfo_stack+stack->stack_used);
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
  inline AlbertGridNeighborIterator<dim,dimworld>
  AlbertGridEntity < 0, dim ,dimworld >::nbegin()
  {
    AlbertGridNeighborIterator<dim,dimworld> it(grid_,level(),elInfo_);
    return it;
  }

  template< int dim, int dimworld>
  inline AlbertGridNeighborIterator<dim,dimworld>
  AlbertGridEntity < 0, dim ,dimworld >::nend()
  {
    AlbertGridNeighborIterator<dim,dimworld> it(grid_,level());
    return it;
  }

  //*********************************************************************
  //
  //  AlbertMarkerVertex
  //
  //*********************************************************************
#if 0
  inline void AlbertMarkerVector::makeNewSize(int newNumberOfEntries)
  {
    vec_.resize(newNumberOfEntries);
    for(Array<int>::Iterator it = vec_.begin(); it != vec_.end(); ++it)
      (*it) = -1;
  }

  inline void AlbertMarkerVector::makeSmaller(int newNumberOfEntries)
  {}

  void AlbertMarkerVector::checkMark(ALBERT EL_INFO * elInfo, int localNum)
  {
    if(vec_[elInfo->el->dof[localNum][0]] == -1)
      vec_[elInfo->el->dof[localNum][0]] = elInfo->el->index;
  }
#endif

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
        for(int local=0; local<GridType::dimension+1; local++)
        {
          int num = it->getElInfo()->el->dof[local][0];
          if( vec_[level * nvx + num] == -1 )
            vec_[level * nvx + num] = it->globalIndex();
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
  {
    vertexMarker_ = new AlbertMarkerVector ();
  }

  template < int dim, int dimworld >
  inline AlbertGrid < dim, dimworld >::AlbertGrid(const char *MacroTriangFilename)
  {
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

      numberOfEntitys_.resize(dim+1);

      for(int i=0; i<dim+1; i++) numberOfEntitys_[i] = NULL;

      numberOfEntitys_[0] = &(mesh_->n_hier_elements);
      numberOfEntitys_[dim] = &(mesh_->n_vertices);

      // we have at least one level, level 0
      maxlevel_ = 0;

      neighOnLevel_.resize( mesh_->n_hier_elements);

      vertexMarker_->markNewVertices( *this );

      markNew();
      wasChanged_ = true;
    }
    else
    {
      read (MacroTriangFilename,time_,0);
    }
  }


  template < int dim, int dimworld >
  inline AlbertGrid < dim, dimworld >::~AlbertGrid()
  {
    ALBERT free_mesh(mesh_);
    if(vertexMarker_) delete vertexMarker_;
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

  //**************************************
  //  refine and coarsen methods
  //**************************************
  template < int dim, int dimworld >
  inline bool AlbertGrid < dim, dimworld >::
  globalRefine(int refCount)
  {
    typedef AlbertGridLevelIterator <0,dim,dimworld> LevIt;

    LevIt endit = lend<0>(maxlevel());
    for(LevIt it = lbegin<0>(maxlevel()); it != endit; ++it)
      (*it).mark(refCount);

    wasChanged_ = refine ();

    return wasChanged_;
  }


  template < int dim, int dimworld >
  inline bool AlbertGrid < dim, dimworld >::refine()
  {
    unsigned char flag;

    // refine underlying mesh
    // AlbertRefine defined in albertextra.hh
    flag = ALBERT AlbertRefine( mesh_ );

    wasChanged_ = (flag == 0) ? false : true;

    if(wasChanged_)
    {
      calcExtras();
    }

    /*
       for(int i=0 ;i<=maxlevel_; i++)
       {
       printf("********************************\n");
       printf("Level %d \n",i);
       ALBERT mesh_traverse(mesh_,i,CALL_LEAF_EL_LEVEL | FILL_ANY, ALBERT printNeighbour );
       }
     */

    return wasChanged_;
  }

  template < int dim, int dimworld >
  inline bool AlbertGrid < dim, dimworld >::
  coarsen()
  {
    unsigned char flag;
    // AlbertCoarsen defined in albertextra.hh
    flag = ALBERT AlbertCoarsen ( mesh_ );
    wasChanged_ = (flag == 0) ? false : true;

    printf("AlbertGrid<%d,%d>::coarsen: Grid coarsend, maxlevel = %d \n",
           dim,dimworld,maxlevel_);

    return wasChanged_;
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
  inline void AlbertGrid < dim, dimworld >::calcExtras ()
  {
    if(numberOfEntitys_.size() != dim+1)
      numberOfEntitys_.resize(dim+1);

    for(int i=0; i<dim+1; i++) numberOfEntitys_[i] = NULL;

    numberOfEntitys_[0] = &(mesh_->n_hier_elements);
    numberOfEntitys_[dim] = &(mesh_->n_vertices);
    // determine new maxlevel and mark neighbours
    maxlevel_ = ALBERT AlbertHelp::calcMaxLevelAndMarkNeighbours( mesh_, neighOnLevel_ );

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
    return static_cast<bool> (ALBERT write_mesh (mesh_ , filename, time) );
  }

  template < int dim, int dimworld >
  inline bool AlbertGrid < dim, dimworld >::readGridXdr (const char * filename, albertCtype & time )
  {
    // use read_mesh_xdr, but works not correctly
    mesh_ = (ALBERT read_mesh (filename, &time , ALBERT AlbertHelp::initLeafData ,
                               ALBERT AlbertHelp::initBoundary) );

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

      typedef AlbertGridNeighborIterator<dim,dimworld> Neighit;
      Neighit nit = it->nbegin();

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
  indexOnLevel(int globalIndex, int level)
  {
    // level = 0 is not interesting for this implementation
    // +1, because if Entity is Boundary then globalIndex == -1
    // an therefore we add 1 and get Entry 0, which schould be -1
    if (globalIndex < 0)
      return globalIndex;
    else
      return levelIndex_[codim][(level * (*(numberOfEntitys_[codim]))) + globalIndex];
  }

  // create lookup table for indices of the elements
  template < int dim, int dimworld >
  inline void AlbertGrid < dim, dimworld >::markNew()
  {
    // only for gcc, means notin'
    //typedef AlbertGrid < dim ,dimworld > GridType;

    int nElements = mesh_->n_hier_elements;
    int nVertices = mesh_->n_vertices;

    int number = (maxlevel_+1) * nElements;
    if(number > levelIndex_[0].size())
      //makeNewSize(levelIndex_[0], number);
      levelIndex_[0].resize(number);

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
      for(LevelIterator it = lbegin<0> (level); it != endit; ++it)
      {
        int no = it->globalIndex();
        levelIndex_[0][level * nElements + no] = num;
        num++;
      }
      // remember the number of entity on level and codim = 0
      size_[level*numCodim /* +0 */] = num;
    };

    if((maxlevel_+1) * nVertices > levelIndex_[dim].size())
      makeNewSize(levelIndex_[dim], ((maxlevel_+1)* nVertices));

    for(int level=0; level <= maxlevel_; level++)
    {
      //std::cout << level << " " << maxlevel_ << "\n";
      typedef AlbertGridLevelIterator<dim,dim,dimworld> LevelIterator;
      int num = 0;
      LevelIterator endit = lend<dim> (level);
      for(LevelIterator it = lbegin<dim> (level); it != endit; ++it)
      {
        int no = it->globalIndex();
        //std::cout << no << " Glob Num\n";
        levelIndex_[dim][level * nVertices + no] = num;
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
  inline void AlbertGrid<3,3>::
  fillElInfo(int ichild, int actLevel , const ALBERT EL_INFO *elinfo_old, ALBERT EL_INFO *elinfo, bool hierarchical) const
  {
    enum { dim = 3 };
    enum { dimworld = 3 };

    ALBERT fill_elinfo(ichild,elinfo_old,elinfo);
  } // end Grid::fillElInfo 3D

} // end namespace dune
