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

#if 0
  static ALBERT EL_INFO statElInfo[DIM+1];

  // singleton holding reference elements
  template<int dim> struct AlbertGridReferenceElement
  {
    enum { dimension = dim };

    static AlbertGridElement<dim,dim> refelem;
    static ALBERT EL_INFO elInfo_;

  };


  // initialize static variable with bool constructor
  // (which makes reference element)
  template<int dim>
  AlbertGridElement<dim,dim> AlbertGridReferenceElement<dim>::refelem(true);

#endif


#include "uggridelement.cc"
#include "uggridentity.cc"
#include "uggridhieriterator.cc"

#if 0
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
    grid_(grid), level_ (level) , neighborCount_ (dim+1), virtualEntity_ (NULL)
    , fakeNeigh_ (NULL)
    , neighGlob_ (NULL) , elInfo_ (NULL)
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
    // calculates the outer_normal
    Vec<dimworld,albertCtype>& tmp = outer_normal(local);

    double norm_1 = (1.0/tmp.norm2());
    assert(norm_1 > 0.0);
    outNormal_ *= norm_1;

    return outNormal_;
  }

  template< int dim, int dimworld>
  inline Vec<dimworld,albertCtype>& AlbertGridNeighborIterator<dim,dimworld>::
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
  inline Vec<dimworld,albertCtype>& AlbertGridNeighborIterator<dim,dimworld>::
  outer_normal(Vec<dim-1,albertCtype>& local)
  {
    // we dont have curved boundary
    // therefore return outer_normal
    return outer_normal();
  }

  template< int dim, int dimworld>
  inline Vec<dimworld,albertCtype>& AlbertGridNeighborIterator<dim,dimworld>::
  outer_normal()
  {
    std::cout << "outer_normal() not correctly implemented yet! \n";
    for(int i=0; i<dimworld; i++)
      outNormal_(i) = 0.0;

    return outNormal_;
  }

  template <>
  inline Vec<2,albertCtype>& AlbertGridNeighborIterator<2,2>::
  outer_normal()
  {
    // seems to work
    ALBERT REAL_D *coord = elInfo_->coord;

    outNormal_(0) = -(coord[(neighborCount_+1)%3][1] - coord[(neighborCount_+2)%3][1]);
    outNormal_(1) =   coord[(neighborCount_+1)%3][0] - coord[(neighborCount_+2)%3][0];

    return outNormal_;
  }

  template <>
  inline Vec<3,albertCtype>& AlbertGridNeighborIterator<3,3>::
  outer_normal()
  {
    // rechne Kreuzprodukt der Vectoren aus
    ALBERT REAL_D *coord = elInfo_->coord;
    Vec<3,albertCtype> v;
    Vec<3,albertCtype> u;

    for(int i=0; i<3; i++)
    {
      v(i) = coord[(neighborCount_+2)%4][i] - coord[(neighborCount_+1)%4][i];
      u(i) = coord[(neighborCount_+3)%4][i] - coord[(neighborCount_+2)%4][i];
    }

    for(int i=0; i<3; i++)
      outNormal_(i) = u((i+1)%3)*v((i+2)%3) - u((i+2)%3)*v((i+1)%3);

    return outNormal_;
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

    if(neighGlob_->builtGeom(elInfo_,neighborCount_,0,0))
      return (*neighGlob_);
    else
      abort();
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

  // end NeighborIterator




#endif



#include "uggridleveliterator.cc"

#if 0
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
        for(int local=0; local<dim+1; local++)
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
#endif
  //***********************************************************************
  //
  // --UGGrid
  // --Grid
  //
  //***********************************************************************

  bool UGGrid < 2, 2 >::hasInitializedUG = false;
  bool UGGrid < 3, 3 >::hasInitializedUG = false;

  template < int dim, int dimworld >
  inline UGGrid < dim, dimworld >::UGGrid() /* :
                                               mesh_ (NULL),  maxlevel_ (0) , wasChanged_ (false), time_ (0.0) */
  {
    if (!hasInitializedUG) {

      // Init the UG system
      int argc = 1;
      char* arg = {"dune.exe"};
      char** argv = &arg;

      UG3d::InitUg(&argc, &argv);

      // Create a dummy problem
      UG3d::CoeffProcPtr coeffs[1];
      UG3d::UserProcPtr upp[1];

      upp[0] = NULL;
      coeffs[0] = NULL;

      if (UG3d::CreateBoundaryValueProblem("DuneDummyProblem", NULL,
                                           1,coeffs,1,upp) == NULL)
        assert(false);


      // A Dummy new format
      //newformat P1_conform $V n1: nt 9 $M implicit(nt): mt 2 $I n1;
      char* newformatArgs[4] = {"newformat DuneFormat",
                                "V n1: nt 9",
                                "M implicit(nt): mt 2",
                                "I n1"};
      UG3d::CreateFormatCmd(4, newformatArgs);

      hasInitializedUG = true;
    }

  }

  template < int dim, int dimworld >
  inline UGGrid < dim, dimworld >::~UGGrid()
  {};

  //template < int dim, int dimworld > template<int codim>
  template <>
#ifndef __GNUC__
  template <>
#endif
  inline UGGridLevelIterator<3,3,3>
  UGGrid < 3, 3 >::lbegin<3> (int level) const
  {
    UG3d::multigrid* theMG = UG3d::GetMultigrid("DuneMG");
    assert(theMG);
    UG3d::grid* theGrid = theMG->grids[level];

    //  AlbertGridLevelIterator<codim,dim,dimworld> it(*this,vertexMarker_,level);
    UGGridLevelIterator<3,3,3> it((*const_cast<UGGrid< 3, 3 >* >(this)),level);
    it.target = theGrid->firstNode[0];
    printf("lbegin codim 3\n");
    return it;
  }

  template <>
#ifndef __GNUC__
  template <>
#endif
  inline UGGridLevelIterator<2,3,3>
  UGGrid < 3, 3 >::lbegin<2> (int level) const
  {
    //  AlbertGridLevelIterator<codim,dim,dimworld> it(*this,vertexMarker_,level);
    UGGridLevelIterator<2,3,3> it((*const_cast<UGGrid< 3, 3 >* >(this)),level);
    printf("lbegin codim 2\n");
    return it;
  }
  template <>
#ifndef __GNUC__
  template <>
#endif
  inline UGGridLevelIterator<1,3,3>
  UGGrid < 3, 3 >::lbegin<1> (int level) const
  {
    //  AlbertGridLevelIterator<codim,dim,dimworld> it(*this,vertexMarker_,level);
    UGGridLevelIterator<1,3,3> it((*const_cast<UGGrid< 3, 3 >* >(this)),level);
    printf("lbegin codim 1\n");
    return it;
  }

  template <>
#ifndef __GNUC__
  template <>
#endif
  inline UGGridLevelIterator<0,3,3>
  UGGrid < 3, 3 >::lbegin<0> (int level) const
  {
    UG3d::multigrid* theMG = UG3d::GetMultigrid("DuneMG");
    assert(theMG);
    UG3d::grid* theGrid = theMG->grids[level];

    //  AlbertGridLevelIterator<codim,dim,dimworld> it(*this,vertexMarker_,level);
    UGGridLevelIterator<0,3,3> it((*const_cast<UGGrid< 3, 3 >* >(this)),level);
    it.target = theGrid->elements[0];
    printf("lbegin codim 0\n");
    return it;
  }


  template < int dim, int dimworld > template<int codim>
  inline UGGridLevelIterator<codim,dim,dimworld>
  UGGrid < dim, dimworld >::lend (int level) const
  {
    UGGridLevelIterator<codim,dim,dimworld> it((*const_cast<UGGrid< dim, dimworld >* >(this)),level);
    return it;
  }

#if 0
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
  inline bool AlbertGrid < dim, dimworld >::coarsen()
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

#endif  // #if 0

  template < int dim, int dimworld >
  inline int UGGrid < dim, dimworld >::size (int level, int codim) const
  {
    //     enum { numCodim = dim+1 };
    //     int ind = (level * numCodim) + codim;

    //     if(size_[ind] == -1)
    {
      int numberOfElements = 0;

      if(codim == 0)
      {
        UGGridLevelIterator<0,dim,dimworld> endit = lend<0>(level);
        for(UGGridLevelIterator<0,dim,dimworld> it = lbegin<0>(level);
            it != endit; ++it)
          numberOfElements++;
      }
      if(codim == 1)
      {
        UGGridLevelIterator<1,dim,dimworld> endit = lend<1>(level);
        for(UGGridLevelIterator<1,dim,dimworld> it = lbegin<1>(level);
            it != endit; ++it)
          numberOfElements++;
      }
      if(codim == 2)
      {
        UGGridLevelIterator<2,dim,dimworld> endit = lend<2>(level);
        for(UGGridLevelIterator<2,dim,dimworld> it = lbegin<2>(level);
            it != endit; ++it)
          numberOfElements++;
      }

      if(codim == 3)
      {
        UGGridLevelIterator<3,dim,dimworld> endit = lend<3>(level);
        for(UGGridLevelIterator<3,dim,dimworld> it = lbegin<3>(level);
            it != endit; ++it)
          numberOfElements++;
      }

      //            size_[ind] = numberOfElements;
      return numberOfElements;
    }
    //     else
    //         {
    //             return size_[ind];
    //         }
  }

#if 0

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

#if 1
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

#endif

} // namespace Dune
