// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCRETEFUNCTION_HH__
#define __DUNE_DISCRETEFUNCTION_HH__

#include "../common/function.hh"
#include "../common/functionspace.hh"
#include "discretefunctionspace.hh"
#include "localfunctionarray.hh"
#include "dofiterator.hh"

#include <fstream>

namespace Dune {


  /** @defgroup DiscreteFunction The Interface for DiscreteFunctions

      The DiscreteFunction is resposible for the dof storage. This can be
      done in various ways an is left to the user. The user has to derive his
      own implemenation from the DiscreteFunctionDefault class. If some of
      the implementations in the default class are for his dof storage
      unefficient, then one can overload this functions.

     @{
   */


  //************************************************************************
  //
  //  --DiscreteFunctionInterface
  //
  //! This is the minimal interface of a discrete function which has to be
  //! implemented. It contains an local function and a dof iterator which can
  //! iterator over all dofs of one level. Via the method access the local
  //! dofs and basfunction can be accessed for a given entity.
  //!
  //************************************************************************
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  class DiscreteFunctionInterface
    : public Function < DiscreteFunctionSpaceType,
          DiscreteFunctionInterface <DiscreteFunctionSpaceType,
              DofIteratorImp , DiscreteFunctionImp > >
  {
    // just for readability
    typedef Function < DiscreteFunctionSpaceType,
        DiscreteFunctionInterface <DiscreteFunctionSpaceType,
            DofIteratorImp , DiscreteFunctionImp > > FunctionType;
  public:
    //! remember the template types
    template <int cc>
    struct Traits
    {
      typedef typename DiscreteFunctionSpaceType::GridType GridType;
      typedef typename DiscreteFunctionSpaceType::Domain Domain;
      typedef typename DiscreteFunctionSpaceType::Range Range;
      typedef typename DiscreteFunctionSpaceType::RangeField RangeField;
    };

    typedef typename DiscreteFunctionSpaceType::GridType GridType;
    typedef DofIteratorImp DofIteratorType;

    DiscreteFunctionInterface ( const DiscreteFunctionSpaceType &f )
      : FunctionType ( f ) {} ;


    //! access to the local function. Local functions can only be accessed
    //! for an existing entity.
    template <class EntityType>
    typename Traits<EntityType::codimension>::LocalFunctionIteratorType
    access (EntityType & en )
    {
      return asImp().access(en);
    }

    //! the implementation of an iterator to iterate efficient over all dof
    //! on one level.
    DofIteratorType dbegin ( int level )
    {
      return asImp().dbegin ( level );
    };

    //! the implementation of an iterator to iterate efficient over all dof
    //! on one level
    DofIteratorType dend ( int level )
    {
      return asImp().dend ( level );
    };

    //! clear all dofs of the discrete function
    void clear( )
    {
      asImp.clear( );
    }

  private:
    // Barton-Nackman trick
    DiscreteFunctionImp &asImp()
    {
      return static_cast<DiscreteFunctionImp&>(*this);
    }
    const DiscreteFunctionImp &asImp() const
    {
      return static_cast<const DiscreteFunctionImp&>(*this);
    }
  };

  //*************************************************************************
  //
  //  --DiscreteFunctionDefault
  //
  //! Default implementation of the discrete function. This class provides
  //! is responsible for the dof storage. Different implementations of the
  //! discrete function use different dof storage.
  //! The default implementation provides +=, -= ans so on operators and
  //! a DofIterator access, which can run over all dofs in an efficient way.
  //! Furthermore with an entity you can access an local function to evaluate
  //! the discrete function by multiplying the dofs and the basefunctions.
  //!
  //*************************************************************************
  template<class DiscreteFunctionSpaceType,
      class DofIteratorImp, class DiscreteFunctionImp >
  class DiscreteFunctionDefault
    : public DiscreteFunctionInterface
      <DiscreteFunctionSpaceType, DofIteratorImp, DiscreteFunctionImp >
  {

    typedef DiscreteFunctionInterface <DiscreteFunctionSpaceType,
        DofIteratorImp, DiscreteFunctionImp >  DiscreteFunctionInterfaceType;
  public:
    //! remember the used types
    template <int cc>
    struct Traits
    {
      typedef typename DiscreteFunctionSpaceType::Domain Domain;
      typedef typename DiscreteFunctionSpaceType::Range Range;
      typedef typename DiscreteFunctionSpaceType::RangeField RangeField;
    };

    typedef DofIteratorImp DofIteratorType;

    //! pass the function space to the interface class
    DiscreteFunctionDefault ( const DiscreteFunctionSpaceType & f ) :
      DiscreteFunctionInterfaceType ( f ) {};

    //! derived from Function
    //! search for element which contains point x an evaluate
    //! dof entity with en
    void evaluate ( const typename Traits<0>::Domain & ,
                    typename Traits<0>::Range &) const
    {
      // search element
    };


    //! evaluate an scalar product of the dofs of two DiscreteFunctions
    DiscreteFunctionSpaceType::RangeField scalarProductDofs( const DiscreteFunctionDefault &g );

    // assign
    Vector<DiscreteFunctionSpaceType::RangeField> &
    assign(const Vector<DiscreteFunctionSpaceType::RangeField> &g);

    // assign
    Vector<DiscreteFunctionSpaceType::RangeField> &
    operator = (const Vector<DiscreteFunctionSpaceType::RangeField> &g);

    // add
    Vector<DiscreteFunctionSpaceType::RangeField> &
    operator += (const Vector<DiscreteFunctionSpaceType::RangeField> &g);

    // substract
    Vector<DiscreteFunctionSpaceType::RangeField> &
    operator -= (const Vector<DiscreteFunctionSpaceType::RangeField> &g);

    // multiply
    Vector<DiscreteFunctionSpaceType::RangeField> &
    operator *= (const DiscreteFunctionSpaceType::RangeField &scalar);

    Vector<DiscreteFunctionSpaceType::RangeField> &
    operator /= (const DiscreteFunctionSpaceType::RangeField &scalar);

    // add
    Vector<DiscreteFunctionSpaceType::RangeField> &
    add(const Vector<DiscreteFunctionSpaceType::RangeField> &g ,
        DiscreteFunctionSpaceType::RangeField scalar );

    template <class EntityType>
    void assignLocal(EntityType &en, const DiscreteFunctionImp &g,
                     const DiscreteFunctionSpaceType::RangeField &scalar)
    {
      std::cout << "AssignLocal \n";
    }

    template <class EntityType>
    void addLocal(EntityType &en, const DiscreteFunctionImp &g,
                  const DiscreteFunctionSpaceType::RangeField &scalar)
    {
      std::cout << "AddLocal \n";
    }

    //! clear all dofs of the discrete function
    void clear( );

  private:
    // Barton-Nackman trick
    DiscreteFunctionImp &asImp()
    {
      return static_cast<DiscreteFunctionImp&>(*this);
    }
    const DiscreteFunctionImp &asImp() const
    {
      return static_cast<const DiscreteFunctionImp&>(*this);
    }

  };

  /** @} end documentation group */


  //**********************************************************************
  //
  //  --DiscFuncTest
  //
  //! this is one special implementation of a discrete function using an
  //! array for storing the dofs.
  //!
  //**********************************************************************
  template<class DiscreteFunctionSpaceType >
  class DiscFuncTest
    : public DiscreteFunctionDefault < DiscreteFunctionSpaceType,
          DofIteratorArray < typename DiscreteFunctionSpaceType::RangeField > ,
          DiscFuncTest <DiscreteFunctionSpaceType> >
  {
    //typedef GlobalDofIteratorArray < typename Traits::RangeField > DofIteratorType;
    typedef DiscreteFunctionDefault < DiscreteFunctionSpaceType,
        DofIteratorArray < typename DiscreteFunctionSpaceType::RangeField > ,
        DiscFuncTest <DiscreteFunctionSpaceType > >
    DiscreteFunctionDefaultType;

    typedef typename DiscreteFunctionSpaceType::RangeField DofType;

    typedef typename DiscreteFunctionSpaceType::GridType GridType;
  public:

    typedef DiscreteFunctionSpaceType FunctionSpaceType;
    typedef LocalFunctionArray < DiscreteFunctionSpaceType > LocalFunctionType;

    DiscFuncTest ( const DiscreteFunctionSpaceType & f,
                   int level , int codim , bool flag )
      : DiscreteFunctionDefaultType ( f ) , level_ ( level ) ,
        allLevels_ ( flag ) , localFunc_ ( f , dofVec_ )
    {
      if(flag)
        levOcu_ = level_+1;
      else
        levOcu_ = 1;

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

    };

    void set ( DofType x, int level )
    {
      std::cout << "Set Level " << level << " with value " << x << std::endl;
      if(!allLevels_ && level != level_)
      {
        std::cout << "Level not set! \n";
        return;
      }
      DofIteratorType endit = dend ( level );
      for(DofIteratorType it = dbegin ( level ); it != endit; ++it)
      {
        (*it) = x;
      }
    }

    // ***********  Interface  *************************

    //! access to dof entity corresponding grid entity en
    //! default implementation is via GlobalDofIterator
    template <class EntityType>
    LocalFunctionType & access (EntityType & en )
    {
      localFunc_.init( en );
      return localFunc_;
    };



    // we use the default implementation
    // Warning!!! returns reference to local object!
    DofIteratorType dbegin ( int level )
    {
      DofIteratorType tmp ( dofVec_ [level] , 0 );
      return tmp;
    };

    //! points behind the last dof of type cc
    // Warning!!! returns reference to local object!
    DofIteratorType dend   ( int level )
    {
      DofIteratorType tmp ( dofVec_ [ level ] , dofVec_[ level ].size() );
      return tmp;
    };

    void clear( )
    {
      DofIteratorType enddof = dend ( level_ );
      for(DofIteratorType itdof = dbegin ( level_ ); itdof != enddof; ++itdof)
      {
        *itdof = 0.;
      }
    }

    void setAll( DofType x )
    {
      set( x, level_ );
    }

    //! print all dofs
    void print()
    {
      DofType sum = 0.;
      int numLevel = const_cast<GridType &> (functionSpace_.getGrid()).maxlevel();
      DofIteratorType enddof = dend ( numLevel );
      for(DofIteratorType itdof = dbegin ( numLevel ); itdof != enddof; ++itdof)
      {
        std::cout << (*itdof) << " DofValue \n";
        sum += *itdof;
      }
      std::cerr << "sum = " << sum << "\n";
    }

    bool write(const char *filename, int level )
    {
      std::fstream out( filename , std::ios::out );
      //ElementType eltype = triangle;
      //out << eltype << " 1 1\n";
      int length = functionSpace_.size( level );
      out << length << " 1 1\n";

      DofIteratorType enddof = dend ( level );
      for(DofIteratorType itdof = dbegin ( level );
          itdof != enddof; ++itdof)
      {
        out << (*itdof)  << "\n";
      }

      out.close();
      std::cout << "Written Dof to file `" << filename << "' !\n";
    }

    void save(const char *filename) {
      std::ofstream out( filename );
      out << "P2\n " << DANZ+1 << " " <<DANZ+1 <<"\n255\n";
      DofIteratorType enddof = dend ( level_ );
      for(DofIteratorType itdof = dbegin ( level_ ); itdof != enddof; ++itdof) {
        out << (int)((*itdof)*255.) << "\n";
      }
      out.close();
    }

    void load(const char *filename) {
      FILE *in;
      int v;
      in = fopen( filename, "r" );
      fscanf( in, "P2\n%d %d\n%d\n", &v, &v, &v );
      DofIteratorType enddof = dend ( level_ );
      for(DofIteratorType itdof = dbegin ( level_ ); itdof != enddof; ++itdof) {
        fscanf( in, "%d", &v );
        (*itdof) = ((double)v)/255.;
      }
      fclose( in );
    }

  private:
    bool allLevels_;

    //! occupied levels
    int levOcu_;

    //! maxlevel which is occupied
    int level_;

    //! Vector of Array for each level, the array holds the dofs of each
    //! level
    LocalFunctionArray < DiscreteFunctionSpaceType > localFunc_;

    //! for all level an Array < DofType >
    std::vector < Array < DofType > > dofVec_;
  };

} // end namespace Dune

#include "discretefunction.cc"

#endif
