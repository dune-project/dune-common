// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// Implementation von AlbertGrid
//////////////////////////////////////////////////////////////////////////

#ifdef __ALBERTNAME__
namespace Albert
{
#endif

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

    // mark Vertices
    if(ldata->reachedVertex[i] != -1)
      ldata->reachedVertex[i] = 1;

    if(elInfo->neigh[i])
    {
      if(elInfo->el->index < elInfo->neigh[i]->index)
      {
        ldataNeigh = (AlbertLeafData *) elInfo->neigh[i]->child[1];
        for(int j =0; j<N_VERTICES-1; j++)
          ldataNeigh->reachedVertex[(elInfo->opp_vertex[i]+1+j)%N_VERTICES] = -1;
      }
    }

  }
}

void AlbertLeafRefine(EL * parent, EL * child[2])
{
  AlbertLeafData *ldata;
  AlbertLeafData *ldataChi;

  ldata = (AlbertLeafData *) parent->child[1];
  // assert(ldata != NULL);
  // assert(ldata->volume != 0.0);
  // std::cout << "AlbertLeafRefine: Muss noch getestet werden \n";



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

  // mark Vertices
  for(int i=0; i<N_VERTICES; i++)
    ldataChi->reachedVertex[i] = ldata->reachedVertex[i];

  ldataChi->reachedVertex[N_VERTICES-1] = 1;


  // child 1
  ldataChi = (AlbertLeafData *) child[1]->child[1];
  for(int i=0; i<N_VERTICES; i++)
    ldataChi->reachedFace[i] = ldata->reachedFace[i];
  // die neue Kante auf Child 1, siehe Albert Doc S. 11

  ldataChi->reachedFace[0] = -1;

  // mark Vertices
  for(int i=0; i<N_VERTICES; i++)
    ldataChi->reachedVertex[i] = ldata->reachedVertex[i];

  ldataChi->reachedVertex[N_VERTICES-1] = -1;

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
    ALBERT EL_INFO * elInfo = new ALBERT EL_INFO ();
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
  inline Vec<dimworld>& AlbertGridElement<dim,dimworld>::
  operator [](int i)
  {
    Vec<dimworld> *vec = new Vec<dimworld> (static_cast<double *>
                                            (elInfo_->coord[mapVertices<dimworld-dim>(i) % corners()]));
    return (*vec);
  }

  /// specialization for codim = dim
  inline Vec<3>& AlbertGridElement<0,3>::
  operator [](int i)
  {
    Vec<3> *vec = new Vec<3> (static_cast<double *> (elInfo_->coord[vertex_]));
    return (*vec);
  }

  /// specialization for codim = dim
  inline Vec<2>& AlbertGridElement<0,2>::
  operator [](int i)
  {
    Vec<2> *vec = new Vec<2> (static_cast<double *> (elInfo_->coord[vertex_]));
    return (*vec);
  }

  /// specialization for codim = dim
  inline Vec<1>& AlbertGridElement<0,1>::
  operator [](int i)
  {
    Vec<1> *vec = new Vec<1> (static_cast<double *> (elInfo_->coord[vertex_]));
    return (*vec);
  }

  template< int dim, int dimworld>
  inline AlbertGridElement<dim,dim> AlbertGridElement<dim,dimworld>::
  refelem()
  {
    AlbertGridElement<dim,dim> refEl;
    return refEl;
  }

  template< int dim, int dimworld>
  inline Vec<dimworld> AlbertGridElement<dim,dimworld>::
  global(Vec<dim> local)
  {}

  template< int dim, int dimworld>
  inline Vec<dim> AlbertGridElement<dim,dimworld>::
  local(Vec<dimworld> global)
  {}

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
  inline int AlbertGrid < dim, dimworld >::size (int level, int codim)
  {
    if((level != -1) || (codim != 0))
    {
      std::cout << "AlbertGrid::size \n";
      std::cout << "Richtige Implementierung fehlt noch\n";
    }
    return mesh_->n_elements;
  }

  //*******************************************************
  //
  // AlbertGridLevelIterator
  //
  //*******************************************************
  template<int codim, int dim, int dimworld>
  inline void AlbertGridLevelIterator<codim,dim,dimworld >::
  makeIterator()
  {
    //std::cout << "make new AlbertGridLevelIterator \n";
    vertex_ = 0;
    face_ = 0;
    edge_ = 0;
    vertexMarker_ = NULL;

    travStack_ = NULL;
    virtualEntity_ = new AlbertGridEntity<codim,dim,dimworld> (); //el_ = new Element();
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
  AlbertGridLevelIterator(ALBERT TRAVERSE_STACK * travStack,
                          ALBERT EL_INFO * elInfo)
  {

    if(travStack && elInfo)
    {
      vertex_ = 0;
      face_ = 0;
      edge_ = 0;

      vertexMarker_ = NULL;

      // set traverse_stack
      travStack_ = travStack;
      virtualEntity_ = new AlbertGridEntity<codim,dim,dimworld> (travStack_);

      // diese Methode muss neu geschrieben werden, da man
      // die ParentElement explizit speichern moechte.
      virtualEntity_->setElInfo(elInfo,face_,edge_,vertex_);
    }
    else
    {
      std::cout << "no travStack or elInfo in AlbertGridLevelIterator Constructor!\n";
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
      travStack_ = ALBERT get_traverse_stack();
      virtualEntity_ = new AlbertGridEntity<codim,dim,dimworld> (travStack_);

      // diese Methode muss neu geschrieben werden, da man
      // die ParentElement explizit speichern moechte.
      ALBERT EL_INFO* elInfo =
        goFirstElement(travStack_, mesh, travLevel,travFlags);
      virtualEntity_->setElInfo(elInfo,face_,edge_,vertex_);

    }
    else
      makeIterator();

  };


  template<int codim, int dim, int dimworld>
  inline AlbertGridLevelIterator<codim,dim,dimworld >::
  AlbertGridLevelIterator(const AlbertGridLevelIterator<codim,dim,dimworld > &I)
  {
    vertexCount_ = I.vertexCount_;
    travStack_ = I.travStack_;
    virtualEntity_ = I.virtualEntity_; // Zeiger kopieren
  }

  template<int codim, int dim, int dimworld>
  inline bool AlbertGridLevelIterator<codim,dim,dimworld >::
  operator ==(const AlbertGridLevelIterator<codim,dim,dimworld > &I) const
  {
    return (virtualEntity_->getElInfo() == I.virtualEntity_->getElInfo());
  }

  template<int codim, int dim, int dimworld>
  inline bool AlbertGridLevelIterator < codim,dim,dimworld >::
  operator !=(const AlbertGridLevelIterator< codim,dim,dimworld > & I) const
  {
    // fraglich, ob das geht
    return !((*this) == I);
  }

  // gehe zum naechsten Element, wie auch immer
  template<int codim, int dim, int dimworld>
  inline AlbertGridLevelIterator < codim,dim,dimworld >&
  AlbertGridLevelIterator < codim,dim,dimworld >::operator++()
  {
    // die 0 ist wichtig, weil Face 0, heist hier jetzt Element
    ALBERT EL_INFO *elInfo =
      goNextEntity<codim>(travStack_, virtualEntity_->getElInfo());
    virtualEntity_->setElInfo(elInfo,face_,edge_,vertex_); //, 0, 0,
    //  travStack_->elinfo_stack + (travStack_->stack_used-1));
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
    // go next Element, Face 0
    edge_++;
    if( edge_ >= dim ) // dim+1 Vertices
    {
      elInfo = goNextFace(travStack_, elInfo);
      edge_ = 0;
    }

    return elInfo;
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
      elInfo = goNextElInfo(travStack_, elInfo);
      vertex_ = 0;
    }

    if(!elInfo)
      return elInfo; // if no more Vertices, return

    // go next, if Vertex is not treated on this Element
    if(vertexMarker_->vertexNotOnElement(elInfo,vertex_))
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
      goNextEntity<codim>(travStack_, virtualEntity_->getElInfo());
    for(int i=1; i<= steps; i++)
      elInfo = goNextEntity<codim>(travStack_,virtualEntity_->getElInfo());

    virtualEntity_->setElInfo(elInfo); //, 0, 0,
    //travStack_->elinfo_stack + (travStack_->stack_used-1));
    return (*this);
  }

  template<int codim, int dim, int dimworld>
  inline typename AlbertGridEntity< codim,dim,dimworld >&
  AlbertGridLevelIterator< codim,dim,dimworld >::operator *()
  {
    return (*virtualEntity_);
  }

  template<int codim, int dim, int dimworld>
  inline typename AlbertGridEntity< codim,dim,dimworld >*
  AlbertGridLevelIterator< codim,dim,dimworld >::operator ->()
  {
    return virtualEntity_;
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

    if (stack->stack_used) {
      ALBERT_TEST_EXIT(elinfo_old == stack->elinfo_stack+stack->stack_used)
        ("invalid old elinfo\n");
    }
    else {
      ALBERT_TEST_EXIT(elinfo_old == nil) ("invalid old elinfo != nil\n");
    }


    if (stack->traverse_fill_flag & CALL_LEAF_EL_LEVEL)
    {
      elinfo = traverseLeafElLevel(stack);
      if (elinfo)
        stack->el_count++;
      else {
        /* MSG("total element count:%d\n",stack->el_count); */
      }
    }
    else
    {
      elinfo = traverse_next(stack,elinfo_old);
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
        if ((el == nil) || (el->child[0] == nil)) {
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
  }

  /// The Element is prescribed by the EL_INFO struct of ALBERT MESH
  /// the pointer to this struct is set and get by setElInfo and
  /// getElInfo.
  template<int codim, int dim, int dimworld>
  inline void AlbertGridEntity < codim, dim ,dimworld >::
  makeDescription()
  {
    elInfo_ = NULL;
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
  inline AlbertGridEntity < codim, dim ,dimworld >::
  AlbertGridEntity(bool makeNeigh)
  {
    makeDescription();
  };

  template<int codim, int dim, int dimworld>
  inline ALBERT EL_INFO* AlbertGridEntity < codim, dim ,dimworld >::
  getElInfo()
  {
    return elInfo_;
  }

  template<int dim, int dimworld>
  inline ALBERT EL_INFO* AlbertGridEntity < 0 , dim ,dimworld >::
  getElInfo()
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
    if ( dim == codim)
      return elInfo_->el->dof[vertex_][0];

    return elInfo_->el->index;
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

  template< int dim, int dimworld>
  inline void AlbertGridEntity < 0, dim ,dimworld >::
  setElInfo(ALBERT EL_INFO * elInfo,  unsigned char face,
            unsigned char edge, unsigned char vertex )
  {
    // in this case the face, edge and vertex information is not used,
    // because we are in the element case
    elInfo_ = elInfo;
  }

  template< int dim, int dimworld>
  inline AlbertGridElement<dim,dimworld>
  AlbertGridEntity < 0, dim ,dimworld >::geometry()
  {
    AlbertGridElement <dim , dimworld> geom (elInfo_);
    return geom;
  }

  template< int codim, int dim, int dimworld>
  inline AlbertGridElement<dim-codim,dimworld>
  AlbertGridEntity < codim, dim ,dimworld >::geometry()
  {
    AlbertGridElement <dim-codim , dimworld> geom (elInfo_,face_,edge_,vertex_);
    return geom;
  }

  template< int dim, int dimworld>
  inline AlbertGridLevelIterator<0,dim,dimworld>
  AlbertGridEntity < 0, dim ,dimworld >::father()
  {
    ALBERT TRAVERSE_STACK * travStack = new ALBERT TRAVERSE_STACK ();
    (*travStack) = (*travStack_);

    travStack->stack_used--;

    AlbertGridLevelIterator <0,dim,dimworld>
    it(travStack,travStack->elinfo_stack+travStack->stack_used);
    return it;
  }

  template< int dim, int dimworld>
  inline AlbertGridElement<dim,dim>
  AlbertGridEntity < 0, dim ,dimworld >::father_relative_local()
  {
    AlbertGridElement<dim,dim> el(elInfo_);
    return el;
  }
  //***************************************************************
  //
  //  AlbertGridEntity Hierarchic Iterator
  //
  //***************************************************************
  template< int dim, int dimworld>
  inline void AlbertGridEntity <0, dim ,dimworld >::HierarchicIterator::
  makeIterator()
  {
    //std::cout << "make new AlbertGridAccess::HierarchicIterator \n";
    travStack_ = NULL;
    virtualEntity_ = new AlbertGridEntity<0, dim ,dimworld > ();
  }


  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >::HierarchicIterator::
  HierarchicIterator()
  {
    makeIterator();
  }


  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >::HierarchicIterator::
  HierarchicIterator(ALBERT TRAVERSE_STACK * travStack,int travLevel)
  {
    if(travStack)
    {
      travStack_ = travStack;
      // default Einstellungen fuer den TraverseStack, siehe
      // traverse_first, traverse_nr_common.cc
      travStack_->traverse_level = travLevel;

      virtualEntity_ = new AlbertGridEntity<0,dim,dimworld> (travStack_);

      // Hier kann ein beliebiges Element uebergeben werden,
      // da jedes AlbertElement einen Zeiger auf das Macroelement
      // enthaelt.
      virtualEntity_->setElInfo(recursiveTraverse(travStack_));

    }
    else
      makeIterator();
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >::
  HierarchicIterator::HierarchicIterator(const HierarchicIterator& I)
  {
    travStack_ = I.travStack_;
    virtualEntity_ = I.virtualEntity_;
  }


  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >::HierarchicIterator
  AlbertGridEntity < 0, dim ,dimworld >::HierarchicIterator::
  operator ++()
  {
    // die 0 ist wichtig, weil Face 0, heist hier jetzt Element
    ALBERT EL_INFO * elIn = recursiveTraverse(travStack_);

    virtualEntity_->setElInfo(elIn);
    return (*this);
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >::HierarchicIterator
  AlbertGridEntity < 0, dim ,dimworld >::HierarchicIterator::
  operator ++(int steps)
  {
    for(int i=0; i<steps; i++)
      ++(*this);
    return (*this);
  }

  template< int dim, int dimworld>
  inline bool AlbertGridEntity < 0, dim ,dimworld >::HierarchicIterator::
  operator ==(const HierarchicIterator& I) const
  {
    return (virtualEntity_->getElInfo() == I.virtualEntity_->getElInfo());
  }

  template< int dim, int dimworld>
  inline bool AlbertGridEntity < 0, dim ,dimworld >::HierarchicIterator::
  operator !=(const HierarchicIterator& I) const
  {
    return !((*this) == I);
    //  (virtualEntity_->getElInfo() != I.virtualEntity_->getElInfo());
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >&
  AlbertGridEntity < 0, dim ,dimworld >::HierarchicIterator::
  operator *()
  {
    return (*virtualEntity_);
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >*
  AlbertGridEntity < 0, dim ,dimworld >::HierarchicIterator::
  operator ->()
  {
    return virtualEntity_;
  }

  template< int dim, int dimworld>
  inline ALBERT EL_INFO *
  AlbertGridEntity < 0, dim ,dimworld >::HierarchicIterator::
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
  //  AlbertGridEntity Neighbor Iterator
  //
  //***************************************************************
  template< int dim, int dimworld>
  inline void AlbertGridEntity <0, dim ,dimworld >::NeighborIterator::
  makeIterator()
  {
    //std::cout << "make new AlbertGridEntity::NeighborIterator \n";
    neighborCount_ = 0;
    travStack_ = NULL;
    elInfo_ = NULL;
    virtualEntity_ = new AlbertGridEntity<0, dim ,dimworld > ();
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >::
  NeighborIterator::NeighborIterator()
  {
    makeIterator();
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >::NeighborIterator::
  NeighborIterator(ALBERT TRAVERSE_STACK * travStack, ALBERT EL_INFO *elInfo)
  {
    if(travStack && !(elInfo->el->child[0]))
    {
      std::cout << "NeighborIterator \n";
      elInfo_ = elInfo;
      neighElInfo_ = NULL;
      neighborCount_ = 0;

      travStack_ = travStack;

      virtualEntity_ = new AlbertGridEntity<0,dim,dimworld>
                         (travStack_->traverse_mesh,travStack_);

      while(elInfo_->neigh[neighborCount_] == NULL)
      {
        neighborCount_++;
        if(neighborCount_ >=dim+1) break;
      }

      if(neighborCount_ < dim+1)
      {
        int oldEdge = elInfo_->opp_vertex[neighborCount_];
        elInfo_ = traverse_neighbour(travStack_,elInfo_,neighborCount_);

        std::cout << "Eins gegangen \n";

        neighElInfo_ = new ALBERT EL_INFO ();
        (*neighElInfo_) = (*elInfo_); // Macht eine Kopie von elInfo

        elInfo_ = traverse_neighbour(travStack_,elInfo_,oldEdge);

        // hier eine Kopie von Stack machen und die dann wieder wegwerfen

      }

      virtualEntity_->setElInfo(neighElInfo_);

      std::cout << "Setup done \n";
    }
    else
    {
      std::cout << "Sorry, can make NeighborIterator only on Leaf Elements! \n\n";
      makeIterator();
    }
  }


  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >::
  NeighborIterator::NeighborIterator(const NeighborIterator& I)
  {
    neighborCount_ = I.neighborCount_;
    elInfo_ = I.elInfo_;
    virtualEntity_ = I.virtualEntity_;
  }


  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >::NeighborIterator
  AlbertGridEntity < 0, dim ,dimworld >::NeighborIterator::
  operator ++()
  {
    // Gehe zum naechsten existierenden Nachbarn
    neighborCount_++;
    if(neighborCount_ < dim+1)
      while(elInfo_->neigh[neighborCount_] == NULL)
      {
        neighborCount_++;
        if(neighborCount_ >= dim+1) break;
      }

    if(neighborCount_ < dim+1)
    {
      // Merke, ueber welche Kante man zurueck kommt
      int helpEdge = elInfo_->opp_vertex[neighborCount_];

      // laufe zum Nachbarn
      elInfo_ = traverse_neighbour(travStack_,elInfo_,neighborCount_);

      // kopiere Nachbar
      (*neighElInfo_) = (*elInfo_);
      virtualEntity_->setElInfo(neighElInfo_);

      // Laufe zurueck zum Element ueber gemerkte Kante
      elInfo_ = traverse_neighbour(travStack_,elInfo_,helpEdge);
    }
    else
      virtualEntity_->setElInfo(NULL);
    return (*this);
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >::NeighborIterator
  AlbertGridEntity < 0, dim ,dimworld >::NeighborIterator::
  operator ++(int steps)
  {
    for(int i=0; i<steps; i++)
      ++(*this);
    return (*this);
  }

  template< int dim, int dimworld>
  inline bool AlbertGridEntity < 0, dim ,dimworld >::NeighborIterator::
  operator ==(const NeighborIterator& I) const
  {
    //return (neighborCount_ == I.neighborCount_);
    return (virtualEntity_->getElInfo() == I.virtualEntity_->getElInfo());
  }

  template< int dim, int dimworld>
  inline bool AlbertGridEntity < 0, dim ,dimworld >::NeighborIterator::
  operator !=(const NeighborIterator& I) const
  {
    //std::cout << "Operator Ungleich \n";
    return !((*this) == I);
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >&
  AlbertGridEntity < 0, dim ,dimworld >::NeighborIterator::
  operator *()
  {
    return (*virtualEntity_);
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >*
  AlbertGridEntity < 0, dim ,dimworld >::NeighborIterator::
  operator ->()
  {
    return virtualEntity_;
  }

  template< int dim, int dimworld>
  inline AlbertGridElement< dim-1, dim >
  AlbertGridEntity < 0, dim ,dimworld >::NeighborIterator::
  intersection_self_local()
  {
    AlbertGridElement<dim-1, dim > * tmp =
      new AlbertGridElement<dim-1, dim > ();

    return (*tmp);
  }



  //////////////////////////////////////////////////////////////////////////




  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >::
  HierarchicIterator AlbertGridEntity < 0, dim ,dimworld >::
  hbegin(int maxlevel)
  {
    // Zur Vorsicht kopiere ganzen Stack
    ALBERT TRAVERSE_STACK * travStack = new ALBERT TRAVERSE_STACK ();
    (*travStack) = (*travStack_);

    HierarchicIterator it(travStack,maxlevel);
    return it;
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >::
  HierarchicIterator AlbertGridEntity < 0, dim ,dimworld >::
  hend(int maxlevel)
  {
    HierarchicIterator it;
    return it;
  }



  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >::
  NeighborIterator AlbertGridEntity < 0, dim ,dimworld >::
  nbegin()
  {
    // Ueberlegung, ob hier der Traverse Stack kopiert werden soll
    ALBERT TRAVERSE_STACK * travStack = new ALBERT TRAVERSE_STACK ();
    (*travStack) = (*travStack_);

    NeighborIterator it(travStack,this->getElInfo());
    return it;
  }

  template< int dim, int dimworld>
  inline AlbertGridEntity < 0, dim ,dimworld >::
  NeighborIterator AlbertGridEntity < 0, dim ,dimworld >::
  nend()
  {
    NeighborIterator it;
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

  void AlbertMarkerVector::makeNewSize(int newNumberOfEntries)
  {
    delete vec_;

    vec_ = new int [newNumberOfEntries];

    for(int i=0; i<newNumberOfEntries; i++)
      vec_[i] = -1;

    numberOfEntries_ = newNumberOfEntries;
  }

  void AlbertMarkerVector::makeSmaller(int newNumberOfEntries)
  {}

  void AlbertMarkerVector::checkMark(ALBERT EL_INFO * elInfo, int vertex)
  {
    if(vec_[elInfo->el->dof[vertex][0]] == -1)
      vec_[elInfo->el->dof[vertex][0]] = elInfo->el->index;
  }

  bool AlbertMarkerVector::vertexNotOnElement(ALBERT EL_INFO * elInfo, int vertex)
  {
    return (vec_[elInfo->el->dof[vertex][0]] != elInfo->el->index);
  }

  void AlbertMarkerVector::markNewVertices(ALBERT MESH * mesh)
  {
    makeNewSize(mesh->n_vertices);

    ALBERT FLAGS travFlags = FILL_NOTHING | CALL_LEAF_EL;

    // get traverse_stack
    ALBERT TRAVERSE_STACK * travStack = ALBERT get_traverse_stack();

    // diese Methode muss neu geschrieben werden, da man
    // die ParentElement explizit speichern moechte.
    ALBERT EL_INFO* elInfo =
      ALBERT traverse_first(travStack, mesh, -1,travFlags);

    while(elInfo)
    {
      for(int i=0; i<N_VERTICES; i++)
        checkMark(elInfo,i);
      elInfo = traverse_next(travStack,elInfo);
    }
  }

  void AlbertMarkerVector::print()
  {
    printf("\nEntries %d \n",numberOfEntries_);
    for(int i=0; i<numberOfEntries_; i++)
      printf("Konten %d visited on Element %d \n",i,vec_[i]);
  }

} // end namespace dune
