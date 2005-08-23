// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_LAGRANGEBASEFUNCTIONS_HH
#define DUNE_LAGRANGEBASEFUNCTIONS_HH

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

    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;

  public:
    LagrangeBaseFunction (int baseNum)
      : BaseFunctionInterface<FunctionSpaceType> (),
        baseNum_ ( baseNum )
    {
      assert((baseNum_ >= 0) || (baseNum_ < DimRange));
    };

    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      phi = 0.0;
      phi[baseNum_] = 1.0;
    }

    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      phi = 0.0;
    }

    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const DomainType & x, RangeType & phi) const
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
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    RangeFieldType factor[2];

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
                            const DomainType & x, RangeType & phi) const
    {
      phi = factor[1];
      phi += factor[0] * x[0];
    }

    //! evaluate first derivative
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      int num = diffVariable[0];
      phi = factor[num];
    }

    //! evaluate second derivative
    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const DomainType & x, RangeType & phi) const
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
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    RangeFieldType factor[3];
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
                            const DomainType & x, RangeType & phi) const
    {
      phi = factor[0];
      for(int i=1; i<3; i++)
        phi += factor[i] * x[i-1];
    }

    //! \todo Please doc me!
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      // x or y ==> 1 or 2
      int num = diffVariable[0];
      assert( (num >= 0) && ( num <= 1));
      phi = factor[num+1];
    }

    //! \todo Please doc me!
    virtual void evaluate ( const DiffVariable<2>::Type &diffVariable,
                            const DomainType & x, RangeType & phi) const
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
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    RangeFieldType factor[4];
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
                            const DomainType & x, RangeType & phi) const
    {
      phi = factor[0];
      for(int i=1; i<4; i++)
        phi += factor[i]*x[i-1];
    }

    //! first Derivative
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      // num = 0 ==> derivative respect to x
      int num = diffVariable[0];
      phi = factor[num+1];
    }

    //! second Derivative
    virtual void evaluate ( const DiffVariable<2>::Type &diffVariable,
                            const DomainType & x, RangeType & phi) const
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
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    enum { dim = 2 };
    //! phi(x,y) = (factor[0][0] + factor[0][1] * x) * ( factor[1][0] + factor[1][1] * y)
    RangeFieldType factor[dim][2];
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
                            const DomainType & x, RangeType & phi) const
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
                            const DomainType & x, RangeType & phi) const
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
                            const DomainType & x, RangeType & phi) const
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

  // *********************************************************************
  //
  //   First-order Lagrange shape functions for the pyramid
  //
  // *********************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction<FunctionSpaceType,pyramid,1>
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    enum { dim = 3 };
    /* phi(x,y,z) = (factor[0] + factor[1]*x)
     *            * (factor[2] + factor[3]*y)
     *            + factor[4] * z * (factor[5] + factor[6]* min(x,y))
     */
    RangeFieldType factor[7];
  public:

    //! Constructor making base function number baseNum
    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum )
      : BaseFunctionInterface<FunctionSpaceType>(f)
    {
      assert((baseNum >= 0) || (baseNum < 5));

      switch (baseNum) {
      case 0 :
        factor[0] = 1;
        factor[1] = -1;
        factor[2] = 1;
        factor[3] = -1;
        factor[4] = -1;
        factor[5] = 1;
        factor[6] = -1;
        break;
      case 1 :
        factor[0] = 0;
        factor[1] = 1;
        factor[2] = 1;
        factor[3] = -1;
        factor[4] = -1;
        factor[5] = 0;
        factor[6] = 1;
        break;
      case 2 :
        factor[0] = 0;
        factor[1] = 1;
        factor[2] = 0;
        factor[3] = 1;
        factor[4] = 1;
        factor[5] = 0;
        factor[6] = 1;
        break;
      case 3 :
        factor[0] = 1;
        factor[1] = -1;
        factor[2] = 0;
        factor[3] = 1;
        factor[4] = -1;
        factor[5] = 0;
        factor[6] = 1;
        break;
      case 4 :
        factor[0] = 0;
        factor[1] = 0;
        factor[2] = 0;
        factor[3] = 0;
        factor[4] = 1;
        factor[5] = 1;
        factor[6] = 0;
        break;
      }

    }


    //! evaluate the basefunction on point x
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      phi = (factor[0] + factor[1]*x[0])
            * (factor[2] + factor[3]*x[1])
            + factor[4] * x[2]
            * (factor[5] + factor[6]* std::min(x[0],x[1]));
    }

    //! derivative with respect to x or y or z
    //! diffVariable(0) == 0   ==> x
    //! diffVariable(0) == 1   ==> y
    //! diffVariable(0) == 2   ==> z,  and so on
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      switch (diffVariable[0]) {
      case 0 :
        phi =  factor[1]*(factor[2] + factor[3]*x[1]) + ((x[0] <= x[1]) ? factor[4]*factor[6]*x[2] : 0);
        return;
      case 1 :
        phi = (factor[0] + factor[1]*x[0])*factor[3] + ((x[0] <= x[1]) ? 0 : factor[4]*factor[6]*x[2]);
        return;
      case 2 :
        phi = factor[4] * (factor[5] + factor[6]*std::min(x[0],x[1]));
        return;

      }
    }

    //! evaluate second derivative
    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      std::cout << "BaseFunction for pyramid, evaluate 2nd derivative not implemented! \n";
      phi = 0.0;
    }

  };



  // *********************************************************************
  //
  //   First-order Lagrange shape functions for the prism
  //
  // *********************************************************************
  template<class FunctionSpaceType>
  class LagrangeBaseFunction<FunctionSpaceType,prism,1>
    : public BaseFunctionInterface<FunctionSpaceType>
  {
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    enum { dim = 3 };
    /* phi(x,y,z) = (factor[0] + factor[1]*x + factor[2]*y)
     *            * (factor[3] + factor[4]*z)
     */
    RangeFieldType factor[5];
  public:

    //! Constructor making base function number baseNum
    LagrangeBaseFunction ( FunctionSpaceType & f , int baseNum )
      : BaseFunctionInterface<FunctionSpaceType>(f)
    {
      assert((baseNum >= 0) || (baseNum < 6));

      switch (baseNum) {
      case 0 :
        factor[0] = 1;
        factor[1] = -1;
        factor[2] = -1;
        factor[3] = 1;
        factor[4] = -1;
        break;
      case 1 :
        factor[0] = 0;
        factor[1] = 1;
        factor[2] = 0;
        factor[3] = 1;
        factor[4] = -1;
        break;
      case 2 :
        factor[0] = 0;
        factor[1] = 0;
        factor[2] = 1;
        factor[3] = 1;
        factor[4] = -1;
        break;
      case 3 :
        factor[0] = 1;
        factor[1] = -1;
        factor[2] = -1;
        factor[3] = 0;
        factor[4] = 1;
        break;
      case 4 :
        factor[0] = 0;
        factor[1] = 1;
        factor[2] = 0;
        factor[3] = 0;
        factor[4] = 1;
        break;
      case 5 :
        factor[0] = 0;
        factor[1] = 0;
        factor[2] = 1;
        factor[3] = 0;
        factor[4] = 1;
        break;
      }

    }


    //! evaluate the basefunction on point x
    virtual void evaluate ( const FieldVector<deriType, 0> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      phi = (factor[0] + factor[1]*x[0] + factor[2]*x[1])
            * (factor[3] + factor[4]*x[2]);
    }

    //! derivative with respect to x or y or z
    //! diffVariable(0) == 0   ==> x
    //! diffVariable(0) == 1   ==> y
    //! diffVariable(0) == 2   ==> z,  and so on
    virtual void evaluate ( const FieldVector<deriType, 1> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      switch (diffVariable[0]) {
      case 0 :
        phi = factor[1] * (factor[3] + factor[4]*x[2]);
        return;
      case 1 :
        phi = factor[2] * (factor[3] + factor[4]*x[2]);
        return;
      case 2 :
        phi = (factor[0] + factor[1]*x[0] + factor[2]*x[1]) * factor[4];
        return;

      }
    }

    //! evaluate second derivative
    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const DomainType & x, RangeType & phi) const
    {
      std::cout << "BaseFunction for pyramid, evaluate 2nd derivative not implemented! \n";
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
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    enum { dim = 3 };
    //! phi(x,y,z) = (factor[0][0] + factor[0][1] * x)
    //!            = (factor[1][0] + factor[1][1] * y)
    //!            = (factor[2][0] + factor[2][1] * z)
    RangeFieldType factor[dim][2];
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
                            const DomainType & x, RangeType & phi) const
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
                            const DomainType & x, RangeType & phi) const
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
    }

    //! evaluate second derivative
    virtual void evaluate ( const FieldVector<deriType, 2> &diffVariable,
                            const DomainType & x, RangeType & phi) const
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
    enum { numOfBaseFct = (dimrange * (polOrd+1)) };
  };

  //! Lagrange Definition for triangles
  template <int polOrd , int dimrange >
  struct LagrangeDefinition< triangle , polOrd, dimrange >
  {
    enum { numOfBaseFct = (dimrange * (polOrd+1) * (polOrd+2) / 2) };
  };

  //! Lagrange Definition for quadrilaterals
  template <int polOrd , int dimrange >
  struct LagrangeDefinition< quadrilateral , polOrd, dimrange >
  {
    enum { numOfBaseFct = dimrange * (polOrd+1) * (polOrd+1) };
  };

  //! Lagrange Definition for tetrahedrons
  template <int polOrd , int dimrange >
  struct LagrangeDefinition< tetrahedron , polOrd, dimrange >
  {
    /** \bug This formula is wrong! */
    enum { numOfBaseFct = (polOrd == 0) ? (1*dimrange) : (dimrange * 4 * polOrd) };
  };

  //! Lagrange Definition for pyramids
  /** \todo Generalize this to higher orders */
  template <int polOrd, int dimrange >
  struct LagrangeDefinition< pyramid , polOrd, dimrange >
  {
    enum { numOfBaseFct = dimrange * 5};
  };

  //! Lagrange Definition for prisms
  /** \todo Generalize this to higher orders */
  template <int polOrd, int dimrange >
  struct LagrangeDefinition< prism , polOrd, dimrange >
  {
    enum { numOfBaseFct = dimrange * 6};
  };

  //! Lagrange Definition for hexahedrons
  template <int polOrd , int dimrange >
  struct LagrangeDefinition< hexahedron , polOrd, dimrange >
  {
    enum { numOfBaseFct = dimrange * (polOrd+1) * (polOrd+1) * (polOrd+1) };
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
          baseFuncList_[i*dimrange + k] = new LagrangeBaseFunctionType ( i ) ;
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
    int getNumberOfDifferentBaseFunctions () const
    {
      return (int) (numOfBaseFct / dimrange);
    }
  private:
    //! Vector with all base functions corresponding to the base function set
    FieldVector <LagrangeBaseFunctionType*, numOfBaseFct> baseFuncList_;
  };

} // end namespace Dune

#include "p2lagrangebasefunctions.hh"

#endif
