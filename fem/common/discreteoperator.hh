// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCRETEOPERATOR_INTERFACE_HH__
#define __DUNE_DISCRETEOPERATOR_INTERFACE_HH__

#include <dune/common/operator.hh>
// object pointer management for combined discrete operators
#include "objpointer.hh"

namespace Dune {


  /** @defgroup DiscreteOperator DiscreteOperator
      @ingroup OperatorCommon

     @{
   */

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
      , public ObjPointerStorage
  {
    typedef DiscreteOperatorDefault<DF_DomainType,DF_RangeType> MyType;
  public:
    //! make new operator with item points to null
    DiscreteOperatorDefault () : level_ (0) {}

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

  protected:
    // current level of operator
    mutable int level_;
  }; // end class DiscreteOperatorDefault



  /** @} end documentation group */

} // end namespace Dune

#endif
