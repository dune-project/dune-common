// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// Implementation von AlbertGrid
//////////////////////////////////////////////////////////////////////////

#ifdef __ALBERTNAME__
namespace Albert
{
#endif

void initialReached(const EL_INFO * elInfo)
{
  AlbertLeafData *ldata = (AlbertLeafData *) elInfo->el->child[1];
  for (int i = 0; i < N_VERTICES; i++)
  {
    ldata->reachedFace[i] = -1;
  }
}
// mark Faces
void setReached(const EL_INFO * elInfo)
{
  AlbertLeafData *ldata = (AlbertLeafData *) elInfo->el->child[1];
  AlbertLeafData *ldataNeigh = NULL;
  for (int i = 0; i < N_VERTICES; i++)
  {

    // mark Faces
    if(elInfo->neigh[i])
    {
      if(ldata->reachedFace[i] != -1)
      {
        ldata->reachedFace[i] = 1;
        ldataNeigh = (AlbertLeafData *) elInfo->neigh[i]->child[1];
        ldataNeigh->reachedFace[elInfo->opp_vertex[i]] = -1;
      }
      else
      {
        ldataNeigh = (AlbertLeafData *) elInfo->neigh[i]->child[1];
        ldataNeigh->reachedFace[elInfo->opp_vertex[i]] = 1;
      }
    }
    else
      ldata->reachedFace[i] = 1;
  }
}

void AlbertLeafRefine(EL * parent, EL * child[2])
{
  AlbertLeafData *ldata;
  AlbertLeafData *ldataChi;

  ldata = (AlbertLeafData *) parent->child[1];

  // child 0
  ldataChi = (AlbertLeafData *) child[0]->child[1];
  // die neue Kante auf Child 0, siehe Albert Doc S. 11
  for(int i=0; i<N_VERTICES; i++)
    ldataChi->reachedFace[i] = ldata->reachedFace[i];


#if DIM == 3
  ldataChi->reachedFace[0] = 1;
#else
  ldataChi->reachedFace[1] = 1;
#endif

  // child 1
  ldataChi = (AlbertLeafData *) child[1]->child[1];
  for(int i=0; i<N_VERTICES; i++)
    ldataChi->reachedFace[i] = ldata->reachedFace[i];
  // die neue Kante auf Child 1, siehe Albert Doc S. 11

  ldataChi->reachedFace[0] = -1;

}

void AlbertLeafCoarsen(EL * parent, EL * child[2])
{
  // do notin'
  ALBERT_ERROR_EXIT("not implemented until now\n");
}

void initLeafData(LEAF_DATA_INFO * linfo)
{
  linfo->leaf_data_size = sizeof(AlbertLeafData);
  linfo->refine_leaf_data = &AlbertLeafRefine;
  linfo->coarsen_leaf_data = &AlbertLeafCoarsen;

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

  static ALBERT EL_INFO statElInfo;

  //****************************************************************
  //
  // AlbertGridElement
  //
  //****************************************************************
  template< int dim, int dimworld>
  inline AlbertGridElement<dim,dimworld>::
  AlbertGridElement()
  {
    elInfo_ = NULL;
  }

  template< int dim, int dimworld>
  inline ALBERT EL_INFO * AlbertGridElement<dim,dimworld>::
  makeEmptyElInfo()
  {
    // heikel, da man elInfo im Destructor wieder freigeben muss
    ALBERT EL_INFO * elInfo = &statElInfo; //new ALBERT EL_INFO ();

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

  inline AlbertGridElement<3,3>::
  AlbertGridElement()
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

  inline AlbertGridElement<2,2>::
  AlbertGridElement()
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

  inline AlbertGridElement<1,1>::
  AlbertGridElement()
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



  template< int dim, int dimworld>
  inline AlbertGridElement<dim,dimworld>::
  AlbertGridElement(ALBERT EL_INFO *elInfo,
                    unsigned char face, unsigned char edge,
                    unsigned char vertex)
  {
    elInfo_ = elInfo;
    face_ = face;
    edge_ = edge;
    vertex_ = vertex;

    if(elInfo_)
    {
      for(int i=0; i<dim+1; i++)
        for(int j=0; j<dimworld; j++)
          coord_(i) (j) = elInfo_->coord[i][j];
    }
  }
  template< int dim, int dimworld>
  inline void AlbertGridElement<dim,dimworld>::
  initGeom()
  {
    elInfo_ = NULL;
    face_ = 0;
    edge_ = 0;
    vertex_ = 0;
  }

  template< int dim, int dimworld>
  inline void AlbertGridElement<dim,dimworld>::
  builtGeom(ALBERT EL_INFO *elInfo, unsigned char face,
            unsigned char edge, unsigned char vertex)
  {
    elInfo_ = elInfo;
    face_ = face;
    edge_ = edge;
    vertex_ = vertex;

    if(elInfo_)
    {
      for(int i=0; i<dim+1; i++)
        for(int j=0; j<dimworld; j++)
          coord_(i) (j) = elInfo_->coord[i][j];
    }
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
    return coord_(mapVertices<dimworld-dim>(i) % corners());
  }

  inline Vec<3,albertCtype>& AlbertGridElement<3,3>::
  operator [](int i)
  {
    return coord_(i);
  }

  inline Vec<2,albertCtype>& AlbertGridElement<2,2>::
  operator [](int i)
  {
    return coord_(i);
  }

  inline Vec<1,albertCtype>& AlbertGridElement<1,1>::
  operator [](int i)
  {
    return coord_(i);
  }

  /// specialization for codim = dim
  inline Vec<3,albertCtype>& AlbertGridElement<0,3>::
  operator [](int i)
  {
    return coord_(vertex_);
  }

  /// specialization for codim = dim
  inline Vec<2,albertCtype>& AlbertGridElement<0,2>::
  operator [](int i)
  {
    return coord_(vertex_);
  }

  /// specialization for codim = dim
  inline Vec<1,albertCtype>& AlbertGridElement<0,1>::
  operator [](int i)
  {
    return coord_(vertex_);
  }


  template< int dim, int dimworld>
  inline AlbertGridElement<dim,dim> AlbertGridElement<dim,dimworld>::
  refelem()
  {
    AlbertGridElement<dim,dim> refEl;
    return refEl;
  }

  template< int dim, int dimworld>
  inline Vec<dimworld,albertCtype>& AlbertGridElement<dim,dimworld>::
  global(Vec<dim> local)
  {
    std::cout << "global not implemented yet! \n";
    Vec<dimworld,albertCtype> tmp(0.0);
    return tmp;
  }

  template< int dim, int dimworld>
  inline Vec<dimworld> AlbertGridElement<dim,dimworld>::
  globalBary(Vec<dim+1> local)
  {
    static ALBERT REAL_D world;
    ALBERT REAL *v = NULL;
    ALBERT REAL c;
    Vec<dimworld> ret(0.0);

    v = (ALBERT REAL *) elInfo_->coord[0];
    c = local(0);
    for (int j = 0; j < dimworld; j++)
      ret(j) = c * v[j];

    for (int i = 1; i < dim+1; i++)
    {
      v = (ALBERT REAL *) elInfo_->coord[i];
      c = local(i);
      for (int j = 0; j < dimworld; j++)
        ret(j) += c * v[j];
    }

    return ret;
  }

  template< int dim, int dimworld>
  inline Vec<dim>& AlbertGridElement<dim,dimworld>::
  local(Vec<dimworld> global)
  {
    std::cout << "local not implemented yet! \n";
    Vec<dim,albertCtype> tmp (0.0);
    return tmp;
  }

  template< int dim, int dimworld>
  inline Vec<dim+1> AlbertGridElement<dim,dimworld>::
  localBary(Vec<dimworld> global)
  {
    //  realWorldToCoord2D(VERTEXtype vertex,REAL * xy, REAL lambda[dim + 1])
    ALBERT REAL edge[dim][dimworld], x[dimworld];
    ALBERT REAL x0, det, det0, det1, lmin;
    int j, k;
    ALBERT REAL_D xy;
    ALBERT REAL lambda[dim+1];

    for(int i=0; i<dimworld; i++)
      xy[i] = global(i);
    /*
     * wir haben das gleichungssystem zu loesen:
     */
    /*
     * ( q1x q2x ) (lambda1) (qx)
     */
    /*
     * ( q1y q2y ) (lambda2) = (qy)
     */
    /*
     * mit qi=pi-p3, q=xy-p3
     */

    for (int j = 0; j < dimworld; j++)
    {
      x0 = elInfo_->coord[dim][j];
      x[j] = xy[j] - x0;
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

    lambda[0] = det0 / det;
    lambda[1] = det1 / det;
    lambda[2] = 1.0 - lambda[0] - lambda[1];

    k = -1;
    lmin = 0.0;
    j = 0;
    for (int i = 0; i <= dim; i++)
    {
      if(lambda[i] < -1.E-5)
      {
        if(lambda[i] < lmin)
        {
          k = i;
          lmin = lambda[i];
        }
        j++;
      }
    }

    Vec<dim+1> lam(lambda);
    return lam;
  }


  //template< int dim, int dimworld>
  inline Vec<4> AlbertGridElement<3,3>::
  localBary(Vec<3> global)
  {
    enum { dim = 3};
    enum { dimworld = 3};

    //  realWorldToCoord3D(VERTEXtype vertex,REAL * xy, REAL lambda[dim + 1])
    ALBERT REAL edge[dim][dimworld], x[dimworld];
    ALBERT REAL x0, det, det0, det1, det2, lmin;
    ALBERT REAL_D xy;
    ALBERT REAL lambda[dim+1];
    int j, k;

    for(int i=0; i<dimworld; i++)
      xy[i] = global(i);


    //! wir haben das gleichungssystem zu loesen:
    //! ( q1x q2x q3x) (lambda1) (qx)
    //! ( q1y q2y q3y) (lambda2) = (qy)
    //! ( q1z q2z q3z) (lambda3) (qz)
    //! mit qi=pi-p3, q=xy-p3

    for (int j = 0; j < dimworld; j++)
    {
      x0 = elInfo_->coord[dim][j];
      x[j] = xy[j] - x0;
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

    lambda[0] = det0 / det;
    lambda[1] = det1 / det;
    lambda[2] = det2 / det;
    lambda[3] = 1.0 - lambda[0] - lambda[1] - lambda[2];

    k = -1;
    lmin = 0.0;
    j = 0;
    for (int i = 0; i <= dim; i++)
    {
      if(lambda[i] < -1.E-5)
      {
        if(lambda[i] < lmin)
        {
          k = i;
          lmin = lambda[i];
        }
        j++;
      }
    }
    Vec<dim+1> lam(lambda);

    return lam;
  }

  //***********************************************************************
  //
  // --- AlbertGrid
  //
  //***********************************************************************
  template < int dim, int dimworld >
  inline AlbertGrid < dim, dimworld >::AlbertGrid(char *MacroTriangFilename)
  {
    mesh_ = ALBERT get_mesh("AlbertGrid", NULL, ALBERT initLeafData); // initDofAdmin,initLeafData);
    ALBERT read_macro(mesh_, MacroTriangFilename, ALBERT initBoundary);

    ALBERT mesh_traverse(mesh_, -1, CALL_LEAF_EL | FILL_NOTHING,
                         ALBERT initialReached);

    ALBERT mesh_traverse(mesh_, -1, CALL_LEAF_EL | FILL_NEIGH,
                         ALBERT setReached);

    maxlevel_ = 0;

    vertexMarker_ = new AlbertMarkerVector ();
    vertexMarker_->markNewVertices(mesh_);
  }

  template < int dim, int dimworld >
  inline void AlbertGrid < dim, dimworld >::
  globalRefine(int refCount)
  {
    unsigned char flag;

    // trage auf jedem Element refCount ein
    flag = ALBERT global_refine(mesh_, refCount);

    // verfeinere
    refineLocal();

    maxlevel_ += refCount;
    printf("AlbertGrid<%d,%d>::globalRefine: Grid refined, maxlevel = %d \n",
           dim,dimworld,maxlevel_);
  }


  template < int dim, int dimworld >
  inline void AlbertGrid < dim, dimworld >::
  refineLocal()
  {
    unsigned char flag;
    flag = ALBERT refine(mesh_);

    vertexMarker_->markNewVertices(mesh_);

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
  inline int AlbertGrid < dim, dimworld >::numberVertices ()
  {
    return mesh_->n_vertices;
  }

  template < int dim, int dimworld >
  inline int AlbertGrid < dim, dimworld >::size (int level, int codim)
  {
    if((level != -1) || (codim != 0))
    {
      std::cout << "AlbertGrid::size \n";
      std::cout << "Richtige Implementierung fehlt noch\n";
    }
    return mesh_->n_elements;
  }

  template < int dim, int dimworld >
  inline int AlbertGrid < dim, dimworld >::hiersize (int level, int codim)
  {
    if((level != -1) || (codim != 0))
    {
      std::cout << "AlbertGrid::size \n";
      std::cout << "Richtige Implementierung fehlt noch\n";
    }
    return mesh_->n_hier_elements;
  }

  template < int dim, int dimworld >
  inline void AlbertGrid < dim, dimworld >::writeGrid ()
  {
    printf("Not implemented for dim=%d , dimworld=%d \n",dim,dimworld);
    abort();
  }

  inline void AlbertGrid <2,2>::writeGrid ()
  {
    cout << "\nStarting USPM Grid write! \n";
    // / USPM 2d

    enum {dim = 2}; enum {dimworld = 2};
    typedef AlbertGridLevelIterator<0,dim,dimworld> LEVit;

    double **coord = new double *[mesh_->n_vertices];
    for (int i = 0; i < mesh_->n_vertices; i++)
      coord[i] = new double[dimworld];
    cout << "Number of Vertices " << mesh_->n_vertices << endl;

    LEVit endit = lend<0>(-1);

    // / die eigentlichen ElementNummer auf die Zahlen 0 bis
    // n_elements-1
    // / abbilden
    int *lookup = new int[mesh_->n_hier_elements];
    int count = 0;
    for (LEVit it = lbegin<0>(-1); it != endit; ++it)
    {
      lookup[(*it).index()] = count;
      count++;
    }

    int **nb = new int *[mesh_->n_elements];
    for (int i = 0; i < mesh_->n_elements; i++)
      nb[i] = new int[dim + 1];

    int **vertex = new int *[mesh_->n_elements];
    for (int i = 0; i < mesh_->n_elements; i++)
      vertex[i] = new int[dim + 1];

    // / setup the USPM Mesh
    for (LEVit it = lbegin<0>(-1); it != endit; ++it)
    {
      int elNum = lookup[it->index()];

      typedef AlbertGridNeighborIterator<dim,dimworld> Neighit;
      Neighit nit = (*it).nbegin();

      for (int i = 0; i < dim+1; i++)
      {
        ALBERT EL_INFO * elInfo = it->getElInfo();

        int k = elInfo->el->dof[i][0];
        vertex[elNum][i] = k;

        if(nit->getElInfo())
          nb[elNum][i] = lookup[nit->index()];
        else
          nb[elNum][i] = -1;

        Vec<dimworld> vec = (it->geometry())[i];
        for (int j = 0; j < dimworld; j++)
          coord[k][j] = vec(j);

        ++nit;
      }
    }

    // / write the USPM Mesh
    FILE *file = fopen("out.uspm", "w");
    if(!file)
    {
      cout << "Couldnt open out.uspm \n";
      abort();
    }
    fprintf(file, "USPM 2\n");
    fprintf(file, "%d %d \n", N_VERTICES, N_VERTICES * mesh_->n_elements);
    fprintf(file, "%d %d 0\n", mesh_->n_elements, mesh_->n_vertices);

    for (int i = 0; i < mesh_->n_vertices; i++)
    {
      fprintf(file, "%d ", i);
      for (int j = 0; j < dimworld; j++)
        fprintf(file, "%le ", coord[i][j]);
      fprintf(file, "\n");
    }

    for (int i = 0; i < mesh_->n_elements; i++)
    {
      fprintf(file, "%d ", i);
      for (int j = 0; j < dim + 1; j++)
        fprintf(file, "%d ", vertex[i][j]);
      for (int j = 0; j < dim + 1; j++)
        fprintf(file, "%d ", nb[i][j]);
      fprintf(file, "\n");
    }

    fclose(file);
    std::cout << "\nUSPM grid 'out.uspm' writen !\n\n";

    for (int i = 0; i < mesh_->n_vertices; i++)
      delete [] coord[i];
    delete [] coord;

    delete [] lookup;

    for (int i = 0; i < mesh_->n_elements; i++)
      delete [] nb[i];
    delete [] nb;

    for (int i = 0; i < mesh_->n_elements; i++)
      delete [] vertex[i];
    delete [] vertex;


  }

  inline void AlbertGrid<3,3>::writeGrid()
  {
    cout << "\nStarting 3d Grid write\n";

    enum {dim = 3}; enum {dimworld = 3};
    typedef AlbertGridLevelIterator<0,dim,dimworld> LEVit;

    double **coord = new double *[mesh_->n_vertices];
    for (int i = 0; i < mesh_->n_vertices; i++)
      coord[i] = new double[dimworld];

    LEVit endit = lend<0>(-1);
    // / die eigentlichen ElementNummer auf die Zahlen 0 bis
    // n_elements-1
    // / abbilden
    int *lookup = new int[mesh_->n_hier_elements];
    int count = 0;
    for (LEVit it  = lbegin<0>(-1); it != endit; ++it)
    {
      lookup[(*it).index()] = count;
      count++;
    }

    int **vertex = new int *[mesh_->n_elements];
    for (int i = 0; i < mesh_->n_elements; i++)
      vertex[i] = new int[dim + 1];

    // / setup the Wesenber 3d Grid
    for (LEVit it = lbegin<0>(-1); it != endit; ++it)
    {
      int elNum = lookup[(*it).index()];

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
    FILE *file = fopen("test3dOUT.0", "w");
    if(!file)
    {
      cout << "Couldnt open test3dOUT.0 \n";
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

    delete [] lookup;

    for (int i = 0; i < mesh_->n_elements; i++)
      delete [] vertex[i];
    delete [] vertex;

    system("gzip -fq test3dOUT.0");
    cout << "3d Grid written! \n";
  }



  //*******************************************************
  //
  // --AlbertGridLevelIterator
  //
  //*******************************************************
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
    virtualEntity_.setElInfo(NULL,0,0,0);
  }

  template<int codim, int dim, int dimworld>
  inline AlbertGridLevelIterator<codim,dim,dimworld >::
  AlbertGridLevelIterator()
  {
    makeIterator();
  }

  // Make LevelIterator with point to element from previous iterations
  template<int codim, int dim, int dimworld>
  inline AlbertGridLevelIterator<codim,dim,dimworld >::
  AlbertGridLevelIterator(ALBERT EL_INFO *elInfo)
  {
    if(elInfo)
    {
      vertex_ = 0;
      face_ = 0;
      edge_ = 0;

      vertexMarker_ = NULL;

      manageStack_.init();

      virtualEntity_.setTraverseStack(NULL);

      // diese Methode muss neu geschrieben werden, da man
      // die ParentElement explizit speichern moechte.

      virtualEntity_.setElInfo(elInfo,face_,edge_,vertex_);
    }
    else
    {
      std::cout << "AlbertGridLevelIterator: No Father given \n" << std::endl;
      abort();
    }
  }

  template<int codim, int dim, int dimworld>
  inline AlbertGridLevelIterator<codim,dim,dimworld >::
  AlbertGridLevelIterator(ALBERT MESH * mesh, AlbertMarkerVector * vertexMark,
                          int travLevel)
  {
    if(mesh)
    {
      vertex_ = 0;
      face_ = 0;
      edge_ = 0;

      vertexMarker_ = vertexMark;

      ALBERT FLAGS travFlags = FILL_COORDS | FILL_NEIGH;
      if(travLevel >= 0)
        travFlags = travFlags | CALL_LEAF_EL_LEVEL;
      else
        travFlags = travFlags | CALL_LEAF_EL;

      // get traverse_stack
      manageStack_.makeItNew(true);

      virtualEntity_.setTraverseStack(manageStack_.getStack());

      // diese Methode muss neu geschrieben werden, da man
      // die ParentElement explizit speichern moechte.
      ALBERT EL_INFO* elInfo =
        goFirstElement(manageStack_.getStack(), mesh, travLevel,travFlags);

      virtualEntity_.setElInfo(elInfo,face_,edge_,vertex_);
    }
    else
      makeIterator();

  };


  template<int codim, int dim, int dimworld>
  inline AlbertGridLevelIterator<codim,dim,dimworld >::
  AlbertGridLevelIterator(const AlbertGridLevelIterator<codim,dim,dimworld > &I)
  {
    manageStack_ = I.manageStack_;
    virtualEntity_ = I.virtualEntity_;
  }

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

    virtualEntity_.setElInfo(
      goNextEntity<codim>(manageStack_.getStack(), virtualEntity_.getElInfo()),
      face_,edge_,vertex_);

    return (*this);
  }

  template<int codim, int dim, int dimworld>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<codim,dim,dimworld >::
  goNextFace(ALBERT TRAVERSE_STACK *stack, ALBERT EL_INFO *elInfo)
  {
    ALBERT AlbertLeafData * ldata = NULL;

    // go next Element, if face_ > numberOfVertices, then go to next elInfo
    face_++;
    if(face_ >= (dim+1)) // dim+1 Faces
    {
      elInfo = goNextElInfo(stack, elInfo);
      face_ = 0;
    }

    if(!elInfo)
      return elInfo; // if no more Faces, return

    // on leaf level, child[1] hides the leaf data pointer
    if(elInfo->el->child[1])
    {
      ldata = (ALBERT AlbertLeafData *) elInfo->el->child[1];

      // if reachedFace before, go next
      if(ldata->reachedFace[face_] != 1)
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
    ALBERT AlbertLeafData * ldata = NULL;
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
      goNextEntity<codim>(&travStack_, virtualEntity_.getElInfo());
    for(int i=1; i<= steps; i++)
      elInfo = goNextEntity<codim>(&travStack_,virtualEntity_.getElInfo());

    virtualEntity_.setElInfo(elInfo,face_,edge_,vertex_);

    return (*this);
  }

  template<int codim, int dim, int dimworld>
  inline typename AlbertGridEntity< codim,dim,dimworld >&
  AlbertGridLevelIterator< codim,dim,dimworld >::operator *()
  {
    return virtualEntity_;
  }

  template<int codim, int dim, int dimworld>
  inline typename AlbertGridEntity< codim,dim,dimworld >*
  AlbertGridLevelIterator< codim,dim,dimworld >::operator ->()
  {
    return &virtualEntity_;
  }


  template<int codim, int dim, int dimworld>
  inline ALBERT EL_INFO * AlbertGridLevelIterator<codim,dim,dimworld >::
  goFirstElement(ALBERT TRAVERSE_STACK *stack,ALBERT MESH *mesh, int level, ALBERT FLAGS fill_flag)
  {
    FUNCNAME("goFirstElement");
    int i;

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

    for (i=0; i<stack->stack_size; i++)
      stack->elinfo_stack[i].fill_flag = fill_flag & FILL_ANY;

    stack->elinfo_stack[0].mesh = stack->elinfo_stack[1].mesh = mesh;

    if (fill_flag & CALL_LEAF_EL_LEVEL) {
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
    ALBERT EL_INFO       *elinfo=nil;

    if (stack->stack_used)
    {
      ALBERT_TEST_EXIT(elinfo_old == stack->elinfo_stack+stack->stack_used)
        ("invalid old elinfo\n");
    }
    else
    {
      ALBERT_TEST_EXIT(elinfo_old == nil) ("invalid old elinfo != nil\n");
    }

    if (stack->traverse_fill_flag & CALL_LEAF_EL_LEVEL)
    {
      // overloaded traverse_leaf_el_level, is not implemened in ALBERT yet
      elinfo = traverseLeafElLevel(stack);
      if (elinfo)
        stack->el_count++;
      else {
        /* MSG("total element count:%d\n",stack->el_count); */
      }
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

    return(stack->elinfo_stack+stack->stack_used);
  }



  //*************************************************************************
  //
  //  AlbertGridEntity
  //
  //*************************************************************************
  template< int dim, int dimworld>
  inline void AlbertGridEntity < 0, dim ,dimworld >::
  makeDescription()
  {
    elInfo_ = NULL;
    geo_.initGeom();
  }

  /// The Element is prescribed by the EL_INFO struct of ALBERT MESH
  /// the pointer to this struct is set and get by setElInfo and
  /// getElInfo.
  template<int codim, int dim, int dimworld>
  inline void AlbertGridEntity < codim, dim ,dimworld >::
  makeDescription()
  {
    elInfo_ = NULL;
    geo_.initGeom();
  }

  template<int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >::
  AlbertGridEntity(ALBERT TRAVERSE_STACK * travStack)
  {
    travStack_ = travStack;
    makeDescription();
  }

  template<int codim, int dim, int dimworld>
  inline AlbertGridEntity < codim, dim ,dimworld >::
  AlbertGridEntity(ALBERT TRAVERSE_STACK * travStack)
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

  template<int dim, int dimworld>
  inline void AlbertGridEntity < 0, dim ,dimworld >::
  setTraverseStack(ALBERT TRAVERSE_STACK * travStack)
  {
    travStack_ = travStack;
  }


  template<int codim, int dim, int dimworld>
  inline AlbertGridEntity < codim, dim ,dimworld >::
  AlbertGridEntity()
  {
    travStack_ = NULL;
    makeDescription();
  }


  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >::
  AlbertGridEntity()
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

  template<int dim, int dimworld>
  inline ALBERT EL_INFO* AlbertGridEntity < 0 , dim ,dimworld >::
  getElInfo() const
  {
    return elInfo_;
  }

  template<int codim, int dim, int dimworld>
  inline void AlbertGridEntity < codim, dim ,dimworld >::
  setElInfo(ALBERT EL_INFO * elInfo, unsigned char face,
            unsigned char edge, unsigned char vertex )
  {
    face_ = face;
    edge_ = edge;
    vertex_ = vertex;
    elInfo_ = elInfo;
    geo_.builtGeom(elInfo_,face,edge,vertex);
  }

  template<int codim, int dim, int dimworld>
  inline int AlbertGridEntity < codim, dim ,dimworld >::
  level()
  {
    return elInfo_->level;
  }


  //*********************************************************************
  //  default Implementation
  template<int codim, int dim, int dimworld>
  inline int AlbertGridEntity < codim, dim ,dimworld >::
  index()
  {
    return indexMap<codim>();
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
    return elInfo_->el->index;
  }

  template<int dim, int dimworld>
  inline bool AlbertGridEntity < 0, dim ,dimworld >::
  pointInEntity(const Vec<dimworld> &point)
  {
    AlbertGridElement<dim,dimworld> el = geometry();

    Vec<dim+1> localCoords = el.localBary(point);

    bool ret=true;
    for(int i=0; i<dim+1; i++)
      if(localCoords(i) < 0.0) ret = false;

    return ret;
  }

  template< int dim, int dimworld>
  inline void AlbertGridEntity < 0, dim ,dimworld >::
  setElInfo(ALBERT EL_INFO * elInfo,  unsigned char face,
            unsigned char edge, unsigned char vertex )
  {
    // in this case the face, edge and vertex information is not used,
    // because we are in the element case
    elInfo_ = elInfo;
    geo_.builtGeom(elInfo_,face,edge,vertex);
  }

  template< int dim, int dimworld>
  inline AlbertGridElement<dim,dimworld>&
  AlbertGridEntity < 0, dim ,dimworld >::geometry()
  {
    return geo_;
  }

  template< int codim, int dim, int dimworld>
  inline AlbertGridElement<dim-codim,dimworld>&
  AlbertGridEntity < codim, dim ,dimworld >::geometry()
  {
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
  inline AlbertGridElement<dim,dim>
  AlbertGridEntity < 0, dim ,dimworld >::father_relative_local()
  {
    AlbertGridElement<dim,dim> el(elInfo_);
    return el;
  }

  template< int dim, int dimworld> template<int cc>
  inline int AlbertGridEntity < 0, dim ,dimworld >::count()
  {
    std::cout << "count not implemented yet! \n";
    return -1;
  }

  template<int dim, int dimworld> template<int cc>
  inline AlbertGridLevelIterator<cc,dim,dimworld>
  AlbertGridEntity<0,dim,dimworld>::entity (int i)
  {
    std::cout << "entity not implemented yet! \n";
    AlbertGridLevelIterator<cc,dim,dimworld> tmp;
    return tmp;
  }
  //***************************************************************
  //
  //  AlbertGridEntity Hierarchic Iterator
  //
  //***************************************************************
  template< int dim, int dimworld>
  inline void AlbertGridHierarchicIterator<dim,dimworld>::
  makeIterator()
  {
    initTraverseStack(&travStack_);
    virtualEntity_.setTraverseStack(NULL);
    virtualEntity_.setElInfo(NULL,0,0,0);
  }


  template< int dim, int dimworld>
  inline AlbertGridHierarchicIterator<dim,dimworld>::
  AlbertGridHierarchicIterator()
  {
    makeIterator();
  }


  template< int dim, int dimworld>
  inline AlbertGridHierarchicIterator<dim,dimworld>::
  AlbertGridHierarchicIterator(ALBERT TRAVERSE_STACK travStack,int travLevel)
  {
    travStack_ = travStack;
    // default Einstellungen fuer den TraverseStack, siehe
    // traverse_first, traverse_nr_common.cc
    travStack_.traverse_level = travLevel;

    virtualEntity_.setTraverseStack(&travStack_);
    // Hier kann ein beliebiges Element uebergeben werden,
    // da jedes AlbertElement einen Zeiger auf das Macroelement
    // enthaelt.
    virtualEntity_.setElInfo(recursiveTraverse(&travStack_));
  }

  template< int dim, int dimworld>
  inline AlbertGridHierarchicIterator<dim,dimworld>::
  AlbertGridHierarchicIterator(const AlbertGridHierarchicIterator& I)
  {
    travStack_ = I.travStack_;
    virtualEntity_ = I.virtualEntity_;
  }


  template< int dim, int dimworld>
  inline AlbertGridHierarchicIterator<dim,dimworld>&
  AlbertGridHierarchicIterator< dim,dimworld >::operator ++()
  {
    // die 0 ist wichtig, weil Face 0, heist hier jetzt Element
    virtualEntity_.setElInfo(recursiveTraverse(&travStack_));
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
    // siehe die Funktion
    // static EL_INFO *traverse_leaf_el(TRAVERSE_STACK *stack)
    // Common/traverse_nr_common.cc Zeile 392
    ALBERT EL * el=NULL;
    int i=0;

    if(stack->stack_used == 0)
    { /* first call */
      if(stack->traverse_mel == NULL)
        return (NULL);

      stack->stack_used = 1;
      fill_macro_info(stack->traverse_mel,
                      stack->elinfo_stack + stack->stack_used);
      stack->info_stack[stack->stack_used] = 0;

      el = stack->elinfo_stack[stack->stack_used].el;
      if((el == NULL) || (el->child[0] == nil))
      {
        return (stack->elinfo_stack + stack->stack_used);
      }
    }
    else
    {
      el = stack->elinfo_stack[stack->stack_used].el;

      /*
       * go up in tree until we can go down again
       */
      while((stack->stack_used > 0) &&
            ((stack->info_stack[stack->stack_used] >= 2) ||
             (el->child[0] == NULL)))
      {
        stack->stack_used--;
        el = stack->elinfo_stack[stack->stack_used].el;
      }

      /*
       * goto next macro element is done by LevelIterator
       */
      if(stack->stack_used < 1)
        return NULL;
    }

    /*
     * go down next child
     */
    if(el->child[0])
    {
      if(stack->stack_used >= stack->stack_size - 1)
        ALBERT enlargeTraverseStack(stack);

      i = stack->info_stack[stack->stack_used];
      el = el->child[i];
      stack->info_stack[stack->stack_used]++;
      ALBERT fill_elinfo(i, stack->elinfo_stack + stack->stack_used,
                         stack->elinfo_stack + (stack->stack_used + 1));
      stack->stack_used++;

      stack->info_stack[stack->stack_used] = 0;
    }

    return (stack->elinfo_stack + stack->stack_used);
  } // recursive traverse over all childs



  //***************************************************************
  //
  //  --AlbertGridNeighborIterator
  //
  //***************************************************************
  template< int dim, int dimworld>
  inline void AlbertGridNeighborIterator<dim,dimworld>::
  makeIterator()
  {
    neighborCount_ = dim+1; // mehr als dim+1 Nachbarn gibt es nicht
    // zumindest nicht in Albert

    travStack_ = NULL;

    initElInfo(&neighElInfo_);

    elInfo_ = NULL;
    virtualEntity_.setTraverseStack(NULL);
    virtualEntity_.setElInfo(NULL,0,0,0);
  }
  template< int dim, int dimworld>
  inline void AlbertGridNeighborIterator<dim,dimworld>::
  initElInfo(ALBERT EL_INFO * elInfo)
  {
    // initialisiert elinfo mit default Werten
    elInfo->mesh = NULL;
    elInfo->el = NULL;
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
    }
  }

  template< int dim, int dimworld>
  inline AlbertGridNeighborIterator<dim,dimworld>::AlbertGridNeighborIterator()
  {
    makeIterator();
  }



  template< int dim, int dimworld>
  inline AlbertGridNeighborIterator<dim,dimworld>::AlbertGridNeighborIterator
    (ALBERT TRAVERSE_STACK * travStack, ALBERT EL_INFO *elInfo)
  {
    if(!(elInfo->el->child[0]))
    {
      elInfo_ = elInfo;
      neighborCount_ = 0;

      travStack_ = travStack;

      virtualEntity_.setTraverseStack(travStack_);

      if(elInfo_->neigh[neighborCount_] != NULL)
      {
        int oldEdge = elInfo_->opp_vertex[neighborCount_];
        elInfo_ = traverse_neighbour(travStack_,elInfo_,neighborCount_);

        neighElInfo_ = (*elInfo_); // Macht eine Kopie von elInfo

        // traversiere zurueck, damit der Stack wieder am Ausgangselement
        // steht, wird sich wolh noch aendern, zu Zeitaufwendig
        elInfo_ = traverse_neighbour(travStack_,elInfo_,oldEdge);

        virtualEntity_.setElInfo(&neighElInfo_);
      }
      else
        virtualEntity_.setElInfo(elInfo_);
    }
    else
    {
      std::cout << "Sorry, can make NeighborIterator only on Leaf Elements! \n\n";
      makeIterator();
    }
  }


  template< int dim, int dimworld>
  inline AlbertGridNeighborIterator<dim,dimworld>::
  AlbertGridNeighborIterator(const AlbertGridNeighborIterator& I)
  {
    neighborCount_ = I.neighborCount_;
    elInfo_ = I.elInfo_;
    virtualEntity_ = I.virtualEntity_;
  }


  template< int dim, int dimworld>
  inline AlbertGridNeighborIterator<dim,dimworld>&
  AlbertGridNeighborIterator<dim,dimworld>::
  operator ++()
  {
    // Gehe zum naechsten existierenden Nachbarn
    neighborCount_++;
    // koennte noch geschickter sein, nicht dim+1, sondern corners oder so
    if(neighborCount_ < dim+1)
    {
      if(elInfo_->neigh[neighborCount_] == NULL)
      {
        // Falls kein Nachbar existiert, dann wird die aktuelle Entity
        // zurueckgegeben
        virtualEntity_.setElInfo(elInfo_);
        return (*this);
      }

      // Merke, ueber welche Kante man zurueck kommt
      int helpEdge = elInfo_->opp_vertex[neighborCount_];

      // laufe zum Nachbarn
      elInfo_ = traverse_neighbour(travStack_,elInfo_,neighborCount_);

      // kopiere Nachbar
      neighElInfo_ = (*elInfo_);
      virtualEntity_.setElInfo(&neighElInfo_);

      // Laufe zurueck zum Element ueber gemerkte Kante
      elInfo_ = traverse_neighbour(travStack_,elInfo_,helpEdge);
    }
    else
      virtualEntity_.setElInfo(NULL);
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
    return !((*this) == I);
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >&
  AlbertGridNeighborIterator<dim,dimworld>::
  operator *()
  {
    return virtualEntity_;
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >*
  AlbertGridNeighborIterator<dim,dimworld>::
  operator ->()
  {
    return &virtualEntity_;
  }

  template< int dim, int dimworld>
  inline AlbertGridElement< dim-1, dim >&
  AlbertGridNeighborIterator<dim,dimworld>::
  intersection_self_local()
  {
    std::cout << "intersection_self_local not implemented yet! \n";
    AlbertGridElement<dim-1, dim > tmp;
    return tmp;
  }

  template< int dim, int dimworld>
  inline AlbertGridElement< dim-1, dimworld >&
  AlbertGridNeighborIterator<dim,dimworld>::
  intersection_self_global()
  {
    std::cout << "intersection_self_global not implemented yet! \n";
    AlbertGridElement<dim-1, dim > tmp;
    return tmp;
  }

  template< int dim, int dimworld>
  inline AlbertGridElement< dim-1, dim >&
  AlbertGridNeighborIterator<dim,dimworld>::
  intersection_neighbor_local()
  {
    std::cout << "intersection_neighbor_local not implemented yet! \n";
    AlbertGridElement<dim-1, dim > tmp;
    return tmp;
  }

  template< int dim, int dimworld>
  inline AlbertGridElement< dim-1, dimworld >&
  AlbertGridNeighborIterator<dim,dimworld>::
  intersection_neighbor_global()
  {
    std::cout << "intersection_neighbor_global not implemented yet! \n";
    AlbertGridElement<dim-1, dim > tmp;
    return tmp;
  }

  template< int dim, int dimworld>
  inline int AlbertGridNeighborIterator<dim,dimworld>::
  number_in_self ()
  {
    std::cout << "number_in_self not implemented yet! \n";
    return -1;
  }

  template< int dim, int dimworld>
  inline int AlbertGridNeighborIterator<dim,dimworld>::
  number_in_neighbor ()
  {
    std::cout << "number_in_self not implemented yet! \n";
    return -1;
  }

  //////////////////////////////////////////////////////////////////////////




  template< int dim, int dimworld>
  inline AlbertGridHierarchicIterator<dim,dimworld>
  AlbertGridEntity < 0, dim ,dimworld >::hbegin(int maxlevel)
  {
    // Kopiere alle Eintraege des stack, da man im Stack weiterlaeuft und
    // sich deshalb die Werte anedern koennen, der elinfo_stack bleibt jedoch
    // der gleiche, deshalb kann man auch nur nach unten, d.h. zu den Kindern
    // laufen
    AlbertGridHierarchicIterator<dim,dimworld> it((*travStack_),maxlevel);
    return it;
  }

  template< int dim, int dimworld>
  inline AlbertGridHierarchicIterator<dim,dimworld>
  AlbertGridEntity < 0, dim ,dimworld >::hend(int maxlevel)
  {
    AlbertGridHierarchicIterator<dim,dimworld> it;
    return it;
  }



  template< int dim, int dimworld>
  inline AlbertGridNeighborIterator<dim,dimworld>
  AlbertGridEntity < 0, dim ,dimworld >::nbegin()
  {
    AlbertGridNeighborIterator<dim,dimworld> it(travStack_,this->getElInfo());
    return it;
  }

  template< int dim, int dimworld>
  inline AlbertGridNeighborIterator<dim,dimworld>
  AlbertGridEntity < 0, dim ,dimworld >::nend()
  {
    AlbertGridNeighborIterator<dim,dimworld> it;
    return it;
  }

  //*********************************************************************
  //
  //  AlbertMarkerVertex
  //
  //*********************************************************************
  AlbertMarkerVector::AlbertMarkerVector ()
  {
    vec_ = NULL;
    numberOfEntries_ = 0;
  }

  AlbertMarkerVector::~AlbertMarkerVector ()
  {
    delete[] vec_;
  }

  void AlbertMarkerVector::makeNewSize(int newNumberOfEntries)
  {
    delete[] vec_;

    vec_ = new int [newNumberOfEntries];

    for(int i=0; i<newNumberOfEntries; i++)
      vec_[i] = -1;

    numberOfEntries_ = newNumberOfEntries;
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
    return (vec_[elInfo->el->dof[localNum][0]] != elInfo->el->index);
  }

  void AlbertMarkerVector::markNewVertices(ALBERT MESH * mesh)
  {
    makeNewSize(mesh->n_vertices);

    ALBERT FLAGS travFlags = FILL_NOTHING | CALL_LEAF_EL;

    // get traverse_stack
    //ALBERT TRAVERSE_STACK * travStack = ALBERT get_traverse_stack();
    ALBERT TRAVERSE_STACK travStack;
    initTraverseStack(&travStack);

    // diese Methode muss neu geschrieben werden, da man
    // die ParentElement explizit speichern moechte.
    ALBERT EL_INFO* elInfo =
      ALBERT traverse_first(&travStack, mesh, -1,travFlags);

    while(elInfo)
    {
      for(int i=0; i<N_VERTICES; i++)
        checkMark(elInfo,i);
      elInfo = ALBERT traverse_next(&travStack,elInfo);
    }

  }

  void AlbertMarkerVector::print()
  {
    printf("\nEntries %d \n",numberOfEntries_);
    for(int i=0; i<numberOfEntries_; i++)
      printf("Konten %d visited on Element %d \n",i,vec_[i]);
  }

} // end namespace dune
