// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
/****************************************************************************/
/*  Header--File for extra Albert Functions                                 */
/****************************************************************************/
#ifndef _ALBERTEXTRA_H_
#define _ALBERTEXTRA_H_

#ifdef __ALBERT++ __
namespace Albert {
#endif

#define ALBERT_ERROR          ALBERT print_error_funcname(funcName, __FILE__, __LINE__),\
  ALBERT print_error_msg
#define ALBERT_ERROR_EXIT     ALBERT print_error_funcname(funcName, __FILE__, __LINE__),\
  ALBERT print_error_msg_exit

#define ALBERT_TEST_EXIT(test) if ((test)) ;else ALBERT_ERROR_EXIT

#define getDofVec( vec, drv ) \
  (assert(drv != NULL); (vec = (drv)->vec); assert(vec != NULL));
//vec = (drv)->vec;

//#define _ALBERTA_

inline void fillMacroInfo(TRAVERSE_STACK *stack,
                          const MACRO_EL *mel, EL_INFO *elinfo)
{
#ifdef _ALBERTA_
  /* Alberta version */
  fill_macro_info(stack->traverse_mesh,mel,elinfo);
#else
  /* Albert version */
  fill_macro_info(mel,elinfo);
#endif
}

// provides the element number generation and management
#include "agelementindex.cc"

// This three function are used by albertgrid.hh ~.cc
// but not defined in the regular albert.h
//extern void free_leaf_data(void *leaf_data, MESH *mesh);
//extern void free_dof(DOF *dof, MESH *mesh, int position);

void enlargeTraverseStack(TRAVERSE_STACK *stack);
static TRAVERSE_STACK *getTraverseStack(void);
static TRAVERSE_STACK *freeTraverseStack(TRAVERSE_STACK *stack);
void printTraverseStack(TRAVERSE_STACK *stack);

//! organize the TRAVERSE_STACK Management, so we can use the nice Albert
//! fucntions get_traverse_stack and free_traverse_stack
//! this count the copy made of this class and call free_traverse_stack
//! only if no more copies left
class ManageTravStack
{
  //! traverse stack for mesh traverse, see Albert Docu
  TRAVERSE_STACK * stack_;

  //! number of copies that exist from this stack_
  mutable int *refCount_;

public:
  //! if a copy is made, the refcout is increased
  ManageTravStack(const ManageTravStack & copy)
  {
    stack_ = NULL;
    refCount_ = NULL;
    if(copy.stack_ != NULL)
    {
      stack_ = copy.stack_;
      refCount_ = copy.refCount_;
      (*refCount_)++;
    }
  }

  //! initialize the member variables
  ManageTravStack() : stack_ (NULL) , refCount_ (NULL) {}

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
        // in free_traverse_stack stack != NULL is checked
        if(stack_)
        {
          stack_ = freeTraverseStack(stack_);
        }
        if(refCount_)
        {
          delete refCount_;
          refCount_ = NULL;
        }
      }
    }
  }

  //! return the TRAVERSE_STACK pointer for use
  TRAVERSE_STACK * getStack() const
  {
    return stack_;
  }
private:
  //! if copy is made than one more Reference exists
  ManageTravStack & operator = (const ManageTravStack & copy)
  {
    // do not use this method
    bool fake = true;
    assert(fake != true);
#if 0
    if(copy.stack_)
    {
      std::cout << "Copy Stack! \n";
      (*copy.refCount_)++;
      stack_ = copy.stack_;
      refCount_ = copy.refCount_;
    }
#endif
    return (*this);
  }
};

static TRAVERSE_STACK *freeStack = NULL;
static int stackCount=0;

static inline void initTraverseStack(TRAVERSE_STACK *stack);

inline static TRAVERSE_STACK *getTraverseStack(void)
{
#if 0
  TRAVERSE_STACK *stack;

  if (!freeStack)
  {
    stack = (TRAVERSE_STACK *) std::malloc(sizeof(TRAVERSE_STACK));
    initTraverseStack(stack);
    stackCount++;
  }
  else
  {
    stack = freeStack;
    freeStack = freeStack->next;
  }
  return(stack);
#else
  return get_traverse_stack();
#endif
}

inline static TRAVERSE_STACK *freeTraverseStack(TRAVERSE_STACK *stack)
{
#if 0
  if (!stack) {
    std::cout << "stack==NULL ???\n";
    abort();
    return NULL;
  }
  stack->next = freeStack;
  freeStack = stack;
  return NULL;
#endif
  free_traverse_stack(stack);
  return NULL;
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

static inline void initTraverseStack(TRAVERSE_STACK *stack)
{
  std::cout << "initTraverseStack !\n";
  stack->traverse_mesh = NULL;
  stack->stack_size = 0;
  stack->stack_used = 0;
  stack->elinfo_stack = NULL;
  stack->info_stack = NULL;
  stack->save_elinfo_stack = NULL;
  stack->save_info_stack = NULL;
  stack->save_stack_used = 0;
  stack->el_count = 0;
  stack->next = NULL;
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

void printTraverseStack(TRAVERSE_STACK *stack)
{
  FUNCNAME("printTraverseStack");
  MSG("****************************************************\n");
  MSG("current stack %8X | size %d, Count = %d \n", stack,stack->stack_size,stackCount);
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

void printElInfo(const EL_INFO *elf)
{
  FUNCNAME("printElInfo");

  MSG("Element %d | level %d  | ",INDEX(elf->el),elf->level);
  printf("Neighs: ");
  for(int i=0; i<N_VERTICES; i++)
  {
    ALBERT EL* el = elf->neigh[i];
    if(el) printf(" %d |",INDEX(el));
  }
  printf("\n");


  for(int i=0; i<N_VERTICES; i++)
    printf("%d %f %f \n",i,elf->coord[i][0],elf->coord[i][1]);


  printf("\n******************************************\n");

}


//****************************************************************
//
//  Wrapper for ALBERT refine and coarsen routines.
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
//  Help Routines for the ALBERT Mesh
//
//*********************************************************************
namespace AlbertHelp
{

  //****************************************************************
  //
  //  Albert reference element local numbering for 3D
  //
  //****************************************************************
  // see Albert Doc page 12 for reference element
  // if we lokk from outside, then face numbering must be clockwise
  static const int tetraFace_0[3] = {3,2,1};
  static const int tetraFace_1[3] = {2,3,0};
  static const int tetraFace_2[3] = {0,3,1};
  static const int tetraFace_3[3] = {0,1,2};

  static const int * localTetraFaceNumber[4] = {tetraFace_0, tetraFace_1,
                                                tetraFace_2 , tetraFace_3 };

  //**************************************************************************
  //  calc Maxlevel of AlbertGrid and remember on wich level an element lives
  //**************************************************************************

  static int Albert_MaxLevel_help=-1;
  static int Albert_GlobalIndex_help=-1;

  static DOF_INT_VEC * Albert_elnums_help=NULL;

  static bool doItAgain = false;
  static std::vector<int> *Albert_neighArray_help;

  // function for mesh_traverse, is called on every element
  inline static void calcMaxLevel (const EL_INFO * elf)
  {
    int level = elf->level;

    const DOF_ADMIN * admin = Albert_elnums_help->fe_space->admin;
    int nv = admin->n0_dof[CENTER];
    int k  = admin->mesh->node[CENTER];
    int dof = elf->el->dof[k][nv];
    int *vec = NULL;

    GET_DOF_VEC(vec, Albert_elnums_help);
    int index = vec[dof];

    // calculate global max index
    if(index > Albert_GlobalIndex_help) Albert_GlobalIndex_help = index;

    assert(index >= 0);

    if(Albert_neighArray_help->size() <= ABS(index))
    {
      doItAgain = true;
    }
    else
    {
      if(Albert_MaxLevel_help < level) Albert_MaxLevel_help = level;
      (* Albert_neighArray_help )[index] = level;
    }
  }

  // remember on which level an element realy lives
  inline int calcMaxLevelAndMarkNeighbours ( MESH * mesh, DOF_INT_VEC * elnums,
                                             std::vector< int > &nb, int & GlobalIndex) //, int & GlobalMinIndex )
  {
    // determine new maxlevel
    nb.resize( mesh->n_hier_elements );

    Albert_neighArray_help = &nb;
    Albert_elnums_help = elnums;
    Albert_MaxLevel_help = -1;
    Albert_GlobalIndex_help = -1;

    doItAgain = false;

    // see ALBERT Doc page 72, traverse over all hierarchical elements
    mesh_traverse(mesh,-1, CALL_EVERY_EL_PREORDER|FILL_NOTHING,calcMaxLevel);
    if(doItAgain)
    {
      nb.resize( Albert_GlobalIndex_help+1 );
      mesh_traverse(mesh,-1, CALL_EVERY_EL_PREORDER|FILL_NOTHING,calcMaxLevel);
    }

    Albert_neighArray_help = NULL;
    doItAgain = false;

    // check if ok
    assert(Albert_MaxLevel_help != -1);

    // for length determination
    GlobalIndex    = Albert_GlobalIndex_help+1;

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

  // Leaf Data for Albert, only the leaf elements have this data set
  typedef struct {
    int processor;
  } AlbertLeafData;

  // keep element numbers
  inline static void AlbertLeafRefine(EL *parent, EL *child[2])
  {
    AlbertLeafData *ldata;
    int i, processor=-1;

    ldata = (AlbertLeafData *) parent->child[1];
    assert(ldata != NULL);

    processor = ldata->processor;

    /* bisection ==> 2 children */
    for(i=0; i<2; i++)
    {
      AlbertLeafData *ldataChi = (AlbertLeafData *) child[i]->child[1];
      assert(ldataChi != NULL);
      ldataChi->processor = processor;
    }

  }

  inline static void AlbertLeafCoarsen(EL *parent, EL *child[2])
  {
    AlbertLeafData *ldata;
    int i;

    ldata = (AlbertLeafData *) parent->child[1];
    assert(ldata != NULL);
    ldata->processor = -1;

    /* bisection ==> 2 children */
    for(i=0; i<2; i++)
    {
      AlbertLeafData *ldataChi = (AlbertLeafData *) child[i]->child[1];
      assert(ldataChi != NULL);
      if(ldataChi->processor >= 0)
        ldata->processor = ldataChi->processor;
    }
  }

  // we dont need Leaf Data
  inline static void initLeafData(LEAF_DATA_INFO * linfo)
  {
    linfo->leaf_data_size = sizeof(AlbertLeafData);
    linfo->refine_leaf_data = &AlbertLeafRefine;
    linfo->coarsen_leaf_data =&AlbertLeafCoarsen;
    return;
  }

  // struct holding the needed DOF_INT_VEC for AlbertGrid
  typedef struct dofvec_stack DOFVEC_STACK;
  struct dofvec_stack
  {
    // storage of unique element numbers
    DOF_INT_VEC * elNumbers;
    // contains information about refine status of element
    DOF_INT_VEC * elNewCheck;
    // stores the processor number of proc where element is master
    DOF_INT_VEC * owner;
  };

  static DOF_INT_VEC * elNumbers  = NULL;
  static DOF_INT_VEC * elNewCheck = NULL;
  static DOF_INT_VEC * elOwner    = NULL;

  // return pointer to created elNumbers Vector to mesh
  inline DOF_INT_VEC * getElNumbers()
  {
    int * vec=NULL;
    GET_DOF_VEC(vec,elNumbers);
    FOR_ALL_DOFS(elNumbers->fe_space->admin, vec[dof] = get_elIndex() );
    return elNumbers;
  }

  // return pointer to created elNewCheck Vector to mesh
  inline DOF_INT_VEC * getElNewCheck()
  {
    int * vec=NULL;
    GET_DOF_VEC(vec,elNewCheck);
    FOR_ALL_DOFS(elNewCheck->fe_space->admin, vec[dof] = 0 );
    return elNewCheck;
  }

  // return pointer to created elNewCheck Vector to mesh
  inline DOF_INT_VEC * getOwner()
  {
    int * vec=NULL;
    GET_DOF_VEC(vec,elOwner);
    FOR_ALL_DOFS(elOwner->fe_space->admin, vec[dof] = 0 );
    return elOwner;
  }

  inline void getDofVecs(DOFVEC_STACK * dofvecs)
  {
    dofvecs->elNumbers   = getElNumbers();   elNumbers  = NULL;
    dofvecs->elNewCheck  = getElNewCheck();  elNewCheck = NULL;
    dofvecs->owner       = getOwner();       elOwner    = NULL;
  }


  // get element index form stack or new number
  inline static void refineElNumbers ( DOF_INT_VEC * drv , RC_LIST_EL *list, int ref)
  {
    const DOF_ADMIN * admin = drv->fe_space->admin;
    const int nv = admin->n0_dof[CENTER];
    const int k  = admin->mesh->node[CENTER];
    int dof;
    int *vec = NULL;

    GET_DOF_VEC(vec,drv);

    assert(ref > 0);

    for(int i=0; i<ref; i++)
    {
      EL * el = list[i].el;
      for(int ch=0; ch<2; ch++)
      {
        dof = el->child[ch]->dof[k][nv];

        // get element index from stack or new, see. elmem.cc
        vec[dof] = get_elIndex();
      }
    }
  }

  // put element index to stack, if element is coarsend
  inline static void coarseElNumbers ( DOF_INT_VEC * drv , RC_LIST_EL *list, int ref)
  {
    const DOF_ADMIN * admin = drv->fe_space->admin;
    const int nv = admin->n0_dof[CENTER];
    const int k  = admin->mesh->node[CENTER];
    int dof;
    int *vec = NULL;

    GET_DOF_VEC(vec,drv);

    assert(ref > 0);

    for(int i=0; i<ref; i++)
    {
      EL * el = list[i].el;
      for(int ch=0; ch<2; ch++)
      {
        dof = el->child[ch]->dof[k][nv];

        // put element index to stack, see elmem.cc
        free_elIndex( vec[dof] );
      }
    }
  }

  // set entry for new elements to 1
  inline static void refineElNewCheck ( DOF_INT_VEC * drv , RC_LIST_EL *list, int ref)
  {
    const DOF_ADMIN * admin = drv->fe_space->admin;
    const int nv = admin->n0_dof[CENTER];
    const int k  = admin->mesh->node[CENTER];
    int dof;
    int *vec = NULL;

    GET_DOF_VEC(vec,drv);

    assert(ref > 0);

    for(int i=0; i<ref; i++)
    {
      EL * el = list[i].el;
      for(int ch=0; ch<2; ch++)
      {
        // set new entry to 1, 0 means old element
        vec[el->child[ch]->dof[k][nv]] = 1;
      }
    }
  }

  // set entry for new elements to 1
  inline static void refineElOwner ( DOF_INT_VEC * drv , RC_LIST_EL *list, int ref)
  {
    const DOF_ADMIN * admin = drv->fe_space->admin;
    const int nv = admin->n0_dof[CENTER];
    const int k  = admin->mesh->node[CENTER];
    int dof;
    int *vec = NULL;
    int val = -1;

    GET_DOF_VEC(vec,drv);

    assert(ref > 0);

    for(int i=0; i<ref; i++)
    {
      EL * el = list[i].el;

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
    int * vec=NULL;
    GET_DOF_VEC(vec,drv);
    FOR_ALL_DOFS(drv->fe_space->admin, vec[dof] = 0 );
  }

  // clear Dof Vec
  inline static void setDofVec ( DOF_INT_VEC * drv , int val )
  {
    int * vec=NULL;
    GET_DOF_VEC(vec,drv);
    FOR_ALL_DOFS(drv->fe_space->admin, vec[dof] = val );
  }

  // clear Dof Vec
  inline static void copyOwner ( DOF_INT_VEC * drv , int * ownvec )
  {
    int * vec=NULL;
    GET_DOF_VEC(vec,drv);
    FOR_ALL_DOFS(drv->fe_space->admin, vec[dof] = ownvec[dof] );
  }

  inline DOF_INT_VEC * getDofNewCheck(const FE_SPACE * espace)
  {
    DOF_INT_VEC * drv = get_dof_int_vec("el_new_check",espace);
    int * vec=NULL;
    drv->refine_interpol = &refineElNewCheck;
    drv->coarse_restrict = NULL;
    GET_DOF_VEC(vec,drv);
    FOR_ALL_DOFS(drv->fe_space->admin, vec[dof] = 0 );
    return drv;
  }

  // setup of DOF_INT_VEC for reading
  inline void makeTheRest( DOFVEC_STACK * dofvecs)
  {
    dofvecs->elNewCheck = getDofNewCheck(dofvecs->elNumbers->fe_space);
    dofvecs->owner    = getDofNewCheck(dofvecs->elNumbers->fe_space);
    dofvecs->owner->refine_interpol = &refineElOwner;
  }

  // initialize dofAdmin for vertex and element numbering
  inline void initDofAdmin(MESH *mesh)
  {
    int edof[DIM+1]; // add one dof at element for element numbering
    int vdof[DIM+1]; // add at each vertex one dof for vertex numbering

    for(int i=0; i<DIM+1; i++)
    {
      vdof[i] = 0;
      edof[i] = 0;
    }

    vdof[0] = 1;
    edof[DIM] = 1;

    get_fe_space(mesh, "vertex_dofs", vdof, NULL);

    // space for center dofs , i.e. element numbers
    const FE_SPACE * eSpace = get_fe_space(mesh, "center_dofs", edof, NULL);

    elNumbers = get_dof_int_vec("element_numbers",eSpace);
    elNumbers->refine_interpol = &refineElNumbers;
    elNumbers->coarse_restrict = &coarseElNumbers;

    elNewCheck = get_dof_int_vec("el_new_check",eSpace);
    elNewCheck->refine_interpol = &refineElNewCheck;
    elNewCheck->coarse_restrict = NULL;

    elOwner = get_dof_int_vec("el_owner",eSpace);
    elOwner->refine_interpol = &refineElOwner;
    elOwner->coarse_restrict = NULL;

    return;
  }

  // initialize dofAdmin only for vertex numbering
  inline void emptyDofAdmin(MESH *mesh)
  {
    int vdof[DIM+1]; // add at each vertex one dof for vertex numbering

    for(int i=0; i<DIM+1; i++)
    {
      vdof[i] = 0;
    }

    vdof[0] = 1;

    get_fe_space(mesh, "vertex_dofs", vdof, NULL);
    return;
  }

  // initialize boundary for mesh
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

  typedef struct Albert_Restore_Mesh ALBERT_RESTORE_MESH;


  static ALBERT_RESTORE_MESH ag_restore = { NULL , NULL ,-1 ,-1 ,-1 ,-1};

  // backu the mesh before removing some macro elements
  inline static void storeMacroElements(MESH * mesh)
  {
    // if other mesh was stored before, then restore was forget
    assert(ag_restore.mesh == NULL);
    int length = mesh->n_macro_el;
    MACRO_EL ** mel = (MACRO_EL **) std::malloc(length * sizeof(MACRO_EL *));
    assert(mel != NULL);

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

  // restore the mesh, that ALBERT can free it normaly
  inline static void resetMacroElements(MESH * mesh)
  {
    // if not the same mesh to restore ==> break
    assert(ag_restore.mesh == mesh);
    int length = ag_restore.n_macro_el;
    MACRO_EL **mel = ag_restore.mels;
    assert(mel != NULL);

    mesh->first_macro_el = mel[0];
    mel[0]->last = NULL;

    for(int i=1; i<length; i++)
    {
      mel[i-1]->next = mel[i];
      (mel[i])->last = mel[i-1];
    }

    mel[length-1]->next = NULL;

    mesh->n_macro_el = ag_restore.n_macro_el;
    mesh->n_elements = ag_restore.n_elements;
    mesh->n_hier_elements = ag_restore.n_hier_elements;
    mesh->n_vertices = ag_restore.n_vertices;

    ag_restore.mesh = NULL;
    std::free(ag_restore.mels); ag_restore.mels = NULL;
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
    MACRO_EL *mel = NULL;

    int length = mesh->n_macro_el;
    int * dofNum = (int *) std::malloc(mesh->n_vertices * sizeof(int));
    int * dofHier = (int *) std::malloc(mesh->n_vertices*sizeof(int));
    assert(dofNum  != NULL);
    assert(dofHier != NULL);

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
        int count = 0;
        for(int i=0; i<N_NEIGH; i++)
        {
          MACRO_EL * neigh = mel->neigh[i];
          if(neigh)
          {
            for(int k=0; k<N_NEIGH; k++)
            {
              if(neigh->neigh[k] == mel)
                neigh->neigh[k] = NULL;
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
        assert(myEl != NULL);

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
    if(fakeMem) std::free(fakeMem);fakeMem = NULL;
    if(dofNum) std::free(dofNum);dofNum  = NULL;
    if(dofHier) std::free(dofHier);dofHier = NULL;
  }

  void printMacroData(MACRO_DATA * mdata)
  {
    FUNCNAME("printMacroData");
    MSG("noe %d , nvx %d \n",mdata->n_macro_elements,mdata->n_total_vertices);
    for(int i=0; i<mdata->n_total_vertices; i++)
      MSG("coords [%f | %f ]\n",mdata->coords[i][0],mdata->coords[i][1]);

    for(int i=0; i<mdata->n_macro_elements; i++)
      MSG("bound [%d | %d | %d ]\n",mdata->boundary[i][0],mdata->boundary[i][1],mdata->boundary[i][2]);


  }

  static int AlbertHelp_processor = -1;
  // function for mesh_traverse, is called on every element
  inline static void setProcessor(const EL_INFO * elf)
  {
    AlbertLeafData *ldata = (AlbertLeafData *) elf->el->child[1];
    assert(ldata != NULL);

    ldata->processor = AlbertHelp_processor;
  }

  // remember on which level an element realy lives
  inline void initProcessor( MESH * mesh, int proc )
  {
    AlbertHelp_processor = proc;

    // see ALBERT Doc page 72, traverse over all hierarchical elements
    mesh_traverse(mesh,-1, CALL_LEAF_EL|FILL_NOTHING,setProcessor);

    AlbertHelp_processor = -1;
  }

  // function for mesh_traverse, is called on every element
  inline static void setElOwner(const EL_INFO * elf)
  {
    const DOF_ADMIN * admin = elOwner->fe_space->admin;
    const int nv = admin->n0_dof[CENTER];
    const int k  = admin->mesh->node[CENTER];
    int *vec = NULL;
    EL * el = elf->el;
    AlbertLeafData *ldata = (AlbertLeafData *) elf->el->child[1];
    assert(ldata != NULL);

    GET_DOF_VEC(vec,elOwner);

    int proc = vec[el->dof[k][nv]];
    int processor = ldata->processor;
    //printf("proc %d \n",proc);

    if((proc == processor) || (proc < 0)) return;

    for(int i=0; i<N_NEIGH; i++)
    {
      EL * nb = NEIGH(el,elf)[i];
      if(nb)
      {
        if(vec[nb->dof[k][nv]] == processor)
          return;
      }
    }
    vec[el->dof[k][nv]] = -1;
  }

  // remember on which level an element realy lives
  inline void setElOwnerNew( MESH * mesh, DOF_INT_VEC * elOwn )
  {
    elOwner = elOwn;
    assert(elOwner != NULL);
    // see ALBERT Doc page 72, traverse over all hierarchical elements
    mesh_traverse(mesh,-1, CALL_LEAF_EL|FILL_NEIGH,setElOwner);
    elOwner = NULL;
  }

#include "part.cc"
} // end namespace AlbertHelp

#ifdef __ALBERT++ __
} // end namespace Albert
#endif

#endif  /* !_ALBERTEXTRA_H_ */
