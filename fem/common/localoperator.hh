// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_LOCALOPERATORS_HH__
#define __DUNE_LOCALOPERATORS_HH__

namespace Dune
{

  /** @defgroup LocalOperator LocalOperator
      @ingroup  DiscreteOperator

     @{
   */

  //***********************************************************************
  //
  /*! Interface LocalOperators
     Local operators only define what to do a one entity which is reached
     during a grid walkthrough on one level. The walkthrough is made by a
     discrete operator which organizes the walktrough and calls the local
     operator on each entity. Furthermore the local operator provides a method
     prepareGlobal which is called once before the grid walktrough and a method
     finalizeGlobal which ic called once after the walktrough. The methods
     prepareLocal and finalizeLocal are called on each entity. If for one
     operator this methods are not needed, the definition of them is left free.
   */
  //***********************************************************************
  template <class DF_DomainType, class DF_RangeType, class LocalOperatorImp>
  class LocalOperatorInterface
  {
  public:
    typedef DF_DomainType DomainType;
    typedef DF_RangeType RangeType;
    typedef typename DF_RangeType::RangeFieldType RangeFieldType;

    //! prepare apply global, called once global
    void prepareGlobal (int level, const DomainType & arg , RangeType & dest,
                        const RangeFieldType scalar)
    {
      asImp().prepareGlobal ( level , arg, dest , scalar );
    }

    //! do a local cleanup or what , called on each entity
    template <class GridIteratorType>
    void prepareLocal ( GridIteratorType &it , const DomainType & arg , RangeType & dest )
    {
      asImp().prepareLocal ( it, arg, dest );
    }

    //! do some local stuff, overload for functionality, called on each entity
    template <class GridIteratorType>
    void finalizeLocal ( GridIteratorType &it , const DomainType & arg , RangeType & dest )
    {
      asImp().finalizeLocal(it,arg,dest);
    }

    //! finalize the operator, called once after apply global
    void finalizeGlobal (int level, const DomainType & arg , RangeType & dest)
    {
      asImp().finalizeGlobal(level,arg,dest);
    }

    //! call applyLocal of CombinedLocalOperatorImp , on each entity
    template <class GridIteratorType>
    void applyLocal ( GridIteratorType &it , const DomainType & arg , RangeType & dest )
    {
      asImp().applyLocal(it,arg,dest);
    }

  private:
    // Barton Nackman
    LocalOperatorImp & asImp()
    {
      return static_cast<LocalOperatorImp &> (*this);
    }

  };

  //**************************************************************************
  //
  //  Default implemenations for LocalOperators
  //
  //**************************************************************************
  template <class DF_DomainType, class DF_RangeType, class LocalOperatorImp>
  class LocalOperatorDefault
    : public LocalOperatorInterface <DF_DomainType,DF_RangeType,LocalOperatorImp>
  {
  public:
    // no default implementation at the moement

  private:
    // Barton Nackman
    LocalOperatorImp & asImp()
    {
      return static_cast<LocalOperatorImp &> (*this);
    }
  };

  //*******************************************************************
  //
  /*!

     This operator combines two local operator in the following way:
     \f[ Op(a,d) = (A + B)(a,d) \f] whereby B is first applied and then A. The
     operators A and B have to be defined such that they localy add thier
     results to a given destination. This workaround can be used directly but
     the normal way should be to define two discrete operators and then add
     them which uses intern this class two add the local operators.

   */
  //*******************************************************************
  //! OP(a,d) = (A + B)(a,d)
  //! Note: first B is evaluated, then A
  template <class A, class B >
  class CombinedLocalOperator :
    public LocalOperatorDefault <typename B::DomainType, typename A::RangeType,
        CombinedLocalOperator <A,B> >
  {
  public:
    typedef typename B::DomainType DomainType;
    typedef typename A::RangeType RangeType;
    typedef typename A::RangeType::RangeFieldType RangeFieldType;

    //! Constructor for combinations with factors
    CombinedLocalOperator ( A & a, B & b ) : _a ( a ) , _b ( b ) {}

    //! prepare apply and get temporary variable
    void prepareGlobal (int level, const DomainType & arg , RangeType & dest,
                        const RangeFieldType scalar=1.0 )
    {
      _b.prepareGlobal(level,arg,dest,scalar);
      _a.prepareGlobal(level,arg,dest,scalar);
    }

    //! do a local cleanup or what
    template <class GridIteratorType>
    void prepareLocal ( GridIteratorType &it , const DomainType & arg , RangeType & dest )
    {
      _b.prepareLocal(it,arg,dest);
      _a.prepareLocal(it,arg,dest);
    }

    //! do some local stuff
    template <class GridIteratorType>
    void finalizeLocal ( GridIteratorType &it , const DomainType & arg , RangeType & dest )
    {
      _b.finalizeLocal(it,arg,dest);
      _a.finalizeLocal(it,arg,dest);
    }

    //! finalize the operator
    void finalizeGlobal (const DomainType & arg , RangeType & dest)
    {
      _b.finalizeGlobal(arg,dest);
      _a.finalizeGlobal(arg,dest);
    }

    //! evaluate the combined local operator
    template <class GridIteratorType>
    void applyLocal ( GridIteratorType &it , const DomainType & arg , RangeType & dest )  const
    {
      _b.applyLocal ( it , arg , dest );
      _a.applyLocal ( it , arg , dest );
    }

  private:
    //! operator A and B
    A & _a;
    B & _b;
  };


  //*******************************************************
  //  create CombinedLocalOperator
  //*******************************************************
  //! return OP = A + B
  template <class A , class B >
  CombinedLocalOperator < A,B> addLocalOperators  ( A & a, B & b )
  {
    CombinedLocalOperator <A,B> tmp (a,b);
    return tmp;
  }

  //********************************************************************
  //********************************************************************
  //********************************************************************
  //********************************************************************
  //
  //! ScaledLocalOperator
  //! which is a LocalOperator multipied by a scalar
  //
  //********************************************************************
  template <class A>
  class ScaledLocalOperator
    : public LocalOperatorDefault <typename A::DomainType, typename A::RangeType,
          ScaledLocalOperator <A> >
  {
  public:
    //! Constructor for combinations with factors
    ScaledLocalOperator ( A & a , const RangeFieldType scalar) : _a ( a ) , scalar_ (scalar) {}

    //! prepare apply and get temporary variable
    void prepareGlobal (int level, const DomainType & arg , RangeType &
                        dest, const RangeFieldType scalar=1.0 )
    {
      _a.prepareGlobal(level,arg,dest,scalar * scalar_);
    }

    //! do local prepare if necessary
    template <class GridIteratorType>
    void prepareLocal ( GridIteratorType &it , const DomainType & arg , RangeType & dest )
    {
      _a.prepareLocal(it,arg,dest);
    }

    //! do local finalize if necessary
    template <class GridIteratorType>
    void finalizeLocal ( GridIteratorType &it , const DomainType & arg , RangeType & dest )
    {
      _a.finalizeLocal(it,arg,dest);
    }

    //! finalize the operator
    void finalizeGlobal (const DomainType & arg , RangeType & dest)
    {
      _a.finalizeGlobal(arg,dest);
    }

    //! call local apply of _a
    template <class GridIteratorType>
    void applyLocal ( GridIteratorType &it , const DomainType & arg , RangeType & dest )  const
    {
      _a.applyLocal ( it , arg , dest );
    }
  protected:
    //! reference to local operator A
    A & _a;

    //! scale factor for operator _a
    const RangeFieldType scalar_;

  }; // end class ScaledLocalOperator


  /** @} end documentation group */

} // end namespace Dune

#endif
