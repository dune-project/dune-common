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

  //************************************************************************
  //! the minimal interface
  //************************************************************************
  template<class DiscreteFunctionSpaceType,
      template <int , class > class LocalFunctionIteratorImp ,
      class DofIteratorImp, class DiscreteFunctionImp >
  class DiscreteFunctionInterface
    : public Function < DiscreteFunctionSpaceType,
          DiscreteFunctionInterface <DiscreteFunctionSpaceType,
              LocalFunctionIteratorImp ,DofIteratorImp , DiscreteFunctionImp > >
  {
    // just for readability
    typedef Function < DiscreteFunctionSpaceType,
        DiscreteFunctionInterface <DiscreteFunctionSpaceType,
            LocalFunctionIteratorImp , DofIteratorImp , DiscreteFunctionImp > > FunctionType;
  public:
    template <int cc>
    struct Traits
    {
      typedef typename DiscreteFunctionSpaceType::GridType GridType;
      typedef typename DiscreteFunctionSpaceType::Domain Domain;
      typedef typename DiscreteFunctionSpaceType::Range Range;
      typedef typename DiscreteFunctionSpaceType::RangeField RangeField;
      typedef LocalFunctionIteratorImp<cc, DiscreteFunctionSpaceType> LocalFunctionIteratorType;
    };

    typedef typename DiscreteFunctionSpaceType::GridType GridType;
    typedef DofIteratorImp DofIteratorType;

    DiscreteFunctionInterface ( const DiscreteFunctionSpaceType &f )
      : FunctionType ( f ) {} ;

    //! iterator for iteration over all dof of one level
    //! for cc = 0 it iterates over dof entities
    //! for cc = max over all dofs
    template <int codim>
    LocalFunctionIteratorImp<codim, DiscreteFunctionSpaceType>
    lfbegin ( int level )
    {
      return asImp().lfbegin<codim>( level );
    };

    //! points behind the last dof of type cc
    template <int codim>
    LocalFunctionIteratorImp<codim, DiscreteFunctionSpaceType>
    lfend ( int level )
    {
      return asImp().lfend<codim>( level );
    };

    template <class EntityType>
    typename Traits<EntityType::codimension>::LocalFunctionIteratorType
    access (EntityType & en )
    {
      return asImp().access(en);
    }
    //! the implementation of an iterator to iterate efficient over all dof
    //! on one level
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
    void clear( ) {
      asImp.clear( );
    }

  private:
    //! Barton-Nackman trick
    DiscreteFunctionImp &asImp()
    {
      return static_cast<DiscreteFunctionImp&>(*this);
    }
    const DiscreteFunctionImp &asImp() const
    {
      return static_cast<const DiscreteFunctionImp&>(*this);
    }
  };

  //! default implementation
  template<class DiscreteFunctionSpaceType,
      template <int, class > class LocalFunctionIteratorImp ,
      class GlobalDofIteratorImp, class DiscreteFunctionImp >
  class DiscreteFunctionDefault
    : public DiscreteFunctionInterface
      <DiscreteFunctionSpaceType, LocalFunctionIteratorImp ,
          GlobalDofIteratorImp, DiscreteFunctionImp >
  {
    typedef DiscreteFunctionInterface
    <DiscreteFunctionSpaceType, LocalFunctionIteratorImp ,
        GlobalDofIteratorImp, DiscreteFunctionImp >
    DiscreteFunctionInterfaceType;


  public:
    template <int cc>
    struct Traits
    {
      typedef typename DiscreteFunctionSpaceType::Domain Domain;
      typedef typename DiscreteFunctionSpaceType::Range Range;
      typedef typename DiscreteFunctionSpaceType::RangeField RangeField;
      typedef LocalFunctionIteratorImp<cc, DiscreteFunctionSpaceType> LocalFunctionIteratorType;
      typedef typename LocalFunctionIteratorType::LocalFunctionType LocalFunctionType;
    };

    typedef GlobalDofIteratorImp GlobalDofIteratorType;


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


    Traits<0>::RangeField scalarProductDofs( const DiscreteFunctionDefault &g ) {
      Traits<0>::RangeField skp = 0.;

      typedef typename GlobalDofIteratorImp DofIteratorType;
      int level = getFunctionSpace().getGrid().maxlevel();

      DofIteratorType endit = dend ( level );
      DofIteratorType git = const_cast<DiscreteFunctionDefault &>( g ).dbegin ( level );
      for(DofIteratorType it = dbegin( level ); it != endit; ++it)
      {
        skp += (*it) * (*git);
        ++git;
      }
      return skp;
    }

    Vector<Traits<0>::RangeField> &assign(const Vector<Traits<0>::RangeField> &g) {

      DiscreteFunctionDefault &gc = const_cast<DiscreteFunctionDefault &>( dynamic_cast<const DiscreteFunctionDefault &> ( g ));
      // we would need const_iterators.....

      typedef typename GlobalDofIteratorImp DofIteratorType;
      int level = getFunctionSpace().getGrid().maxlevel();

      DofIteratorType endit = dend ( level );
      DofIteratorType git = gc.dbegin ( level );
      for(DofIteratorType it = dbegin( level ); it != endit; ++it) {
        *it = *git;
        ++git;
      }
      return *this;
    }

    Vector<Traits<0>::RangeField> &operator=(const Vector<Traits<0>::RangeField> &g) {

      DiscreteFunctionDefault &gc = const_cast<DiscreteFunctionDefault &>( dynamic_cast<const DiscreteFunctionDefault &> ( g ));
      // we would need const_iterators.....

      typedef typename GlobalDofIteratorImp DofIteratorType;
      int level = getFunctionSpace().getGrid().maxlevel();

      DofIteratorType endit = dend ( level );
      DofIteratorType git = gc.dbegin ( level );
      for(DofIteratorType it = dbegin( level ); it != endit; ++it) {
        *it = *git;
        ++git;
      }
      return *this;
    }

    Vector<Traits<0>::RangeField> &operator+=(const Vector<Traits<0>::RangeField> &g) {

      DiscreteFunctionDefault &gc = const_cast<DiscreteFunctionDefault &>( dynamic_cast<const DiscreteFunctionDefault &> ( g ));
      // we would need const_iterators.....

      typedef typename GlobalDofIteratorImp DofIteratorType;
      int level = getFunctionSpace().getGrid().maxlevel();

      DofIteratorType endit = dend ( level );
      DofIteratorType git = gc.dbegin ( level );
      for(DofIteratorType it = dbegin( level ); it != endit; ++it) {
        *it += *git;
        ++git;
      }
      return *this;
    }

    Vector<Traits<0>::RangeField> &operator-=(const Vector<Traits<0>::RangeField> &g) {

      DiscreteFunctionDefault &gc = const_cast<DiscreteFunctionDefault &>( dynamic_cast<const DiscreteFunctionDefault &> ( g ));
      // we would need const_iterators.....

      typedef typename GlobalDofIteratorImp DofIteratorType;
      int level = getFunctionSpace().getGrid().maxlevel();

      DofIteratorType endit = dend ( level );
      DofIteratorType git = gc.dbegin ( level );
      for(DofIteratorType it = dbegin( level ); it != endit; ++it) {
        *it -= *git;
        ++git;
      }
      return *this;
    }

    Vector<Traits<0>::RangeField> &operator*=(const Traits<0>::RangeField &scalar) {

      typedef typename GlobalDofIteratorImp DofIteratorType;
      int level = getFunctionSpace().getGrid().maxlevel();

      DofIteratorType endit = dend ( level );
      for(DofIteratorType it = dbegin( level ); it != endit; ++it) {
        *it *= scalar;
      }
      return *this;
    }

    Vector<Traits<0>::RangeField> &operator/=(const Traits<0>::RangeField &scalar) {
      *this *= 1./scalar;
    }

    Vector<Traits<0>::RangeField> &add(const Vector<Traits<0>::RangeField> &g, Traits<0>::RangeField scalar) {

      DiscreteFunctionDefault &gc = const_cast<DiscreteFunctionDefault &>( dynamic_cast<const DiscreteFunctionDefault &> ( g ));
      // we would need const_iterators.....

      typedef typename GlobalDofIteratorImp DofIteratorType;
      int level = getFunctionSpace().getGrid().maxlevel();

      DofIteratorType endit = dend ( level );
      DofIteratorType git = gc.dbegin ( level );
      for(DofIteratorType it = dbegin( level ); it != endit; ++it) {
        *it += *git * scalar;
        ++git;
      }
      return *this;
    }

  };


  //**********************************************************************
  //
  //
  //
  //**********************************************************************
  template<class DiscreteFunctionSpaceType >
  class DiscFuncTest
    : public DiscreteFunctionDefault < DiscreteFunctionSpaceType,  LocalFunctionArrayIterator ,
          DofIteratorArray < typename DiscreteFunctionSpaceType::RangeField > ,
          DiscFuncTest <DiscreteFunctionSpaceType> >
  {
    //typedef GlobalDofIteratorArray < typename Traits::RangeField > GlobalDofIteratorType;
    typedef DiscreteFunctionDefault < DiscreteFunctionSpaceType,
        LocalFunctionArrayIterator , DofIteratorArray < typename DiscreteFunctionSpaceType::RangeField > ,
        DiscFuncTest <DiscreteFunctionSpaceType > >
    DiscreteFunctionDefaultType;

    typedef typename DiscreteFunctionSpaceType::RangeField DofType;

    typedef LocalFunctionArrayIterator<0, DiscreteFunctionSpaceType > LocalFunctionIteratorType;

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

      int numLevel = const_cast<GridType &> (functionSpace_.getGrid()).maxlevel() +1;
      dofVec_.resize(numLevel);
      // this is done only if levOcu_ > 1
      for(int i=0; i< levOcu_-1; i++)
      {
        int length = functionSpace_.size( i );
        (dofVec_[i]).realloc( length );
        for( int j=0; j<length; j++)
          (dofVec_[i])[j] = 0.0;
      }
      int length = functionSpace_.size( level_ );
      (dofVec_[level_]).realloc( length );
      for( int j=0; j<length; j++) (dofVec_[level_])[j] = 0.0;

      // I want a special operator for  (dofVec_[levOcu_-1]) = 0.0;
    };

    void set ( DofType x, int level )
    {
      GlobalDofIteratorType endit = dend ( level );
      for(GlobalDofIteratorType it = dbegin ( level ); it != endit; ++it)
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

    //! iterator for iteration over all dof of one level
    //! for cc = 0 the local function contains all dof on the entity<0>
    //! and so on
    template <int codim>
    LocalFunctionArrayIterator<codim , DiscreteFunctionSpaceType >
    lfbegin ( int level )
    {
      //typename Traits<codim>::LocalFunctionIteratorType tmp
      typename DiscreteFunctionDefaultType::Traits<codim>::LocalFunctionIteratorType tmp
        (functionSpace_, const_cast<GridType &>
        (functionSpace_.getGrid()).lbegin<codim> (level) , dofVec_ );
      return tmp;
    };

    //! points behind the last dof of type cc
    template <int codim>
    LocalFunctionArrayIterator<codim , DiscreteFunctionSpaceType >
    lfend ( int level )
    {
      typename DiscreteFunctionDefaultType::Traits<codim>::LocalFunctionIteratorType tmp
      //typename Traits<codim>::LocalFunctionIteratorType tmp
        (functionSpace_, const_cast<GridType &>
        (functionSpace_.getGrid()).lend<codim> (level) , dofVec_ );
      return tmp;
    };


    // we use the default implementation
    // Warning!!! returns reference to local object!
    GlobalDofIteratorType dbegin ( int level )
    {
      GlobalDofIteratorType tmp ( dofVec_ [level] , 0 );
      return tmp;
    };

    //! points behind the last dof of type cc
    // Warning!!! returns reference to local object!
    GlobalDofIteratorType dend   ( int level )
    {
      GlobalDofIteratorType tmp ( dofVec_ [ level ] , dofVec_[ level ].size() );
      return tmp;
    };

    void clear( ) {
      GlobalDofIteratorType enddof = dend ( level_ );
      for(GlobalDofIteratorType itdof = dbegin ( level_ ); itdof != enddof; ++itdof)
      {
        *itdof = 0.;
      }
    }

    void setAll( DofType x ) {
      set( x, level_ );
    }

    //! print all dofs
    void print()
    {
      DofType sum = 0.;
      GlobalDofIteratorType enddof = dend ( level_ );
      for(GlobalDofIteratorType itdof = dbegin ( level_ ); itdof != enddof; ++itdof) {
        std::cout << (*itdof) << " Dof \n";
        sum += *itdof;
      }
      std::cerr << "sum = " << sum << "\n";
    }

    void save(const char *filename) {
      std::ofstream out( filename );
      out << "P2\n129 129\n255\n";
      GlobalDofIteratorType enddof = dend ( level_ );
      for(GlobalDofIteratorType itdof = dbegin ( level_ ); itdof != enddof; ++itdof) {
        out << (int)((*itdof)*255.) << "\n";
      }
      out.close();
    }

    void load(const char *filename) {
      FILE *in;
      int v;
      in = fopen( filename, "r" );
      fscanf( in, "P2\n%d %d\n%d\n", &v, &v, &v );
      GlobalDofIteratorType enddof = dend ( level_ );
      for(GlobalDofIteratorType itdof = dbegin ( level_ ); itdof != enddof; ++itdof) {
        fscanf( in, "%d", &v );
        std::cerr << v << "  ";
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

#endif
