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
extern void free_leaf_data(void *leaf_data, MESH *mesh);
extern void free_dof(DOF *dof, MESH *mesh, int position);

void enlargeTraverseStack(TRAVERSE_STACK *stack);

//! organize the TRAVERSE_STACK Management, so we can use the nice Albert
//! fucntions get_traverse_stack and free_traverse_stack
//! this count the copy made of this class and call free_traverse_stack
//! only if no more copies left
class ManageTravStack
{
  //! traverse stack for mesh traverse, see Albert Docu
  TRAVERSE_STACK * stack_;

  //! number of copies that exist from this stack_
  int *refCount_;

public:
  //! if a copy is made, the refcout is increased
  ManageTravStack(const ManageTravStack & copy)
  {
    stack_ = copy.stack_;
    if(copy.stack_ != NULL)
    {
      (*const_cast<ManageTravStack &> (copy).refCount_)++;
    }
    refCount_ = copy.refCount_;
  }

  //! initialize the member variables
  ManageTravStack()
  {
    init();
  }

  //! get new TRAVERSE_STACK using the original Albert Routine
  //! get_traverse_stack, which get an new or free stack
  void makeItNew(bool realyMakeIt)
  {
    if(realyMakeIt)
    {
      stack_ = get_traverse_stack();
      int *tmp = new int;
      refCount_ = tmp;
      (*refCount_) = 1;
    }
  }

  //! initialize the member variables
  void init()
  {
    stack_ = NULL;
    refCount_ = NULL;
  }

  //! set Stack free, if no more refences exist
  ~ManageTravStack()
  {
    if(refCount_)
    {
      (*refCount_)--;
      if((*refCount_) <= 0)
      {
        // in free_traverse_stack stack != NULL is checked
        if(stack_)
        {
          free_traverse_stack(stack_);
          stack_ = NULL;
        }
        if(refCount_) delete refCount_ ;
      }
    }
  }

  //! if copy is made than one more Reference exists
  ManageTravStack & operator = (const ManageTravStack & copy)
  {
    (*copy.refCount_)++;
    stack_ = copy.stack_;
    refCount_ = copy.refCount_;
    return (*this);
  }

  //! return the TRAVERSE_STACK pointer for use
  TRAVERSE_STACK * getStack() const
  {
    return stack_;
  }

};

//! copy all memory entries from org to copy via memcpy
inline void hardCopyStack(TRAVERSE_STACK* copy, TRAVERSE_STACK* org)
{
  copy->traverse_mesh = org->traverse_mesh;
  copy->traverse_level = org->traverse_level;
  copy->traverse_fill_flag = org->traverse_fill_flag;
  copy->traverse_mel = org->traverse_mel;

  copy->stack_size = org->stack_size;
  copy->stack_used = org->stack_used;
  copy->elinfo_stack = NULL;
  copy->elinfo_stack = MEM_ALLOC(org->stack_size, EL_INFO);

  memcpy(copy->elinfo_stack,org->elinfo_stack,
         copy->stack_size * sizeof(EL_INFO));

  copy->info_stack = NULL;
  copy->info_stack = MEM_ALLOC(copy->stack_size, U_CHAR);
  memcpy(copy->info_stack,org->info_stack,
         copy->stack_size * sizeof(U_CHAR));

  copy->save_elinfo_stack = NULL;
  copy->save_elinfo_stack = MEM_ALLOC(copy->stack_size, EL_INFO);
  memcpy(copy->save_elinfo_stack,org->save_elinfo_stack,
         copy->stack_size * sizeof(EL_INFO));

  copy->save_info_stack = NULL;
  copy->save_info_stack = MEM_ALLOC(copy->stack_size,U_CHAR);
  memcpy(copy->save_info_stack,org->save_info_stack,
         copy->stack_size * sizeof(U_CHAR));
  copy->save_stack_used = org->save_stack_used;
  copy->el_count = org->el_count;

  copy->next = org->next;
  org->next = copy;

  return;
}

//! copy all memory entries from org to copy via memcpy
inline void cutHierarchicStack(TRAVERSE_STACK* copy, TRAVERSE_STACK* org)
{
  copy->traverse_mesh = org->traverse_mesh;
  copy->traverse_level = org->traverse_level;
  copy->traverse_fill_flag = org->traverse_fill_flag;
  copy->traverse_mel = org->traverse_mel;

  if(copy->stack_size < 5)
    enlargeTraverseStack(copy);

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

  copy->next = org->next;
  org->next = copy;

  return;
}


inline TRAVERSE_STACK & removeTraverseStack(TRAVERSE_STACK& copy)
{
  FUNCNAME("removeStack");

  if(copy.elinfo_stack)
    MEM_FREE(copy.elinfo_stack, copy.stack_size, EL_INFO);
  copy.elinfo_stack = NULL;

  if(copy.info_stack)
    MEM_FREE(copy.info_stack,copy.stack_size, U_CHAR);
  copy.info_stack = NULL;

  if(copy.save_elinfo_stack)
    MEM_FREE(copy.save_elinfo_stack,
             copy.stack_size, EL_INFO);
  copy.save_elinfo_stack = NULL;

  if(copy.save_info_stack)
    MEM_FREE(copy.save_info_stack,copy.stack_size,U_CHAR);
  copy.save_info_stack = NULL;

  return copy;
}


inline void initTraverseStack(TRAVERSE_STACK *stack)
{
  FUNCNAME("initTraverseStack");

  stack->traverse_mesh = nil;
  stack->stack_size = 0;
  stack->stack_used = 0;
  stack->elinfo_stack = nil;
  stack->info_stack = nil;
  stack->save_elinfo_stack = nil;
  stack->save_info_stack = nil;
  stack->save_stack_used = 0;
  stack->el_count = 0;
  stack->next = nil;

}


inline void enlargeTraverseStack(TRAVERSE_STACK *stack)
{
  FUNCNAME("enlargeTraverseStack");
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
#if 0
  MSG("increase stack at %8X from %d to %d\n", stack,
      stack->stack_size, new_stack_size);
#endif

  stack->stack_size = new_stack_size;
}

void printTraverseStack(TRAVERSE_STACK *stack)
{
  FUNCNAME("printTraverseStack");

  MSG("****************************************************\n");
  MSG("current stack %8X | size %d \n", stack,stack->stack_size);
  MSG("traverse_level %d \n",stack->traverse_level);
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

  MSG("level %d\n",elf->level);
  printf("Neighs: ");
  for(int i=0; i<2; i++)
  {
    ALBERT EL* el = elf->neigh[i];
    if(el)
      printf(" %d |",el->index);
  }
  printf("\n");
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


#endif  /* !_ALBERTEXTRA_H_ */
