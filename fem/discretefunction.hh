// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCRETEFUNCTION_HH__
#define __DUNE_DISCRETEFUNCTION_HH__

#include "../common/function.hh"
#include "../common/functionspace.hh"
#include "discretefunctionspace.hh"
#include "localfunctionarray.hh"
#include "dofiterator.hh"

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
      typedef LocalFunctionIteratorImp<cc, DiscreteFunctionSpaceType> LocalFunctionIteratorType;
    };

    typedef Traits<0>::GridType GridType;
    typedef DofIteratorImp DofIteratorType;

    DiscreteFunctionInterface ( const DiscreteFunctionSpaceType &f )
      : FunctionType ( f ) {} ;

    //! iterator for iteratation over all dof of one level
    //! for cc = 0 it iterates over dof entities
    //! for cc = max over all dofs
    template <int codim>
    LocalFunctionIteratorImp<codim, DiscreteFunctionSpaceType>
    lfbegin (typename Traits<codim>::GridType &grid, int level )
    {
      return asImp().lfbegin<codim>( grid, level );
    };

    //! points behind the last dof of type cc
    template <int codim>
    LocalFunctionIteratorImp<codim, DiscreteFunctionSpaceType>
    lfend (typename Traits<codim>::GridType &grid, int level )
    {
      return asImp().lfend<codim>( grid, level );
    };

    //! the implementation of an iterator to iterate efficient over all dof
    //! on one level
    DofIteratorType dbegin (typename Traits<0>::GridType &grid, int level )
    {
      return asImp().dbegin ( grid , level );
    };

    //! the implementation of an iterator to iterate efficient over all dof
    //! on one level
    DofIteratorType dend (typename Traits<0>::GridType &grid, int level )
    {
      return asImp().dend ( grid , level );
    };

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
      typedef typename DiscreteFunctionSpaceType::GridType GridType;
      typedef typename DiscreteFunctionSpaceType::Domain Domain;
      typedef typename DiscreteFunctionSpaceType::Range Range;
      typedef LocalFunctionIteratorImp<cc, DiscreteFunctionSpaceType> LocalFunctionIteratorType;
    };
    typedef typename DiscreteFunctionInterfaceType::Traits<0>::GridType GridType;

    typedef typename GlobalDofIteratorImp GlobalDofIteratorType;


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

#if 0
    //! access to dof entity corresponding grid entity en
    //! default implementation is via GlobalDofIterator
    template <int codim , class EntityType>
    typename Traits<codim>::LocalFunctionIteratorType & access (EntityType & en )
    {
      LocalFunctionIteratorImp<codim> endit = dend<codim> ( functionSpace_.getGrid() , en.level() );
      for(LocalFunctionIteratorImp<codim> it = dbegin<codim>( functionSpace_.getGrid() , en.level() ); it != endit; ++it)
      {
        if(it.index() == en.index())
          return it;
      }
      std::cout << "Warning, no dof entity found in access! \n";
      return endit;
    };
#endif

#if 0
    //! Default Implementation , take the LocalFunctionIterator ...
    template <class GridType>
    GlobalDofIteratorType dbegin ( GridType &grid , int level )
    {
      CompileTimeChecker < GlobalDofIteratorType::IamTheGlobalDofIterator > check;

      enum { alldofs = GridType::dimension + 1};
      typedef DofIteratorDefaultImp < LocalFunctionIterator< alldofs > >
      GlobalDofIteratorDef;

      LocalFunctionIterator < alldofs > it = ebegin <alldofs> ( grid , level );
      GlobalDofIteratorDef tmp ( it );
      return tmp;
    };

    template <class GridType>
    GlobalDofIteratorType dend ( GridType &grid , int level )
    {
      CompileTimeChecker < GlobalDofIteratorType::IamTheGlobalDofIterator > check;

      enum { alldofs = GridType::dimension + 1};
      typedef DofIteratorDefaultImp < LocalFunctionIterator< alldofs > >
      GlobalDofIteratorDef;

      LocalFunctionIterator < alldofs > it = eend <alldofs> ( grid , level );
      GlobalDofIteratorDef tmp ( it );
      return tmp;
    };
#endif

  protected:
    //! know your discrete function space
    //const DiscreteFunctionSpaceType & functionSpace_;
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
        LocalFunctionArrayIterator , DofIteratorArray < typename
            DiscreteFunctionSpaceType::RangeField > ,
        DiscFuncTest <DiscreteFunctionSpaceType > >
    DiscreteFunctionDefaultType;

    typedef typename DiscreteFunctionSpaceType::RangeField DofType;

    typedef LocalFunctionArrayIterator<0, DiscreteFunctionSpaceType > LocalFunctionIteratorType;


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

    //! access to dof entity corresponding grid entity en
    //! default implementation is via GlobalDofIterator
    template <class EntityType>
    LocalFunctionType & access (EntityType & en )
    {
      localFunc_.init( en );
      return localFunc_;
    };
    // ***********  Interface  *************************

    //! iterator for iteratation over all dof of one level
    //! for cc = 0 the local function contains all dof on the entity<0>
    //! and so on
    template <int codim>
    LocalFunctionArrayIterator<codim , DiscreteFunctionSpaceType >
    lfbegin ( int level )
    {
      typename Traits<codim>::LocalFunctionIteratorType tmp
        (functionSpace_, const_cast<GridType &>
        (functionSpace_.getGrid()).lbegin<codim> (level) , dofVec_ );
      return tmp;
    };

    //! points behind the last dof of type cc
    template <int codim>
    LocalFunctionArrayIterator<codim , DiscreteFunctionSpaceType >
    lfend ( int level )
    {
      typename Traits<codim>::LocalFunctionIteratorType tmp
        (functionSpace_, const_cast<GridType &>
        (functionSpace_.getGrid()).lend<codim> (level) , dofVec_ );
      return tmp;
    };


    // we use the default implementation
    GlobalDofIteratorType & dbegin ( int level )
    {
      GlobalDofIteratorType tmp ( dofVec_ [level] , 0 );
      return tmp;
    };

    //! points behind the last dof of type cc
    GlobalDofIteratorType & dend   ( int level )
    {
      GlobalDofIteratorType tmp ( dofVec_ [ level ] , dofVec_[ level ].size() );
      return tmp;
    };

    //! print all dofs
    void print()
    {
      GlobalDofIteratorType enddof = dend ( level_ );
      for(GlobalDofIteratorType itdof = dbegin ( level_ ); itdof != enddof; ++itdof)
      {
        std::cout << (*itdof) << " Dof \n";
      }
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

    std::vector < Array < DofType > > dofVec_;
    //Array < DofType > dofVec_;
  };

  // comilation time check
  //template <bool> struct CompileTimeChecker;
  //template <> struct CompileTimeChecker<true> {};

#if 0
  template<class DiscreteFunctionSpaceType, class LocalFunctionType>
  class DiscreteFunction : Function < DiscreteFunctionSpaceType,
                               DiscreteFunction<DiscreteFunctionSpaceType,LocalFunctionType> >
  {

  public:
    DiscreteFunction ( DiscreteFunctionSpaceType & f)
      : Function < Field, DiscreteFunctionSpaceType>( f )  {} ;

    void evaluate ( const Domain & , Range &) const ;

    template <class EvalEntityType>
    void evaluate ( const EvalEntityType &, const Domain &, Range & );

    void gradient ( const Domain &, GradientRange &) const;

    void hessian  ( const Domain &, HessianRange &) const;

  };

  template<class DiscreteFunctionSpaceType, class DofStorageType ,
      class LocalFunctionType >
  class DiscFuncLocalFunc : Function < DiscreteFunctionSpaceType,
                                DiscFuncLocalFunc<DiscreteFunctionSpaceType, DofStorageType ,
                                    LocalFunctionType   > >
  {
    //! just for this implementation
    typedef Vec<0,char> EvaluateType;

  public:
    DiscFuncLocalFunc ( DiscreteFunctionSpaceType & f,
                        int level , int codim , bool flag )
      : Function < DiscreteFunctionSpaceType, DiscFuncLocalFunc<DiscreteFunctionSpaceType ,
                DofStorageType , LocalFunctionType > > ( f ) ,
        level_ ( level ) , dofStorage_ ( f, level , codim, flag ) ,
        localFunc_ ( f , dofStorage_ )
    {
      std::cout << "DiscFuncLocalFunc () done! \n";
    };

    template <class FunctionType >
    void setFunction ( FunctionType &func)
    {
      // some useful typedefs
      typedef DiscreteFunctionSpaceType::GridType::Traits<0>::LevelIterator LevelIterator;
      typedef DiscreteFunctionSpaceType::GridType GridType;

      // const is annoying
      GridType & grid = const_cast<GridType &> (functionSpace_.getGrid());

#if 0
      if(levOcu_ > 1)
      {
        for(int level=0; level < levOcu_-1; level++)
        {
          // search the element in which point x lies
          LevelIterator endit = grid.lend<0>(level);
          for(LevelIterator it = grid.lbegin<0>(level);
              it != endit; ++it)
          {}
        }
      }
#endif

      std::cout << "setFunction ... ";
      LevelIterator endit = grid.lend<0>(level_);
      for(LevelIterator it = grid.lbegin<0>(level_); it != endit; ++it)
      {
        typedef DofStorageType::Traits::LocalDofIterator Iter;

        const FastBaseFunctionSet < DiscreteFunctionSpaceType >& baseSet
          = ((functionSpace_).getBaseFunctionSet( *it ));

        Range tmp (0.0);
        Iter dofend = dofStorage_.endLocal( *it );
        for(Iter dofit = dofStorage_.beginLocal( *it ); dofit != dofend; ++dofit )
        {
          func.evaluate( it->geometry()[dofit.index()] , tmp);
          (*dofit) = tmp.read(0);
        }
      }

      typedef DofStorageType::Traits::GlobalDofIterator Iter;

      std::cout << "Check the global dof iterator! \n";
      Iter endglob = dofStorage_.endGlobal( grid , level_ );
      for(Iter it = dofStorage_.beginGlobal( grid, level_ ); it != endglob; ++it)
      {
        std::cout << (*it) << " Dofval \n";
      }
      std::cout << " done!\n";
    };

    //! evaluate on a given point x
    void evaluate ( const Domain & x , Range & ret)
    {
      // some useful typedefs
      typedef DiscreteFunctionSpaceType::GridType::Traits<0>::LevelIterator LevelIterator;
      typedef DiscreteFunctionSpaceType::GridType GridType;

      // const is annoying sometimes , to be discussed
      GridType & grid = const_cast<GridType &> (functionSpace_.getGrid());

      // search the element in which point x lies
      LevelIterator endit = grid.lend<0>(0);
      for(LevelIterator it = grid.lbegin<0>(0);
          it != endit; ++it)
      {
        if(it->geometry().pointIsInside(x))
        {
          if(it->level() != grid.maxlevel()-1)
          {
            // go down the tree
            goDeeper(*it, x, ret, grid.maxlevel()-1 );
            return;
          }
          else
          {
            // evalutae on entity
            evaluate ( (*it) , x , ret );
            return;
          }
          break;
        }
      }

      // if no element is found throw error message
      std::cerr << "No Element found with Point ";
      x.print(std::cout,1); std::cout << std::endl;
      // if no element is found, then an element with zero entries is returned
      ret = 0.0;
    };

    //! evaluate the function on an given entity
    template <class EvalEntityType>
    void evaluate ( EvalEntityType &en, const Domain &x, Range &ret )
    {
      EvaluateType diffVar;
      localFunc_.init(en);
      localFunc_.evaluate(en,diffVar,x,ret);
      ret.print(std::cout , 3);
    };

    LocalFunctionIteratorType beginLocalFunc (int level)
    {};

    LocalFunctionIteratorType endLocalFunc (int level)
    {};

  private:
    //! go down the hierarchical tree
    template <class EntityType>
    void goDeeper ( EntityType &it, const Domain &x , Range &ret, int maxlevel )
    {
      typedef typename EntityType::Traits::HierarchicIterator HIERit;
      HIERit hierend = it.hend(maxlevel);
      for(HIERit hierit = it.hbegin(maxlevel); hierit != hierend; ++hierit)
      {
        // Hier kann noch Option mit Levelabfrage eingebaut werden
        if((hierit->geometry().pointIsInside(x)) && (hierit->level() == maxlevel ) )
        {
          evaluate((*hierit),x,ret);
          return;
        }

      }
    };

    //! occupied levels
    int levOcu_;

    //! maxlevel which is occupied
    int level_;

    //! Vector of Array for each level, the array holds the dofs of each
    //! level
    DofStorageType dofStorage_;

    LocalFunctionType localFunc_;

  };
#endif


  //#include "discretefunction.cc"

}

#endif
