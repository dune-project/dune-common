// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
/****************************************************************************/
/*  Header--File for extra Albert Functions                                 */
/****************************************************************************/
#ifndef _ALBERTEXTRA_H_
#define _ALBERTEXTRA_H_

#define ALBERT_ERROR          ALBERT print_error_funcname(funcName, __FILE__, __LINE__),\
  ALBERT print_error_msg
#define ALBERT_ERROR_EXIT     ALBERT print_error_funcname(funcName, __FILE__, __LINE__),\
  ALBERT print_error_msg_exit

#define ALBERT_TEST_EXIT(test) if ((test)) ;else ALBERT_ERROR_EXIT


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
    std::cout << "ManageTravStack::operator = do not use!\n";
    abort();
    if(copy.stack_)
    {
      std::cout << "Copy Stack! \n";
      (*copy.refCount_)++;
      stack_ = copy.stack_;
      refCount_ = copy.refCount_;
    }

    return (*this);
  }
};


static TRAVERSE_STACK *freeStack = NULL;
static int stackCount=0;

static inline void initTraverseStack(TRAVERSE_STACK *stack);

static TRAVERSE_STACK *getTraverseStack(void)
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

static TRAVERSE_STACK *freeTraverseStack(TRAVERSE_STACK *stack)
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

#if 0
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
#endif

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

  MSG("Element %d | level %d  | ",elf->el->index,elf->level);
  printf("Neighs: ");
  for(int i=0; i<N_VERTICES; i++)
  {
    ALBERT EL* el = elf->neigh[i];
    if(el) printf(" %d |",el->index);
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
  return coarsen ( mesh );
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
  //  Albert reference element local numbering for 2D
  //
  //****************************************************************
#if 0
  template <int dim>
  const int * Albert2Dune_localNumbers ();


  static const int localNum_2d[3] = {2,0,1};
  // the 2d case , i.e. triangles
  template <>
  const int * Albert2Dune_localNumbers<2> ();
  {
    return localNum_2d;
  }

  static const int localNum_3d[4] = {2,0,1};
  // the 2d case , i.e. triangles
  template <>
  const int * Albert2Dune_localNumbers<3> ();
  {
    return localNum_2d;
  }
#endif

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
  static bool doItAgain = false;
  static std::vector<int> *Albert_neighArray_help;

  // function for mesh_traverse, is called on every element
  inline static void calcMaxLevel (const EL_INFO * elf)
  {
    int level = elf->level;
    int index = elf->el->index;

    if(index > Albert_GlobalIndex_help) Albert_GlobalIndex_help = index;
    if(Albert_neighArray_help->size() <= index)
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
  inline int calcMaxLevelAndMarkNeighbours ( MESH * mesh, std::vector< int > &nb, int & GlobalIndex )
  {
    // determine new maxlevel
    nb.resize( 2 * mesh->n_hier_elements );

    Albert_neighArray_help = &nb;
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

    if(Albert_MaxLevel_help == -1)
    {
      std::cerr << "Error: in calcMaxLevelAndMarkNeighbours!\n";
      abort();
    }
    GlobalIndex = Albert_GlobalIndex_help+1;
    return Albert_MaxLevel_help;
  }

  //**************************************************************************
  inline static void printNeighbour (const EL_INFO * elf)
  {
    int i;
    printf("%d EL \n",elf->el->index);
    for(i=0; i<3; i++)
      if(elf->neigh[i])
        printf("%d Neigh \n",elf->neigh[i]->index);
      else printf("%d Neigh \n",-1);
    printf("----------------------------------\n");
  }

  //*********************************************************************

  // Leaf Data for Albert, only the leaf elements have this data set
  typedef struct {} AlbertLeafData;

  inline static void AlbertLeafRefine(EL *parent, EL *child[2]) {}
  inline static void AlbertLeafCoarsen(EL *parent, EL *child[2]){}

  // we dont need Leaf Data
  inline static void initLeafData(LEAF_DATA_INFO * linfo)
  {
    linfo->leaf_data_size = sizeof(AlbertLeafData);
    linfo->refine_leaf_data = &AlbertLeafRefine;
    linfo->coarsen_leaf_data =&AlbertLeafCoarsen;
    return;
  }

  // initialize dofAdmin for vertex numbering
  void initDofAdmin(MESH *mesh)
  {
    FUNCNAME("initDofAdmin");
    int degree = 1;
    const BAS_FCTS  *lagrange;
    const FE_SPACE  *feSpace;

    // just for vertex numbering
    lagrange = get_lagrange(degree);
    TEST_EXIT(lagrange) ("no lagrange BAS_FCTS\n");
    feSpace = get_fe_space(mesh, "Linear Lagrangian Elements", NULL, lagrange);

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

} // end namespace AlbertHelp



#endif  /* !_ALBERTEXTRA_H_ */
