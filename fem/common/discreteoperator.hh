// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCRETEOPERATOR_INTERFACE_HH__
#define __DUNE_DISCRETEOPERATOR_INTERFACE_HH__

#include <dune/common/operator.hh>

namespace Dune {

  /** @defgroup DiscreteOperator DiscreteOperator
      @ingroup OperatorCommon

     @{
   */

  //! strorage class for new generated operators during operator + on
  //! DiscreteOperator
  template <class ObjType>
  struct ObjPointer
  {
    typedef ObjPointer<ObjType> ObjPointerType;
    // remember object item and next pointer
    ObjPointerType *next;
    ObjType * item;

    //! new ObjPointer is only created with pointer for item
    ObjPointer () : item (NULL) , next (NULL) {}

    //! new ObjPointer is only created with pointer for item
    ObjPointer (ObjType  *t) : item (t) , next (NULL) {}

    //! delete the next ObjPointer and the item
    ~ObjPointer ()
    {
      if(next) delete next;next = NULL;
      if(item) delete item;item = NULL;
    }
  };

  //*******************************************************************
  //*******************************************************************
  //*******************************************************************
  //*******************************************************************

  /*! Default Implementation class for DiscreteOperators
     Here the ObjPointer is stored which is used during
     the generation of new objects with the operator + method.
     There is corresponding interface class is Operator.
   */
  template < class DF_DomainType , class DF_RangeType>
  class DiscreteOperatorDefault
    : public Operator <typename DF_DomainType::RangeFieldType,
          typename DF_RangeType::RangeFieldType,
          DF_DomainType , DF_RangeType>
  {
    typedef DiscreteOperatorDefault<DF_DomainType,DF_RangeType> MyType;
  public:
    //! make new operator with item points to null
    DiscreteOperatorDefault () : item_ (NULL) {}

    // set level
    void initLevel (int level) const
    {
      level_ = level;
    }

    //! call initLevel
    void initialize (int level) const
    {
      initLevel ( level );
    }

    //! apply operator, only used by mapping
    void apply ( const DF_DomainType & arg, DF_RangeType & dest) const
    {
      std::cerr << "ERROR: DiscreteOperatorDefault::apply called! \n";
    }

    //! apply operator
    void operator () ( const DF_DomainType & arg, DF_RangeType & dest ) const
    {
      std::cerr << "ERROR: DiscreteOperatorDefault::operator () called! \n";
    }


    //! delete all other operators with are generated
    //! using the operator + of derived operators
    ~DiscreteOperatorDefault ()
    {
      std::cout << "delete Operator "<< this << " \n";
      if(item_) delete item_;item_ = NULL;
    }

  protected:
    // store the objects created by operator + in here
    typedef ObjPointer<MyType> ObjPointerType;
    ObjPointer<MyType> * item_;

    // store new generated DiscreteOperator Pointer
    template <class DiscrOpType>
    void saveObjPointer ( DiscrOpType * discrOp )
    {
      ObjPointerType *next = new ObjPointerType ( discrOp );
      next->next = item_;
      item_ = next;
    }

    // current level of operator
    mutable int level_;
  }; // end class DiscreteOperatorDefault



  /** @} end documentation group */

} // end namespace Dune

#endif
