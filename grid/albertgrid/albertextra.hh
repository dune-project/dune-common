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

//! organize the TRAVERSE_STACK Management, so we can use the nice Albert
//! fucntions get_traverse_stack and free_traverse_stack
//! this count the copy made of this class and call free_traverse_stack
//! only if no more copies left
class ManageTravStack
{
  TRAVERSE_STACK * stack_;
  int *refCount_;

public:
  ManageTravStack(const ManageTravStack & copy)
  {
    //    std::cout << "ManageTravStack CopyConstructor.... ";
    stack_ = copy.stack_;
    if(copy.stack_ != NULL)
    {
      (*const_cast<ManageTravStack &> (copy).refCount_)++;
    }
    refCount_ = copy.refCount_;
    //    std::cout << "done \n";
  }

  ManageTravStack()
  {
    init();
  }

  void makeItNew(bool realyMakeIt)
  {
    //    std::cout << "Makeing new ManageStack ....";
    if(realyMakeIt)
    {
      stack_ = get_traverse_stack();
      int *tmp = new int;
      refCount_ = tmp;
      (*refCount_) = 1;
    }
    //    std::cout << "done \n";
  }

  void init()
  {
    //    std::cout << "ManageTravStack init .....";
    stack_ = NULL;
    refCount_ = NULL;
    //    std::cout << "done \n";
  }

  // set Stack free, if no more refences exist
  ~ManageTravStack()
  {
    if(refCount_)
    {
      //      std::cout << "Destructor called .....";
      (*refCount_)--;
      //std::cout << "RefCount for " << stack_ << " is " << (*refCount_) << std::endl;
      if((*refCount_) <= 0)
      {
        //std::cout << "Call free_travserse_stack \n";
        free_traverse_stack(stack_);
        stack_ = NULL;
        if(refCount_) delete refCount_ ;
      }
      //      std::cout << "done \n";
    }
  }

  ManageTravStack & operator = (const ManageTravStack & copy)
  {
    //std::cout << "operator = .........";
    (*copy.refCount_)++;
    stack_ = copy.stack_;
    refCount_ = copy.refCount_;
    //std::cout << "done \n";
    return (*this);
  }

  TRAVERSE_STACK * getStack()
  {
    return stack_;
  }

};

TRAVERSE_STACK & hardCopyStack(TRAVERSE_STACK& copy, TRAVERSE_STACK& org)
{
  FUNCNAME("hardCopyStack");

  copy.traverse_mesh = org.traverse_mesh;
  copy.traverse_level = org.traverse_level;
  copy.traverse_fill_flag = org.traverse_fill_flag;
  copy.traverse_mel = org.traverse_mel;

  copy.stack_size = org.stack_size;
  copy.stack_used = org.stack_used;

  copy.elinfo_stack = NULL;
  copy.elinfo_stack = MEM_ALLOC(org.stack_size, EL_INFO);

  memcpy(copy.elinfo_stack,org.elinfo_stack,
         copy.stack_size * sizeof(EL_INFO));

  //if (copy.stack_size > 0)
  //  for (int i=0; i<copy.stack_size; i++)
  //    copy.elinfo_stack[i].fill_flag = org.elinfo_stack[i].fill_flag;

  copy.info_stack = NULL;
  copy.info_stack = MEM_ALLOC(copy.stack_size, U_CHAR);
  memcpy(copy.info_stack,org.info_stack,
         copy.stack_size * sizeof(U_CHAR));

  copy.save_elinfo_stack = NULL;
  copy.save_elinfo_stack = MEM_ALLOC(copy.stack_size, EL_INFO);
  memcpy(copy.save_elinfo_stack,org.save_elinfo_stack,
         copy.stack_size * sizeof(EL_INFO));

  copy.save_info_stack = NULL;
  copy.save_info_stack = MEM_ALLOC(copy.stack_size,U_CHAR);
  memcpy(copy.save_info_stack,org.save_info_stack,
         copy.stack_size * sizeof(U_CHAR));
  copy.save_stack_used = org.save_stack_used;
  copy.el_count = org.el_count;
  copy.next = org.next;
  org.next = &copy;

  return copy;
}

TRAVERSE_STACK & removeStack(TRAVERSE_STACK& copy)
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


void initTraverseStack(TRAVERSE_STACK *stack)
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


void enlargeTraverseStack(TRAVERSE_STACK *stack)
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

#endif  /* !_ALBERTEXTRA_H_ */
