// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISFUNCARRAY_HH__
#define __DUNE_DISFUNCARRAY_HH__

#include "discretefunction.hh"
#include "localfunctionarray.hh"

#include <fstream>
#include <rpc/xdr.h>

namespace Dune {

  //**********************************************************************
  //
  //  --DiscFuncArray
  //
  //! this is one special implementation of a discrete function using an
  //! array for storing the dofs.
  //!
  //**********************************************************************
  template<class DiscreteFunctionSpaceType >
  class DiscFuncArray
    : public DiscreteFunctionDefault < DiscreteFunctionSpaceType,
          DofIteratorArray < typename DiscreteFunctionSpaceType::RangeField > ,
          DiscFuncArray <DiscreteFunctionSpaceType> >
  {
    typedef DiscreteFunctionDefault < DiscreteFunctionSpaceType,
        DofIteratorArray < typename DiscreteFunctionSpaceType::RangeField > ,
        DiscFuncArray <DiscreteFunctionSpaceType > >
    DiscreteFunctionDefaultType;

    typedef typename DiscreteFunctionSpaceType::RangeField DofType;
    typedef typename DiscreteFunctionSpaceType::DomainField DomainField;
    typedef typename DiscreteFunctionSpaceType::RangeField RangeField;

    typedef typename DiscreteFunctionSpaceType::GridType GridType;

    enum { myId_ = 0};

  public:

    typedef DiscreteFunctionSpaceType FunctionSpaceType;
    typedef LocalFunctionArray < DiscreteFunctionSpaceType > LocalFunctionType;

    // Constructor make empty DiscFuncArray
    DiscFuncArray ( const DiscreteFunctionSpaceType & f );

    // Constructor make Discrete Function for all or leaf level
    DiscFuncArray ( const DiscreteFunctionSpaceType & f,
                    int level , int codim , bool flag ) ;

    void getMemory()
#if 1
    {
      // for all grid levels we have at least a vector with length 0
      int numLevel = const_cast<GridType &> (functionSpace_.getGrid()).maxlevel() +1;
      dofVec_.resize(numLevel);
      for(int i=0; i<numLevel; i++)
        dofVec_[i] = NULL;

      // this is done only if levOcu_ > 1
      for(int i=0; i< levOcu_-1; i++)
      {
        int length = functionSpace_.size( i );
        (dofVec_[i]).resize( length );
        for( int j=0; j<length; j++)
          (dofVec_[i])[j] = 0.0;
      }

      // the last level is done always
      int length = functionSpace_.size( level_ );
      (dofVec_[level_]).resize( length );
      for( int j=0; j<length; j++) (dofVec_[level_])[j] = 0.0;
    }
#endif

    // ***********  Interface  *************************

    //! access to dof entity corresponding grid entity en
    //! default implementation is via GlobalDofIterator
    template <class EntityType>
    LocalFunctionType & access (EntityType & en );

    // we use the default implementation
    // Warning!!! returns reference to local object!
    DofIteratorType dbegin ( int level );

    //! points behind the last dof of type cc
    // Warning!!! returns reference to local object!
    DofIteratorType dend   ( int level );

    //! set all dofs to zero
    void clearLevel( int level );
    void clear( );

    //! set all dof to value x
    void set( DofType x );
    void setLevel( DofType x, int level );

    //! print all dofs
    void print(std::ostream& s, int level);

    //! write leaf data to file in USPM format for Grape
    bool write_USPM(const char *filename , int timestep);

    //! write data of discrete function to file filename|timestep
    //! with xdr methods
    bool write_xdr( const char *filename , int timestep );

    //! write data of discrete function to file filename|timestep
    //! with xdr methods
    bool read_xdr( const char *filename , int timestep );

    //! write function data to file filename|timestep in ascii Format
    bool write_ascii(const char *filename, int timestep);

    //! read function data from file filename|timestep in ascii Format
    bool read_ascii(const char *filename, int timestep);

    //! write function data in pgm fromat file
    bool write_pgm(const char *filename, int timestep) ;

    //! read function data from pgm fromat file
    bool read_pgm(const char *filename, int timestep);

  private:
    //! true if memory was allocated
    bool built_;

    //! false if only leaf level is allocated
    bool allLevels_;

    //! occupied levels
    int levOcu_;

    //! maxlevel which is occupied
    int level_;

    //! Vector of Array for each level, the array holds
    //! the dofs of each level
    LocalFunctionArray < DiscreteFunctionSpaceType > localFunc_;

    //! for all level an Array < DofType > , the data
    std::vector < Array < DofType > > dofVec_;
  };

} // end namespace Dune

#include "discfuncarray.cc"

#endif
