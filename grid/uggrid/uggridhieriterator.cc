// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#if 0
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
AlbertGridHierarchicIterator(AlbertGrid<dim,dimworld> &grid,int actLevel,
                             int maxLevel) : grid_(grid), level_ (actLevel)
                                             , maxlevel_ (maxLevel) , virtualEntity_(grid,level_)
{
  makeIterator();
}

template< int dim, int dimworld>
inline AlbertGridHierarchicIterator<dim,dimworld>::
AlbertGridHierarchicIterator(AlbertGrid<dim,dimworld> &grid,
                             ALBERT TRAVERSE_STACK *travStack,int actLevel, int maxLevel) :
  grid_(grid), level_ (actLevel),
  maxlevel_ ( maxLevel), virtualEntity_(grid,level_)
{
  if(travStack)
  {
    // get new ALBERT TRAVERSE STACK
    manageStack_.makeItNew(true);
    ALBERT TRAVERSE_STACK *stack = manageStack_.getStack();

    // cut old traverse stack, kepp only actual element
    cutHierarchicStack(stack, travStack);

    // set new traverse level
    if(maxlevel_ < 0)
    {
      // this means, we go until leaf level
      stack->traverse_fill_flag = CALL_LEAF_EL | stack->traverse_fill_flag;
      // exact here has to stand Grid->maxlevel, but is ok anyway
      maxlevel_ = grid_.maxlevel(); //123456789;
    }
    // set new traverse level
    stack->traverse_level = maxlevel_;

    virtualEntity_.setTraverseStack(stack);
    // Hier kann ein beliebiges Element uebergeben werden,
    // da jedes AlbertElement einen Zeiger auf das Macroelement
    // enthaelt.
    virtualEntity_.setElInfo(recursiveTraverse(stack));

    // set new level
    virtualEntity_.setLevel(level_);
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
  virtualEntity_.setElInfo(recursiveTraverse(manageStack_.getStack()));
  // set new actual level
  virtualEntity_.setLevel(level_);
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
    // go up until we can go down again
    el = stack->elinfo_stack[stack->stack_used].el;

    while((stack->stack_used > 0) &&
          ((stack->info_stack[stack->stack_used] >= 2)
           || ((el->child[0]==NULL) && level_ >= maxlevel_ )
           || ( stack->traverse_level <=
                (stack->elinfo_stack+stack->stack_used)->level)) )
    {
      stack->stack_used--;
      el = stack->elinfo_stack[stack->stack_used].el;
      level_ = stack->elinfo_stack[stack->stack_used].level;
    }

    // goto next father is done by other iterator and not our problem
    if(stack->stack_used < 1)
    {
      return NULL;
    }
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

    // new: go down maxlevel, but fake the elements
    level_++;
    grid_.fillElInfo(i, level_, stack->elinfo_stack+stack->stack_used,
                     stack->elinfo_stack+stack->stack_used+1 ,true);
    //ALBERT fill_elinfo(i, stack->elinfo_stack + stack->stack_used,
    //  stack->elinfo_stack + (stack->stack_used + 1));

    stack->stack_used++;

    stack->info_stack[stack->stack_used] = 0;
  }
  // the case if we have no child but level_ < maxlevel_
  // then we want to fake the next maxlevel_ - level_ elements
  else if(level_ < maxlevel_)
  {
    // new: go down until maxlevel, but fake the not existant elements
    if(stack->stack_used >= stack->stack_size - 1)
      ALBERT enlargeTraverseStack(stack);

    el = el;

    // means all elements visited
    stack->info_stack[stack->stack_used] = 2;

    // new: go down maxlevel, but fake the elements
    level_++;
    grid_.fillElInfo(0, level_, stack->elinfo_stack+stack->stack_used,stack->elinfo_stack+stack->stack_used+1, true);
    //ALBERT fill_elinfo(i, stack->elinfo_stack + stack->stack_used,
    //  stack->elinfo_stack + (stack->stack_used + 1));
    stack->stack_used++;

    stack->info_stack[stack->stack_used] = 0;
  }

  return (stack->elinfo_stack + stack->stack_used);
}  // recursive traverse over all childs

// end AlbertGridHierarchicIterator
#endif
