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
  template <class FstPType, class SecPType, class SType ,
      class LocalOperatorImp>
  class LocalOperatorInterface
  {
  public:
    //! remember the parameter types
    typedef FstPType FirstParamType;
    typedef SecPType SecondParamType;
    typedef SType ScalarType;

    //**************************************************************
    /*! default implementation of the prepare and finalize methods
        default is make nothing
     */
    //! prepareGlobal is called before the grid walktrough
    void prepareGlobal(const FirstParamType &pa, SecondParamType &pb)
    {
      asImp().prepareGlobal(pa,pb);
    }

    //! prepare for grid walktrough
    void prepareGlobal ()
    {
      asImp().prepareGlobal();
    }

    //! finalize the walktrough
    void finalizeGlobal()
    {
      asImp().finalizeGlobal();
    }

    //! one entity
    template<class EntityType>
    void prepareLocal (EntityType & en)
    {
      asImp().prepareLocal(en);
    }

    //! \todo Please doc me!
    template<class EntityType>
    void finalizeLocal(EntityType & en)
    {
      asImp().finalizeLocal(en);
    }

    //! two entities
    template<class EntityType>
    void prepareLocal (EntityType & en1, EntityType &en2)
    {
      asImp().prepareLocal(en1,en2);
    }

    //! \todo Please doc me!
    template<class EntityType>
    void finalizeLocal(EntityType & en1, EntityType &en2)
    {
      asImp().finalizeLocal(en1,en2);
    }

    //! things to do on one entity
    template<class EntityType>
    void applyLocal(EntityType & en)
    {
      asImp().applyLocal(en);
    }

    //! things to do on two entity
    template<class EntityType>
    void applyLocal(EntityType & en1, EntityType &en2)
    {
      asImp().applyLocal(en1,en2);
    }
  private:
    //! Barton Nackman
    LocalOperatorImp & asImp()
    {
      return static_cast<LocalOperatorImp &> (*this);
    }
  };

  //**************************************************************************
  //  Default implemenations for LocalOperators
  //**************************************************************************
  /** \brief Default implementation of a local operator
   *  A local operator works on entities only and is used by a DiscreteOperator
   *  during a mesh traversal. This class implements the standard behaviour for
   *  prepareLocal(), finalizeLocal() and possibly other methods.
   */
  template <class FstPType, class SecPType, class SType ,
      class LocalOperatorImp>
  class LocalOperatorDefault
    : public LocalOperatorInterface <FstPType,SecPType,
          SType,LocalOperatorImp>
  {
  public:
    //! remember the parameter types
    typedef FstPType FirstParamType;
    typedef SecPType SecondParamType;
    typedef SType ScalarType;

    //! no default implementation at the moement
    LocalOperatorDefault () : scalar_ (1.0) {}

    //! scale operator , for inheritance
    void scaleIt ( const ScalarType scalar )
    {
      scalar_ = scalar;
    }

    //**************************************************************
    /*! default implementation of the prepare and finalize methods
        default is make nothing
     */
    // prepare for grid walktrough
    void prepareGlobal () {}

    // finalize the walktrough
    void finalizeGlobal() {}

    // one entity
    template<class EntityType>
    void prepareLocal (EntityType & en) {};

    //! \todo Please doc me!
    template<class EntityType>
    void finalizeLocal(EntityType & en) {};

    // two entities
    template<class EntityType>
    void prepareLocal (EntityType & en1, EntityType &en2){};

    //! \todo Please doc me!
    template<class EntityType>
    void finalizeLocal(EntityType & en1, EntityType &en2){};
    //**************************************************************

  protected:
    // scalar for operator
    ScalarType scalar_;
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
    : public ObjPointerStorage
  {
  public:
    //! Constructor for combinations storing the two operators
    CombinedLocalOperator ( A & a, B & b , bool printMsg = false )
      : _a ( a ) , _b ( b ) , printMSG_ ( printMsg )
    {
      if(printMSG_)
        std::cout << "Create CombinedLocalOperator " << this << "\n";
    }

    //! Destructor
    ~CombinedLocalOperator ()
    {
      if(printMSG_)
        std::cout << "Delete CombinedLocalOperator " << this << "\n";
    }

    //! method to scale the belonging operators
    template <class ScalarType>
    void scaleIt( const ScalarType scalar);

    //*******************************************************
    /*! the parameters of prepareGlobal and finalizeGlobal are set outside
        via a setParameter method or something
     */
    //! prepareGlobal is called before the grid walktrough
    template <class FirstParamType, class SecondParamType>
    void prepareGlobal(const FirstParamType &pa, SecondParamType &pb);

    //! finalizeGlobal is called after the grid walktrough
    void finalizeGlobal();

    //*******************************************************
    /*! prepareLocal is called before applyLocal is called,
        it has different length of parameter list, which are always
        entities. For example one might give no parameters to prepareLocal or
        father and son element, depends on grid walktrough outside
     */

    // one entity
    template<class EntityType>
    void prepareLocal (EntityType & en);

    //! \todo Please doc me!
    template<class EntityType>
    void finalizeLocal(EntityType & en);

    // two entities
    template<class EntityType>
    void prepareLocal (EntityType & en1, EntityType &en2);

    //! \todo Please doc me!
    template<class EntityType>
    void finalizeLocal(EntityType & en1, EntityType &en2);

    //**********************************************************
    /*! applyLocal defines the action one might do on a given or on two given
        entities. Up to now only max two entities may be parameter but this
        can be advanced. There is also an applyLocal method with no entity
        which means that also operators which dont need a grid walktrough can
        be combined.
     */

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

    // if true some messages in Constructor and Destructor are printed
    bool printMSG_;
  };


  //********************************************************************
  //
  //  Implementation
  //
  //********************************************************************
  template <class A, class B >
  template <class ScalarType>
  inline void CombinedLocalOperator<A,B>::
  scaleIt(const ScalarType scalar)
  {
    _a.scaleIt(scalar);
    _b.scaleIt(scalar);
  }
  template <class A, class B >
  template <class FirstParamType, class SecondParamType>
  inline void CombinedLocalOperator<A,B>::
  prepareGlobal(const FirstParamType &pa, SecondParamType &pb)
  {
    _a.scaleIt(1.0);
    _b.scaleIt(1.0);
    _b.prepareGlobal(pa,pb);
    _a.prepareGlobal(pa,pb);
  }

  template <class A, class B >
  inline void CombinedLocalOperator<A,B>::finalizeGlobal()
  {
    _b.finalizeGlobal();
    _a.finalizeGlobal();
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
  template <class A,class ScalarType>
  class ScaledLocalOperator
    : public ObjPointerStorage
  {
  public:
    //! Constructor for combinations with factors
    ScaledLocalOperator ( A & a , const ScalarType scalar,
                          bool printMsg = false)
      : _a ( a ) , scalar_ (scalar), tmpScalar_ (scalar) ,
        printMSG_ ( printMsg )
    {
      if(printMSG_)
        std::cout << "Create ScaledLocalOperator " << this << "\n";
    }

    ~ScaledLocalOperator ()
    {
      if(printMSG_)
        std::cout << "Delete ScaledLocalOperator " << this << "\n";
    }
    // scale this operator from outside
    void scaleIt ( const ScalarType & scalar);

    //*******************************************************
    /*! the parameters of prepareGlobal and finalizeGlobal are set outside
        via a setParameter method or something
     */
    //! prepareGlobal is called before the grid walktrough
    template <class FirstParamType, class SecondParamType>
    void prepareGlobal(const FirstParamType &pa, SecondParamType &pb);

    //! finalizeGlobal is called after the grid walktrough
    void finalizeGlobal();

    //*******************************************************
    /*! prepareLocal is called before applyLocal is called,
        it has different length of parameter list, which are always
        entities. For example one might give no parameters to prepareLocal or
        father and son element, depends on grid walktrough outside
     */
    // one entity
    template<class EntityType>
    void prepareLocal (EntityType & en);

    //! \todo Please doc me!
    template<class EntityType>
    void finalizeLocal(EntityType & en);

    // two entities
    template<class EntityType>
    void prepareLocal (EntityType & en1, EntityType &en2);

    //! \todo Please doc me!
    template<class EntityType>
    void finalizeLocal(EntityType & en1, EntityType &en2);

    //**********************************************************
    /*! applyLocal defines the action one might do on a given or on two given
        entities. Up to now only max two entities may be parameter but this
        can be advanced. There is also an applyLocal method with no entity
        which means that also operators which dont need a grid walktrough can
        be combined.
     */
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
    const ScalarType scalar_;
    ScalarType tmpScalar_;

    // if true some messages in Contructor and Destructor are printed
    bool printMSG_;
  }; // end class ScaledLocalOperator

  //****************************************************
  //  Implementation
  //****************************************************
  template <class A, class ScalarType>
  inline void ScaledLocalOperator<A,ScalarType>::
  scaleIt ( const ScalarType & scalar )
  {
    tmpScalar_ = scalar_ * scalar;
  }

  template <class A, class ScalarType>
  template <class FirstParamType, class SecondParamType>
  inline void ScaledLocalOperator<A,ScalarType>::
  prepareGlobal(const FirstParamType &pa, SecondParamType &pb)
  {
    _a.scaleIt(tmpScalar_);
    _a.prepareGlobal(pa,pb);
  }

  template <class A, class ScalarType>
  inline void ScaledLocalOperator<A,ScalarType>::finalizeGlobal()
  {
    _a.finalizeGlobal();
  }

  template <class A, class ScalarType>
  template <class EntityType>
  inline void ScaledLocalOperator<A,ScalarType>::prepareLocal(EntityType &en)
  {
    _a.prepareLocal(en);
  }

  template <class A, class ScalarType>
  template <class EntityType>
  inline void ScaledLocalOperator<A,ScalarType>::prepareLocal(EntityType &en1, EntityType & en2)
  {
    _a.prepareLocal(en1,en2);
  }

  template <class A, class ScalarType>
  template <class EntityType>
  inline void ScaledLocalOperator<A,ScalarType>::finalizeLocal(EntityType &en)
  {
    _a.finalizeLocal(en);
  }

  template <class A, class ScalarType>
  template <class EntityType>
  inline void ScaledLocalOperator<A,ScalarType>::finalizeLocal(EntityType &en1, EntityType & en2)
  {
    _a.finalizeLocal(en1,en2);
  }

  template <class A, class ScalarType>
  template <class EntityType>
  inline void ScaledLocalOperator<A,ScalarType>::applyLocal(EntityType &en)
  {
    _a.applyLocal(en);
  }

  template <class A, class ScalarType>
  template <class EntityType>
  inline void ScaledLocalOperator<A,ScalarType>::applyLocal(EntityType &en1, EntityType &en2 )
  {
    _a.applyLocal(en1,en2);
  }


  /** @} end documentation group */

} // end namespace Dune

#endif
