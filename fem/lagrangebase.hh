// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE__LAGRANGEBASE_H__
#define __DUNE__LAGRANGEBASE_H__

#include <dune/grid/common/grid.hh>

#include "common/fastbase.hh"
#include "common/discretefunctionspace.hh"
#include "quadtetratri.hh"
#include "quadlqh.hh"


namespace Dune {

  //! definition of LagrangeBaseFunction, implementation via specialization
  template<class FunctionSpaceType, ElementType ElType, int polOrd>
  class LagrangeBaseFunction;

  //! picewise const base functions
  template<class FunctionSpaceType, ElementType ElType>
  class LagrangeBaseFunction < FunctionSpaceType , ElType , 0 >
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    enum { DimRange = FunctionSpaceType::DimRange };
    int baseNum_;
  public:
    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum  )
      : BaseFunctionInterface<FunctionSpaceType> (f) , baseNum_ ( baseNum )
    {
      if((baseNum_ < 0) || (baseNum_ >= DimRange))
      {
        std::cout << "Wrong base number in LagrangeBaseFunction<polOrd = 0>!\n";
        abort();
      }
    };

    virtual void evaluate ( const Vec<0, deriType> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      phi = 0.0;
      phi(baseNum_) = 1.0;
    }

    virtual void evaluate ( const Vec<1, deriType> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      phi = 0.0;
    }

    virtual void evaluate ( const Vec<2,deriType> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      phi = 0.0 ;
    }

  };


  //*****************************************************************
  //
  //! Lagrange base for lines and polynom order = 1
  //! (0) 0-----1 (1)
  //
  //*****************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction < FunctionSpaceType , line , 1 >
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::RangeField RangeField;
    RangeField factor[2];

  public:
    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum  )
      : BaseFunctionInterface<FunctionSpaceType> (f)
    {
      if(baseNum == 0)
      { // x
        factor[0] = 1.0;
        factor[1] = 0.0;
      }
      else
      { // 1 - x
        factor[0] = -1.0;
        factor[1] =  1.0;
      }
    }

    virtual void evaluate ( const Vec<0, deriType> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      phi = factor[1];
      phi += factor[0] * x.get(0);
    }

    virtual void evaluate ( const Vec<1, deriType> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      int num = diffVariable.get(0);
      phi = factor[num];
    }

    virtual void evaluate ( const Vec<2,deriType> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // function is linear, therfore
      phi = 0.0 ;
    }

  };

  //*****************************************************************
  //
  //!    (0,1)
  //!     1|\    coordinates and local node numbers
  //!      | \
  // //!      |  \
  //!      |   \
  // //!      |    \
  // //!      |     \
  // //!     2|______\0
  //!    (0,0)    (1,0)
  //
  //*****************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction < FunctionSpaceType , triangle , 1 >
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::RangeField RangeField;
    RangeField factor[3];
    int baseNum_;

  public:
    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum  )
      : BaseFunctionInterface<FunctionSpaceType> (f)
    {
      baseNum_ = baseNum;
      if(baseNum == 2)
      { // 1 - x - y
        factor[0] = -1.0;
        factor[1] = -1.0;
        factor[2] =  1.0;
      }
      else
      {
        factor[2] = 0.0;
        for(int i=0; i<2; i++) // x , y
          if(baseNum == i)
            factor[i] = 1.0;
          else
            factor[i] = 0.0;
      }
    }
    virtual void evaluate ( const Vec<0, deriType> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      phi = factor[2];
      for(int i=0; i<2; i++)
        phi += factor[i] * x.get(i);
    }

    virtual void evaluate ( const Vec<1, deriType> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // x or y ==> 1 or 2
      int num = diffVariable.get(0);
      phi = factor[num];
    }

    virtual void evaluate ( const DiffVariable<2>::Type &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // function is linear, therfore
      phi = 0.0 ;
    }
  };

  //*****************************************************************
  //
  //! LagrangeBaseFunction for tetrahedrons and polynom order = 1
  //!
  //!
  //!             1 (1,1,1)
  //!            /|\        coordinate and local node numbers
  //!           / | \       ok, this is ascii drawing
  //!          /  |  \
  // //!(1,1,0) 2/...|...\3 (1,0,0)          z
  //!         \   |   /              y\   |   /x  coordinate
  //!          \  |  /                 \  |  /    system
  //!           \ | /                   \ | /
  //!            \|/                     \|/
  //!             0 (0,0,0)
  //
  //*****************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction < FunctionSpaceType , tetrahedron , 1 >
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::RangeField RangeField;
    RangeField factor[4];
  public:

    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum  )
      : BaseFunctionInterface<FunctionSpaceType> (f)
    {
      for(int i=1; i<4; i++) // x,y,z
        if(baseNum == i)
          factor[i] = 1.0;
        else
          factor[i] = 0.0;

      if(baseNum == 0) // 1 - x - y - z
      {
        for(int i=1; i<4; i++)
          factor[i] = -1.0;
        factor[0] = 1.0;
      }
      else
        factor[0] = 0.0;
    }

    //! evaluate function
    virtual void evaluate ( const Vec<0, deriType> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      phi = factor[0];
      for(int i=1; i<4; i++)
        phi += factor[i]*x.get(i-1);
    }

    //! first Derivative
    virtual void evaluate ( const Vec<1, deriType> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // num = 0 ==> derivative respect to x
      int num = diffVariable.get(0);
      phi = factor[num+1];
      //phi.print(std::cout,1); std::cout << "\n*************\n";
    }

    //! second Derivative
    virtual void evaluate ( const DiffVariable<2>::Type &diffVariable,
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
    enum { dim = 2 };
    //! phi(x,y) = (factor[0][0] + factor[0][1] * x) * ( factor[1][0] + factor[1][1] * y)
    typedef typename FunctionSpaceType::RangeField RangeField;
    RangeField factor[dim][2];
  public:

    //! Constructor making base function number baseNum
    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum )
      : BaseFunctionInterface<FunctionSpaceType>(f)
    {
      if((baseNum < 0) || (baseNum > 3))
      {
        std::cout << "Wrong baseNum given to LagrangeBase for quadrilaterals \n";
        abort();
      }
      // looks complicated but works
      int fak[dim] = {0,0};

      fak[0] = baseNum%2; // 0,2 ==> 0, 1,3 ==> 1
      fak[1] = (baseNum%4 > 1) ? 1 : 0; // 2,3,6,7 ==> 1 | 0,1,4,5 ==> 0

#if 0
      switch(baseNum)
      {
      case 0 : {
        break;
      }
      case 1 : {
        fak[0] = 1; break;
      }
      case 2 : {
        fak[1] = 1; break;
      }
      case 3 : {
        fak[0] = 1; fak[1] = 1; break;
      }
      default :
      {
        std::cout << "Wrong baseNum given to LagrangeBase for quadrilaterals! \n";
        abort();
      }
      }
#endif

      // tensor product
      for(int i=0; i<dim; i++)
      {
        factor[i][0] = ( fak[i] == 0 ) ?  1.0 : 0.0 ;
        factor[i][1] = ( fak[i] == 0 ) ? -1.0 : 1.0 ;
      }
    };

    //! evaluate the basefunction on point x
    virtual void evaluate ( const Vec<0,deriType> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // dim == 2, tested
      phi = 1.0;
      for(int i=0; i<dim; i++)
        phi *= (factor[i][0] + (factor[i][1] * x.get(i)));
    }

    //! derivative with respect to x or y
    //! diffVariable(0) == 0   ==> x
    //! diffVariable(0) == 1   ==> y
    //! diffVariable(0) == 2   ==> z,  and so on
    virtual void evaluate ( const Vec<1,deriType> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      int num = diffVariable.get(0);
      phi = 1.0;
      for(int i=0; i<dim; i++)
      {
        if(num == i)
          phi *= factor[num][1];
        else
          phi *= (factor[i][0] + factor[i][1] * x.get(i));
      }
      return;
    }

    virtual void evaluate ( const Vec<2,deriType> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // which means derivative xx or yy
      if(diffVariable.get(0) == diffVariable.get(1))
      {
        phi = 0.0;
        return;
      }
      // which means derivative xy or yx
      else
      {
        phi = factor[0][1] * factor[1][1];
        return;
      }
      phi = 0.0;
    }

  };



  //*********************************************************************
  //
  //
  //! Trilinear BaseFunctions for hexahedrons
  //! v(x,y,z) = (alpha + beta * x) * ( gamma + delta * y) * (omega + eps * z)
  //
  //
  // local node numbers and face numbers for DUNE hexahedrons
  //
  //             6---------7
  //            /.        /|
  //           / .  5    / |
  //          /  .      /  |
  //         4---------5   | <-- 3 (back side)
  //   0 --> |   .     | 1 |
  //         |   2.....|...3 (1,1,0)
  //         |  .      |  /
  //         | .   2   | / <-- 4 (front side)
  //         |.        |/
  //         0---------1
  //      (0,0,0)    (1,0,0)
  //  this is the DUNE local coordinate system for hexahedrons
  //
  //*********************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction<FunctionSpaceType,hexahedron,1>
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    enum { dim = 3 };
    //! phi(x,y,z) = (factor[0][0] + factor[0][1] * x)
    //!            = (factor[1][0] + factor[1][1] * y)
    //!            = (factor[2][0] + factor[2][1] * z)
    typedef typename FunctionSpaceType::RangeField RangeField;
    RangeField factor[dim][2];
  public:

    //! Constructor making base function number baseNum
    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum )
      : BaseFunctionInterface<FunctionSpaceType>(f)
    {
      if((baseNum < 0) || (baseNum > 7))
      {
        std::cout << "Wrong baseNum given to LagrangeBase for hexahedrons \n";
        abort();
      }
      // looks complicated but works
      int fak[dim] = {0,0,0};
      fak[0] =  baseNum%2; // 0,2 ==> 0, 1,3 ==> 1
      fak[1] = (baseNum%4 > 1) ? 1 : 0; // 2,3,6,7 ==> 1 | 0,1,4,5 ==> 0
      fak[2] = (baseNum > 3) ? 1 : 0;

#if 0
      switch(baseNum)
      {
      case 0 : {
        break;
      }
      case 1 : {
        fak[0] = 1; break;
      }
      case 2 : {
        fak[1] = 1; break;
      }
      case 3 : {
        fak[0] = 1; fak[1] = 1; break;
      }
      case 4 : {
        fak[2] = 1; break;
      }
      case 5 : {
        fak[0] = 1; fak[2] = 1; break;
      }
      case 6 : {
        fak[1] = 1; fak[2] = 1; break;
      }
      case 7 : {
        fak[0] = 1; fak[1] = 1; fak[2] = 1; break;
      }
      default :
      {
        std::cout << "Wrong baseNum given to LagrangeBase for hexahedrons \n";
        abort();
      }

      }
#endif
      // tensor product
      for(int i=0; i<dim; i++)
      {
        factor[i][0] = ( fak[i] == 0 ) ?  1.0 : 0.0 ;
        factor[i][1] = ( fak[i] == 0 ) ? -1.0 : 1.0 ;
      }

    };

    //! evaluate the basefunction on point x
    virtual void evaluate ( const Vec<0,deriType> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // dim == 3, tested
      phi = 1.0;
      for(int i=0; i<dim; i++)
        phi *= (factor[i][0] + (factor[i][1] * x.get(i)));
    }

    //! derivative with respect to x or y or z
    //! diffVariable(0) == 0   ==> x
    //! diffVariable(0) == 1   ==> y
    //! diffVariable(0) == 2   ==> z,  and so on
    virtual void evaluate ( const Vec<1,deriType> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      int num = diffVariable.get(0);
      phi = 1.0;
      for(int i=0; i<dim; i++)
      {
        if(num == i)
          phi *= factor[num][1];
        else
          phi *= (factor[i][0] + factor[i][1] * x.get(i));
      }
      return;
    }

    virtual void evaluate ( const Vec<2,deriType> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      std::cout << "BaseFunction for hexahedron, evaluate 2nd derivative not implemented! \n";
      phi = 0.0;
    }

  };

  //! default definition stays empty because implementation via
  //! specialization
  template <ElementType ElType, int polOrd ,int dimrange > struct LagrangeDefinition;

  //! Lagrange Definition for lines
  template <int polOrd , int dimrange >
  struct LagrangeDefinition< line , polOrd, dimrange >
  {
    enum { numOfBaseFct = (polOrd == 0) ? (1*dimrange) : (dimrange * 2 * polOrd) };
  };

  //! Lagrange Definition for triangles
  template <int polOrd , int dimrange >
  struct LagrangeDefinition< triangle , polOrd, dimrange >
  {
    enum { numOfBaseFct = (polOrd == 0) ? (1*dimrange) : (dimrange * 3 * polOrd) };
  };

  //! Lagrange Definition for quadrilaterals
  template <int polOrd , int dimrange >
  struct LagrangeDefinition< quadrilateral , polOrd, dimrange >
  {
    enum { numOfBaseFct = (polOrd == 0) ? (1*dimrange) : (dimrange * 4 * polOrd) };
  };

  //! Lagrange Definition for tetrahedrons
  template <int polOrd , int dimrange >
  struct LagrangeDefinition< tetrahedron , polOrd, dimrange >
  {
    enum { numOfBaseFct = (polOrd == 0) ? (1*dimrange) : (dimrange * 4 * polOrd) };
  };

  //! Lagrange Definition for hexahedrons
  template <int polOrd , int dimrange >
  struct LagrangeDefinition< hexahedron , polOrd, dimrange >
  {
    enum { numOfBaseFct = (polOrd == 0) ? (1*dimrange) : (dimrange * 8 * polOrd) };
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
    enum { dimrange = FunctionSpaceType::DimRange };
    //! know the number of base functions
    enum { numOfBaseFct = LagrangeDefinition < ElType, polOrd, dimrange >::numOfBaseFct };

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
  template <int polOrd, int dimrange>
  class LagrangeMapper
    : public MapperDefault < LagrangeMapper <polOrd,dimrange> >
  {
    int numLocalDofs_;
  public:
    LagrangeMapper ( int numLocalDofs ) : numLocalDofs_ (numLocalDofs) {}

    //! default is Lagrange with polOrd = 1
    template <class GridType>
    int size (const GridType &grid , int level ) const
    {
      // return number of vertices
      return (dimrange*grid.size( level , GridType::dimension ));
    };

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
      return (dimrange*en.subIndex<codim>(locNum)) + locDim;
    };

  };

  template <int dimrange>
  class LagrangeMapper<0,dimrange>
    : public MapperDefault < LagrangeMapper <0,dimrange> >
  {
  public:
    LagrangeMapper ( int numDofs ) {}

    //! default is Lagrange with polOrd = 1
    template <class GridType>
    int size (const GridType &grid , int level ) const
    {
      // return number of vertices
      return dimrange*grid.size( level , 0 );
    };

    //! map Entity an local Dof number to global Dof number
    //! for Lagrange with polOrd = 1
    template <class EntityType>
    int mapToGlobal (EntityType &en, int localNum ) const
    {
      // get global vertex number
      return (dimrange*en.index()) + localNum;
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
    typedef BaseFunctionSetType FastBaseFunctionSetType;

    //! id is neighbor of the beast
    static const IdentifierType id = 665;

    // Lagrange 1 , to be revised in this matter
    enum { numOfDiffBase_ = 20 };

  public:
    typedef LagrangeDiscreteFunctionSpace < FunctionSpaceType, GridType, polOrd+1 > Next;

    //! remember polynomial order
    enum { polynomialOrder =  polOrd };

    //! Constructor generating for each different element type of the grid a
    //! LagrangeBaseSet with polOrd
    LagrangeDiscreteFunctionSpace ( GridType & g ) :
      DiscreteFunctionSpaceType (g,id) // ,baseFuncSet_(*this)  { };
    {
      mapper_ = NULL;

      //std::cout << "Constructor of LagrangeDiscreteFunctionSpace! \n";
      for(int i=0; i<numOfDiffBase_; i++)
        baseFuncSet_(i) = NULL;

      // search the macro grid for diffrent element types
      typedef typename GridType::Traits<0>::LevelIterator LevelIterator;
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
      return (*baseFuncSet_.get( type ));
    };

    //! return true if we have continuous discrete functions
    bool continuous ( ) const
    {
      bool ret = (polOrd == 0) ? false : true;
      return ret;
    }

    //! get maximal global polynom order
    int polynomOrder ( ) const
    {
      return polOrd;
    }

    //! get local polynom order on entity
    template <class EntityType>
    int localPolynomOrder ( EntityType &en ) const
    {
      return polOrd;
    }

    //! length of the dof vector
    //! size knows the correct way to calculate the size of the functionspace
    int size ( int level ) const
    {
      return mapper_->size ( grid_ ,level );
    };

    //! for given entity map local dof number to global dof number
    template <class EntityType>
    int mapToGlobal ( EntityType &en, int localNum ) const
    {
      return mapper_->mapToGlobal ( en , localNum );
    };

  protected:

    //! get the right BaseFunctionSet for a given Entity
    template <class EntityType>
    FastBaseFunctionSetType* setBaseFuncSetPointer ( EntityType &en )
    {
      switch (en.geometry().type())
      {
      case line         : return makeBaseSet<line,polOrd> ();
      case triangle     : return makeBaseSet<triangle,polOrd> ();
      case quadrilateral : return makeBaseSet<quadrilateral,polOrd> ();
      case tetrahedron  : return makeBaseSet<tetrahedron,polOrd> ();
      case hexahedron   : return makeBaseSet<hexahedron,polOrd> ();
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

      mapper_ = new LagrangeMapper< pO, DimRange >
                  (baseFuncSet->getNumberOfBaseFunctions());

      return baseFuncSet;
    }

    //! the corresponding vector of base function sets
    //! lenght is diffrent element types
    Vec< numOfDiffBase_ , FastBaseFunctionSetType * > baseFuncSet_;
  private:
    //! the corresponding LagrangeMapper
    LagrangeMapper<polOrd,DimRange> *mapper_;

  }; // end class LagrangeDiscreteFunctionSpace


  //***************************************************************************
  //
  //  -- Discontinous Galerkin Space
  //
  //***************************************************************************
  template <int polOrd>
  class DGMapper
    : public MapperDefault < DGMapper <polOrd> >
  {
    int numberOfDofs_;
  public:
    DGMapper ( int numDof ) : numberOfDofs_ (numDof) {};

    template <class GridType>
    int size (const GridType &grid , int level ) const
    {
      // return number of dofs * number of elements
      return (numberOfDofs_ * grid.size( level , 0 ));
    };

    //! map Entity an local Dof number to global Dof number
    template <class EntityType>
    int mapToGlobal (EntityType &en, int localNum ) const
    {
      return ((en.index() * numberOfDofs_) + localNum);
    };

  };

  template< class FunctionSpaceType, class GridType, int polOrd  >
  class DGDiscreteFunctionSpace
    : public LagrangeDiscreteFunctionSpace < FunctionSpaceType , GridType, polOrd >
  {
    typedef LagrangeDiscreteFunctionSpace < FunctionSpaceType , GridType,polOrd >
    LagrangeSpaceType;

  public:
    DGDiscreteFunctionSpace ( GridType & g ) : LagrangeSpaceType (g)
    {
      mapper_ = NULL;
      typedef typename GridType::Traits<0>::LevelIterator LevelIterator;
      LevelIterator endit = g.lend<0>(0);
      for(LevelIterator it = g.lbegin<0>(0); it != endit; ++it)
      {
        if(!mapper_)
        {
          ElementType type = (*it).geometry().type(); // Hack
          int numDofs = (*baseFuncSet_ ( type )).getNumberOfBaseFunctions();

          mapper_ = new DGMapper < polOrd > (numDofs);
          break;
        }
      }
    };

    ~DGDiscreteFunctionSpace ()
    {
      if(mapper_) delete mapper_;
    }

    //! length of the dof vector
    //! size knows the correct way to calculate the size of the functionspace
    int size ( int level ) const
    {
      return mapper_->size ( grid_ ,level );
    };

    //! for given entity map local dof number to global dof number
    template <class EntityType>
    int mapToGlobal ( EntityType &en, int localNum ) const
    {
      return mapper_->mapToGlobal ( en , localNum );
    };

  private:
    DGMapper<polOrd> *mapper_;

  };



} // end namespace Dune
#endif
