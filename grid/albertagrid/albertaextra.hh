// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
/****************************************************************************/
/*  Header--File for extra Albert Functions                                 */
/****************************************************************************/
#ifndef DUNE_ALBERTAEXTRA_HH
#define DUNE_ALBERTAEXTRA_HH

#include <algorithm>

#ifdef __ALBERTApp__
namespace Albert {
#endif

#define ALBERTA_ERROR          ALBERTA print_error_funcname(funcName, __FILE__, __LINE__),\
  ALBERTA print_error_msg
#define ALBERTA_ERROR_EXIT     ALBERTA print_error_funcname(funcName, __FILE__, __LINE__),\
  ALBERTA print_error_msg_exit

#define ALBERTA_TEST_EXIT(test) if ((test)) ;else ALBERTA_ERROR_EXIT

#define getDofVec( vec, drv ) \
  (assert(drv != 0); (vec = (drv)->vec); assert(vec != 0));

//! recompute setting of neighbours, because macro_el_info of ALBERTA does
//! that wrong for the Dune context.
inline void computeNeigh(const MACRO_EL *mel, EL_INFO *elinfo, int neigh)
{
  // set right neighbour element
  elinfo->neigh[neigh]      = mel->neigh[neigh]->el;
  // get vertex of opposite coord
  int oppvx = mel->opp_vertex[neigh];
  elinfo->opp_vertex[neigh] = oppvx;

  // copy to opp_coord
  REAL_D *coord  = elinfo->opp_coord;
  const REAL * const * neighcoord  = mel->neigh[neigh]->coord;
  std::memcpy(coord[neigh],neighcoord[oppvx],sizeof(REAL_D));
}

//! if level iterator is used macro_el_info does not the right thing
inline void fillMacroInfo(TRAVERSE_STACK *stack,
                          const MACRO_EL *mel, EL_INFO *elinfo, int level)
{
  /* Alberta version */
  fill_macro_info(stack->traverse_mesh,mel,elinfo);

#if DIM == 2
  // only works for dim 2 at the moment
  // because there we have a different fill_elinfo method
  // quick solution, the method fill_macro_info has to be rewritten
  // not now, dont have the time
  if(level == elinfo->level)
  {
    for(int i=0 ; i<N_NEIGH; i++)
    {
      if(mel->neigh[i])
      {
        computeNeigh(mel,elinfo,i);
      }
      else
      {
        elinfo->neigh[i] = 0;
        elinfo->opp_vertex[i] = 0;
      }
    }
  }
#endif
}


// provides the element number generation and management
#include "agelementindex.cc"

// This three function are used by albertgrid.hh ~.cc
// but not defined in the regular albert.h
//extern void free_leaf_data(void *leaf_data, MESH *mesh);
//extern void free_dof(DOF *dof, MESH *mesh, int position);

inline void enlargeTraverseStack(TRAVERSE_STACK *stack);
inline static TRAVERSE_STACK *getTraverseStack(void);
inline static TRAVERSE_STACK *freeTraverseStack(TRAVERSE_STACK *stack);
inline void printTraverseStack(TRAVERSE_STACK *stack);

//! organize the TRAVERSE_STACK Management, so we can use the nice Albert
//! functions get_traverse_stack and free_traverse_stack
//! this count the copy made of this class and call free_traverse_stack
//! only if no more copies left
class ManageTravStack
{
  //! traverse stack for mesh traverse, see Albert Docu
  TRAVERSE_STACK * stack_;

  //! number of copies that exist from this stack_
  mutable int *refCount_;

  mutable bool owner_;

public:
  //! if a copy is made, the refcout is increased
  ManageTravStack(const ManageTravStack & copy)
  {
    stack_ = 0;
    refCount_ = 0;
    if(copy.stack_ != 0)
    {
      stack_ = copy.stack_;
      refCount_ = copy.refCount_;
      (*refCount_)++;
      copy.owner_ = false;
      owner_ = true;
    }
  }

  //! initialize the member variables
  ManageTravStack() : stack_ (0) , refCount_ (0) , owner_(false) {}

  //! get new TRAVERSE_STACK using the original Albert Routine
  //! get_traverse_stack, which get an new or free stack
  void makeItNew(bool realyMakeIt)
  {
    if(realyMakeIt)
    {
      stack_ = getTraverseStack();
      int * tmp = new int;
      refCount_ = tmp;
      (*refCount_) = 1;
      owner_ = true;
    }
  }

  //! set Stack free, if no more refences exist
  ~ManageTravStack()
  {
    if(refCount_ && stack_)
    {
      (*refCount_)--;
      if((*refCount_) <= 0)
      {
        // in free_traverse_stack stack != 0 is checked
        if(stack_)
        {
          stack_ = freeTraverseStack(stack_);
          owner_ = false;
        }
        if(refCount_)
        {
          delete refCount_;
          refCount_ = 0;
        }
      }
    }
  }

  //! return the TRAVERSE_STACK pointer for use
  TRAVERSE_STACK * getStack() const
  {
    // if this assertion is thrown then either the stack = 0
    // or we want to uese the pointer but are not the owner
    assert( stack_ );
    assert( (!owner_) ? (std::cerr << "\nERROR:The feature of copying iterators is not supported by AlbertaGrid at the moment! \n\n", 0) : 1);
    return stack_;
  }

  //private:
  //! if copy is made than one more Reference exists
  ManageTravStack & operator = (const ManageTravStack & copy)
  {
    // do not use this method
    if(copy.stack_ != 0)
    {
      stack_ = copy.stack_;
      refCount_ = copy.refCount_;
      (*refCount_)++;
      copy.owner_ = false;
      owner_ = true;
    }
    //assert(false);
    return (*this);
  }
};


//***********************************************************
// Traverse Stacks
//***********************************************************
static inline void initTraverseStack(TRAVERSE_STACK *stack);

inline static TRAVERSE_STACK *getTraverseStack(void)
{
  TRAVERSE_STACK * stack = get_traverse_stack();
  // if we use copyTraverseStack we should only create stacks with
  // stack_size > 0 otherwise we get errors in TreeIterator
  if(stack->stack_size <= 0) enlargeTraverseStack( stack );
  return stack;
}

inline static TRAVERSE_STACK *freeTraverseStack(TRAVERSE_STACK *stack)
{
  free_traverse_stack(stack);
  return 0;
}

inline void cutHierarchicStack(TRAVERSE_STACK* copy, TRAVERSE_STACK* org)
{
  copy->traverse_mesh = org->traverse_mesh;
  copy->traverse_level = org->traverse_level;
  copy->traverse_fill_flag = org->traverse_fill_flag;
  copy->traverse_mel = org->traverse_mel;

  if(copy->stack_size < org->stack_size)
  {
    enlargeTraverseStack(copy);
  }

  int used = org->stack_used;
  copy->stack_used = 1;

  // copy only the last 2 elements
  int copyUse = used-1;
  if(copyUse < 0) copyUse = 0;

  memcpy(copy->elinfo_stack,org->elinfo_stack+copyUse,
         2*sizeof(EL_INFO));

  copy->info_stack[0] = org->info_stack[used];
  // go to child 0
  copy->info_stack[1] = 0;

  memcpy(copy->save_elinfo_stack,org->save_elinfo_stack+copyUse,
         2*sizeof(EL_INFO));

  copy->save_info_stack[0] = org->save_info_stack[used];
  copy->save_info_stack[1] = 0;

  copy->save_stack_used = org->save_stack_used;
  copy->el_count = 1;
  return;
}

inline void copyTraverseStack( TRAVERSE_STACK* stack, const TRAVERSE_STACK* org )
{
  const int & used = stack->stack_size;
  // we have problems to copy stack of length 0
  assert( used > 0 );

  if(stack->elinfo_stack) MEM_FREE(stack->elinfo_stack,used, EL_INFO);
  if(stack->info_stack) MEM_FREE(stack->info_stack,used, U_CHAR );
  if(stack->save_elinfo_stack) MEM_FREE(stack->save_elinfo_stack,used,EL_INFO );
  if(stack->save_info_stack) MEM_FREE(stack->save_info_stack,used,U_CHAR);

  // NOTE: at this point also the used value changes
  // because stack->stack_size is changed
  memcpy( stack, org, sizeof(TRAVERSE_STACK));

  stack->elinfo_stack = 0;
  stack->elinfo_stack = MEM_ALLOC(used, EL_INFO);

  // here we have to copy all EL_INFOs seperately, the normal way does not
  // work, unfortunately
  if (used > 0)
  {
    for (int i=0; i<used; i++)
    {
      memcpy(&(stack->elinfo_stack[i]),&(org->elinfo_stack[i]),sizeof(EL_INFO));
    }
  }

  assert( used == org->stack_size );

  // the pointer have to be created new
  stack->info_stack        = 0;
  stack->info_stack        = MEM_ALLOC(used, U_CHAR);
  stack->save_elinfo_stack = 0;
  stack->save_elinfo_stack = MEM_ALLOC(used, EL_INFO);
  stack->save_info_stack   = 0;
  stack->save_info_stack   = MEM_ALLOC(used, U_CHAR);

  memcpy(stack->elinfo_stack     ,org->elinfo_stack,     used * sizeof(EL_INFO));
  memcpy(stack->info_stack       ,org->info_stack,       used * sizeof(U_CHAR));
  memcpy(stack->save_elinfo_stack,org->save_elinfo_stack,used * sizeof(EL_INFO));
  memcpy(stack->save_info_stack  ,org->save_info_stack,  used * sizeof(U_CHAR));

  return;
}

static inline void initTraverseStack(TRAVERSE_STACK *stack)
{
  stack->traverse_mesh = 0;
  stack->traverse_level = 0;
  stack->traverse_mel = 0;
  stack->stack_size = 0;
  stack->stack_used = 0;
  stack->elinfo_stack = 0;
  stack->info_stack = 0;
  stack->save_elinfo_stack = 0;
  stack->save_info_stack = 0;
  stack->save_stack_used = 0;
  stack->el_count = 0;
  stack->next = 0;
  return;
}

inline void enlargeTraverseStack(TRAVERSE_STACK *stack)
{
  int i;
  int new_stack_size = stack->stack_size + 10;

  stack->elinfo_stack = MEM_REALLOC(stack->elinfo_stack, stack->stack_size,
                                    new_stack_size, EL_INFO);

  if (stack->stack_size > 0)
    for (i=stack->stack_size; i<new_stack_size; i++)
      stack->elinfo_stack[i].fill_flag = stack->elinfo_stack[0].fill_flag;

  stack->info_stack = MEM_REALLOC(stack->info_stack, stack->stack_size,
                                  new_stack_size, U_CHAR);
  stack->save_elinfo_stack = MEM_REALLOC(stack->save_elinfo_stack,
                                         stack->stack_size,
                                         new_stack_size, EL_INFO);
  stack->save_info_stack   = MEM_REALLOC(stack->save_info_stack,
                                         stack->stack_size,
                                         new_stack_size, U_CHAR);

  stack->stack_size = new_stack_size;
}

inline void printTraverseStack(TRAVERSE_STACK *stack)
{
  FUNCNAME("printTraverseStack");
  MSG("****************************************************\n");
  MSG("current stack %8X | size %d \n", stack,stack->stack_size);
  MSG("traverse_level %d \n",stack->traverse_level);
  MSG("traverse_mesh  %8X \n",stack->traverse_mesh);
  MSG("elinfo_stack      = %8X\n",stack->elinfo_stack);
  MSG("info_stack        = %8X\n",stack->info_stack);
  MSG("save_elinfo_stack = %8X\n",stack->save_elinfo_stack);
  MSG("save_info_stack   = %8X\n\n",stack->save_info_stack);

  MSG("stack_used        = %d\n",stack->stack_used);
  MSG("save_stack_used   = %d\n",stack->save_stack_used);
  MSG("****************************************************\n");
}

inline void printElInfo(const EL_INFO *elf)
{
  FUNCNAME("printElInfo");

  MSG("Element %d | level %d  | ",INDEX(elf->el),elf->level);
  printf("Neighs: ");
  for(int i=0; i<N_VERTICES; i++)
  {
    ALBERTA EL* el = elf->neigh[i];
    printf(" %p |",el);
  }
  printf("\n");


  for(int i=0; i<N_VERTICES; i++)
    printf("%d %f %f \n",i,elf->coord[i][0],elf->coord[i][1]);


  printf("\n******************************************\n");

}


//****************************************************************
//
//  Wrapper for ALBERTA refine and coarsen routines.
//  Calling direct refine in the grid.refine() method leads to
//  infinite loop. Donno wy?
//  This wrappers solved the problem.
//
//****************************************************************

// wrapper for Albert refine routine
inline static U_CHAR AlbertRefine ( MESH * mesh )
{
  return refine ( mesh );
}

// wrapper for Albert coarsen routine
inline static U_CHAR AlbertCoarsen ( MESH * mesh )
{
  U_CHAR flag = coarsen ( mesh );
  // is mesh was really coarsend, then make dof_compress
  if(flag == MESH_COARSENED) dof_compress ( mesh );
  return flag;
}

//*********************************************************************
//
//  Help Routines for the ALBERTA Mesh
//
//*********************************************************************
namespace AlbertHelp
{

  static EL_INFO * getFatherInfo(TRAVERSE_STACK * stack, EL_INFO * elInfo, int level)
  {
    assert( level == elInfo->level );
    EL_INFO * fatherInfo = 0;

    // if this level > 0 return father = elInfoStack -1,
    // else return father = this
    assert(stack != 0);

    if(level > 0)
    {
      fatherInfo = & (stack->elinfo_stack)[level];
    }
    else
    {
      printf("No Father for macro element, return macro element\n");
      return elInfo;
    }
    return fatherInfo;
  }

  //**************************************************************************
  //  calc Maxlevel of AlbertGrid and remember on wich level an element lives
  //**************************************************************************

  static int Albert_MaxLevel_help=-1;

  // function for mesh_traverse, is called on every element
  inline static void calcmxl (const EL_INFO * elf)
  {
    int level = elf->level;
    if(Albert_MaxLevel_help < level) Albert_MaxLevel_help = level;
  }

  // remember on which level an element realy lives
  inline int calcMaxLevel ( MESH * mesh , DOF_INT_VEC * levelVec )
  {
    Albert_MaxLevel_help = -1;

    // see ALBERTA Doc page 72, traverse over all hierarchical elements
    mesh_traverse(mesh,-1, CALL_LEAF_EL|FILL_NOTHING, calcmxl);

    // check if ok
    assert(Albert_MaxLevel_help != -1);
    return Albert_MaxLevel_help;
  }



  //**************************************************************************
  inline static void printNeighbour (const EL_INFO * elf)
  {
    int i;
    printf("%d EL \n",INDEX(elf->el));
    for(i=0; i<3; i++)
      if(elf->neigh[i])
        printf("%d Neigh \n",INDEX(elf->neigh[i]));
      else printf("%d Neigh \n",-1);
    printf("----------------------------------\n");
  }

  //*********************************************************************

  static int AlbertaLeafDataHelp_processor = -1;
  // Leaf Data for Albert, only the leaf elements have this data set
  template <int cdim, int vertices>
  struct AlbertLeafData
  {
#ifdef LEAFDATACOORDS
    typedef Dune::FieldMatrix<double,vertices,cdim> CoordinateMatrixType;
    typedef Dune::FieldVector<double,cdim> CoordinateVectorType;
#endif
    // type of stored data
    typedef struct {
#ifdef LEAFDATACOORDS
      CoordinateMatrixType coord;
#endif
      double determinant;
      int processor;
    } Data;

    // keep element numbers
    inline static void AlbertLeafRefine(EL *parent, EL *child[2])
    {
      Data * ldata;
      int i, processor=-1;

      ldata = (Data *) parent->child[1];
      assert(ldata != 0);

      //std::cout << "Leaf refine for el = " << parent << "\n";

      double childDet = 0.5 * ldata->determinant;
      processor = ldata->processor;

      /* bisection ==> 2 children */
      for(i=0; i<2; i++)
      {
        Data *ldataChi = (Data *) child[i]->child[1];
        assert(ldataChi != 0);
        ldataChi->determinant = childDet;
        ldataChi->processor = processor;

#ifdef LEAFDATACOORDS
        // calculate the coordinates
        {
          const CoordinateMatrixType &oldCoord = ldata->coord;
          CoordinateMatrixType &coord = ldataChi->coord;
          for (int j = 0; j < cdim; ++j)
          {
            coord[2][j] = 0.5 * (oldCoord[0][j] + oldCoord[1][j]);
            coord[i  ][j] = oldCoord[2][j];
            coord[1-i][j] = oldCoord[i][j];
          }
          //    std::cout << coord[0] << " " << coord[1] << " " << coord[2] << "\n";
        }
#endif
      }
    }

    inline static void AlbertLeafCoarsen(EL *parent, EL *child[2])
    {
      Data *ldata;
      int i;

      ldata = (Data *) parent->child[1];
      assert(ldata != 0);
      ldata->processor = -1;
      double & det = ldata->determinant;
      det = 0.0;

      //std::cout << "Leaf coarsen for el = " << parent << "\n";

      /* bisection ==> 2 children */
      for(i=0; i<2; i++)
      {
        Data *ldataChi = (Data *) child[i]->child[1];
        assert(ldataChi != 0);
        det += ldataChi->determinant;
        if(ldataChi->processor >= 0)
          ldata->processor = ldataChi->processor;
      }
    }

    // we dont need Leaf Data
    inline static void initLeafData(LEAF_DATA_INFO * linfo)
    {
      linfo->leaf_data_size = sizeof(Data);
      linfo->refine_leaf_data  = &AlbertLeafRefine;
      linfo->coarsen_leaf_data = &AlbertLeafCoarsen;
      return;
    }

    // function for mesh_traverse, is called on every element
    inline static void setLeafData(const EL_INFO * elf)
    {
      assert( elf->el->child[0] == 0 );
      Data *ldata = (Data *) elf->el->child[1];
      assert(ldata != 0);

#ifdef LEAFDATACOORDS
      for(int i=0; i<vertices; ++i)
      {
        CoordinateVectorType & c = ldata->coord[i];
        const ALBERTA REAL_D & coord = elf->coord[i];
        for(int j=0; j<cdim; ++j) c[j] = coord[j];
        //      std::cout << c << " coord \n";
      }
#endif

      ldata->determinant = ALBERTA el_det(elf);
      ldata->processor   = AlbertaLeafDataHelp_processor;
    }

    // remember on which level an element realy lives
    inline static void initLeafDataValues( MESH * mesh, int proc )
    {
      AlbertaLeafDataHelp_processor = proc;

      // see ALBERTA Doc page 72, traverse over all hierarchical elements
      ALBERTA mesh_traverse(mesh,-1, CALL_LEAF_EL|FILL_COORDS,setLeafData);

      AlbertaLeafDataHelp_processor = -1;
    }

  }; // end of AlbertLeafData

  // struct holding the needed DOF_INT_VEC for AlbertGrid
  typedef struct dofvec_stack DOFVEC_STACK;
  struct dofvec_stack
  {
    // storage of unique element numbers
    DOF_INT_VEC * elNumbers[numOfElNumVec];
    // contains information about refine status of element
    DOF_INT_VEC * elNewCheck;
    // stores the processor number of proc where element is master
    DOF_INT_VEC * owner;
  };

  static DOF_INT_VEC * elNumbers[numOfElNumVec];
  static DOF_INT_VEC * elNewCheck = 0;
  static DOF_INT_VEC * elOwner    = 0;

  // return pointer to created elNumbers Vector to mesh
  inline DOF_INT_VEC * getElNumbers(int i)
  {
    int * vec=0;
    GET_DOF_VEC(vec,elNumbers[i]);
    FOR_ALL_DOFS(elNumbers[i]->fe_space->admin, vec[dof] = getElementIndexForCodim(i) );
    return elNumbers[i];
  }

  // return pointer to created elNumbers Vector to mesh
  inline static int calcMaxIndex(DOF_INT_VEC * drv)
  {
    int maxindex = 0;
    int * vec=0;
    GET_DOF_VEC(vec,drv);
    FOR_ALL_DOFS(drv->fe_space->admin, if(vec[dof] > maxindex) { maxindex = vec[dof] } );
    // we return +1 because this means a size
    return maxindex+1;
  }

  // return pointer to created elNewCheck Vector to mesh
  inline DOF_INT_VEC * getElNewCheck()
  {
    int * vec=0;
    GET_DOF_VEC(vec,elNewCheck);
    FOR_ALL_DOFS(elNewCheck->fe_space->admin, vec[dof] = 0 );
    return elNewCheck;
  }

  // return pointer to created elNewCheck Vector to mesh
  inline DOF_INT_VEC * getOwner()
  {
    int * vec=0;
    GET_DOF_VEC(vec,elOwner);
    FOR_ALL_DOFS(elOwner->fe_space->admin, vec[dof] = 0 );
    return elOwner;
  }

  inline void getDofVecs(DOFVEC_STACK * dofvecs)
  {
    for(int i=0; i<numOfElNumVec; i++)
    {
      dofvecs->elNumbers[i]  = getElNumbers(i);
      elNumbers[i]  = 0;
    }

    dofvecs->elNewCheck   = getElNewCheck();  elNewCheck = 0;
    dofvecs->owner        = getOwner();       elOwner    = 0;
  }

  // set entry for new elements to 1
  inline static void refineElNewCheck ( DOF_INT_VEC * drv , RC_LIST_EL *list, int ref)
  {
    const DOF_ADMIN * admin = drv->fe_space->admin;
    const int nv = admin->n0_dof[CENTER];
    const int k  = admin->mesh->node[CENTER];
    int *vec = 0;

    GET_DOF_VEC(vec,drv);
    assert(ref > 0);

    for(int i=0; i<ref; i++)
    {
      const EL * el = list[i].el;
      //std::cout << "refine elNewCheck for el = " << el << "\n";

      // get level of father which is the absolute value
      // if value is < 0 then this just means that element
      // was refined
      int level = std::abs( vec[el->dof[k][nv]] ) + 1;
      for(int ch=0; ch<2; ch++)
      {
        // set new to negative level of the element
        // which then can be used to check whether an element is new on not
        // also this vector stores the level of the element which is needed
        // for the face iterator
        vec[el->child[ch]->dof[k][nv]] = -level;
      }
    }
  }

  // put element index to stack, if element is coarsend
  inline static void coarseElNewCheck ( DOF_INT_VEC * drv , RC_LIST_EL *list, int ref)
  {}


  // set entry for new elements to 1
  inline static void refineElOwner ( DOF_INT_VEC * drv , RC_LIST_EL *list, int ref)
  {
    const DOF_ADMIN * admin = drv->fe_space->admin;
    const int nv = admin->n0_dof[CENTER];
    const int k  = admin->mesh->node[CENTER];
    int *vec = 0;
    int val = -1;

    GET_DOF_VEC(vec,drv);

    assert(ref > 0);

    for(int i=0; i<ref; i++)
    {
      const EL * el = list[i].el;

      //std::cout << "refine ElOwner for el = " << el << "\n";
      val = vec[el->dof[k][nv]];
      //printf("refine owner %d \n",val);
      // in case of ghosts
      for(int ch=0; ch<2; ch++)
      {
        // set processor
        vec[el->child[ch]->dof[k][nv]] = val;
        //printf("child %d \n",vec[el->child[ch]->dof[k][nv]]);
      }
    }
  }

  // clear Dof Vec
  inline static void clearDofVec ( DOF_INT_VEC * drv )
  {
    int * vec=0;
    GET_DOF_VEC(vec,drv);
    FOR_ALL_DOFS(drv->fe_space->admin, vec[dof] = 0 );
  }

  // calculate max absolute value of given vector
  inline int calcMaxAbsoluteValueOfVector ( const DOF_INT_VEC * drv )
  {
    const int * vec = 0;
    int maxi = 0;
    GET_DOF_VEC(vec,drv);
    FOR_ALL_DOFS(drv->fe_space->admin, maxi = std::max( maxi , std::abs(vec[dof]) ) );
    return maxi;
  }

  // set all values of vector to its positive value
  inline static void set2positive ( DOF_INT_VEC * drv )
  {
    int * vec=0;
    GET_DOF_VEC(vec,drv);
    FOR_ALL_DOFS(drv->fe_space->admin, vec[dof] = std::abs( vec[dof] ) );
  }

  // clear Dof Vec
  inline static void setDofVec ( DOF_INT_VEC * drv , int val )
  {
    int * vec=0;
    GET_DOF_VEC(vec,drv);
    FOR_ALL_DOFS(drv->fe_space->admin, vec[dof] = val );
  }

  // clear Dof Vec
  inline static void copyOwner ( DOF_INT_VEC * drv , int * ownvec )
  {
    int * vec=0;
    GET_DOF_VEC(vec,drv);
    FOR_ALL_DOFS(drv->fe_space->admin, vec[dof] = ownvec[dof] );
  }


#define PROCRESTORE 66666666

  // save processor number in owner vec
  // to the value at the position with the processor number
  // the significant value PROCRESTORE is added or if the value is smaller
  // then zero substracted.
  inline static int saveMyProcNum ( DOF_INT_VEC * drv , const int myProc,
                                    int & entry)
  {
    int * vec=0;
    int spot = -1;
    GET_DOF_VEC(vec,drv);
    FOR_ALL_DOFS(drv->fe_space->admin,
                 if(dof == myProc)
                 {
                   spot = dof;
                   entry = vec[dof];
                   if(vec[dof] >= 0)
                     vec[dof] += PROCRESTORE;
                   else
                     vec[dof] -= PROCRESTORE;
                 }
                 );
    return spot;
  }

  // restore processor number for owner vec
  inline static int restoreMyProcNum ( DOF_INT_VEC * drv)
  {
    int myProc = -1;
    int * vec=0;

    GET_DOF_VEC(vec,drv);
    FOR_ALL_DOFS(drv->fe_space->admin,
                 if(vec[dof] >= PROCRESTORE)
                 {
                   vec[dof] -= PROCRESTORE;
                   myProc = dof;
                 }
                 else if (vec[dof] <= -PROCRESTORE)
                 {
                   vec[dof] += PROCRESTORE;
                   myProc = dof;
                 }
                 );

    return myProc;
  }

  inline DOF_INT_VEC * getDofNewCheck(const FE_SPACE * espace)
  {
    DOF_INT_VEC * drv = get_dof_int_vec("el_new_check",espace);
    int * vec=0;
    drv->refine_interpol = &refineElNewCheck;
    drv->coarse_restrict = 0;
    GET_DOF_VEC(vec,drv);
    FOR_ALL_DOFS(drv->fe_space->admin, vec[dof] = 0 );
    return drv;
  }

  // setup of DOF_INT_VEC for reading
  inline void makeTheRest( DOFVEC_STACK * dofvecs)
  {
    dofvecs->elNewCheck = getDofNewCheck(dofvecs->elNumbers[0]->fe_space);
    // if owner vec not has been read
    if(!dofvecs->owner) dofvecs->owner = getDofNewCheck(dofvecs->elNumbers[0]->fe_space);
    dofvecs->owner->refine_interpol = &refineElOwner;
  }

  // initialize dofAdmin for vertex and element numbering
  // and a given dim
  template <int dim>
  static inline void initDofAdmin(MESH *mesh)
  {
    int edof[dim+1]; // add one dof at element for element numbering
    int vdof[dim+1]; // add at each vertex one dof for vertex numbering
    int fdof[dim+1]; // add at each face one dof for face numbering
    int edgedof[dim+1]; // add at each edge one dof for edge numbering

    for(int i=0; i<dim+1; i++)
    {
      vdof[i] = 0; fdof[i]    = 0;
      edof[i] = 0; edgedof[i] = 0;
    }

    vdof[0] = 1;

    if(dim == 3) edgedof[1] = 1; // edge dof only in 3d

    fdof[dim-1] = 1; // means edges in 2d and faces in 3d
    edof[dim] = 1;

    get_fe_space(mesh, "vertex_dofs", vdof, 0);

    //**********************************************************************
    // all the element vectors
    //**********************************************************************
    {
      // space for center dofs , i.e. element numbers
      const FE_SPACE * eSpace = get_fe_space(mesh, "center_dofs", edof, 0);

      // the first entry is called at last for refinement and coarsening
      // the methods for the adaptation call back are put to elNewCheck
      // refine and coarsening procedures
      elNewCheck = get_dof_int_vec("el_new_check",eSpace);
      elNewCheck->refine_interpol = &refineElNewCheck;
      elNewCheck->coarse_restrict = &coarseElNewCheck;

      elOwner = get_dof_int_vec("el_owner",eSpace);
      elOwner->refine_interpol = &refineElOwner;
      elOwner->coarse_restrict = 0;

      // the element numbers, ie. codim = 0
      elNumbers[0] = get_dof_int_vec("element_numbers",eSpace);
      elNumbers[0]->refine_interpol = &RefineNumbering<dim,0>::refineNumbers;
      elNumbers[0]->coarse_restrict = &RefineNumbering<dim,0>::coarseNumbers;
    }

    //**********************************************************************

    {
      // the face number space , i.e. codim == 1
      const FE_SPACE * eSpace = get_fe_space(mesh, "face_dofs", fdof, 0);

      // the face numbers, i.e. codim = 1
      elNumbers[1] = get_dof_int_vec("face_numbers",eSpace);
      elNumbers[1]->refine_interpol = &RefineNumbering<dim,1>::refineNumbers;
      elNumbers[1]->coarse_restrict = &RefineNumbering<dim,1>::coarseNumbers;
    }

    if(dim == 3)
    {
      // the edge number space , i.e. codim == 2
      const FE_SPACE * eSpace = get_fe_space(mesh, "edge_dofs", edgedof, 0);

      // the edge numbers, i.e. codim = 2
      elNumbers[2] = get_dof_int_vec("edge_numbers",eSpace);
      elNumbers[2]->refine_interpol = &RefineNumbering<dim,2>::refineNumbers;
      elNumbers[2]->coarse_restrict = &RefineNumbering<dim,2>::coarseNumbers;
    }

    return;
  }

  static std::stack < BOUNDARY * > * Alberta_tmpBndStack = 0;

  inline static void initBndStack( std::stack < BOUNDARY * > * bndStack )
  {
    Alberta_tmpBndStack = bndStack;
  }
  inline static void removeBndStack ()
  {
    Alberta_tmpBndStack = 0;
  }

  // initialize boundary for mesh
  inline const BOUNDARY *initBoundary(MESH * Spmesh, int bound)
  {
    BOUNDARY *b = (BOUNDARY *) new BOUNDARY ();
    assert(b != 0);

    assert(Alberta_tmpBndStack);
    Alberta_tmpBndStack->push( b );

    // bound is of type signed char which goes from -127 to 128
    if((bound < -127) && (bound > 128))
    {
      std::cerr << "Got boundary id = " << bound << "\n";
      std::cerr << "Wrong boundary id: range is only from -127 to 128 !\n";
      std::cerr << "Correct your macro grid file!\n";
      abort();
    }

    b->param_bound = 0;
    b->bound = bound;

    return b;
  }

  //*******************************************************************
  //*******************************************************************
  //*******************************************************************
  //
  //  functions for making the partition on one processor
  //
  //*******************************************************************
  //*******************************************************************
  //*******************************************************************

  struct Albert_Restore_Mesh {

    MESH * mesh;
    MACRO_EL ** mels;
    int n_macro_el;
    int n_elements;
    int n_hier_elements;
    int n_vertices;
  };

  typedef struct Albert_Restore_Mesh ALBERTA_RESTORE_MESH;


  static ALBERTA_RESTORE_MESH ag_restore = { 0 , 0 ,-1 ,-1 ,-1 ,-1};

  // backu the mesh before removing some macro elements
  inline static void storeMacroElements(MESH * mesh)
  {
    // if other mesh was stored before, then restore was forget
    assert(ag_restore.mesh == 0);
    int length = mesh->n_macro_el;
    MACRO_EL ** mel = (MACRO_EL **) std::malloc(length * sizeof(MACRO_EL *));
    assert(mel != 0);

    int no=0;
    for(MACRO_EL * el = mesh->first_macro_el; el; el = el->next)
    {
      mel[no] = el;
      no++;
    }

    assert(no == length);

    ag_restore.mesh = mesh;
    ag_restore.mels = mel;
    ag_restore.n_macro_el = mesh->n_macro_el;
    ag_restore.n_elements = mesh->n_elements;
    ag_restore.n_hier_elements = mesh->n_hier_elements;
    ag_restore.n_vertices = mesh->n_vertices;
  }

  // restore the mesh, that ALBERTA can free it normaly
  inline static void resetMacroElements(MESH * mesh)
  {
    // if not the same mesh to restore ==> break
    assert(ag_restore.mesh == mesh);
    int length = ag_restore.n_macro_el;
    MACRO_EL **mel = ag_restore.mels;
    assert(mel != 0);

    mesh->first_macro_el = mel[0];
    mel[0]->last = 0;

    for(int i=1; i<length; i++)
    {
      mel[i-1]->next = mel[i];
      (mel[i])->last = mel[i-1];
    }

    mel[length-1]->next = 0;

    mesh->n_macro_el = ag_restore.n_macro_el;
    mesh->n_elements = ag_restore.n_elements;
    mesh->n_hier_elements = ag_restore.n_hier_elements;
    mesh->n_vertices = ag_restore.n_vertices;

    ag_restore.mesh = 0;
    std::free(ag_restore.mels); ag_restore.mels = 0;
    ag_restore.n_macro_el = -1;
    ag_restore.n_elements = -1;
    ag_restore.n_hier_elements = -1;
    ag_restore.n_vertices = -1;
  }

  // mark elements that not belong to my processor
  inline void partitioning ( MACRO_EL * mel, int proc, int mynumber  )
  {
    if(proc == mynumber)
    {
      mel->el->mark = 0;
    }
    else
    {
      mel->el->mark = 1;
    }
  }

  // unmark neighbours of not interior elements
  inline void ghosts ( MACRO_EL * mel )
  {
    if(mel->el->mark == 0)
    {
      for(int i=0; i<N_NEIGH; i++)
      {
        if(mel->neigh[i])
        {
          if(mel->neigh[i]->el->mark != 0)
          {
            mel->neigh[i]->el->mark = -1;
          }
        }
      }
    }
  }

  // reset mark
  inline void afterGhosts ( MACRO_EL * mel )
  {
    if(mel->el->mark < 0)
    {
      mel->el->mark = 0;
    }
  }

  // remove macro elements that not belong to this processor or that are not ghost
  inline void removeMacroEls ( MESH * mesh , int proc , int * ownvec )
  {
    MACRO_EL *mel = 0;

    int length = mesh->n_macro_el;
    int * dofNum = (int *) std::malloc(mesh->n_vertices * sizeof(int));
    int * dofHier = (int *) std::malloc(mesh->n_vertices*sizeof(int));
    assert(dofNum  != 0);
    assert(dofHier != 0);

    dof_compress(mesh);

    for(int i=0; i<mesh->n_vertices; i++) dofNum[i] = -1;
    for(int i=0; i<mesh->n_vertices; i++) dofHier[i] = -1;

    for(mel = mesh->first_macro_el; mel; mel = mel->next)
    {
      partitioning( mel , proc, ownvec[mel->index] );
    }

    for(mel = mesh->first_macro_el; mel; mel = mel->next)
    {
      ghosts ( mel );
    }

    for(mel = mesh->first_macro_el; mel; mel = mel->next)
    {
      afterGhosts ( mel );
    }

    for(mel = mesh->first_macro_el; mel; mel = mel->next)
    {
      if(mel->el->mark == 1)
      {
        for(int i=0; i<N_NEIGH; i++)
        {
          MACRO_EL * neigh = mel->neigh[i];
          if(neigh)
          {
            for(int k=0; k<N_NEIGH; k++)
            {
              if(neigh->neigh[k] == mel)
                neigh->neigh[k] = 0;
            }
          }
        }
      }
    }

    for(mel = mesh->first_macro_el; mel; mel = mel->next)
    {
      // mark all dof that remain on mesh
      if(mel->el->mark != 1)
      {
        EL * myEl = mel->el;
        for(int l=0; l<N_VERTICES; l++)
        {
          dofNum[myEl->dof[l][0]] = 1;
        }
      }
    }

    for(mel = mesh->first_macro_el; mel; mel = mel->next)
    {
      // mark all dof that remain on mesh
      if(mel->el->mark == 1)
      {
        EL * myEl = mel->el;
        for(int l=0; l<N_VERTICES; l++)
        {
          int dof = myEl->dof[l][0];
          if(dofNum[dof] == -1)
          {
            dofHier[dof] = mel->index;
          }
        }
      }
    }

    for(mel = mesh->first_macro_el; mel; mel = mel->next)
    {
      if(mel->el->mark == 1)
      {
        DOF * dofs[N_VERTICES];
        EL *myEl = mel->el;
        for(int k=0; k<N_VERTICES; k++)
        {
          dofs[k] = myEl->dof[k];
        }
        assert(myEl != 0);

        for(int k=0; k<N_VERTICES; k++)
        {
          if((dofNum[dofs[k][0]] == -1) && (dofHier[dofs[k][0]] == mel->index ) )
          {
            int dof = dofs[k][0];
            dofNum[dof] = 1;
            mesh->n_vertices--;
          }
        }
      }
    }

    // remove the macro elements
    mel = mesh->first_macro_el;
    while(mel)
    {
      if(mel->el->mark == 1)
      {
        if(mel->last)
        {
          mel->last->next = mel->next;
        }
        else
        {
          // if no last exists that means we have the first and change the
          // first macro el of the mesh
          mesh->first_macro_el = mel->next;
        }

        if(mel->next)
        {
          mel->next->last = mel->last;
        }

        mesh->n_hier_elements--;
        mesh->n_elements--;
        mesh->n_macro_el--;

      }
      mel = mel->next;
    }

    dof_compress(mesh);

    // remember old owner
    int * fakeMem = (int *) std::malloc(length * sizeof(int));
    std::memcpy(fakeMem,ownvec,length * sizeof(int));
    for(int i=0; i<length; i++) ownvec[i] = -1;
    int no = 0;
    for(mel = mesh->first_macro_el; mel; mel = mel->next)
    {
      ownvec[no] = fakeMem[mel->index];
      no++;
    }

    // free memory
    if(fakeMem) std::free(fakeMem);fakeMem = 0;
    if(dofNum) std::free(dofNum);dofNum  = 0;
    if(dofHier) std::free(dofHier);dofHier = 0;
  }

  inline void printMacroData(MACRO_DATA * mdata)
  {
    FUNCNAME("printMacroData");
    MSG("noe %d , nvx %d \n",mdata->n_macro_elements,mdata->n_total_vertices);
    for(int i=0; i<mdata->n_total_vertices; i++)
      MSG("coords [%f | %f ]\n",mdata->coords[i][0],mdata->coords[i][1]);

    for(int i=0; i<mdata->n_macro_elements; i++)
      MSG("bound [%d | %d | %d ]\n",mdata->boundary[i][0],mdata->boundary[i][1],mdata->boundary[i][2]);


  }

  // function for mesh_traverse, is called on every element
  inline static void setElOwner(const EL_INFO * elf)
  {
    const DOF_ADMIN * admin = elOwner->fe_space->admin;
    const int nv = admin->n0_dof[CENTER];
    const int k  = admin->mesh->node[CENTER];
    int *vec = 0;
    EL * el   = elf->el;
    EL * papi = elf->parent;

    if(elf->level <= 0) return;

    assert(el && papi);
    GET_DOF_VEC(vec,elOwner);

    int papiProc = vec[papi->dof[k][nv]];
    vec[el->dof[k][nv]] = papiProc;
    return ;
  }

  // remember on which level an element realy lives
  inline void setElOwnerNew( MESH * mesh, DOF_INT_VEC * elOwn )
  {
    elOwner = elOwn;
    assert(elOwner != 0);
    // see ALBERTA Doc page 72, traverse over all hierarchical elements
    mesh_traverse(mesh,-1, CALL_EVERY_EL_PREORDER|FILL_NEIGH,setElOwner);
    elOwner = 0;
    return ;
  }

  // function for mesh_traverse, is called on every element
  inline static void storeLevelOfElement(const EL_INFO * elf)
  {
    const DOF_ADMIN * admin = elNewCheck->fe_space->admin;
    const int nv = admin->n0_dof[CENTER];
    const int k  = admin->mesh->node[CENTER];
    int *vec = 0;
    const EL * el   = elf->el;

    int level = elf->level;
    if( level <= 0 ) return;

    assert(el);
    GET_DOF_VEC(vec,elNewCheck);

    vec[el->dof[k][nv]] = level;
    return ;
  }

  // remember on which level an element realy lives
  inline void restoreElNewCheck( MESH * mesh, DOF_INT_VEC * elNChk )
  {
    elNewCheck = elNChk;
    assert(elNewCheck != 0);

    // see ALBERTA Doc page 72, traverse over all hierarchical elements
    mesh_traverse(mesh,-1,CALL_EVERY_EL_PREORDER|FILL_NEIGH,storeLevelOfElement);
    elNewCheck = 0;
    return ;
  }

} // end namespace AlbertHelp

#ifdef __ALBERTApp__
} // end namespace Albert
#endif

#endif  /* !_ALBERTAEXTRA_H_ */
