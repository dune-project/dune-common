// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// Implementation von AlbertGrid
//////////////////////////////////////////////////////////////////////////

#ifdef __ALBERTNAME__
namespace Albert
{
#endif

void AlbertLeafRefine(EL *parent, EL *child[2]){};
void AlbertLeafCoarsen(EL *parent, EL *child[2]){};

void initLeafData(LEAF_DATA_INFO * linfo)
{
  linfo->leaf_data_size = sizeof(AlbertLeafData);
  linfo->refine_leaf_data = &AlbertLeafRefine;
  linfo->coarsen_leaf_data =&AlbertLeafCoarsen;

  return;
}
void initDofAdmin(MESH *mesh)
{
  FUNCNAME("initDofAdmin");
  int degree = 1;
  const BAS_FCTS  *lagrange;

  lagrange = get_lagrange(degree);
  TEST_EXIT(lagrange) ("no lagrange BAS_FCTS\n");
  feSpace = get_fe_space(mesh, "Linear Lagrangian Elements", nil, lagrange);

  return;
}

const BOUNDARY *initBoundary(MESH * Spmesh, int bound)
{
  FUNCNAME("initBoundary");
  static const BOUNDARY Diet1 = { NULL, 1 };
  static const BOUNDARY PaulN1 = { NULL, -1 };

  static const BOUNDARY Diet2 = { NULL, 2 };
  static const BOUNDARY PaulN2 = { NULL, -2 };

  static const BOUNDARY Diet3 = { NULL, 3 };
  static const BOUNDARY PaulN3 = { NULL, -3 };

  static const BOUNDARY Diet4 = { NULL, 4 };
  static const BOUNDARY PaulN4 = { NULL, -4 };

  static const BOUNDARY Diet5 = { NULL, 5 };
  static const BOUNDARY PaulN5 = { NULL, -5 };


  switch (bound)
  {
  case 1 :
    return (&Diet1);
  case 2 :
    return (&Diet2);
  case 3 :
    return (&Diet3);
  case 4 :
    return (&Diet4);
  case 5 :
    return (&Diet5);

  case -1 :
    return (&PaulN1);
  case -2 :
    return (&PaulN2);
  case -3 :
    return (&PaulN3);
  case -4 :
    return (&PaulN4);
  case -5 :
    return (&PaulN5);
  default :
    ALBERT_ERROR_EXIT("no Boundary for %d. Och! \n", bound);
  }
  return &Diet1;
}

#ifdef __ALBERTNAME__
} // end namespace Albert
#endif

//////////////////////////////////////////////////////////////////////
//
//  namespace Dune
//
//////////////////////////////////////////////////////////////////////
namespace Dune
{

  //********************************************************************
  //
  //  some template spezialization for AlbertGrid
  //  the rest of AlbertGrid is on the bottom of this file
  //
  //********************************************************************
  //! map the global index from the Albert Mesh to the local index on Level
#if 0
  template <>
  inline int AlbertGrid<2,2>::
  indexOnLevel<2>(int globalIndex, int level)
  {
    return levelIndex_[2][level*mesh_->n_vertices + globalIndex];
  };

  template <>
  inline int AlbertGrid<2,3>::
  indexOnLevel<2>(int globalIndex, int level)
  {
    return levelIndex_[2][level*mesh_->n_vertices + globalIndex];
  };

  template <>
  inline int AlbertGrid<3,3>::
  indexOnLevel<3>(int globalIndex, int level)
  {
    return levelIndex_[3][level*mesh_->n_vertices + globalIndex];
  };
#endif
  //********************************************************************


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
  // default, do nothing
#if 0
  template< int dim, int dimworld> template <int cc>
  inline int AlbertGridElement<dim,dimworld>::mapVertices (int i)
  {
    return i;
  };

  // specialication for codim == 1
  template <>
  inline int AlbertGridElement<2,2>::mapVertices<1> (int i)
  {
    // N_VERTICES (from ALBERT) = dim +1
    return ((face_ + 1 + i) % (dimension+1));
  };
  template <>
  inline int AlbertGridElement<2,3>::mapVertices<1> (int i)
  {
    // N_VERTICES (from ALBERT) = dim +1
    return ((face_ + 1 + i) % (dimension+1));
  };
  template <>
  inline int AlbertGridElement<3,3>::mapVertices<1> (int i)
  {
    // N_VERTICES (from ALBERT) = dim +1
    return ((face_ + 1 + i) % (dimension+1));
  };

  // specialization for codim == 2, edges
  template <>
  inline int AlbertGridElement<3,3>::mapVertices<2> (int i)
  {
    // N_VERTICES (from ALBERT) = dim +1 = 4
    return ((face_+1)+ (edge_+1) +i)% (dimension+1);
  }

  // specialization for codim == dim , Vertices
  template <>
  inline int AlbertGridElement<2,2>::mapVertices<2> (int i)
  {
    // N_VERTICES (from ALBERT) = dim +1 = 3
    return ((face_+1)+ (edge_+1) +(vertex_+1) +i)% (dimension+1);
  }
  template <>
  inline int AlbertGridElement<2,3>::mapVertices<2> (int i)
  {
    // N_VERTICES (from ALBERT) = dim +1 = 3
    return ((face_+1)+ (edge_+1) +(vertex_+1) +i)% (dimension+1);
  }
  template <>
  inline int AlbertGridElement<3,3>::mapVertices<3> (int i)
  {
    // N_VERTICES (from ALBERT) = dim +1 = 4
    return ((face_+1)+ (edge_+1) +(vertex_+1) +i)% (dimension+1);
  }

  // end specializations for mapVertices
#endif


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
    builtinverse = false;
  }

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
    builtinverse = false;

    if(elInfo_)
    {
      for(int i=0; i<dim+1; i++)
        (coord_(i)) = static_cast< albertCtype  * >
                      ( elInfo_->coord[mapVertices<dimworld-dim>(i)] );

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
    builtinverse = false;
    if(elInfo_)
    {
      for(int i=0; i<dim+1; i++)
        (coord_(i)) = static_cast< albertCtype  * >
                      ( elInfo_->coord[mapVertices<dimworld-dim>(i)] );
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
    builtinverse = false;
    if(elInfo_)
    {
      for(int i=0; i<dim+1; i++)
        (coord_(i)) = static_cast< albertCtype  * >
                      ( elInfo_->coord[mapVertices<dimworld-dim>(i)] );
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
    // Umrechnen von localen Koordinaten zu baryzentrischen Koordinaten
    Vec<dim+1> tmp (1.0); // Wichtig, da tmp(0) = 1 - tmp(1)- ... -tmp(dim+1)
    for(int i=0; i<dim; i++)
      tmp(0) -= local.read(i);
    for(int i=1; i<dim+1; i++)
      tmp(i) = local.read(i-1);

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
    Vec<dim+1,albertCtype> tmp = localBary(global);

    // Umrechnen von baryzentrischen localen Koordinaten nach
    // localen Koordinaten,
    for(int i=0; i<dim; i++)
      localCoord_(i) = tmp(i+1);

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
     * wir haben das gleichungssystem zu loesen:
     */
    /*
     * ( q1x q2x ) (lambda1) = (qx)
     */
    /*
     * ( q1y q2y ) (lambda2) = (qy)
     */
    /*
     * mit qi=pi-p3, q=global-p3
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

    //! wir haben das gleichungssystem zu loesen:
    //! ( q1x q2x q3x) (lambda1) (qx)
    //! ( q1y q2y q3y) (lambda2) = (qy)
    //! ( q1z q2z q3z) (lambda3) (qz)
    //! mit qi=pi-p3, q=xy-p3

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
    if(builtinverse)
      return volume_;

    // builds the jacobian inverse and calculates the volume
    builtJacobianInverse(local);
    return volume_;
  }

  template< int dim, int dimworld>
  inline Mat<dim,dim>& AlbertGridElement<dim,dimworld>::
  Jacobian_inverse (const Vec<dim,albertCtype>& local)
  {
    if(builtinverse)
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
    builtinverse = true;
  }

  template<int dim, int dimworld>
  inline bool AlbertGridElement <dim ,dimworld >::
  pointIsInside(const Vec<dimworld> &point)
  {
    Vec<dim+1> localCoords = localBary(point);

    // return true if point is inside element
    bool ret=true;

    // if one of the barycentric coordinates is negativ
    // then the point must be outside of the element
    for(int i=0; i<dim+1; i++)
      if(localCoords(i) < 0.0) ret = false;

    return ret;
  }

  template< int dim, int dimworld>
  inline Vec<dimworld,albertCtype>& AlbertGridElement<dim,dimworld>::
  unit_outer_normal()
  {
    Vec<dimworld,albertCtype> tmp = outer_normal();

    double norm = tmp.norm2();
    if(!(norm > 0.0)) norm = 1.0;

    for(int i=0; i<dimworld; i++)
      outerNormal_(i) = tmp(i)/norm;

    return outerNormal_;
  }

  template< int dim, int dimworld>
  inline Vec<dimworld,albertCtype>& AlbertGridElement<dim,dimworld>::
  outer_normal()
  {
    std::cout << "outer_normal not correctly available for this elementtype! \n";
    for(int i=0; i<dimworld; i++)
      outerNormal_(i) = 0.0;

    return outerNormal_;
  }

  inline Vec<2,albertCtype>& AlbertGridElement<1,2>::
  outer_normal()
  {
    // checked, ok
    enum { dimworld = 2};
    // Faces in 2d
    Vec<dimworld,albertCtype>& v = coord_(1);
    Vec<dimworld,albertCtype>& u = coord_(0);

    outerNormal_(0) =   v(1) - u(1);
    outerNormal_(1) = -(v(0) - u(0));

    return outerNormal_;
  }

  inline Vec<3,albertCtype>& AlbertGridElement<2,3>::
  outer_normal()
  {
    enum { dimworld = 3};

    Vec<dimworld,albertCtype> v = coord_(0) - coord_(2);
    Vec<dimworld,albertCtype> u = coord_(1) - coord_(2);

    // calc vector product
    for(int i=0; i<dimworld; i++)
      outerNormal_(i) = u((i+1)%dimworld)*v((i+2)%dimworld)
                        - u((i+2)%dimworld)*v((i+1)%dimworld);

    return outerNormal_;
  }


#if 0
  //************************************************************************
  //
  //************************************************************************
  template<int dim, int dimworld>
  inline void AlbertGridEntity < dim, dim ,dimworld >::
  makeDescription()
  {
    elInfo_ = NULL;
    builtgeometry_ = false;
    //geo_.initGeom();
  }

  template<int dim, int dimworld>
  inline AlbertGridEntity < dim, dim ,dimworld >::
  AlbertGridEntity(AlbertGrid<dim,dimworld> &grid,
                   ALBERT TRAVERSE_STACK * travStack) : grid_(grid)
                                                        , vxEntity_ ( grid_ , NULL, 0, 0, 0, 0)
                                                        , geo_ (false)
  {
    travStack_ = travStack;
    makeDescription();
  }


  template< int dim, int dimworld>
  inline void AlbertGridEntity < dim, dim ,dimworld >::
  setTraverseStack(ALBERT TRAVERSE_STACK * travStack)
  {
    travStack_ = travStack;
  }

  template<int dim, int dimworld>
  inline AlbertGridEntity < dim, dim ,dimworld >::
  AlbertGridEntity(AlbertGrid<dim,dimworld> &grid) :
    grid_(grid)
    , geo_ ( false )
  {
    travStack_ = NULL;
    makeDescription();
  }

  template<int dim, int dimworld>
  inline ALBERT EL_INFO* AlbertGridEntity < dim, dim ,dimworld >::
  getElInfo() const
  {
    return elInfo_;
  }

  template<int dim, int dimworld>
  inline void AlbertGridEntity <dim, dim ,dimworld >::
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

  template<int dim, int dimworld>
  inline int AlbertGridEntity < dim, dim ,dimworld >::
  level()
  {
    return elInfo_->level;
  }

#if 0
  template<int dim, int dimworld>
  inline int AlbertGridEntity < dim, dim ,dimworld >::
  index()
  {
    return elNum_;
  }
#endif
#if 1
  //! Specialization for Vertices
  inline int AlbertGridEntity < 2, 2 ,2 >::
  index()
  {
    return grid_.indexOnLevel<2>(globalIndex(),level());
  }

  inline int AlbertGridEntity < 3, 3,3 >::
  index()
  {
    return grid_.indexOnLevel<3>(globalIndex(),level());
  }
#endif
  template< int dim, int dimworld>
  inline AlbertGridElement<0,dimworld>&
  AlbertGridEntity < dim, dim ,dimworld >::geometry()
  {
    return geo_;
  }

  template<int dim, int dimworld>
  inline Vec<dim,albertCtype>&
  AlbertGridEntity < dim, dim ,dimworld >::local()
  {
    return localFatherCoords_;
  }

  template< int dim, int dimworld>
  inline AlbertGridLevelIterator<0,dim,dimworld>
  AlbertGridEntity < dim, dim ,dimworld >::father()
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
#endif


  //*************************************************************************
  //
  //  --AlbertGridEntity
  //  --Entity
  //
  //*************************************************************************
  //
  //  codim > 0
  //
  /// The Element is prescribed by the EL_INFO struct of ALBERT MESH
  /// the pointer to this struct is set and get by setElInfo and
  /// getElInfo.
  template<int codim, int dim, int dimworld>
  inline void AlbertGridEntity < codim, dim ,dimworld >::
  makeDescription()
  {
    elInfo_ = NULL;
    builtgeometry_ = false;
    //geo_.initGeom();
  }

  template<int codim, int dim, int dimworld>
  inline AlbertGridEntity < codim, dim ,dimworld >::
  AlbertGridEntity(AlbertGrid<dim,dimworld> &grid,
                   ALBERT TRAVERSE_STACK * travStack) : grid_(grid)
                                                        //      , vxEntity_ ( grid_ , NULL, 0, 0, 0, 0)
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
  AlbertGridEntity(AlbertGrid<dim,dimworld> &grid) :
    grid_(grid)
    //, vxEntity_ ( grid , NULL, 0, 0, 0, 0)
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
    return elInfo_->level;
  }

  template<int codim, int dim, int dimworld>
  inline int AlbertGridEntity < codim, dim ,dimworld >::
  index()
  {
    return elNum_;
  }

#if 0
  //! Specialization for Vertices
  inline int AlbertGridEntity < 2, 2 ,2 >::
  index()
  {
    return grid_.indexOnLevel<2>(globalIndex(),level());
  }

  inline int AlbertGridEntity < 3, 3,3 >::
  index()
  {
    return grid_.indexOnLevel<3>(globalIndex(),level());
  }
#endif

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
  inline void AlbertGridEntity < 0, dim ,dimworld >::
  makeDescription()
  {
    elInfo_ = NULL;
    builtgeometry_ = false;

    // not fast
    //geo_.initGeom();
  }

  template<int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >::
  AlbertGridEntity(ALBERT TRAVERSE_STACK * travStack)
  {
    travStack_ = travStack;
    makeDescription();
  }

  template<int dim, int dimworld>
  inline void AlbertGridEntity < 0, dim ,dimworld >::
  setTraverseStack(ALBERT TRAVERSE_STACK * travStack)
  {
    travStack_ = travStack;
  }

  // specialization of destructor, default is empty
  inline AlbertGridEntity < 0, 2, 2 >::
  ~AlbertGridEntity()
  {
    // we only delete faceEntity_ because edgeEntity_ can only be
    // created in entity<2,3,3> which is not the same class
    if(faceEntity_) delete faceEntity_;
  }

  //template <int dim, int dimworld>
  //inline AlbertGridEntity < 0, dim, dimworld >::
  inline AlbertGridEntity < 0, 3, 3 >::
  ~AlbertGridEntity()
  {
    if(faceEntity_) delete faceEntity_;
    if(edgeEntity_) delete edgeEntity_;
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >::
  AlbertGridEntity(AlbertGrid<dim,dimworld> &grid) : grid_(grid)
                                                     , vxEntity_ ( grid_ , NULL, 0, 0, 0, 0)
                                                     , geo_(false)
  {
    travStack_ = NULL;
    elInfo_ = NULL;
    builtgeometry_ = false;

    faceEntity_ = NULL;
    edgeEntity_ = NULL;
  }

#if 0
  //****************************************
  //
  //  specialization of count and entity
  //
  //****************************************
  // specialization for codim == 2 , edges
  template <>
  inline int AlbertGridEntity<0,3,3>::count<2> ()
  {
    // number of edges of a tetrahedron
    return 6;
  }

  //****************************
  // the vertex entitys
  template <>
  inline AlbertGridLevelIterator<2,2,2>& AlbertGridEntity <0,2,2>::
  entity<2> ( int i)
  {
    vxEntity_.virtualEntity_.setElInfo(elInfo_,elInfo_->el->dof[i][0],0,0,i);
    return vxEntity_;
  }
  template <>
  inline AlbertGridLevelIterator<3,3,3>& AlbertGridEntity <0,3,3>::
  entity<3> ( int i)
  {
    vxEntity_.virtualEntity_.setElInfo(elInfo_,elInfo_->el->dof[i][0],0,0,i);
    return vxEntity_;
  }

  //*********************
  // the face access
  template <>
  inline AlbertGridLevelIterator<1,2,2>& AlbertGridEntity <0,2,2>::
  entity<1> ( int i)
  {
    std::cout << "Not checked yet! \n";
    if(!faceEntity_)
    {
      faceEntity_ = new AlbertGridLevelIterator<1,2,2>
                      ( grid_ , elInfo_,index(),i,0,0 );
      return (*faceEntity_);
    }

    faceEntity_->virtualEntity_.setElInfo(elInfo_,index(),i,0,0);
    return (*faceEntity_);
  }

  template <>
  inline AlbertGridLevelIterator<1,3,3>& AlbertGridEntity <0,3,3>::
  entity<1> ( int i)
  {
    std::cout << "Not checked yet! \n";
    if(!faceEntity_)
    {
      faceEntity_ = new AlbertGridLevelIterator<1,3,3>
                      ( grid_ , elInfo_,index(),i,0,0 );
      return (*faceEntity_);
    }

    faceEntity_->virtualEntity_.setElInfo(elInfo_,index(),i,0,0);
    return (*faceEntity_);
  }
  //*********************
  // the edge access, only for dim == 3
  template <>
  inline AlbertGridLevelIterator<2,3,3>& AlbertGridEntity <0,3,3>::
  entity<2> ( int i)
  {
    printf("Entity::entity<codim = %d>: Warning elNum may be not correct! \n",2);
    if(!edgeEntity_)
    {
      edgeEntity_ = new AlbertGridLevelIterator<2,3,3>
                      ( grid_ , NULL , 0,0,0,0);
    }

    if(i < 3)
    { // 0,1,2
      edgeEntity_->virtualEntity_.setElInfo(elInfo_,index(),0,i,0);
    }
    else
    { // 3,4,5
      edgeEntity_->virtualEntity_.setElInfo(elInfo_,index(),i-2,1,0);
    }
    return (*edgeEntity_);
  }
#endif
  template <int dim, int dimworld>
  inline AlbertGridLevelIterator<1,dim,dimworld>& AlbertGridEntity <0,dim,dimworld>::
  faceEntity ( int i)
  {
    if(!faceEntity_)
    {
      faceEntity_ = new AlbertGridLevelIterator<1,dim,dimworld>
                      ( grid_ , elInfo_,index(),i,0,0 );
      return (*faceEntity_);
    }

    faceEntity_->virtualEntity_.setElInfo(elInfo_,index(),i,0,0);
    return (*faceEntity_);
  }

  template <int dim, int dimworld>
  inline AlbertGridLevelIterator<2,dim,dimworld>& AlbertGridEntity <0,dim,dimworld>::
  edgeEntity ( int i)
  {
    printf("Entity::entity<codim = %d>: Warning elNum may be not correct! \n",2);
    if(!edgeEntity_)
    {
      edgeEntity_ = new AlbertGridLevelIterator<2,dim,dimworld>
                      ( grid_ , NULL , 0,0,0,0);
    }

    if(i < 3)
    { // 0,1,2
      edgeEntity_->virtualEntity_.setElInfo(elInfo_,index(),0,i,0);
    }
    else
    { // 3,4,5
      edgeEntity_->virtualEntity_.setElInfo(elInfo_,index(),i-2,1,0);
    }
    return (*edgeEntity_);
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
    return elInfo_->level;
  }

  template<int dim, int dimworld>
  inline int AlbertGridEntity < 0, dim ,dimworld >::
  index()
  {
#if 0
    int num = globalIndex();
    if(num < 0)
      return num;
    else
      // last 0 stands for level an is not used in this context,
      // because the global index of elinfo is unique for all levels
      return grid_.indexOnLevel<0>(num,0);
#endif
    return grid_.indexOnLevel<0>( this->globalIndex() , 0);
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
    ALBERT TRAVERSE_STACK travStack;
    initTraverseStack(&travStack);

    travStack = (*travStack_);

    travStack.stack_used--;

    AlbertGridLevelIterator <0,dim,dimworld>
    it(travStack.elinfo_stack+travStack.stack_used);
    return it;
  }

  template< int dim, int dimworld>
  inline AlbertGridElement<dim,dim>&
  AlbertGridEntity < 0, dim ,dimworld >::father_relative_local()
  {
    std::cout << "\nfather_realtive_local not implemented yet! \n";
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
  AlbertGridHierarchicIterator(AlbertGrid<dim,dimworld> &grid) :
    grid_(grid), virtualEntity_(grid)
  {
    makeIterator();
  }

  template< int dim, int dimworld>
  inline AlbertGridHierarchicIterator<dim,dimworld>::
  AlbertGridHierarchicIterator(AlbertGrid<dim,dimworld> &grid,
                               ALBERT TRAVERSE_STACK *travStack,int travLevel) :
    grid_(grid), virtualEntity_(grid)
  {
    if(travStack)
    {
      // get new ALBERT TRAVERSE STACK
      manageStack_.makeItNew(true);
      ALBERT TRAVERSE_STACK *stack = manageStack_.getStack();

      // cut old traverse stack, kepp only actual element
      cutHierarchicStack(stack, travStack);

      // set new traverse level
      if(travLevel < 0)
      {
        // this means, we go until leaf level
        stack->traverse_fill_flag = CALL_LEAF_EL | stack->traverse_fill_flag;
        // exact here has to stand Grid->maxlevel, but is ok anyway
        travLevel = grid_.maxlevel(); //123456789;
      }
      // set new traverse level
      stack->traverse_level = travLevel;

      virtualEntity_.setTraverseStack(stack);
      // Hier kann ein beliebiges Element uebergeben werden,
      // da jedes AlbertElement einen Zeiger auf das Macroelement
      // enthaelt.
      virtualEntity_.setElInfo(recursiveTraverse(stack));
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
    // die 0 ist wichtig, weil Face 0, heist hier jetzt Element
    virtualEntity_.setElInfo(recursiveTraverse(manageStack_.getStack()));
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
      el = stack->elinfo_stack[stack->stack_used].el;

      while((stack->stack_used > 0) &&
            ((stack->info_stack[stack->stack_used] >= 2)
             || (el->child[0]==NULL)
             || ( stack->traverse_level <=
                  (stack->elinfo_stack+stack->stack_used)->level)) )
      {
        stack->stack_used--;
        el = stack->elinfo_stack[stack->stack_used].el;
      }

      // goto next father is done by other iterator and not our problem
      if(stack->stack_used < 1)
        return NULL;
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
      ALBERT fill_elinfo(i, stack->elinfo_stack + stack->stack_used,
                         stack->elinfo_stack + (stack->stack_used + 1));
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
  template< int dim, int dimworld>
  inline AlbertGridNeighborIterator<dim,dimworld>::~AlbertGridNeighborIterator ()
  {
    if(virtualEntity_) delete virtualEntity_;
    if(fakeNeigh_) delete fakeNeigh_;
    if(neighGlob_) delete neighGlob_;
  }

  template< int dim, int dimworld>
  inline void AlbertGridNeighborIterator<dim,dimworld>::
  makeIterator()
  {
    // not more than dim+1 neighbours, not in ALBERT
    neighborCount_ = dim+1;
    elInfo_ = NULL;
  }

  template< int dim, int dimworld>
  inline AlbertGridNeighborIterator<dim,dimworld>::
  AlbertGridNeighborIterator(AlbertGrid<dim,dimworld> &grid) :
    grid_(grid)
  {
    virtualEntity_ = NULL;
    fakeNeigh_ = NULL;
    neighGlob_ = NULL;
    makeIterator();
  }

  template< int dim, int dimworld>
  inline AlbertGridNeighborIterator<dim,dimworld>::AlbertGridNeighborIterator
    (AlbertGrid<dim,dimworld> &grid, ALBERT EL_INFO *elInfo) :
    grid_(grid) , neighborCount_ (0), elInfo_ ( elInfo )
  {
    virtualEntity_ = NULL;
    fakeNeigh_ = NULL;
    neighGlob_ = NULL;
    if(!elInfo_)
    {
      std::cout << "Sorry, elInfo == NULL, no Neighbour Iterator! \n\n";
      makeIterator();
    }

  }

  template< int dim, int dimworld>
  inline AlbertGridNeighborIterator<dim,dimworld>&
  AlbertGridNeighborIterator<dim,dimworld>::
  operator ++()
  {
    // is like go to the next neighbour
    neighborCount_++;
    return (*this);
  }

  template< int dim, int dimworld>
  inline AlbertGridNeighborIterator<dim,dimworld>&
  AlbertGridNeighborIterator<dim,dimworld>::operator ++(int steps)
  {
    for(int i=0; i<steps; i++)
      ++(*this);
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
  inline void AlbertGridNeighborIterator<dim,dimworld>::
  setupVirtualEntity(int neighbor)
  {
    if((neighbor >= 0) && (neighbor < dim+1))
    {
      if(elInfo_->neigh[neighbor] == NULL)
      {
        // if no neighbour exists, then return the
        // the default neighbour, which means boundary
        initElInfo(&neighElInfo_);
        virtualEntity_->setElInfo(&neighElInfo_);
        return;
      }
      else
      {
        setNeighInfo(elInfo_,&neighElInfo_,neighbor);
        virtualEntity_->setElInfo(&neighElInfo_);
        return;
      }
    }
    else
    {
      std::cout << "No Neighbour for this number! \n";
      abort();
    }
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >&
  AlbertGridNeighborIterator<dim,dimworld>::
  operator *()
  {
    if(!virtualEntity_)
    {
      virtualEntity_ = new AlbertGridEntity<0,dim,dimworld> (grid_);
      virtualEntity_->setTraverseStack(NULL);
    }

    setupVirtualEntity(neighborCount_);
    return (*virtualEntity_);
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >*
  AlbertGridNeighborIterator<dim,dimworld>::
  operator ->()
  {
    if(!virtualEntity_)
    {
      virtualEntity_ = new AlbertGridEntity<0,dim,dimworld> (grid_);
      virtualEntity_->setTraverseStack(NULL);
    }

    setupVirtualEntity(neighborCount_);
    return virtualEntity_;
  }

  template< int dim, int dimworld>
  inline bool AlbertGridNeighborIterator<dim,dimworld>::boundary()
  {
    return (elInfo_->neigh[neighborCount_] == NULL);
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
    // scheint zu funktionieren
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
    if(!fakeNeigh_)
      fakeNeigh_ = new AlbertGridElement<dim-1,dim> (false);

    fakeNeigh_->builtGeom(elInfo_,neighborCount_,0,0);
    return (*fakeNeigh_);
  }

  template< int dim, int dimworld>
  inline AlbertGridElement< dim-1, dimworld >&
  AlbertGridNeighborIterator<dim,dimworld>::
  intersection_self_global()
  {
    std::cout << "intersection_self_global not check until now! \n";
    if(!neighGlob_)
      neighGlob_ = new AlbertGridElement<dim-1,dimworld> (false);

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
    if(!fakeNeigh_)
      fakeNeigh_ = new AlbertGridElement<dim-1,dim> (false);

    setNeighInfo(elInfo_,&neighElInfo_,neighborCount_);
    fakeNeigh_->builtGeom(&neighElInfo_,neighborCount_,0,0);
    return (*fakeNeigh_);
  }

  template< int dim, int dimworld>
  inline AlbertGridElement< dim-1, dimworld >&
  AlbertGridNeighborIterator<dim,dimworld>::
  intersection_neighbor_global()
  {
    std::cout << "intersection_neighbor_global not check until now! \n";
    if(!neighGlob_)
      neighGlob_ = new AlbertGridElement<dim-1,dimworld> (false);

    // built neighGlob_ first
    setNeighInfo(elInfo_,&neighElInfo_,neighborCount_);
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


  template< int dim, int dimworld>
  inline void AlbertGridNeighborIterator<dim,dimworld>::
  initElInfo(ALBERT EL_INFO * elInfo)
  {

    boundEl_.index = -1;
    boundEl_.child[0] = NULL;
    boundEl_.child[1] = NULL;
    boundEl_.mark = 0;
    boundEl_.new_coord = NULL;

    //#if DIM == 3
    //  boundEl_.el_type = 0;
    //#endif

    // initialisiert elinfo mit default Werten
    elInfo->mesh = NULL;
    elInfo->el = &boundEl_;
    elInfo->parent = NULL;
    elInfo->macro_el = NULL;
    elInfo->level = 0;
    // hatte keine Lust fuer jede Dim eine neue Methode zu schreiben
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
      elInfo->neigh[i] = NULL;
    }
  }

  // setup neighbor element with the information of elInfo_
  template< int dim, int dimworld>
  inline void AlbertGridNeighborIterator<dim,dimworld>::
  setNeighInfo(ALBERT EL_INFO * elInfo, ALBERT EL_INFO * neighElInfo, int neigh)
  {
    neighElInfo->mesh = elInfo->mesh;

    neighElInfo->el = elInfo->neigh[neigh];
    //neighElInfo->el->index = elInfo->neigh[neigh]->index;
    // no parent Infomation for Neighbors
    neighElInfo->parent = NULL;
    // no macro Information for Neighbors
    neighElInfo->macro_el = NULL;

    neighElInfo->level = elInfo->level;

    // hatte keine Lust fuer jede Dim eine neue Methode zu schreiben
#if DIM > 2
    neighElInfo->orientation = elInfo->orientation;
    neighElInfo->el_type = elInfo->el_type;
#endif

    for(int i =0; i<dim+1; i++)
    {
      for(int j =0; j< dimworld; j++)
      {
        neighElInfo->coord[i][j] = elInfo->coord[i][j];
        neighElInfo->opp_coord[i][j] = 0.0;

      }
      //neighElInfo->neigh[i] = NULL;
      //neighElInfo->bound[i] = 0;
    }

    if(elInfo->neigh[neigh])
      for(int j =0; j< dimworld; j++)
        neighElInfo->coord[neigh][j] = elInfo->opp_coord[neigh][j];
    else
    {
      Vec<dimworld> midPoint (0.0);
      Vec<dimworld> newPoint (0.0);
      Vec<dimworld> oldPoint (static_cast<double *> (elInfo->coord[neigh]));

      for(int i=0; i< dim+1; i++)
      {
        double *coord = static_cast<double *> (elInfo->coord[i]);
        for(int j=0; j< dimworld; j++)
        {
          midPoint(j) += 0.5 * coord[j];
        }
      }
      newPoint = oldPoint + 2.0 * (oldPoint - midPoint);

      for(int j =0; j< dimworld; j++)
      {
        neighElInfo->coord[neigh][j] = newPoint(j);
      }
    }
  }

  // end NeighborIterator


  //*******************************************************
  //
  // --AlbertGridLevelIterator
  // --LevelIterator
  //
  //*******************************************************

  //***********************************************************
  //
  //  some template specialization of goNextEntity
  //
  //***********************************************************
  // default implementation, go next elInfo
#if 0
  template<int codim, int dim, int dimworld> template <int cc>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<codim,dim,dimworld >::
  goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextElInfo(stack,elinfo_old);
  };

  // specializations for codim 1, go next face
  template <>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<1,2,2>::
  goNextEntity<1>(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextFace(stack,elinfo_old);
  };
  template <>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<1,2,3>::
  goNextEntity<1>(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextFace(stack,elinfo_old);
  };
  template <>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<1,3,3>::
  goNextEntity<1>(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextFace(stack,elinfo_old);
  };

  // specialization for codim 2, if dim > 2, go next edge,
  // only if dim == dimworld == 3
  template <>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<2,3,3>::
  goNextEntity<2>(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextEdge(stack,elinfo_old);
  };

  // specialization for codim == dim , go next vertex
  template <>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<2,2,2>::
  goNextEntity<2>(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextVertex(stack,elinfo_old);
  };
  template <>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<2,2,3>::
  goNextEntity<2>(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextVertex(stack,elinfo_old);
  };
  template <>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<3,3,3>::
  goNextEntity<3>(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
  {
    return goNextVertex(stack,elinfo_old);
  };
  // end specialization of goNextEntity

#endif
  template<int codim, int dim, int dimworld>
  inline void AlbertGridLevelIterator<codim,dim,dimworld >::
  makeIterator()
  {
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
  AlbertGridLevelIterator(AlbertGrid<dim,dimworld> &grid) :
    grid_(grid), virtualEntity_(grid)
  {
    makeIterator();
  }

  // Make LevelIterator with point to element from previous iterations
  template<int codim, int dim, int dimworld>
  inline AlbertGridLevelIterator<codim,dim,dimworld >::
  AlbertGridLevelIterator(AlbertGrid<dim,dimworld> &grid,
                          ALBERT EL_INFO *elInfo,int elNum,int face,int edge,int vertex) :
    grid_(grid), virtualEntity_(grid) , elNum_ ( elNum ) , face_ ( face ) ,
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
                          int travLevel) : grid_(grid) , virtualEntity_(grid)
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
        printf("AlbertGridLevelIterator<%d,%d,%d>: Wrong Level (%d) in Contructor, grid,maxlevel() = %d ! \n",
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
      goNextEntity<codim>(manageStack_.getStack(),virtualEntity_.getElInfo()),
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
    if(vertexMarker_->notOnThisElement(elInfo,vertex_))
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
      goNextEntity<codim>(manageStack_.getStack(), virtualEntity_.getElInfo());
    for(int i=1; i<= steps; i++)
      elInfo = goNextEntity<codim>(manageStack_.getStack(),virtualEntity_.getElInfo());

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

    // go to first enInfo, therefore goNextEntity<0>
    return(goNextEntity<0>(stack,NULL));
  }


  template<int codim, int dim, int dimworld>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<codim,dim,dimworld >::
  goNextElInfo(ALBERT TRAVERSE_STACK *stack, ALBERT EL_INFO *elinfo_old)
  {
    FUNCNAME("goNextElInfo");
    ALBERT EL_INFO       *elinfo=NULL;

#if 0
    if (stack->stack_used)
    {
      ALBERT_TEST_EXIT(elinfo_old == stack->elinfo_stack+stack->stack_used)
        ("invalid old elinfo\n");
    }
    else
    {
      ALBERT_TEST_EXIT(elinfo_old == nil) ("invalid old elinfo != nil\n");
    }
#endif

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
      fill_elinfo(i, stack->elinfo_stack+stack->stack_used,
                  stack->elinfo_stack+stack->stack_used+1);
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

    AlbertGridHierarchicIterator<dim,dimworld> it(grid_,travStack_,maxlevel);
    return it;
  }

  template< int dim, int dimworld>
  inline AlbertGridHierarchicIterator<dim,dimworld>
  AlbertGridEntity < 0, dim ,dimworld >::hend(int maxlevel)
  {
    AlbertGridHierarchicIterator<dim,dimworld> it(grid_);
    return it;
  }



  template< int dim, int dimworld>
  inline AlbertGridNeighborIterator<dim,dimworld>
  AlbertGridEntity < 0, dim ,dimworld >::nbegin()
  {
    AlbertGridNeighborIterator<dim,dimworld> it(grid_,elInfo_);
    return it;
  }

  template< int dim, int dimworld>
  inline AlbertGridNeighborIterator<dim,dimworld>
  AlbertGridEntity < 0, dim ,dimworld >::nend()
  {
    AlbertGridNeighborIterator<dim,dimworld> it(grid_);
    return it;
  }

  //*********************************************************************
  //
  //  AlbertMarkerVertex
  //
  //*********************************************************************
  AlbertMarkerVector::AlbertMarkerVector ()
  {}

  void AlbertMarkerVector::makeNewSize(int newNumberOfEntries)
  {
    vec_.realloc(newNumberOfEntries);
    for(Array<int>::Iterator it = vec_.begin(); it != vec_.end(); ++it)
      (*it) = -1;
  }

  void AlbertMarkerVector::makeSmaller(int newNumberOfEntries)
  {}

  void AlbertMarkerVector::checkMark(ALBERT EL_INFO * elInfo, int localNum)
  {
    if(vec_[elInfo->el->dof[localNum][0]] == -1)
      vec_[elInfo->el->dof[localNum][0]] = elInfo->el->index;
  }

  bool AlbertMarkerVector::notOnThisElement(ALBERT EL_INFO * elInfo, int localNum)
  {
    return (vec_[numVertex_*elInfo->level +
                 elInfo->el->dof[localNum][0]] != elInfo->el->index);
  }

  template <class Grid>
  void AlbertMarkerVector::markNewVertices(Grid &grid)
  {
    ALBERT MESH *mesh_ = grid.getMesh();
    int nvx = mesh_->n_vertices;
    numVertex_ = nvx;
    int maxlevel = grid.maxlevel();

    if(vec_.size() < maxlevel* nvx)
      makeNewSize(2*maxlevel* nvx);

    for(int level=0; level <= maxlevel; level++)
    {
      typedef typename Grid::Traits<0>::LevelIterator LevelIterator;
      LevelIterator endit = grid.lend<0> (level);
      for(LevelIterator it = grid.lbegin<0> (level); it != endit; ++it)
      {
        for(int local=0; local<Grid::dimension+1; local++)
        {
          int num = it->getElInfo()->el->dof[local][0];
          if( vec_[it->level()* nvx + num] == -1 )
            vec_[it->level()* nvx + num] = it->globalIndex();
        }
      }
      // remember the number of entity on level and codim = 0
    }
  }

  void AlbertMarkerVector::print()
  {
    printf("\nEntries %d \n",vec_.size());
    for(int i=0; i<vec_.size(); i++)
      printf("Konten %d visited on Element %d \n",i,vec_[i]);
  }

  //***********************************************************************
  //
  // --AlbertGrid
  //
  //***********************************************************************
  template < int dim, int dimworld >
  inline AlbertGrid < dim, dimworld >::AlbertGrid(char *MacroTriangFilename)
  {
    mesh_ = ALBERT get_mesh("AlbertGrid", ALBERT initDofAdmin, ALBERT initLeafData);
    ALBERT read_macro(mesh_, MacroTriangFilename, ALBERT initBoundary);

    // we have at least one level, level 0
    maxlevel_ = 0;

    vertexMarker_ = new AlbertMarkerVector ();
    vertexMarker_->markNewVertices(*this);

    markNew();

  }


  template < int dim, int dimworld >
  inline AlbertGrid < dim, dimworld >::~AlbertGrid()
  {
    ALBERT free_mesh(mesh_);
    delete vertexMarker_;
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
    AlbertGridLevelIterator<codim,dim,dimworld> it((*this));
    return it;
  }

  template < int dim, int dimworld >
  inline void AlbertGrid < dim, dimworld >::
  globalRefine(int refCount)
  {
    unsigned char flag;

    // trage auf jedem Element refCount ein
    flag = ALBERT global_refine(mesh_, refCount);

    // verfeinere
    refineLocal(refCount);

    printf("AlbertGrid<%d,%d>::globalRefine: Grid refined, maxlevel = %d \n",
           dim,dimworld,maxlevel_);
  }


  template < int dim, int dimworld >
  inline void AlbertGrid < dim, dimworld >::
  refineLocal(int refCount)
  {
    unsigned char flag;
    flag = ALBERT refine(mesh_);
    maxlevel_ += refCount;

    vertexMarker_->markNewVertices(*this);

    // map the indices
    markNew();

  }

  template < int dim, int dimworld >
  inline void AlbertGrid < dim, dimworld >::
  coarsenLocal()
  {
    unsigned char flag;
    flag = ALBERT coarsen(mesh_);
  }


  template < int dim, int dimworld >
  inline int AlbertGrid < dim, dimworld >::maxlevel()
  {
    return maxlevel_;
  }

  template < int dim, int dimworld >
  inline int AlbertGrid < dim, dimworld >::size (int level, int codim)
  {
    enum { numCodim = dim+1 };

    if(size_[level * (maxlevel_ +1) + codim] == -1)
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
#if DIM > 2
      if(codim == 3)
      {
        AlbertGridLevelIterator<3,dim,dimworld> endit = lend<3>(level);
        for(AlbertGridLevelIterator<3,dim,dimworld> it = lbegin<3>(level);
            it != endit; ++it)
          numberOfElements++;
      }
#endif

      size_[level * numCodim + codim] = numberOfElements;
      return numberOfElements;
    }
    else
    {
      return size_[level * numCodim + codim];
    }
  }

  template < int dim, int dimworld >
  inline void AlbertGrid < dim, dimworld >::writeGrid (int level)
  {
    printf("Not implemented for dim=%d , dimworld=%d \n",dim,dimworld);
    abort();
  }

  inline void AlbertGrid <2,2>::writeGrid (int level)
  {
    std::cout << "\nStarting USPM Grid write! \n";
    // USPM 2d

    enum {dim = 2}; enum {dimworld = 2};
    typedef AlbertGridLevelIterator<0,dim,dimworld> LEVit;

    if(level == -1) level = maxlevel_;
    LEVit endit = lend<0>(level);

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
    for (LEVit it = lbegin<0>(level); it != endit; ++it)
    {
      int elNum = it->index();

      typedef AlbertGridNeighborIterator<dim,dimworld> Neighit;
      Neighit nit = it->nbegin();

      for (int i = 0; i < dim+1; i++)
      {
        int k = it->entity<dim>(i)->index();
        vertex[elNum][i] = k;

        nb[elNum][i] = nit->index();

        Vec<dimworld> vec = (it->geometry())[i];
        for (int j = 0; j < dimworld; j++)
          coord[k][j] = vec(j);

        ++nit;
      }
    }

    // write the USPM Mesh
    FILE *file = fopen("grid.uspm", "w");
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
    std::cout << "\nUSPM grid 'grid.uspm' written !\n\n";

    for (int i = 0; i < nvx; i++)
      delete [] coord[i];
    delete [] coord;

    for (int i = 0; i < noe; i++)
      delete [] nb[i];
    delete [] nb;

    for (int i = 0; i < noe; i++)
      delete [] vertex[i];
    delete [] vertex;


  }

  inline void AlbertGrid<3,3>::writeGrid(int level)
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
    FILE *file = fopen("grid3d.0", "w");
    if(!file)
    {
      std::cout << "Couldnt open grid3d.0 \n";
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

    system("gzip -fq grid3d.0");
    std::cout << "3d Grid written! \n";
  }


  //! Index Mapping
  template < int dim, int dimworld >
  inline void AlbertGrid < dim, dimworld >::
  makeNewSize(Array<int> &a, int newNumberOfEntries)
  {
    a.realloc(newNumberOfEntries);
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
      return levelIndex_[codim][globalIndex];
  }

  // create lookup table for indices of the elements
  template < int dim, int dimworld >
  inline void AlbertGrid < dim, dimworld >::markNew()
  {
    // only for gcc, means notin'
    typedef AlbertGrid < dim ,dimworld > GridType;

    if(mesh_->n_hier_elements > levelIndex_[0].size())
      makeNewSize(levelIndex_[0], mesh_->n_hier_elements);

    if((maxlevel_+1)*(numCodim) > size_.size())
      makeNewSize(size_, 2*((maxlevel_+1)*numCodim));

    //  AlbertGrid<dim,dimworld> &grid = (*this);

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
        levelIndex_[0][no] = num;
        num++;
      }
      // remember the number of entity on level and codim = 0
      size_[level*numCodim + 0] = num;
    };

    if((maxlevel_+1) * mesh_->n_vertices > levelIndex_[dim].size())
      makeNewSize(levelIndex_[dim], ((maxlevel_+1)* mesh_->n_vertices));

    for(int level=0; level <= maxlevel_; level++)
    {
      typedef AlbertGridLevelIterator<dim,dim,dimworld> LevelIterator;
      int num = 0;
      LevelIterator endit = lend<dim>(level);
      for(LevelIterator it = lbegin<dim> (level); it != endit; ++it)
      {
        int no = it->globalIndex();
        levelIndex_[dim][level*mesh_->n_vertices + no] = num;
        num++;
      }
      // remember the number of entity on level and codim = 0
      size_[level*numCodim + dim] = num;
    };
  }


} // end namespace dune
