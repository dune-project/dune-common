// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_FORMULA_HH
#define DUNE_FORMULA_HH

//- Dune includes
#include <dune/common/interfaces.hh>


namespace Dune {
  //! \brief Interface class for a generic formula
  template <class FunctionSpaceImp>
  class FormulaInterface : public Cloneable {
  public:
    //- Typedefs
    typedef typename FunctionSpaceImp::DomainType DomainType;
    typedef typename FunctionSpaceImp::RangeType RangeType;
    typedef typename FunctionSpaceImp::RangeFieldType RangeFieldType;

    //- Methods
    virtual void operator() (const DomainType& x,
                             RangeType& result,
                             double t = 0.0) const = 0;

  }; // end class FormulaInterface

  //! \brief Constant valued function
  template <class FunctionSpaceImp>
  class Constant : public FormulaInterface<FunctionSpaceImp> {
    typedef FormulaInterface<FunctionSpaceImp> BaseType;
  public:
    typedef typename BaseType::RangeType RangeType;
    typedef typename BaseType::DomainType DomainType;

    //! Constructor with scalar value
    explicit Constant(typename BaseType::RangeFieldType c = 0.0) : c_(c) {}

    //! Constructor with vector value
    explicit Constant(typename BaseType::RangeType c) : c_(c) {}

    //! Copy constructor
    Constant(const Constant<FunctionSpaceImp>& other) :
      c_(other.c_) {}

    //! Assignment operator
    Constant& operator= (const Constant<FunctionSpaceImp>& other) {
      if (this != &other) {
        for (int i = 0; i < RangeType::size; ++i) {
          c_[i] = other.c_[i];
        }
      }
      return *this;
    }

    virtual void operator() (const DomainType& x,
                             RangeType& result,
                             double t = 0.0) const {
      result = c_;
    }

    virtual Constant* clone() const { return new Constant(c_); }
  private:
    typename BaseType::RangeType c_;
  };

} // end namespace Dune

#endif
