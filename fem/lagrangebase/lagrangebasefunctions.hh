// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE__LAGRANGEBASEFUNCTIONS_HH__
#define __DUNE__LAGRANGEBASEFUNCTIONS_HH__

#include <dune/common/matvec.hh>
#include <dune/common/array.hh>
#include <dune/grid/common/grid.hh>

#include <dune/fem/common/fastbase.hh>

namespace Dune {

  //! definition of LagrangeBaseFunction, implementation via specialization
  template<class FunctionSpaceType, GeometryType ElType, int polOrd>
  class LagrangeBaseFunction;

  //! Piecewise const base functions
  template<class FunctionSpaceType, GeometryType ElType>
  class LagrangeBaseFunction < FunctionSpaceType , ElType , 0 >
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    enum { DimRange = FunctionSpaceType::DimRange };
    int baseNum_;

    typedef typename FunctionSpaceType::Domain Domain;
    typedef typename FunctionSpaceType::Range Range;
    typedef typename FunctionSpaceType::RangeField RangeField;

  public:
    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum  )
      : BaseFunctionInterface<FunctionSpaceType> (f) , baseNum_ ( baseNum )
    {
      assert((baseNum_ >= 0) || (baseNum_ < DimRange));
    };

    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      phi = 0.0;
      phi[baseNum_] = 1.0;
    }

    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      phi = 0.0;
    }

    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
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
    typedef typename FunctionSpaceType::Domain Domain;
    typedef typename FunctionSpaceType::Range Range;
    typedef typename FunctionSpaceType::RangeField RangeField;
    RangeField factor[2];

  public:
    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum  )
      : BaseFunctionInterface<FunctionSpaceType> (f)
    {
      if(baseNum == 0)
      { // 1 - x
        factor[0] = -1.0;
        factor[1] =  1.0;
      }
      else
      { // x
        factor[0] = 1.0;
        factor[1] = 0.0;
      }
    }

    //! evaluate the function
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      phi = factor[1];
      phi += factor[0] * x[0];
    }

    //! evaluate first derivative
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      int num = diffVariable[0];
      phi = factor[num];
    }

    //! evaluate second derivative
    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // function is linear, therefore
      phi = 0.0 ;
    }

  };

  //*****************************************************************
  //
  /** \brief ???

     \verbatim
     (0,1)
      2|\    coordinates and local node numbers
   | \
   |  \
   |   \
   |    \
   |     \
      0|______\1
      (0,0)    (1,0)
     \endverbatim
   */
  //
  //*****************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction < FunctionSpaceType , triangle , 1 >
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::Domain Domain;
    typedef typename FunctionSpaceType::Range Range;
    typedef typename FunctionSpaceType::RangeField RangeField;
    RangeField factor[3];
    int baseNum_;

  public:
    //! \todo Please doc me!
    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum  )
      : BaseFunctionInterface<FunctionSpaceType> (f)
    {
      baseNum_ = baseNum;
      if(baseNum == 0)
      { // 1 - x - y
        factor[0] =  1.0;
        factor[1] = -1.0;
        factor[2] = -1.0;
      }
      else
      {
        factor[0] = 0.0;
        for(int i=1; i<3; i++) // x , y
          if(baseNum == i)
            factor[i] = 1.0;
          else
            factor[i] = 0.0;
      }
    }
    //! evaluate the function
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      phi = factor[0];
      for(int i=1; i<3; i++)
        phi += factor[i] * x[i-1];
    }

    //! \todo Please doc me!
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // x or y ==> 1 or 2
      int num = diffVariable[0];
      assert( (num >= 0) && ( num <= 1));
      phi = factor[num+1];
    }

    //! \todo Please doc me!
    virtual void evaluate ( const DiffVariable<2>::Type &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // function is linear, therefore
      phi = 0.0 ;
    }
  };

  //*****************************************************************
  //
  //! LagrangeBaseFunction for tetrahedrons and polynom order = 1
  //!
  //!  see reference element Dune tetrahedra
  //!
  //*****************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction < FunctionSpaceType , tetrahedron , 1 >
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::RangeField RangeField;
    typedef typename FunctionSpaceType::Domain Domain;
    typedef typename FunctionSpaceType::Range Range;
    RangeField factor[4];
  public:

    //! \todo Please doc me!
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
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      phi = factor[0];
      for(int i=1; i<4; i++)
        phi += factor[i]*x[i-1];
    }

    //! first Derivative
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // num = 0 ==> derivative respect to x
      int num = diffVariable[0];
      phi = factor[num+1];
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
    typedef typename FunctionSpaceType::RangeField RangeField;
    typedef typename FunctionSpaceType::Domain Domain;
    typedef typename FunctionSpaceType::Range Range;
    enum { dim = 2 };
    //! phi(x,y) = (factor[0][0] + factor[0][1] * x) * ( factor[1][0] + factor[1][1] * y)
    RangeField factor[dim][2];
  public:

    //! Constructor making base function number baseNum
    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum )
      : BaseFunctionInterface<FunctionSpaceType>(f)
    {
      assert((baseNum >= 0) || (baseNum < 4));
      // looks complicated but works
      int fak[dim] = {0,0};

      fak[0] = baseNum%2; // 0,2 ==> 0, 1,3 ==> 1
      fak[1] = (baseNum%4 > 1) ? 1 : 0; // 2,3,6,7 ==> 1 | 0,1,4,5 ==> 0

      // tensor product
      for(int i=0; i<dim; i++)
      {
        factor[i][0] = ( fak[i] == 0 ) ?  1.0 : 0.0 ;
        factor[i][1] = ( fak[i] == 0 ) ? -1.0 : 1.0 ;
      }
    };

    //! evaluate the basefunction on point x
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // dim == 2, tested
      phi = 1.0;
      for(int i=0; i<dim; i++)
        phi *= (factor[i][0] + (factor[i][1] * x[i]));
    }

    //! derivative with respect to x or y
    //! diffVariable(0) == 0   ==> x
    //! diffVariable(0) == 1   ==> y
    //! diffVariable(0) == 2   ==> z,  and so on
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      int num = diffVariable[0];
      assert( (num >= 0) && ( num <= 1));
      phi = 1.0;
      for(int i=0; i<dim; i++)
      {
        if(num == i)
          phi *= factor[num][1];
        else
          phi *= (factor[i][0] + factor[i][1] * x[i]);
      }
      return;
    }

    //! evaluate second derivative
    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // which means derivative xx or yy
      if(diffVariable[0] == diffVariable[1])
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
  //!
  //!
  //! local node numbers and face numbers for DUNE hexahedrons
  //!
  //!             6---------7
  //!            /.        /|
  //!           / .  5    / |
  //!          /  .      /  |
  //!         4---------5   | <-- 3 (back side)
  //!   0 --> |   .     | 1 |
  //!         |   2.....|...3 (1,1,0)
  //!         |  .      |  /
  //!         | .   2   | / <-- 4 (front side)
  //!         |.        |/
  //!         0---------1
  //!      (0,0,0)    (1,0,0)
  //!  this is the DUNE local coordinate system for hexahedrons
  //!
  //*********************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction<FunctionSpaceType,hexahedron,1>
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::RangeField RangeField;
    typedef typename FunctionSpaceType::Domain Domain;
    typedef typename FunctionSpaceType::Range Range;
    enum { dim = 3 };
    //! phi(x,y,z) = (factor[0][0] + factor[0][1] * x)
    //!            = (factor[1][0] + factor[1][1] * y)
    //!            = (factor[2][0] + factor[2][1] * z)
    RangeField factor[dim][2];
  public:

    //! Constructor making base function number baseNum
    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum )
      : BaseFunctionInterface<FunctionSpaceType>(f)
    {
      assert((baseNum >= 0) || (baseNum < 8));
      // looks complicated but works
      int fak[dim] = {0,0,0};
      fak[0] =  baseNum%2; // 0,2 ==> 0, 1,3 ==> 1
      fak[1] = (baseNum%4 > 1) ? 1 : 0; // 2,3,6,7 ==> 1 | 0,1,4,5 ==> 0
      fak[2] = (baseNum > 3) ? 1 : 0;

      // tensor product
      for(int i=0; i<dim; i++)
      {
        factor[i][0] = ( fak[i] == 0 ) ?  1.0 : 0.0 ;
        factor[i][1] = ( fak[i] == 0 ) ? -1.0 : 1.0 ;
      }

    };

    //! evaluate the basefunction on point x
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      // dim == 3, tested
      phi = 1.0;
      for(int i=0; i<dim; i++)
        phi *= (factor[i][0] + (factor[i][1] * x[i]));
    }

    //! derivative with respect to x or y or z
    //! diffVariable(0) == 0   ==> x
    //! diffVariable(0) == 1   ==> y
    //! diffVariable(0) == 2   ==> z,  and so on
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      int num = diffVariable[0];
      phi = 1.0;
      for(int i=0; i<dim; i++)
      {
        if(num == i)
          phi *= factor[num][1];
        else
          phi *= (factor[i][0] + factor[i][1] * x[i]);
      }
      return;
    }

    //! evaluate second derivative
    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const Domain & x, Range & phi) const
    {
      std::cout << "BaseFunction for hexahedron, evaluate 2nd derivative not implemented! \n";
      phi = 0.0;
    }

  };

  //! default definition stays empty because implementation via
  //! specialization
  template <GeometryType ElType, int polOrd ,int dimrange > struct LagrangeDefinition;

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
  /*!
     This class is in charge for setting the correct pointers for the
     FastBaseFunctionSet via the Constructor of this class
     Note that each function space hold a basefunction set of type
     FastBaseFunctionSet, because one grid can contain different element
     types for which we then need different basefunction set :(.
     But this isn't a problem, because we use caching of evaluations of
     basefunction which is always done on the reference element.
     Therefore the basefunction have virtual methods.
   */
  //
  //*********************************************************************
  template<class FunctionSpaceType, GeometryType ElType, int polOrd >
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
      int numOfDifferentFuncs = (int) numOfBaseFct / dimrange;
      for(int i=0; i<numOfDifferentFuncs; i++)
      {
        for(int k=0; k<dimrange; k++)
        {
          baseFuncList_[i*dimrange + k] = new LagrangeBaseFunctionType ( fuSpace, i ) ;
          this->setBaseFunctionPointer ( i*dimrange + k , baseFuncList_[i*dimrange + k] );
        }
      }
      this->setNumOfDiffFct ( numOfDifferentFuncs );
    };

    //! Destructor deleting the base functions
    ~LagrangeFastBaseFunctionSet( )
    {
      for(int i=0; i<numOfBaseFct; i++)
        delete baseFuncList_(i);
    };

    //! return number of base function for this base function set
    int getNumberOfBaseFunctions() const { return numOfBaseFct; };

    //! return number of different basefunction, i.e. we can have more than
    //! one dof a vertices for example
    int getNumberOfDiffrentBaseFunctions () const
    {
      return (int) (numOfBaseFct / dimrange);
    }
  private:
    //! Vector with all base functions corresponding to the base function set
    FieldVector <LagrangeBaseFunctionType*, numOfBaseFct> baseFuncList_;
  };

} // end namespace Dune
#endif
