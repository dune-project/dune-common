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

    int numberOfHoles () const
    {
      assert(false);
      int s = 0;
      for(int i=0; i<numCodims; i++)
      {
        if(dofCodim_[i]) s += indexSet_.numberOfCodims(i);
      }
      return dimrange * s;
    }

    // is called once and calcs the insertion points too
    int additionalSizeEstimate () const
    {
      return indexSet_.additionalSizeEstimate();
    }

    //! return number of dofs per entity
    int numDofs () const
    {
      return numberOfDofs_;
    }

    //! return new size of the set
    int newSize() const
    {
      return this->size();
    }

    //! return if compress of data is needed
    bool needsCompress () const { return indexSet_.needsCompress(); }
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

    //! return old index, for dof manager only
    int oldIndex (int hole) const
    {
      return indexSet_.oldIndex(hole, myCodim );
    }

    //! return new index, for dof manager only
    int newIndex (int hole) const
    {
      return indexSet_.newIndex(hole, myCodim );
    }

    //! return number of holes
    int numberOfHoles () const
    {
      return indexSet_.numberOfHoles( myCodim );
    }

    // is called once and calcs the insertion points too
    int additionalSizeEstimate () const
    {
      return indexSet_.additionalSizeEstimate();
    }

    //! not used at the moment
    void calcInsertPoints () {};

    //! return number of dofs per entity, i.e. number of basis funcitons per entity
    int numDofs () const
    {
      return numLocalDofs_;
    }

    //! return newSize of functions space
    int newSize() const
    {
      return this->size();
    }

    //! return the sets needsCompress
    bool needsCompress () const { return indexSet_.needsCompress(); }
  };

  //*****************************************************************
  //
  // specialisation for polynom order 0 and arbitray dimrange
  //
  //*****************************************************************

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

    //! return old index, for dof manager only
    //! this is the mapping from gobal to old leaf index
    int oldIndex (int hole) const
    {
      // corresponding number of set is newn
      const int newn  = static_cast<int> (hole/dimrange);
      // local number of dof is local
      const int local = (hole % dimrange);
      return (dimrange * indexSet_.oldIndex(newn,0)) + local;
    }

    //! return new index, for dof manager only
    //! this is the mapping from global to leaf index
    int newIndex (int hole) const
    {
      // corresponding number of set is newn
      const int newn = static_cast<int> (hole / dimrange);
      // local number of dof is local
      const int local = (hole % dimrange);
      return (dimrange * indexSet_.newIndex(newn,0)) + local;
    }

    //! return old size of functions space
    int numberOfHoles () const
    {
      // this index set works only for codim = 0 at the moment
      return dimrange * indexSet_.numberOfHoles(0);
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

    //! return number of dof per element
    int numDofs () const
    {
      assert( numberOfDofs_ == dimrange );
      return numberOfDofs_;
    }

    //! return the sets needsCompress
    bool needsCompress () const { return indexSet_.needsCompress(); }
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

    //! return old index, for dof manager only
    int oldIndex (int hole) const
    {
      return indexSet_.oldIndex(hole,0);
    }

    //! return new index, for dof manager only
    int newIndex (int hole) const
    {
      return indexSet_.newIndex(hole,0);
    }

    //! return old size of functions space
    int numberOfHoles () const
    {
      // this index set works only for codim = 0 at the moment
      return indexSet_.numberOfHoles(0);
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

    //! return number of dof per entity, here this method returns 1
    int numDofs () const
    {
      return 1;
    }
    //! return the sets needsCompress
    bool needsCompress () const { return indexSet_.needsCompress(); }
  };

} // end namespace Dune
#endif
