// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCRETEFUNCTION_HH__
#define __DUNE_DISCRETEFUNCTION_HH__

#include "../common/function.hh"
#include "pwlinfunc.hh"
#include "dofstoragearray.hh"

namespace Dune {


  // comilation time check
  template <bool> struct CompileTimeChecker;
  template <> struct CompileTimeChecker<true> {};


  template<class DiscreteFunctionSpaceType, class LocalFunctionType>
  class DiscreteFunction : Function < DiscreteFunctionSpaceType,
                               DiscreteFunction<DiscreteFunctionSpaceType,LocalFunctionType> >
  {

  public:
    DiscreteFunction ( const DiscreteFunctionSpaceType & f)
      : Function < Field, DiscreteFunctionSpaceType>( f )  {} ;

    void evaluate ( const Domain & , Range &) const ;

    template <class EvalEntityType>
    void evaluate ( const EvalEntityType &, const Domain &, Range & );

    void gradient ( const Domain &, GradientRange &) const;

    void hessian  ( const Domain &, HessianRange &) const;

  };

  template<class DiscreteFunctionSpaceType, class DofStorageType >
  class DiscFuncArray : Function < DiscreteFunctionSpaceType,
                            DiscFuncArray<DiscreteFunctionSpaceType, DofStorageType  > >
  {
    typedef typename DiscreteFunctionSpaceType::RangeField RangeField;
    typedef std::vector < Array<RangeField> > ArrayType;

    typedef Vec<0,char> EvaluateType;

  public:
    DiscFuncArray ( const DiscreteFunctionSpaceType & f,
                    int level , int codim , bool flag )
      : Function < DiscreteFunctionSpaceType, DiscFuncArray<DiscreteFunctionSpaceType , DofStorageType  > > ( f ) ,
        level_ ( level ) , dofStorage_ ( f, level , codim, flag ) //, locFunc_ ( dofVec_ )
    {
      std::cout << "DiscFuncArray () done! \n";
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
      int level = level_;
      LevelIterator endit = grid.lend<0>(level);
      for(LevelIterator it = grid.lbegin<0>(level); it != endit; ++it)
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
      Iter endglob = dofStorage_.endGlobal( grid , level );
      for(Iter it = dofStorage_.beginGlobal( grid, level ); it != endglob; ++it)
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
      ret.print(std::cout,1); std::cout << std::endl;
      // if no element is found, then an element with zero entries is returned
      ret = 0.0;
    };

    //! evaluate the function on an given entity
    template <class EvalEntityType>
    void evaluate ( EvalEntityType &en, const Domain &x, Range &ret )
    {
      EvaluateType diffVar;
      Range tmp;
      ret = 0.0;
      typedef DofStorageType::Traits::LocalDofIterator Iter;

      const FastBaseFunctionSet < DiscreteFunctionSpaceType >& baseSet
        = ((functionSpace_).getBaseFunctionSet( en ));

      Iter endit = dofStorage_.endLocal(en);
      for(Iter it = dofStorage_.beginLocal(en); it != endit; ++it )
      {
        baseSet.evaluate (it.index(), diffVar, (en.geometry().local(x)) ,tmp);
        ret += (*it) * tmp;
      }
      ret.print(std::cout , 3);
    };

    template <class EvalEntityType>
    void evaluate ( int level, const EvalEntityType &, const Domain &, Range & );

    DofStorageType& getDofStorage () { return dofStorage_; }

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

  };

#include "discretefunction.cc"

}

#endif
