// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_UGHIERITERATOR_HH__
#define __DUNE_UGHIERITERATOR_HH__

//**********************************************************************
//
// --UGGridHierarchicIterator
// --HierarchicIterator
/*!
   Mesh entities of codimension 0 ("elements") allow to visit all entities of
   codimension 0 obtained through nested, hierarchic refinement of the entity.
   Iteration over this set of entities is provided by the HIerarchicIterator,
   starting from a given entity.
   This is redundant but important for memory efficient implementations of unstru
   hierarchically refined meshes.
 */

template<int dim, int dimworld>
class UGGridHierarchicIterator :
  public HierarchicIteratorDefault <dim,dimworld, UGCtype,
      UGGridHierarchicIterator,UGGridEntity>
{
public:

#if 1

#if 0
  //! the normal Constructor
  UGGridHierarchicIterator(UGGrid<dim,dimworld> &grid,
                           ALBERT TRAVERSE_STACK *travStack, int actLevel, int maxLevel);
#endif

  //! the default Constructor
  UGGridHierarchicIterator(UGGrid<dim,dimworld> &grid,
                           int actLevel,int maxLevel);
#else
  //! the normal Constructor
  UGGridHierarchicIterator(UGGrid<dim,dimworld> &grid,
                           ALBERT TRAVERSE_STACK *travStack, int travLevel);

  //! the default Constructor
  UGGridHierarchicIterator(UGGrid<dim,dimworld> &grid);
#endif

  //! prefix increment
  UGGridHierarchicIterator& operator ++();

  //! postfix increment
  UGGridHierarchicIterator& operator ++(int i);

  //! equality
  bool operator== (const UGGridHierarchicIterator& i) const;

  //! inequality
  bool operator!= (const UGGridHierarchicIterator& i) const;

  //! dereferencing
  UGGridEntity<0,dim,dimworld>& operator*();

  //! arrow
  UGGridEntity<0,dim,dimworld>* operator->();

private:
  //! implement with virtual element
  UGGridEntity<0,dim,dimworld> virtualEntity_;

  //! know the grid were im comming from
  UGGrid<dim,dimworld> &grid_;

  //! the actual Level of this Hierarichic Iterator
  int level_;

  //! max level to go down
  //int maxlevel_;

#if 0
  //! we need this for Albert traversal, and we need ManageTravStack, which
  //! does count References when copied
  ALBERT ManageTravStack manageStack_;

  //! The nessesary things for Albert
  ALBERT EL_INFO * recursiveTraverse(ALBERT TRAVERSE_STACK * stack);
#endif

  //! make empty HierarchicIterator
  void makeIterator();
};


#endif
