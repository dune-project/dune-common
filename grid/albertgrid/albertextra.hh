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

  MSG("increase stack at %8X from %d to %d\n", stack,
      stack->stack_size, new_stack_size);


  stack->stack_size = new_stack_size;
}

#endif  /* !_ALBERTEXTRA_H_ */
