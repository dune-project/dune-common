// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_LAGRNAGEMAPPER_HH
#define DUNE_LAGRANGEMAPPER_HH

#include <dune/fem/common/dofmapperinterface.hh>

namespace Dune {

  //************************************************************************
  //
  //  --LagrangeMapper
  //
  //! This Class knows what the space dimension is and how to map for a
  //! given grid entity from local dof number to global dof number
  //
  //************************************************************************
  template <class IndexSetImp, int polOrd, int dimrange>
  class LagrangeMapper
    : public DofMapperDefault < LagrangeMapper <IndexSetImp,polOrd,dimrange> >
  {
    enum { numCodims = IndexSetImp::ncodim };

    int numberOfDofs_;
    const IndexSetImp & indexSet_;

    int insertionPoint_ [numCodims];
    Array<int> codimOfDof_;
    Array<int> numInCodim_;

    int dofCodim_[numCodims];

  public:
    typedef IndexSetImp IndexSetType;

    //! Constructor
    LagrangeMapper (const IndexSetType & is, int numLocalDofs )
      : numberOfDofs_ (numLocalDofs) , indexSet_ (is)
    {
      // this is not ready yet
      assert(false);
      codimOfDof_.resize(numberOfDofs_);
      numInCodim_.resize(numberOfDofs_);

      // differrent for dimension and element type
      for(int i=0; i<codimOfDof_.size(); i++)
        codimOfDof_[i] = 0;

      // is set for dofs
      for(int i=0; i<numInCodim_.size(); i++)
        numInCodim_[i] = 0;

      for(int i=0; i<numCodims; i++)
      {
        insertionPoint_[i] = 0;
        dofCodim_[i] = 0;
      }
      dofCodim_[0] = dimrange;
    }

    virtual ~LagrangeMapper () {}

    //! return size, i.e. size of functions space
    //! the default is Lagrange polord == 1
    int size () const
    {
      int s = 0;
      for(int i=0; i<numCodims; i++)
      {
        (indexSet_.size( numCodims - 1 ));
      }
      return s;
    }

    //! map Entity an local Dof number to global Dof number
    //! for Lagrange with polOrd = 1
    template <class EntityType>
    int mapToGlobal (EntityType &en, int localNum ) const
    {
      enum { codim = EntityType::dimension };
      // get global vertex number
      return indexSet_.template index<codim> (en,localNum);
    }

    //! for dof manager, to check whether it has to copy dof or not
    bool indexNew (int num) const
    {
      return (num != oldIndex(num));
    }

    //! return old index, for dof manager only
    int oldIndex (int elNum) const
    {
      for(int i=0; i<numCodims; i++)
      {
        if(dofCodim_[i] > 0)
        {
          // corresponding number of set is newn
          const int newn = static_cast<int> (elNum / dimrange);
          // local number of dof is local
          const int local = (elNum % dimrange);
          int idx = dimrange * indexSet_.newIndex(newn,i) + local;
          if(elNum == idx) return idx;
        }
      }

      assert(false);
      return -1;
    }

    //! return new index, for dof manager only
    //! which is elNum
    int newIndex (int elNum) const
    {
      return elNum;
    }

    //! return old size of function space
    int oldSize () const
    {
      int s = 0;
      for(int i=0; i<numCodims; i++)
      {
        if(dofCodim_[i]) s += indexSet_.oldSize(i);
      }
      return dimrange * s;
    }

    // is called once and calcs the insertion points too
    int additionalSizeEstimate () const
    {
      return indexSet_.additionalSizeEstimate();
    }

    int numberOfDofs () const DUNE_DEPRECATED
    {
      return numberOfDofs_;
    }

    int numDofs () const
    {
      return numberOfDofs_;
    }

    int newSize() const
    {
      return this->size();
    }
  };



  //! LagrangeMapper for linear Lagrange elements
  template <class IndexSetImp, int dimrange>
  class LagrangeMapper<IndexSetImp,1,dimrange>
    : public DofMapperDefault < LagrangeMapper <IndexSetImp,1,dimrange> >
  {
    enum { numCodims = IndexSetImp::ncodim };
    enum { myCodim   = IndexSetImp::ncodim - 1 };
    int numLocalDofs_;

    const IndexSetImp & indexSet_;
    //const IndexSetWrapper<IndexSetImp> indexSet_;
  public:
    typedef IndexSetImp IndexSetType;

    //! Constructor
    LagrangeMapper (const IndexSetType & is, int numLocalDofs )
      : numLocalDofs_ (numLocalDofs) , indexSet_ (is) {}

    virtual ~LagrangeMapper () {}

    //! return size, i.e. size of functions space == number of vertices
    int size () const
    {
      return (indexSet_.size( myCodim ));
    }

    //! map Entity an local Dof number to global Dof number
    //! for Lagrange with polOrd = 1
    template <class EntityType>
    int mapToGlobal (EntityType &en, int localNum ) const
    {
      enum { codim = EntityType::dimension };
      // get global vertex number
      return indexSet_.template index<codim> (en,localNum);
    }

    //! for dof manager, to check whether it has to copy dof or not
    bool indexNew (int num) const
    {
      return indexSet_.indexNew(num, myCodim );
    }

    //! return old index, for dof manager only
    int oldIndex (int elNum) const
    {
      return indexSet_.oldIndex(elNum, myCodim );
    }

    //! return new index, for dof manager only
    int newIndex (int elNum) const
    {
      return indexSet_.newIndex(elNum, myCodim );
    }

    //! return old size of functions space
    int oldSize () const
    {
      // this index set works only for codim = 0 at the moment
      return indexSet_.oldSize( myCodim );
    }

    // is called once and calcs the insertion points too
    int additionalSizeEstimate () const
    {
      return indexSet_.additionalSizeEstimate();
    }

    void calcInsertPoints () {};

    //! use numDofs instead
    int numberOfDofs () const DUNE_DEPRECATED
    {
      return numLocalDofs_;
    }

    int numDofs () const DUNE_DEPRECATED
    {
      return numLocalDofs_;
    }

    int newSize() const
    {
      return this->size();
    }
  };

  template <class IndexSetImp, int dimrange>
  class LagrangeMapper<IndexSetImp,0,dimrange>
    : public DofMapperDefault < LagrangeMapper <IndexSetImp,0,dimrange> >
  {
    enum { numCodims = IndexSetImp::ncodim };

    int numberOfDofs_;
    const IndexSetImp & indexSet_;

  public:
    typedef IndexSetImp IndexSetType;

    LagrangeMapper ( const IndexSetType  & is , int numDofs )
      : numberOfDofs_ (numDofs) , indexSet_ (is) {
      assert(numberOfDofs_ == dimrange);
    }

    //! return size of function space, here number of elements
    int size () const
    {
      return dimrange * indexSet_.size(0);
    }

    //! map Entity an local Dof number to global Dof number
    //! for Lagrange with polOrd = 0
    template <class EntityType>
    int mapToGlobal (EntityType &en, int localNum ) const
    {
      return (dimrange * indexSet_.template index<0> (en,localNum)) + localNum;
    }

    //! for dof manager, to check whether it has to copy dof or not
    bool indexNew (int num) const
    {
      // all numbers of one entity are maped to the one number of the set
      const int newn = static_cast<int> (num/dimrange);
      return indexSet_.template indexNew(newn,0);
    }

    //! return old index, for dof manager only
    //! this is the mapping from gobal to old leaf index
    int oldIndex (int num) const
    {
      // corresponding number of set is newn
      const int newn  = static_cast<int> (num/dimrange);
      // local number of dof is local
      const int local = (num % dimrange);
      return (dimrange * indexSet_.oldIndex(newn,0)) + local;
    }

    //! return new index, for dof manager only
    //! this is the mapping from global to leaf index
    int newIndex (int num) const
    {

      // corresponding number of set is newn
      const int newn = static_cast<int> (num / dimrange);
      // local number of dof is local
      const int local = (num % dimrange);
      return (dimrange * indexSet_.newIndex(newn,0)) + local;
    }

    //! return old size of functions space
    int oldSize () const
    {
      // this index set works only for codim = 0 at the moment
      return dimrange * indexSet_.oldSize(0);
    }

    // is called once and calcs the insertion points too
    int newSize() const
    {
      return this->size();
    }

    // is called once and calcs the insertion points too
    int additionalSizeEstimate () const
    {
      return dimrange * indexSet_.additionalSizeEstimate();
    }

    //! use numDofs instead
    int numberOfDofs () const DUNE_DEPRECATED
    {
      return numberOfDofs_;
    }

    int numDofs () const
    {
      assert( numberOfDofs_ == dimrange );
      return numberOfDofs_;
    }
  };

  template <class IndexSetImp>
  class LagrangeMapper<IndexSetImp,0,1>
    : public DofMapperDefault < LagrangeMapper <IndexSetImp,0,1> >
  {
    // corresp. index set
    const IndexSetImp & indexSet_;
  public:
    typedef IndexSetImp IndexSetType;

    LagrangeMapper ( const IndexSetType  & is , int numDofs ) : indexSet_ (is) {}

    // we have virtual function ==> virtual destructor
    virtual ~LagrangeMapper () {}

    //! return size of function space, here number of elements
    int size () const
    {
      return indexSet_.size(0);
    }

    //! map Entity an local Dof number to global Dof number
    //! for Lagrange with polOrd = 0
    template <class EntityType>
    int mapToGlobal (EntityType &en, int localNum ) const
    {
      return indexSet_.template index<0> (en,localNum);
    }

    //! for dof manager, to check whether it has to copy dof or not
    bool indexNew (int num) const
    {
      return indexSet_.template indexNew(num,0);
    }

    //! return old index, for dof manager only
    int oldIndex (int num) const
    {
      return indexSet_.oldIndex(num,0);
    }

    //! return new index, for dof manager only
    int newIndex (int num) const
    {
      return indexSet_.newIndex(num,0);
    }

    //! return size of grid entities per level and codim
    //! for dof mapper
    int oldSize () const
    {
      // this index set works only for codim = 0 at the moment
      return indexSet_.oldSize(0);
    }

    // is called once and calcs the insertion points too
    int newSize() const
    {
      return this->size();
    }

    // is called once and calcs the insertion points too
    int additionalSizeEstimate () const
    {
      return indexSet_.additionalSizeEstimate();
    }

    //! use numDofs instead
    int numberOfDofs () const DUNE_DEPRECATED
    {
      return 1;
    }

    int numDofs () const
    {
      return 1;
    }
  };

} // end namespace Dune

#endif
