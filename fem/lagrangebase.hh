// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE__LAGRANGEBASE_H__
#define __DUNE__LAGRANGEBASE_H__

#include "../grid/common/grid.hh"
#include "fastbase.hh"
#include "discretefunctionspace.hh"
#include "quadtriangle.hh"
#include "quadlqh.hh"


namespace Dune {

  //! definition of LagrangeBaseFunction, implementation via specialization
  template<class FunctionSpaceType, ElementType ElType, int polOrd>
  class LagrangeBaseFunction;


  template<class FunctionSpaceType>
  class LagrangeBaseFunction < FunctionSpaceType , triangle , 1 >
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    // alpha = 1, falls baseNum = 1 ...
    char alpha,beta,gamma;
    //  enum { alpha = 1 - baseNum };
    //  enum { beta  = (alpha == -1) ? alpha : baseNum };
    //  enum { gamma = ( baseNum == 2) ? 1 : 0 };

  public:

    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum  )
      : BaseFunctionInterface<FunctionSpaceType> (f)
    {
      alpha = (1 - baseNum);
      beta  = ((alpha == -1) ? alpha : baseNum);
      gamma = (( baseNum == 2) ? 1 : 0 );
    }

    virtual void evaluate ( const Vec<0,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      //std::cout << "Evaluate phi0 \n";
      phi = alpha * x.read(0) + beta * x.read(1) + gamma ;
    }

    virtual void evaluate ( const Vec<1,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // derivate phi_0
      if((diffVariable.read(0) == 0) && (alpha == 1))
      {
        phi = 1.0;
        return;
      }

      // derivate phi_1
      if((diffVariable.read(0) == 1) && (beta == 1))
      {
        phi = 1.0;
        return;
      }

      // derivate phi_2
      if(gamma == 1)
      {
        if(diffVariable.read(0) == 0)
        {
          phi = alpha;
          return;
        }
        if(diffVariable.read(0) == 1)
        {
          phi = beta;
          return;
        }
      }

      std::cout << "No fitting derivative found! \n";
      phi = 0.0;
    }

    virtual void evaluate ( const Vec<2,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // function is linear, therfore
      phi = 0.0 ;
    }
  };

  //*********************************************************************
  //
  //! Bilinear BaseFunctions for quadrilaterals
  //! v(x,y) = (alpha + beta * x) * ( gamma + delta * y)
  //! see W. Hackbusch, page 162
  //
  //*********************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction<FunctionSpaceType,quadrilateral,1>
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    //! phi(x,y) = (alpha + beta * x) * ( gamma + delta * y)
    //
    char alpha,beta,gamma,delta;
#if 0
    enum { alpha = ( baseNum%2 == 0 ) ?  1 : 0 };
    enum { beta  = ( baseNum%2 == 0 ) ? -1 : 1 };
    enum { gamma = ( baseNum < 2    ) ?  1 : 0 };
    enum { delta = ( baseNum < 2    ) ? -1 : 1 };
#endif
  public:

    //! Constructor making base function number baseNum
    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum )
      : BaseFunctionInterface<FunctionSpaceType>(f)
    {
      alpha = ( baseNum%2 == 0 ) ?  1 : 0 ;
      beta  = ( baseNum%2 == 0 ) ? -1 : 1 ;
      gamma = ( baseNum < 2    ) ?  1 : 0 ;
      delta = ( baseNum < 2    ) ? -1 : 1 ;
    };

    //! evaluate the basefunction on point x
    virtual void evaluate ( const Vec<0,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // supposse that phi is element R
      phi = (alpha + beta * x.read(0)) * ( gamma + delta * x.read(1));
    }

    //! derivative with respect to x or y
    //! diffVariable(0) == 0   ==> x
    //! diffVariable(0) == 1   ==> y
    //! diffVariable(0) == 2   ==> z,  and so on
    virtual void evaluate ( const Vec<1,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      if(diffVariable.read(0)) // differtiate to x component
      {
        phi = beta * ( gamma + delta * x.read(1));
        return;
      }
      else // differtiate to y component
      {
        phi = (alpha + beta * x.read(0)) * delta;
        return;
      }
      phi = 0.0;
    }

    virtual void evaluate ( const Vec<2,char> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // which means derivative xx or yy
      if(diffVariable.read(0) == diffVariable.read(1))
      {
        phi = 0.0;
        return;
      }
      // which means derivative xy or yx
      else
      {
        phi = beta * delta;
        return;
      }
      phi = 0.0;
    }

  };

  //! default definition stays empty because implementation via
  //! specialization
  template <ElementType ElType, int polOrd > struct LagrangeDefinition;

  template <int polOrd >
  struct LagrangeDefinition< triangle , polOrd>
  {
    enum { numOfBaseFct = 3 * polOrd };
  };

  template <int polOrd >
  struct LagrangeDefinition< quadrilateral , polOrd>
  {
    enum { numOfBaseFct = 4 * polOrd };
  };


  //*********************************************************************
  //
  //  --LinFastBaseFunctionSet
  //
  //! This class is in charge for setting the correct pointers for the
  //! FastBaseFunctionSet via the Constructor of this class
  //
  //*********************************************************************
  template<class FunctionSpaceType, ElementType ElType, int polOrd >
  class LagrangeFastBaseFunctionSet
    : public FastBaseFunctionSet<FunctionSpaceType >
  {
    //! know the number of base functions
    enum { numOfBaseFct = LagrangeDefinition < ElType, polOrd >::numOfBaseFct };

    //! type of LagrangeBaseFunctions
    typedef LagrangeBaseFunction < FunctionSpaceType , ElType , polOrd >
    LagrangeBaseFunctionType;
  public:

    //! Constructor, calls Constructor of FastBaseFunctionSet, which is the
    //! InterfaceImplementation
    LagrangeFastBaseFunctionSet( FunctionSpaceType &fuSpace )
      :  FastBaseFunctionSet<FunctionSpaceType >
          ( fuSpace, numOfBaseFct )
    {
      for(int i=0; i<numOfBaseFct; i++)
      {
        baseFuncList_(i) = new LagrangeBaseFunctionType ( fuSpace, i ) ;
        setBaseFunctionPointer ( i , baseFuncList_ (i) );
      }
    };

    //! Destructor deleting the base functions
    ~LagrangeFastBaseFunctionSet( )
    {
      for(int i=0; i<numOfBaseFct; i++)
        delete baseFuncList_(i);
    };

    //! return number of base function for this base function set
    int getNumberOfBaseFunctions() const { return numOfBaseFct; };
  private:
    //! Vector with all base functions corresponding to the base function set
    Vec < numOfBaseFct , LagrangeBaseFunctionType *> baseFuncList_;
  };


  //************************************************************************
  //
  //  --LagrangeMapper
  //
  //! This Class knows what the space dimension is and how to map for a
  //! given grid entity from local dof number to global dof number
  //
  //************************************************************************
  template <int polOrd>
  class LagrangeMapper
    : public MapperDefault < LagrangeMapper <polOrd> >
  {
  public:

    //! default is Lagrange with polOrd = 1
    template <class GridType>
    int size (const GridType &grid , int level ) const
    {
      // return number of vertices
      return grid.size( level , GridType::dimension );
    };

    //! map Entity an local Dof number to global Dof number
    //! for Lagrange with polOrd = 1
    template <class EntityType>
    int mapToGlobal (EntityType &en, int localNum ) const
    {
      enum { codim = EntityType::dimension };
      // return vertex number
      return (*en.entity<codim>( localNum )).index();
    };

  };

  //****************************************************************
  //
  //  --LagrangeDiscreteFunctionSpace
  //
  //! Provides access to base function set for different element
  //! type in one grid and size of functionspace
  //! and map from local to global dof number
  //
  //****************************************************************
  template< class FunctionSpaceType, class GridType, int polOrd  >
  class LagrangeDiscreteFunctionSpace
    : public DiscreteFunctionSpaceInterface <  FunctionSpaceType , GridType,
          LagrangeDiscreteFunctionSpace < FunctionSpaceType , GridType, polOrd >,
          FastBaseFunctionSet < LagrangeDiscreteFunctionSpace
              < FunctionSpaceType , GridType, polOrd   > > >
  {

    typedef DiscreteFunctionSpaceInterface <
        FunctionSpaceType , GridType,
        LagrangeDiscreteFunctionSpace < FunctionSpaceType , GridType, polOrd >,
        FastBaseFunctionSet < LagrangeDiscreteFunctionSpace
            < FunctionSpaceType , GridType, polOrd   > > >  DiscreteFunctionSpaceType;

    typedef LagrangeDiscreteFunctionSpace
    < FunctionSpaceType , GridType , polOrd > LagrangeDiscreteFunctionSpaceType;
    typedef LagrangeFastBaseFunctionSet < LagrangeDiscreteFunctionSpaceType, triangle , 1 > LagrangeFastBaseFunctionSetType;
    typedef FastBaseFunctionSet < LagrangeDiscreteFunctionSpaceType > FastBaseFunctionSetType;

    // id is  neighbor of the beast
    static const IdentifierType id = 665;

    // Lagrange 1 , to be revised in this matter
    enum { numOfDiffBase_ = 20 };

  public:
    //! Constructor generating for each different element type of the grid a
    //! LagrangeBaseSet with polOrd
    LagrangeDiscreteFunctionSpace ( GridType & g ) :
      DiscreteFunctionSpaceType (g,id) // ,baseFuncSet_(*this)  { };
    {
      std::cout << "Constructor of LagrangeDiscreteFunctionSpace! \n";
      for(int i=0; i<numOfDiffBase_; i++)
        baseFuncSet_(i) = NULL;

      typedef GridType::Traits<0>::LevelIterator LevelIterator;
      LevelIterator endit = g.lend<0>(0);
      for(LevelIterator it = g.lbegin<0>(0); it != endit; ++it)
      {
        ElementType type = (*it).geometry().type(); // Hack
        if(baseFuncSet_( type ) == NULL )
          baseFuncSet_ ( type ) = setBaseFuncSetPointer(*it);
      }
    }

    ~LagrangeDiscreteFunctionSpace ( )
    {
      for(int i=0; i<numOfDiffBase_; i++)
        if (baseFuncSet_(i) != NULL)
          delete baseFuncSet_(i);
    }

    //! provide the access to the base function set
    template <class EntityType>
    const FastBaseFunctionSetType& getBaseFunctionSet ( EntityType &en ) const
    {
      ElementType type =  en.geometry().type();
      if( baseFuncSet_.read ( type ) == NULL )
      {
        std::cerr << "BaseFunctionSetPointer points to NULL! \n";
        abort();
      }
      return (*baseFuncSet_.read( type ));
    };

    //! length of the dof vector
    //! size knows the correct way to calculate the size of the functionspace
    int size ( int level ) const
    {
      return mapper_.size ( grid_ ,level );
    };

    //! for given entity map local dof number to global dof number
    template <class EntityType>
    int mapToGlobal ( EntityType &en, int localNum ) const
    {
      return mapper_.mapToGlobal ( en , localNum );
    };

  protected:

    //! get the right BaseFunctionSet for a given Entity
    template <class EntityType>
    FastBaseFunctionSetType* setBaseFuncSetPointer ( EntityType &en )
    {
      switch (en.geometry().type())
      {
      case triangle :      return makeBaseSet<triangle,polOrd> ();
      case quadrilateral : return makeBaseSet<quadrilateral,polOrd> ();
      default : {
        std::cerr << en.geometry().type() << " This element type is not provided yet! \n";
        abort();
        return NULL;
      }
      }
    }

    //! make base function set depending on ElementType and polynomial order
    template <ElementType ElType, int pO >
    FastBaseFunctionSetType* makeBaseSet ()
    {
      typedef LagrangeFastBaseFunctionSet < LagrangeDiscreteFunctionSpaceType,
          ElType, pO > BaseFuncSetType;

      BaseFuncSetType * baseFuncSet = new BaseFuncSetType ( *this );
      return baseFuncSet;
    }

    //! the corresponding vector of base function sets
    //! lenght is diffrent element types
    Vec< numOfDiffBase_ , FastBaseFunctionSetType * > baseFuncSet_;

    //! the corresponding LagrangeMapper
    LagrangeMapper<polOrd> mapper_;

  }; // end class LinDiscreteFunctionSpace


} // end namespace Dune
#endif
