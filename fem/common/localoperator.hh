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
  template <class DF_DomainType, class DF_RangeType,
      class RFType, class LocalOperatorImp>
  class LocalOperatorInterface
  {
  public:
    typedef DF_DomainType DomainType;
    typedef DF_RangeType RangeType;
    typedef RFType RangeFieldType;

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
  template <class DF_DomainType, class DF_RangeType, class RFType ,
      class LocalOperatorImp>
  class LocalOperatorDefault
    : public LocalOperatorInterface <DF_DomainType,DF_RangeType,RFType,LocalOperatorImp>
  {
  public:
    // no default implementation at the moement

    // no default implementation at the moement
    LocalOperatorDefault () : arg_ (NULL) , dest_(NULL) , scalar_ (1.0) {}

    //! store pointers to actual discrete functions
    void setArguments(const DF_DomainType & arg, DF_RangeType & dest)
    {
      arg_  = &arg;
      dest_ = &dest;
    }

    //! store pointers to actual discrete functions
    void removeArguments()
    {
      arg_  = NULL;
      dest_ = NULL;
    }

    //! scale operator
    void scaleIt ( const RangeFieldType scalar )
    {
      scalar_ = scalar;
    }

    //**************************************************************
    /*! default implementation of the prepare and finalize methods
     */
    void prepareGlobal()  {}
    void finalizeGlobal() {}
    void prepareLocal()  {}
    void finalizeLocal() {}

    // one entity
    template<class EntityType>
    void prepareLocal (EntityType & en) {};

    template<class EntityType>
    void finalizeLocal(EntityType & en) {};

    // two entities
    template<class EntityType>
    void prepareLocal (EntityType & en1, EntityType &en2){};

    template<class EntityType>
    void finalizeLocal(EntityType & en1, EntityType &en2){};
    //**************************************************************

  protected:
    //! Argument to store, i.e. DiscreteFunction pointers on which the
    //! operators will be applied
    const DF_DomainType * arg_;
    DF_RangeType *dest_;

    RangeFieldType scalar_;
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
  class CombinedLocalOperator
  {
  public:
    //! Constructor for combinations storing the two operators
    CombinedLocalOperator ( A & a, B & b ) : _a ( a ) , _b ( b ) {}

    //! set argument of operator
    template <class ArgType, class DestType>
    void setArguments(const ArgType & arg, DestType &dest)
    {
      _a.setArguments(arg,dest);
      _b.setArguments(arg,dest);
    }

    //! remove arguments  set by setArguments method
    void removeArguments()
    {
      _a.removeArguments();
      _b.removeArguments();
    }

    //*******************************************************
    /*! the parameters of prepareGlobal and finalizeGlobal are set outside
        via a setParameter method or something
     */
    //! prepareGlobal is called before the grid walktrough
    void prepareGlobal();

    //! finalizeGlobal is called after the grid walktrough
    void finalizeGlobal();

    //*******************************************************
    /*! prepareLocal is called before applyLocal is called,
        it has different length of parameter list, which are always
        entities. For example one might give no parameters to prepareLocal or
        father and son element, depends on grid walktrough outside
     */
    // no entity
    void prepareLocal ();

    //! finalizeLocal is called after applyLocal is called
    //! see prepareLocal
    void finalizeLocal();

    // one entity
    template<class EntityType>
    void prepareLocal (EntityType & en);

    template<class EntityType>
    void finalizeLocal(EntityType & en);

    // two entities
    template<class EntityType>
    void prepareLocal (EntityType & en1, EntityType &en2);

    template<class EntityType>
    void finalizeLocal(EntityType & en1, EntityType &en2);

    //**********************************************************
    /*! applyLocal defines the action one might do on a given or on two given
        entities. Up to now only max two entities may be parameter but this
        can be advanced. There is also an applyLocal method with no entity
        which means that also operators which dont need a grid walktrough can
        be combined.
     */
    // things to do
    void applyLocal ();

    // things to do on one entity
    template<class EntityType>
    void applyLocal(EntityType & en);

    // things to do on two entity
    template<class EntityType>
    void applyLocal(EntityType & en1, EntityType &en2);

    //****************************************************************
    /*! the setParameter and getReturn methods are to pass parameters from
        and to the underlying operators
     */
  private:
    //! operator A and B
    A & _a;
    B & _b;
  };


  //********************************************************************
  //
  //  Implementation
  //
  //********************************************************************
  template <class A, class B >
  inline void CombinedLocalOperator<A,B>::prepareGlobal()
  {
    _b.prepareGlobal();
    _a.prepareGlobal();
  }

  template <class A, class B >
  inline void CombinedLocalOperator<A,B>::finalizeGlobal()
  {
    _b.finalizeGlobal();
    _a.finalizeGlobal();
  }

  template <class A, class B >
  inline void CombinedLocalOperator<A,B>::prepareLocal()
  {
    _b.prepareLocal();
    _a.prepareLocal();
  }

  template <class A, class B >
  inline void CombinedLocalOperator<A,B>::finalizeLocal()
  {
    _b.finalizeLocal();
    _a.finalizeLocal();
  }

  template <class A, class B >
  template <class EntityType>
  inline void CombinedLocalOperator<A,B>::prepareLocal(EntityType &en)
  {
    _b.prepareLocal(en);
    _a.prepareLocal(en);
  }

  template <class A, class B >
  template <class EntityType>
  inline void CombinedLocalOperator<A,B>::finalizeLocal(EntityType &en)
  {
    _b.finalizeLocal(en);
    _a.finalizeLocal(en);
  }

  template <class A, class B >
  inline void CombinedLocalOperator<A,B>::applyLocal()
  {
    _b.applyLocal();
    _a.applyLocal();
  }

  template <class A, class B >
  template <class EntityType>
  inline void CombinedLocalOperator<A,B>::applyLocal(EntityType &en)
  {
    _b.applyLocal(en);
    _a.applyLocal(en);
  }


  template <class A, class B >
  template <class EntityType>
  inline void CombinedLocalOperator<A,B>::prepareLocal(EntityType &en1, EntityType & en2)
  {
    _b.prepareLocal(en1,en2);
    _a.prepareLocal(en1,en2);
  }

  template <class A, class B >
  template <class EntityType>
  inline void CombinedLocalOperator<A,B>::applyLocal(EntityType &en1, EntityType &en2 )
  {
    _b.applyLocal(en1,en2);
    _a.applyLocal(en1,en2);
  }

  template <class A, class B >
  template <class EntityType>
  inline void CombinedLocalOperator<A,B>::finalizeLocal(EntityType &en1, EntityType & en2)
  {
    _b.finalizeLocal(en1,en2);
    _a.finalizeLocal(en1,en2);
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
  template <class A,class RangeFieldType>
  class ScaledLocalOperator
  {
  public:
    //! Constructor for combinations with factors
    ScaledLocalOperator ( A & a , const RangeFieldType scalar) : _a ( a ) , scalar_ (scalar) {}

    void scaleIt ( const RangeFieldType & scalar)
    {
      _a.scaleIt( scalar * scalar_ );
    }

    //! set argument of operator
    template <class ArgType, class DestType>
    void setArguments(const ArgType & arg, DestType &dest)
    {
      _a.scaleIt(scalar_);
      _a.setArguments(arg,dest);
    }

    //! remove arguments  set by setArguments method
    void removeArguments()
    {
      _a.removeArguments();
      _a.scaleIt(1.0);
    }

    //*******************************************************
    /*! the parameters of prepareGlobal and finalizeGlobal are set outside
        via a setParameter method or something
     */
    //! prepareGlobal is called before the grid walktrough
    void prepareGlobal();

    //! finalizeGlobal is called after the grid walktrough
    void finalizeGlobal();

    //*******************************************************
    /*! prepareLocal is called before applyLocal is called,
        it has different length of parameter list, which are always
        entities. For example one might give no parameters to prepareLocal or
        father and son element, depends on grid walktrough outside
     */
    // no entity
    void prepareLocal ();

    //! finalizeLocal is called after applyLocal is called
    //! see prepareLocal
    void finalizeLocal();

    // one entity
    template<class EntityType>
    void prepareLocal (EntityType & en);

    template<class EntityType>
    void finalizeLocal(EntityType & en);

    // two entities
    template<class EntityType>
    void prepareLocal (EntityType & en1, EntityType &en2);

    template<class EntityType>
    void finalizeLocal(EntityType & en1, EntityType &en2);

    //**********************************************************
    /*! applyLocal defines the action one might do on a given or on two given
        entities. Up to now only max two entities may be parameter but this
        can be advanced. There is also an applyLocal method with no entity
        which means that also operators which dont need a grid walktrough can
        be combined.
     */
    // things to do
    void applyLocal ();

    // things to do on one entity
    template<class EntityType>
    void applyLocal(EntityType & en);

    // things to do on two entity
    template<class EntityType>
    void applyLocal(EntityType & en1, EntityType &en2);
  protected:
    //! reference to local operator A
    A & _a;

    //! scale factor for operator _a
    const RangeFieldType scalar_;

  }; // end class ScaledLocalOperator

  //****************************************************
  //  Implementation
  //****************************************************
  template <class A, class B >
  inline void ScaledLocalOperator<A,B>::prepareGlobal()
  {
    _a.prepareGlobal();
  }

  template <class A, class B >
  inline void ScaledLocalOperator<A,B>::finalizeGlobal()
  {
    _a.finalizeGlobal();
  }

  template <class A, class B >
  inline void ScaledLocalOperator<A,B>::prepareLocal()
  {
    _a.prepareLocal();
  }

  template <class A, class B >
  inline void ScaledLocalOperator<A,B>::finalizeLocal()
  {
    _a.finalizeLocal();
  }

  template <class A, class B >
  template <class EntityType>
  inline void ScaledLocalOperator<A,B>::prepareLocal(EntityType &en)
  {
    _a.prepareLocal(en);
  }

  template <class A, class B >
  template <class EntityType>
  inline void ScaledLocalOperator<A,B>::finalizeLocal(EntityType &en)
  {
    _a.finalizeLocal(en);
  }

  template <class A, class B >
  inline void ScaledLocalOperator<A,B>::applyLocal()
  {
    _a.applyLocal();
  }

  template <class A, class B >
  template <class EntityType>
  inline void ScaledLocalOperator<A,B>::applyLocal(EntityType &en)
  {
    _a.applyLocal(en);
  }


  template <class A, class B >
  template <class EntityType>
  inline void ScaledLocalOperator<A,B>::prepareLocal(EntityType &en1, EntityType & en2)
  {
    _a.prepareLocal(en1,en2);
  }

  template <class A, class B >
  template <class EntityType>
  inline void ScaledLocalOperator<A,B>::applyLocal(EntityType &en1, EntityType &en2 )
  {
    _a.applyLocal(en1,en2);
  }

  template <class A, class B >
  template <class EntityType>
  inline void ScaledLocalOperator<A,B>::finalizeLocal(EntityType &en1, EntityType & en2)
  {
    _a.finalizeLocal(en1,en2);
  }


  /** @} end documentation group */

} // end namespace Dune

#endif
