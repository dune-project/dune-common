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
  template <class IndexSetType, int polOrd, int dimrange>
  class LagrangeMapper
    : public DofMapperDefault < LagrangeMapper <IndexSetType,polOrd,dimrange> >
  {
    enum { numCodims = IndexSetType::ncodim };
    int numLocalDofs_;
    int level_;
    IndexSetType & indexSet_;
  public:
    LagrangeMapper ( IndexSetType & is, int numLocalDofs , int level )
      : numLocalDofs_ (numLocalDofs) , level_(level) , indexSet_ (is) {}

    virtual ~LagrangeMapper () {}

    int size () const
    {
      return this->codimsize(numCodims-1);
    }

    //! default is Lagrange with polOrd = 1
    int codimsize (int codim ) const
    {
      // return number of vertices * dimrange
      return (dimrange* indexSet_.size( level_ , codim ));
    }

    //! map Entity an local Dof number to global Dof number
    //! for Lagrange with polOrd = 1
    template <class EntityType>
    int mapToGlobal (EntityType &en, int localNum ) const
    {
      enum { codim = EntityType::dimension };
      // Gaussklammer
      int locNum = (int) localNum / dimrange;
      int locDim = localNum % dimrange;

      // get global vertex number
      return (dimrange* indexSet_.template index<codim> (en,locNum) ) + locDim;
    }

    virtual void calcInsertPoints () {};

    virtual int numberOfDofs () const
    {
      return numLocalDofs_;
    }

    virtual int newSize() const
    {

      /*
         int s=0;
         for(int i=0; i<numCodims; i++)
         s+= (dofCodim_[i] * indexSet_.size(20,i));
       */
      return this->size();
    }

  };

  template <class IndexSetType, int dimrange>
  class LagrangeMapper<IndexSetType,0,dimrange>
    : public DofMapperDefault < LagrangeMapper <IndexSetType,0,dimrange> >
  {
    enum { numCodims = IndexSetType::ncodim };

    int numberOfDofs_;
    IndexSetType & indexSet_;

    // insertion point is the point from which
    // the dof for codim .. begin
    // insertion point only changes if grid changes
    // and memory has to be rearranged
    int insertionPoint_ [numCodims];
    mutable int index_ [numCodims];
    Array<int> codimOfDof_;
    Array<int> numInCodim_;

    int dofCodim_[numCodims];

    // level of function space
    int level_;

  public:
    LagrangeMapper ( IndexSetType  & is , int numDofs , int level)
      : numberOfDofs_ (numDofs) , indexSet_ (is) , level_(level)
    {
      codimOfDof_.resize(numberOfDofs_);
      numInCodim_.resize(numberOfDofs_);

      // differrent for dimension and element type
      for(int i=0; i<codimOfDof_.size(); i++)
        codimOfDof_[i] = 0;

      // is set for dofs
      for(int i=0; i<codimOfDof_.size(); i++)
        numInCodim_[i] = 0;

      for(int i=0; i<numCodims; i++)
      {
        insertionPoint_[i] = 0;
        dofCodim_[i] = 0;
      }
      dofCodim_[0] = dimrange;
    }

    // we have virtual function ==> virtual destructor
    virtual ~LagrangeMapper () {}

    int size () const
    {
      return this->codimsize(0);
    }

    //! default is Lagrange with polOrd = 0
    int codimsize (int codim ) const
    {
      // return number of vertices
      return dimrange * indexSet_.size(level_,codim);
    }

    //! map Entity an local Dof number to global Dof number
    //! for Lagrange with polOrd = 0
    template <class EntityType>
    int mapToGlobal (EntityType &en, int localNum ) const
    {
      getIndex ( en , 0 );
      // return insertionPoint + index(codim)
      //return (insertionPoint_[codimOfDof_[localNum]] + index_[codimOfDof_[localNum]]);
      return index_[0];
    }

    // is called once and calcs the insertion points too
    virtual int newSize() const
    {
      return this->size();
    }

    virtual int numberOfDofs () const
    {
      return numberOfDofs_;
    }

    //! calc the new insertion points
    virtual void calcInsertPoints ()
    {
      // insertion point is 0
    }

  private:
    // calc the indices
    template <class EntityType>
    void getIndex (EntityType &en, int num) const
    {
      index_[0] = dimrange * indexSet_.template index<0> (en,num);
    }
  };

} // end namespace Dune

#endif
