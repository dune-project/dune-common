// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "config.h"

#include "../onedgrid.hh"

// ///////////////////////////////////////////////////////////////
//
//    OneDGridLevelIteratorFactory, a class used to simulate
//    specialization of member templates
//
// ///////////////////////////////////////////////////////////////

namespace Dune {

  template <int codim>
  struct OneDGridLevelIteratorFactory {};

  template <>
  struct OneDGridLevelIteratorFactory<1>
  {
    template <Dune::PartitionIteratorType PiType>
    static Dune::OneDGridLevelIterator<1,PiType, const Dune::OneDGrid<1,1> >
    lbegin(const Dune::OneDGrid<1,1> * g, int level) {
      if (level<0 || level>g->maxLevel())
        DUNE_THROW(Dune::GridError, "LevelIterator in nonexisting level " << level << " requested!");

      return Dune::OneDGridLevelIterator<1,PiType, const Dune::OneDGrid<1,1> >(g->vertices[level].begin);
    }

  };

  template <>
  struct OneDGridLevelIteratorFactory<0>
  {
    template <Dune::PartitionIteratorType PiType>
    static Dune::OneDGridLevelIterator<0,PiType, const Dune::OneDGrid<1,1> >
    lbegin(const Dune::OneDGrid<1,1> * g, int level) {
      if (level<0 || level>g->maxLevel())
        DUNE_THROW(Dune::GridError, "LevelIterator in nonexisting level " << level << " requested!");

      return Dune::OneDGridLevelIterator<0,PiType, const Dune::OneDGrid<1,1> >(g->elements[level].begin);
    }

  };

}


template <int dim, int dimworld>
Dune::OneDGrid<dim,dimworld>::OneDGrid(int numElements, double leftBoundary, double rightBoundary)
  : refinementType_(LOCAL),
    leafIndexSet_(*this),
    idSet_(*this),
    freeVertexIdCounter_(0),
    freeElementIdCounter_(0)
{
  typedef const OneDGrid<dim,dimworld> GridImp;

  // Init grid hierarchy
  vertices.resize(1);
  elements.resize(1);

  // Init vertex set
  for (int i=0; i<numElements+1; i++) {
    double newCoord = leftBoundary + i*(rightBoundary-leftBoundary) / numElements;

    OneDEntityImp<0>* newVertex = new OneDEntityImp<0>(0, newCoord);
    newVertex->id_ = getNextFreeId(1);
    vertices[0].insert_after(vertices[0].rbegin, newVertex);

  }

  // Init element set
  OneDEntityImp<0>* it = vertices[0].begin;
  for (int i=0; i<numElements; i++) {

    OneDEntityImp<1>* newElement = new OneDEntityImp<1>(0, getNextFreeId(0));
    newElement->vertex_[0] = it;
    it = it->succ_;
    newElement->vertex_[1] = it;

    elements[0].insert_after(elements[0].rbegin, newElement);

  }

  setIndices();
}

template <int dim, int dimworld>
Dune::OneDGrid<dim,dimworld>::OneDGrid(const SimpleVector<OneDCType>& coords)
  : refinementType_(LOCAL),
    leafIndexSet_(*this),
    idSet_(*this),
    freeVertexIdCounter_(0),
    freeElementIdCounter_(0)
{
  // Init grid hierarchy
  vertices.resize(1);
  elements.resize(1);

  // Init vertex set
  for (int i=0; i<coords.size(); i++) {
    OneDEntityImp<0>* newVertex = new OneDEntityImp<0>(0, coords[i], getNextFreeId(1));
    vertices[0].insert_after(vertices[0].rbegin, newVertex);
  }

  // Init element set
  OneDEntityImp<0>* it = vertices[0].begin;
  for (int i=0; i<coords.size()-1; i++) {

    OneDEntityImp<1>* newElement = new OneDEntityImp<1>(0, getNextFreeId(0));
    newElement->vertex_[0] = it;
    it = it->succ_;
    newElement->vertex_[1] = it;

    elements[0].insert_after(elements[0].rbegin, newElement);

  }

  setIndices();
}


template <int dim, int dimworld>
Dune::OneDGrid<dim,dimworld>::~OneDGrid()
{
  typedef const OneDGrid<dim,dimworld> GridImp;

  // Delete all vertices
  for (unsigned int i=0; i<vertices.size(); i++) {

    OneDEntityImp<0>* v = vertices[i].begin;

    while (v) {

      OneDEntityImp<0>* vSucc = v->succ_;
      vertices[i].remove(v);
      delete(v);
      v = vSucc;

    }

  }

  // Delete all elements
  for (unsigned int i=0; i<elements.size(); i++) {

    OneDEntityImp<1>* e = elements[i].begin;

    while (e) {

      OneDEntityImp<1>* eSucc = e->succ_;
      elements[i].remove(e);
      delete(e);
      e = eSucc;

    }

  }

  // Delete levelIndexSets
  for (unsigned int i=0; i<levelIndexSets_.size(); i++)
    if (levelIndexSets_[i])
      delete levelIndexSets_[i];
}

template <int dim, int dimworld>
template <int codim>
typename Dune::OneDGrid<dim,dimworld>::Traits::template Codim<codim>::LevelIterator
Dune::OneDGrid<dim,dimworld>::lbegin(int level) const
{
  return OneDGridLevelIteratorFactory<codim>::template lbegin<All_Partition>(this, level);
}

template <int dim, int dimworld>
template <int codim>
typename Dune::OneDGrid<dim,dimworld>::Traits::template Codim<codim>::LevelIterator
Dune::OneDGrid<dim,dimworld>::lend(int level) const
{
  if (level<0 || level>maxLevel())
    DUNE_THROW(GridError, "LevelIterator in nonexisting level " << level << " requested!");

  OneDGridLevelIterator<codim,All_Partition, const Dune::OneDGrid<dim,dimworld> > it(0);
  return it;
}

template <int dim, int dimworld>
template <int codim, Dune::PartitionIteratorType PiType>
typename Dune::OneDGrid<dim,dimworld>::Traits::template Codim<codim>::template Partition<PiType>::LevelIterator
Dune::OneDGrid<dim,dimworld>::lbegin(int level) const
{
  return OneDGridLevelIteratorFactory<codim>::template lbegin<PiType>(this, level);
}

template <int dim, int dimworld>
template <int codim, Dune::PartitionIteratorType PiType>
typename Dune::OneDGrid<dim,dimworld>::Traits::template Codim<codim>::template Partition<PiType>::LevelIterator
Dune::OneDGrid<dim,dimworld>::lend(int level) const
{
  if (level<0 || level>maxLevel())
    DUNE_THROW(GridError, "LevelIterator in nonexisting level " << level << " requested!");

  OneDGridLevelIterator<codim,PiType, const Dune::OneDGrid<dim,dimworld> > it(0);
  return it;
}


template <int dim, int dimworld>
template <int codim>
typename Dune::OneDGrid<dim,dimworld>::Traits::template Codim<codim>::LeafIterator
Dune::OneDGrid<dim,dimworld>::leafbegin() const
{
  return OneDGridLeafIterator<codim,All_Partition,const OneDGrid<dim,dimworld> >(*this);
}

template <int dim, int dimworld>
template <int codim>
typename Dune::OneDGrid<dim,dimworld>::Traits::template Codim<codim>::LeafIterator
Dune::OneDGrid<dim,dimworld>::leafend() const
{
  return OneDGridLeafIterator<codim,All_Partition, const Dune::OneDGrid<dim,dimworld> >();
}

template <int dim, int dimworld>
Dune::OneDEntityImp<0>*
Dune::OneDGrid<dim,dimworld>::getLeftUpperVertex(const OneDEntityImp<1>* eIt)
{
  OneDEntityImp<1>* l = eIt->pred_;

  if (!l)
    return 0;

  // return NULL if there is no geometrical left neighbor
  if (l->vertex_[1] != eIt->vertex_[0])
    return 0;

  // return NULL if that neighbor doesn't have sons
  if (l->isLeaf())
    return 0;

  // return the right vertex of the right son
  return l->sons_[1]->vertex_[1];

}

template <int dim, int dimworld>
Dune::OneDEntityImp<0>*
Dune::OneDGrid<dim,dimworld>::getRightUpperVertex(const OneDEntityImp<1>* eIt)
{
  OneDEntityImp<1>* r = eIt->succ_;

  if (!r)
    return 0;

  // return NULL if there is no geometrical right neighbor
  if (r->vertex_[0]!=eIt->vertex_[1])
    return 0;

  // return NULL if that neighbor doesn't have sons
  if (r->isLeaf())
    return 0;

  // return the left vertex of the left son
  return r->sons_[0]->vertex_[0];

}

template <int dim, int dimworld>
Dune::OneDEntityImp<1>*
Dune::OneDGrid<dim,dimworld>::getLeftNeighborWithSon(OneDEntityImp<1>* eIt)
{
  OneDEntityImp<1>* l = eIt;

  do {
    l = l->pred_;
  } while (l && l->isLeaf());

  return l;
}


template <int dim, int dimworld>
bool Dune::OneDGrid<dim,dimworld>::adapt()
{
  typedef const OneDGrid<dim,dimworld> GridImp;

  OneDEntityImp<1>* eIt;

  // for the return value:  true if the grid was changed
  bool changedGrid = false;

  // remove all elements that have been marked for coarsening
  for (int i=1; i<=maxLevel(); i++) {

    for (eIt = elements[i].begin; eIt!=NULL; eIt = eIt->succ_) {

      if (eIt->markState_ == COARSEN)
        DUNE_THROW(NotImplemented, "Coarsening is not implemented for OneDGrid");
#if 0
      if (eIt->markState_ == COARSEN
          && !eIt->isLeaf()) {

        OneDEntityImp<1>* leftElement = eIt->pred_;

        OneDEntityImp<1>* rightElement = eIt->succ_;

        // Is the left vertex obsolete?
        if (leftElement->geo_.vertex(1) != eIt->geo_.vertex(0))
          vertices[i].erase(eIt->geo_.vertex(0));

        // Is the right vertex obsolete?
        if (rightElement->geo_.vertex(0) != eIt->geo_.vertex(1))
          vertices[i].erase(eIt->geo_.vertex(1));

        // Remove reference from the father element
        if (eIt->father()->sons_[0] == eIt)
          eIt->father()->sons_[0] = elements[i].end();
        else {
          assert (eIt->father()->sons_[1] == eIt);
          eIt->father()->sons_[1] = elements[i].end();
        }

        // Actually delete element
        elements[i].erase(eIt);

        // The grid has been changed
        changedGrid = true;

      }
#endif
    }

  }

  // /////////////////////////////////////////////////////////////////////////
  //  Check if one of the elements on the toplevel is marked for refinement
  //  In that case add another level
  // /////////////////////////////////////////////////////////////////////////
  bool toplevelRefinement = false;
  for (eIt = elements[maxLevel()].begin; eIt!=NULL; eIt=eIt->succ_)
    if (eIt->markState_ == REFINED) {
      toplevelRefinement = true;
      break;
    }

  if (toplevelRefinement) {
    List<OneDEntityImp<0> > newVertices;
    List<OneDEntityImp<1> > newElements;
    vertices.push_back(newVertices);
    elements.push_back(newElements);
  }

  // //////////////////////////////
  // refine all marked elements
  // //////////////////////////////
  int oldMaxlevel = (toplevelRefinement) ? maxLevel()-1 : maxLevel();
  for (int i=0; i<=oldMaxlevel; i++) {

    for (eIt = elements[i].begin; eIt!=NULL; eIt = eIt->succ_) {

      if (eIt->markState_ == REFINED
          && eIt->isLeaf()) {

        // Does the left vertex exist on the next-higher level?
        // If no create it
        OneDEntityImp<0>* leftUpperVertex = getLeftUpperVertex(eIt);

        if (leftUpperVertex==NULL)
          leftUpperVertex = new OneDEntityImp<0>(i+1, eIt->vertex_[0]->pos_, getNextFreeId(1));

        eIt->vertex_[0]->son_ = leftUpperVertex;

        // Does the right vertex exist on the next-higher level?
        // If no create it
        OneDEntityImp<0>* rightUpperVertex = getRightUpperVertex(eIt);

        if (rightUpperVertex==NULL)
          rightUpperVertex = new OneDEntityImp<0>(i+1, eIt->vertex_[1]->pos_, getNextFreeId(1));

        eIt->vertex_[1]->son_ = rightUpperVertex;

        // Create center vertex
        double a = eIt->vertex_[0]->pos_[0];
        double b = eIt->vertex_[1]->pos_[0];
        double p = 0.5*(a+b);

        // The following line didn't give the correct result.  And I dont't know why
        //double p = 0.5*(eIt->geometry()[0][0] + eIt->geometry()[1][0]);
        //                 std::cout << "left: " << eIt->geometry()[0][0]
        //                           << " right: " << eIt->geometry()[1][0] << " p: " << p << std::endl;

        OneDEntityImp<0>* centerVertex = new OneDEntityImp<0>(i+1, p, getNextFreeId(1));

        // //////////////////////////////////////
        // Insert new vertices into vertex list
        // //////////////////////////////////////

        OneDEntityImp<1>* leftNeighbor = getLeftNeighborWithSon(eIt);

        if (leftNeighbor!=NULL) {

          // leftNeighbor exists
          if ( leftNeighbor->sons_[1]->vertex_[1] != leftUpperVertex)
            vertices[i+1].insert_after(leftNeighbor->sons_[1]->vertex_[1], leftUpperVertex);

        } else {
          // leftNeighbor does not exist
          vertices[i+1].insert_before(vertices[i+1].begin, leftUpperVertex);

        }

        vertices[i+1].insert_after(leftUpperVertex, centerVertex);

        // Check if rightUpperVertex is already in the list
        OneDEntityImp<0>* succOfCenter = centerVertex->succ_;

        if (succOfCenter==NULL || succOfCenter != rightUpperVertex)
          vertices[i+1].insert_after(centerVertex, rightUpperVertex);

        // ///////////////////////
        // Create new elements
        // ///////////////////////
        OneDEntityImp<1>* newElement0 = new OneDEntityImp<1>(i+1, getNextFreeId(0));
        newElement0->vertex_[0] = leftUpperVertex;
        newElement0->vertex_[1] = centerVertex;
        newElement0->father_ = eIt;
        newElement0->adaptationState_ = REFINED;

        OneDEntityImp<1>* newElement1 = new OneDEntityImp<1>(i+1, getNextFreeId(0));
        newElement1->vertex_[0] = centerVertex;
        newElement1->vertex_[1] = rightUpperVertex;
        newElement1->father_ = eIt;
        newElement1->adaptationState_ = REFINED;

        // Insert new elements into element list
        if (leftNeighbor!=NULL)
          // leftNeighbor exists
          elements[i+1].insert_after(leftNeighbor->sons_[1], newElement0);
        else
          // leftNeighbor does not exist
          elements[i+1].insert_before(elements[i+1].begin, newElement0);

        elements[i+1].insert_after(newElement0, newElement1);

        // Mark the two new elements as the sons of the refined element
        eIt->sons_[0] = newElement0;
        eIt->sons_[1] = newElement1;

        // The grid has been modified
        changedGrid = true;

      }

    }

  }

  // delete uppermost level if it doesn't contain elements anymore
  if (elements[maxLevel()].size()==0) {
    assert(vertices[maxLevel()].size()==0);
    elements.pop_back();
    vertices.pop_back();
  }


  // If the refinement mode is 'COPY', fill the empty slots in the grid
  // by copying elements
  if (refinementType_ == COPY) {

    for (int i=0; i<maxLevel(); i++) {

      OneDEntityImp<1>* eIt;
      for (eIt = elements[i].begin; eIt!=NULL; eIt = eIt->succ_) {

        //printf("level %d, element %d \n", i, eIt->index());
        if (eIt->isLeaf()) {

          // Does the left vertex exist on the next-higher level?
          // If no create it
          OneDEntityImp<0>* leftUpperVertex = getLeftUpperVertex(eIt);

          if (leftUpperVertex==NULL)
            leftUpperVertex = new OneDEntityImp<0>(i+1, eIt->vertex_[0]->pos_, getNextFreeId(1));

          eIt->vertex_[0]->son_ = leftUpperVertex;

          // Does the right vertex exist on the next-higher level?
          // If no create it
          OneDEntityImp<0>* rightUpperVertex = getRightUpperVertex(eIt);

          if (rightUpperVertex==NULL)
            rightUpperVertex = new OneDEntityImp<0>(i+1, eIt->vertex_[1]->pos_, getNextFreeId(1));

          eIt->vertex_[1]->son_ = rightUpperVertex;

          // //////////////////////////////////////
          // Insert new vertices into vertex list
          // //////////////////////////////////////

          OneDEntityImp<1>* leftNeighbor = getLeftNeighborWithSon(eIt);

          if (leftNeighbor!=NULL) {

            // leftNeighbor exists
            if ( leftNeighbor->sons_[1]->vertex_[1] != leftUpperVertex)
              vertices[i+1].insert_after(leftNeighbor->sons_[1]->vertex_[1], leftUpperVertex);

          } else {
            // leftNeighbor does not exist
            vertices[i+1].insert_before(vertices[i+1].begin, leftUpperVertex);

          }

          // Check if rightUpperVertex is already in the list
          OneDEntityImp<0>* succOfLeft = leftUpperVertex->succ_;

          if (succOfLeft==NULL || succOfLeft != rightUpperVertex)
            vertices[i+1].insert_after(leftUpperVertex, rightUpperVertex);

          // /////////////////////////
          //   Create new element
          // /////////////////////////
          OneDEntityImp<1>* newElement = new OneDEntityImp<1>(i+1, getNextFreeId(0));
          newElement->vertex_[0] = leftUpperVertex;
          newElement->vertex_[1] = rightUpperVertex;
          newElement->father_ = eIt;
          newElement->adaptationState_ = REFINED;

          // Insert new elements into element list
          if (leftNeighbor!=NULL)
            // leftNeighbor exists
            elements[i+1].insert_after(leftNeighbor->sons_[1], newElement);
          else
            // leftNeighbor does not exist
            elements[i+1].insert_before(elements[i+1].begin, newElement);

          // Mark the new element as the sons of the refined element
          eIt->sons_[0] = eIt->sons_[1] = newElement;

        }

      }

    }

  }

  // ////////////////////////////////////
  //   renumber vertices and elements
  // ////////////////////////////////////
  setIndices();

  return changedGrid;
}

template < int dim, int dimworld >
void Dune::OneDGrid < dim, dimworld >::setIndices()
{
  for (int i=levelIndexSets_.size(); i<maxLevel()+1; i++)
    levelIndexSets_.push_back(0);

  for (int i=0; i<=maxLevel(); i++)
    if (levelIndexSets_[i])
      levelIndexSets_[i]->update(*this, i);

  leafIndexSet_.update();

  idSet_.update();

}

template <int dim, int dimworld>
void Dune::OneDGrid<dim,dimworld>::globalRefine(int refCount)
{
  // mark all entities for grid refinement
  typename Traits::template Codim<0>::LevelIterator iIt    = lbegin<0>(maxLevel());
  typename Traits::template Codim<0>::LevelIterator iEndIt = lend<0>(maxLevel());

  for (; iIt!=iEndIt; ++iIt)
    mark(1, iIt);

  this->preAdapt();
  adapt();
  this->postAdapt();
}

template <int dim, int dimworld>
bool Dune::OneDGrid < dim, dimworld >::mark(int refCount,
                                            const typename Traits::template Codim<0>::EntityPointer & e )
{
  if (refCount < 0) {

    if (getRealEntity<0>(*e).target_->level_ == 0)
      return false;
    else {
      getRealEntity<0>(*e).target_->markState_ = COARSEN;
      return true;
    }

  } else if (refCount > 0)
    getRealEntity<0>(*e).target_->markState_ = REFINED;
  else
    getRealEntity<0>(*e).target_->markState_ = NONE;

  return true;
}

// /////////////////////////////////////////////////////////////////////////
//   Explicitly instantiate the OnedGrid for dim == dimworld == 1,
//   which is the only valid instantiation
//   gcc-4.0 wants these instantiations after the method implementations
// /////////////////////////////////////////////////////////////////////////
template class Dune::OneDGrid<1,1>;

// Explicitly instantiate the necessary member templates contained in OneDGrid<1,1>
template Dune::OneDGrid<1,1>::Traits::Codim<0>::LevelIterator Dune::OneDGrid<1,1>::lbegin<0>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::LevelIterator Dune::OneDGrid<1,1>::lbegin<1>(int level) const;

template Dune::OneDGrid<1,1>::Traits::Codim<0>::LevelIterator Dune::OneDGrid<1,1>::lend<0>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::LevelIterator Dune::OneDGrid<1,1>::lend<1>(int level) const;

template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::Interior_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lbegin<0,Dune::Interior_Partition>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::InteriorBorder_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lbegin<0,Dune::InteriorBorder_Partition>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::Overlap_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lbegin<0,Dune::Overlap_Partition>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::OverlapFront_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lbegin<0,Dune::OverlapFront_Partition>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::All_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lbegin<0,Dune::All_Partition>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::Ghost_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lbegin<0,Dune::Ghost_Partition>(int level) const;

template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::Interior_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lend<0,Dune::Interior_Partition>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::InteriorBorder_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lend<0,Dune::InteriorBorder_Partition>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::Overlap_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lend<0,Dune::Overlap_Partition>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::OverlapFront_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lend<0,Dune::OverlapFront_Partition>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::All_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lend<0,Dune::All_Partition>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::Ghost_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lend<0,Dune::Ghost_Partition>(int level) const;

template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::Interior_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lbegin<1,Dune::Interior_Partition>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::InteriorBorder_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lbegin<1,Dune::InteriorBorder_Partition>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::Overlap_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lbegin<1,Dune::Overlap_Partition>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::OverlapFront_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lbegin<1,Dune::OverlapFront_Partition>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::All_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lbegin<1,Dune::All_Partition>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::Ghost_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lbegin<1,Dune::Ghost_Partition>(int level) const;

template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::Interior_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lend<1,Dune::Interior_Partition>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::InteriorBorder_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lend<1,Dune::InteriorBorder_Partition>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::Overlap_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lend<1,Dune::Overlap_Partition>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::OverlapFront_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lend<1,Dune::OverlapFront_Partition>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::All_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lend<1,Dune::All_Partition>(int level) const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::Ghost_Partition>::LevelIterator
Dune::OneDGrid<1,1>::lend<1,Dune::Ghost_Partition>(int level) const;

template Dune::OneDGrid<1,1>::Traits::Codim<0>::LeafIterator Dune::OneDGrid<1,1>::leafbegin<0>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::LeafIterator Dune::OneDGrid<1,1>::leafbegin<1>() const;

template Dune::OneDGrid<1,1>::Traits::Codim<0>::LeafIterator Dune::OneDGrid<1,1>::leafend<0>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::LeafIterator Dune::OneDGrid<1,1>::leafend<1>() const;

template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::Interior_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafbegin<0,Dune::Interior_Partition>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::InteriorBorder_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafbegin<0,Dune::InteriorBorder_Partition>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::Overlap_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafbegin<0,Dune::Overlap_Partition>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::OverlapFront_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafbegin<0,Dune::OverlapFront_Partition>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::All_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafbegin<0,Dune::All_Partition>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::Ghost_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafbegin<0,Dune::Ghost_Partition>() const;

template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::Interior_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafend<0,Dune::Interior_Partition>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::InteriorBorder_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafend<0,Dune::InteriorBorder_Partition>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::Overlap_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafend<0,Dune::Overlap_Partition>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::OverlapFront_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafend<0,Dune::OverlapFront_Partition>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::All_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafend<0,Dune::All_Partition>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<0>::Partition<Dune::Ghost_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafend<0,Dune::Ghost_Partition>() const;

template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::Interior_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafbegin<1,Dune::Interior_Partition>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::InteriorBorder_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafbegin<1,Dune::InteriorBorder_Partition>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::Overlap_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafbegin<1,Dune::Overlap_Partition>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::OverlapFront_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafbegin<1,Dune::OverlapFront_Partition>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::All_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafbegin<1,Dune::All_Partition>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::Ghost_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafbegin<1,Dune::Ghost_Partition>() const;

template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::Interior_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafend<1,Dune::Interior_Partition>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::InteriorBorder_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafend<1,Dune::InteriorBorder_Partition>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::Overlap_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafend<1,Dune::Overlap_Partition>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::OverlapFront_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafend<1,Dune::OverlapFront_Partition>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::All_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafend<1,Dune::All_Partition>() const;
template Dune::OneDGrid<1,1>::Traits::Codim<1>::Partition<Dune::Ghost_Partition>::LeafIterator
Dune::OneDGrid<1,1>::leafend<1,Dune::Ghost_Partition>() const;
