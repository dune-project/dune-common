// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_OPERATOR_HH__
#define __DUNE_OPERATOR_HH__

#include "mapping.hh"

namespace Dune
{
  /** @defgroup OperatorCommon Operators
     @ingroup AlgebraCommon
     Operators are mappings from function spaces into function spaces.
   */

  /** @defgroup Operator Operator
      @ingroup OperatorCommon

     @{
   */

  template <typename DFieldType, typename RFieldType,
      typename DType , typename RType>
  class Operator : public Mapping <DFieldType,RFieldType,DType,RType>
  {
  public:
    //! remember template parameters for derived classes
    typedef DType DomainType;
    typedef RType RangeType;
    typedef DFieldType DomainFieldType;
    typedef RFieldType RangeFieldType;

    //! apply operator, used by mapping
    void apply ( const DomainType & arg, RangeType & dest ) const
    {
      std::cerr << "Operator::apply is called! \n";
      abort();
    }

  private:

  }; // end class Operator

  /** @} end documentation group */

} // end namespace Dune

#endif
