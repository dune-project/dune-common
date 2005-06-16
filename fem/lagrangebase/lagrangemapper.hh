// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_LAGRNAGEMAPPER_HH__
#define __DUNE_LAGRANGEMAPPER_HH__

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
    int level_;
    IndexSetImp & indexSet_;

    int insertionPoint_ [numCodims];
    Array<int> codimOfDof_;
    Array<int> numInCodim_;

    int dofCodim_[numCodims];

  public:
    typedef IndexSetImp IndexSetType;

    //! Constructor
    LagrangeMapper ( IndexSetType & is, int numLocalDofs , int level )
      : numberOfDofs_ (numLocalDofs) , level_(level) , indexSet_ (is)
    {
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
        (indexSet_.size( level_ , numCodims - 1 ));
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
    bool indexNew (int num)
    {
      //int check = (int) (num / dimrange);
      //return indexSet_.indexNew(num,2);
      return false;
    }

    //! return old index, for dof manager only
    int oldIndex (int elNum) const
    {
      int check = (int) (elNum / dimrange);
      int rest  = elNum % dimrange;
      return (indexSet_.oldIndex(elNum,2) * dimrange) + rest;
    }

    //! return new index, for dof manager only
    int newIndex (int elNum) const
    {
      int check = (int) (elNum / dimrange);
      int rest  = elNum % dimrange;
      return (indexSet_.newIndex(elNum,2) * dimrange) + rest;
    }

    //! return size of grid entities per level and codim
    //! for dof mapper
    int oldSize () const
    {
      // this index set works only for codim = 0 at the moment
      return indexSet_.oldSize(level_,2);
    }

    // is called once and calcs the insertion points too
    int additionalSizeEstimate () const
    {
      return indexSet_.additionalSizeEstimate();
    }

    void calcInsertPoints () {};

    int numberOfDofs () const
    {
      return numberOfDofs_;
    }

    int newSize() const
    {
      return this->size();
    }

    int elementDofs () const
    {
      int sum = 0;
      for(int i = 0; i<numCodims; i++) sum += dofCodim_[i];
      return sum;
    }
  };



  //! LagrangeMapper for linear Lagrange elements
  template <class IndexSetImp, int dimrange>
  class LagrangeMapper<IndexSetImp,1,dimrange>
    : public DofMapperDefault < LagrangeMapper <IndexSetImp,1,dimrange> >
  {
    enum { numCodims = IndexSetImp::ncodim };
    int numLocalDofs_;
    int level_;

    IndexSetImp & indexSet_;

  public:
    typedef IndexSetImp IndexSetType;

    //! Constructor
    LagrangeMapper ( IndexSetType & is, int numLocalDofs , int level )
      : numLocalDofs_ (numLocalDofs) , level_(level) , indexSet_ (is) {}

    virtual ~LagrangeMapper () {}

    //! return size, i.e. size of functions space == number of vertices
    int size () const
    {
      return (indexSet_.size( level_ , numCodims - 1 ));
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
    bool indexNew (int num)
    {
      //int check = (int) (num / dimrange);
      //return indexSet_.indexNew(num,2);
      return false;
    }

    //! return old index, for dof manager only
    int oldIndex (int elNum) const
    {
      return indexSet_.oldIndex(elNum, numCodims - 1 );
    }

    //! return new index, for dof manager only
    int newIndex (int elNum) const
    {
      return indexSet_.newIndex(elNum, numCodims - 1 );
    }

    //! return size of grid entities per level and codim
    //! for dof mapper
    int oldSize () const
    {
      // this index set works only for codim = 0 at the moment
      return indexSet_.oldSize(level_, numCodims - 1);
    }

    // is called once and calcs the insertion points too
    int additionalSizeEstimate () const
    {
      return indexSet_.additionalSizeEstimate();
    }

    void calcInsertPoints () {};

    int numberOfDofs () const
    {
      return numLocalDofs_;
    }

    int newSize() const
    {
      return this->size();
    }

    int elementDofs () const
    {
      return numCodims;
    }
  };

  template <class IndexSetImp, int dimrange>
  class LagrangeMapper<IndexSetImp,0,dimrange>
    : public DofMapperDefault < LagrangeMapper <IndexSetImp,0,dimrange> >
  {
    enum { numCodims = IndexSetImp::ncodim };

    int numberOfDofs_;
    IndexSetImp & indexSet_;

    // level of function space
    int level_;

  public:
    typedef IndexSetImp IndexSetType;

    LagrangeMapper ( IndexSetType  & is , int numDofs , int level)
      : numberOfDofs_ (numDofs) , indexSet_ (is) , level_(level) {}

    // we have virtual function ==> virtual destructor
    virtual ~LagrangeMapper () {}

    //! return size of function space, here number of elements
    int size () const
    {
      return dimrange * indexSet_.size(level_,0);
    }

    //! map Entity an local Dof number to global Dof number
    //! for Lagrange with polOrd = 0
    template <class EntityType>
    int mapToGlobal (EntityType &en, int localNum ) const
    {
      return (dimrange * indexSet_.template index<0> (en,localNum)) + localNum;
    }

    //! for dof manager, to check whether it has to copy dof or not
    bool indexNew (int num)
    {
      int newn = (num / dimrange) + num % dimrange;
      if(dimrange == 1) assert(newn == num);
      return indexSet_.template indexNew(newn,0);
    }

    //! return old index, for dof manager only
    int oldIndex (int num) const
    {
      int newn = (num / dimrange) + num % dimrange;
      return dimrange * indexSet_.oldIndex(newn,0);
    }

    //! return new index, for dof manager only
    int newIndex (int num) const
    {
      int newn = (num / dimrange) + num % dimrange;
      return dimrange * indexSet_.newIndex(newn,0);
    }

    //! return size of grid entities per level and codim
    //! for dof mapper
    int oldSize () const
    {
      // this index set works only for codim = 0 at the moment
      return dimrange * indexSet_.oldSize(level_,0);
    }

    // is called once and calcs the insertion points too
    int newSize() const
    {
      return this->size();
    }

    int elementDofs () const
    {
      return dimrange;
    }

    // is called once and calcs the insertion points too
    int additionalSizeEstimate () const
    {
      return dimrange * indexSet_.additionalSizeEstimate();
    }

    int numberOfDofs () const
    {
      return numberOfDofs_;
    }

    //! calc the new insertion points
    void calcInsertPoints ()
    {
      // insertion point is 0
    }
  };

  template <class IndexSetImp>
  class LagrangeMapper<IndexSetImp,0,1>
    : public DofMapperDefault < LagrangeMapper <IndexSetImp,0,1> >
  {
    IndexSetImp & indexSet_;
    // level of function space
    int level_;
  public:
    typedef IndexSetImp IndexSetType;

    LagrangeMapper ( IndexSetType  & is , int numDofs , int level)
      : indexSet_ (is) , level_(level) {}

    // we have virtual function ==> virtual destructor
    virtual ~LagrangeMapper () {}

    //! return size of function space, here number of elements
    int size () const
    {
      return indexSet_.size(level_,0);
    }

    //! map Entity an local Dof number to global Dof number
    //! for Lagrange with polOrd = 0
    template <class EntityType>
    int mapToGlobal (EntityType &en, int localNum ) const
    {
      return indexSet_.template index<0> (en,localNum);
    }

    //! for dof manager, to check whether it has to copy dof or not
    bool indexNew (int num)
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
      return indexSet_.oldSize(level_,0);
    }

    // is called once and calcs the insertion points too
    int newSize() const
    {
      return this->size();
    }

    int elementDofs () const
    {
      return 1;
    }

    // is called once and calcs the insertion points too
    int additionalSizeEstimate () const
    {
      return indexSet_.additionalSizeEstimate();
    }

    int numberOfDofs () const
    {
      return 1;
    }

    //! calc the new insertion points
    void calcInsertPoints ()
    {
      // insertion point is 0
    }
  };

} // end namespace Dune

#endif
