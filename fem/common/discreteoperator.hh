// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCRETEOPERATOR_INTERFACE_HH__
#define __DUNE_DISCRETEOPERATOR_INTERFACE_HH__

#include <dune/common/operator.hh>
// object pointer management for combined discrete operators
#include "objpointer.hh"
#include "localoperator.hh"

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
  template <class LocalOperatorImp, class DFDomainType, class DFRangeType ,
      template <class,class,class> class DiscreteOperatorImp >
  class DiscreteOperatorDefault
    : public Operator <typename DFDomainType::RangeFieldType,
          typename DFRangeType::RangeFieldType,
          DFDomainType , DFRangeType>
      , public ObjPointerStorage
  {

  protected:
    // define my type for inherited classes
    typedef DiscreteOperatorDefault < LocalOperatorImp , DFDomainType ,
        DFRangeType , DiscreteOperatorImp > DiscreteOperatorDefaultType;

    //! Operator which is called on each entity
    LocalOperatorImp & localOp_;

  public:
    //! make new operator with item points to null
    DiscreteOperatorDefault (LocalOperatorImp & op)
      : localOp_ (op) , level_ (0) {}

    //! no public method, but has to be public, because all discrete operator
    //! must be able to call this method an the template parameters are
    //! allways diffrent
    LocalOperatorImp & getLocalOp ()
    {
      return localOp_;
    }

    /*! add another discrete operator by combining the two local operators
       and creating a new discrete operator whereby a pointer to the new
       object is kept in the object where the operator was called.
       If this object is deleted then all objects created during operator +
       called are deleted too.
     */
    template <class LocalOperatorType>
    DiscreteOperatorImp<  CombinedLocalOperator < LocalOperatorImp , LocalOperatorType> ,
        DFDomainType, DFRangeType >  &
    operator + (const DiscreteOperatorImp < LocalOperatorType, DFDomainType , DFRangeType > &op )
    {
      typedef DiscreteOperatorImp < LocalOperatorType , DFDomainType, DFRangeType> CopyType;
      typedef CombinedLocalOperator < LocalOperatorImp , LocalOperatorType > COType;

      COType *locOp =
        new COType ( localOp_ , const_cast<CopyType &> (op).getLocalOp (), asImp().printInfo() );

      typedef DiscreteOperatorImp <COType, DFDomainType, DFRangeType > OPType;

      OPType *discrOp = new OPType (op, *locOp );

      // memorize this new generated object because is represents this
      // operator and is deleted if this operator is deleted
      // this is managed by the ObjPointerStorage class
      this->saveObjPointer( discrOp , locOp );

      return (*discrOp);
    }

    //! This method work just the same way like the operator +
    //! but this time for mutiplication with scalar
    DiscreteOperatorImp< ScaledLocalOperator < LocalOperatorImp ,
            typename  DFDomainType::RangeFieldType > ,  DFDomainType,DFRangeType > &
    operator * (const RangeFieldType& scalar)
    {
      typedef ScaledLocalOperator < LocalOperatorImp,
          typename  DFDomainType::RangeFieldType > ScalOperatorType;

      typedef DiscreteOperatorImp < ScalOperatorType , DFDomainType,DFRangeType > ScalDiscrType;

      ScalOperatorType * sop = new ScalOperatorType ( localOp_ , scalar, asImp().printInfo() );
      ScalDiscrType *discrOp = new ScalDiscrType ( asImp() , *sop );

      // memorize this new generated object because is represents this
      // operator and is deleted if this operator is deleted
      this->saveObjPointer ( discrOp, sop );

      return (*discrOp);
    }

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
    void apply ( const DFDomainType & arg, DFRangeType & dest) const
    {
      std::cerr << "ERROR: DiscreteOperatorDefault::apply called! \n";
    }

    //! apply operator
    void operator () ( const DFDomainType & arg, DFRangeType & dest ) const
    {
      std::cerr << "ERROR: DiscreteOperatorDefault::operator () called! \n";
    }

    bool printInfo () { return false; }

  protected:
    // current level of operator
    mutable int level_;

  private:
    DiscreteOperatorImp < LocalOperatorImp , DFDomainType , DFRangeType > &
    asImp() { return static_cast <DiscreteOperatorImp < LocalOperatorImp , DFDomainType , DFRangeType > &> (*this); }

    const DiscreteOperatorImp < LocalOperatorImp , DFDomainType , DFRangeType > &
    asImp() const { return static_cast <const DiscreteOperatorImp < LocalOperatorImp , DFDomainType , DFRangeType > &> (*this); }

  }; // end class DiscreteOperatorDefault

  /** @} end documentation group */

} // end namespace Dune

#endif
