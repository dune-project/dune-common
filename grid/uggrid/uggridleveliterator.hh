// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRIDLEVELITERATOR_HH
#define DUNE_UGGRIDLEVELITERATOR_HH



//**********************************************************************
//
// --UGGridLevelIterator
// --LevelIterator
/*!
   Enables iteration over all entities of a given codimension and level of a grid.
 */
template<int codim, int dim, int dimworld>
class UGGridLevelIterator :
  public LevelIteratorDefault <codim,dim,dimworld, UGCtype,
      UGGridLevelIterator,UGGridEntity>
{
  friend class UGGridEntity<2,dim,dimworld>;
  friend class UGGridEntity<1,dim,dimworld>;
  friend class UGGridEntity<0,dim,dimworld>;
  friend class UGGrid < dim , dimworld >;
public:

  //friend class UGGrid<dim,dimworld>;

  //! Constructor
  UGGridLevelIterator(UGGrid<dim,dimworld> &grid, int travLevel);

#if 0
  //! Constructor
  UGGridLevelIterator(UGGrid<dim,dimworld> &grid,
                      ALBERT EL_INFO *elInfo,int elNum = 0 , int face=0, int edge=0,int vertex=0);
#endif

  //! Constructor
  UGGridLevelIterator(UGGrid<dim,dimworld> &grid,
                      UGMarkerVector * vec ,int travLevel);

  //! prefix increment
  UGGridLevelIterator<codim,dim,dimworld>& operator ++();

  //! postfix increment
  //UGGridLevelIterator<codim,dim,dimworld>& operator ++(int i);

  //! equality
  bool operator== (const UGGridLevelIterator<codim,dim,dimworld>& i) const;

  //! inequality
  bool operator!= (const UGGridLevelIterator<codim,dim,dimworld>& i) const;

  //! dereferencing
  UGGridEntity<codim,dim,dimworld>& operator*() ;

  //! arrow
  UGGridEntity<codim,dim,dimworld>* operator->() ;

  //! ask for level of entity
  int level ();

private:
  // private Methods
  void makeIterator();

  void setToTarget(void* target) {
    target_ = target;
    virtualEntity_.setToTarget(target);
  }

  //! the grid were it all comes from
  UGGrid<dim,dimworld> &grid_;

  // private Members
  UGGridEntity<codim,dim,dimworld> virtualEntity_;

  //! element number
  int elNum_;

  //! level
  int level_;

  //   //! which face, edge and vertex are we watching of an elInfo
  //   unsigned char face_;
  //   unsigned char edge_;
  //   unsigned char vertex_;

  //   // knows on which element a point is viewed
  //   UGMarkerVector * vertexMarker_;


  void* target_;
};

#endif
