// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef ADI_BOUNDARY_HH
#define ADI_BOUNDARY_HH

// System header includes
#include <map>
#include <utility>

// Dune includes
#include <dune/common/mapping.hh>
#include <dune/common/interfaces.hh>
#include "formula.hh"

// Local includes

namespace Dune {

  //! The Boundary interface
  template <class FunctionSpaceT>
  class BoundaryInterface : public Cloneable {
  public:
    //- Enums and typedefs
    // * Cannot use capital letter, otherwise I get a conflict with Alberta
    enum Type { Dirichlet, Neumann };

    //! Domain type
    typedef typename FunctionSpaceT::Domain DomainType;
    //! Range type
    typedef typename FunctionSpaceT::Range RangeType;

    //- Constructors and destructors
    virtual ~BoundaryInterface() {}

    //- Public methods
    //! Evaluates the boundary at a given location.
    //! \param x The (global) coordinate on the boundary
    //! \param result The value on the boundary. In the case of a Neumann
    //! boundary condition, this corresponds to the flux in normal direction
    //! on that boundary
    //! \param time Optional time argument for instationary boundary conditions
    virtual void evaluate(const DomainType& x,
                          RangeType& result,
                          double time = 0.0) const = 0;

    //! Evaluates the boundary at a given location.
    //! This version of evaluate additionally takes additional arguments which
    //! are sometimes needed in Finite Volume formulations. Per default, the
    //! evaluate function with only x and t as parameter is called.
    //! \param val The computed value right at the boundary
    //! \param x The (global) coordinate on the boundary
    //! \param normal The normal vector on the boundary
    //! \param result The value on the boundary. In the case of a Neumann
    //! boundary condition, this corresponds to the flux in normal direction
    //! on that boundary
    //! \param time Optional time argument for instationary boundary conditions
    virtual void evaluate(const RangeType& val,
                          const DomainType& x,
                          const DomainType& normal,
                          RangeType& result,
                          double time = 0.0) const {
      evaluate(x, result, time);
    }

    //! Repeat the clone member function with an enhanced return type
    virtual BoundaryInterface<FunctionSpaceT>* clone() const = 0;

    //! The boundary type of the boundary
    virtual Type boundaryType() const = 0;
  };

  // * Insert a class BoundaryDefault ?

  //! A boundary with an underlying function
  template <class FunctionSpaceT, class FormulaT>
  class FunctionBoundary : public BoundaryInterface<FunctionSpaceT> {
  public:
    //- Public typedefs
    typedef FormulaT FormulaType;
    typedef typename BoundaryInterface<FunctionSpaceT>::Type Type;
    typedef FunctionBoundary<FunctionSpaceT, FormulaT> BoundaryType;
    typedef typename FunctionSpaceT::Domain DomainType;
    typedef typename FunctionSpaceT::Range RangeType;

    //- Constructors and destructors
    //! Constructor
    FunctionBoundary(Type type, const FormulaType& formula) :
      bType_(type),
      formula_(formula) {}

    //! Copy constructor
    FunctionBoundary(const BoundaryType& other) :
      bType_(other.bType_),
      formula_(other.formula_) {}

    //- Methods
    //! Assignment operator
    const BoundaryType& operator= (const BoundaryType& other) {
      if (this != &other) {
        bType_ = other.bType_;
        formula_ = other.formula_;
      }
      return *this;
    }

    //! Clone that boundary object
    virtual BoundaryType* clone() const {
      return new BoundaryType(*this);
    }

    //! Evaluate operator to evaluate the boundary object
    virtual void evaluate (const DomainType& x,
                           RangeType& result,
                           double t = 0.0) const {
      formula_(x, result, t);
    }

    //! The boundary type of this boundary
    virtual Type boundaryType() const { return bType_; }
  private:
    //- Data members
    //! The boundary type
    Type bType_;

    //! The defining formula object
    FormulaType formula_;

  }; // end class FunctionBoundary

  //! Boundary manager class.
  //! A boundary manager class is created and configured by the user. It stores
  //! the boundary objects and manages the correspondence between boundary
  //! identifiers and the actual boundary objects
  template <class FunctionSpaceT>
  class BoundaryManager {
    typedef FunctionBoundary<FunctionSpaceT,
        Constant<FunctionSpaceT> > DefBoundary;
  public:
    //- Public typedefs
    typedef BoundaryInterface<FunctionSpaceT> BoundaryType;

    //- Constructors and destructors
    //! Default constructor generating empty manager with a default Neumann
    //! boundary per default
    explicit BoundaryManager(bool dummy = true) :
      defaultBoundary_(dummy ?
                       new DefBoundary(BoundaryInterface<FunctionSpaceT>::Neumann,
                                       Constant<FunctionSpaceT>()) :
                       0) {}

    //! Constructor with an explicit default boundary
    explicit BoundaryManager(const BoundaryType& bc) :
      defaultBoundary_(bc.clone()) {}

    //! Copy constructor
    BoundaryManager(const BoundaryManager<FunctionSpaceT>& other) {
      if (other.defaultBoundary_) {
        defaultBoundary_ = other.defaultBoundary_->clone();
      } else {
        defaultBoundary_ = 0;
      }
      BoundaryMapIterator endit = other.bc_.end();
      for (BoundaryMapIterator it = other.bc_.begin(); it != endit; ++it) {
        bc_.insert(std::make_pair(it->first, it->second->clone()));
      }
    }

    //! Destructor
    ~BoundaryManager() {
      typedef typename BoundaryMap::iterator Iterator;
      Iterator endit = bc_.end();
      for (Iterator it = bc_.begin(); it != bc_.end(); ++it) {
        delete it->second;
      }
      bc_.clear();
      delete defaultBoundary_;
    }

    //! Assignment operator
    const BoundaryManager<FunctionSpaceT>&
    operator=(const BoundaryManager<FunctionSpaceT>& other) {
      if (this != &other) {
        delete defaultBoundary_;
        if (other.defaultBoundary_) {
          defaultBoundary_ = other.defaultBoundary_->clone();
        } else {
          defaultBoundary_ = 0;
        }
        BoundaryMapIterator endit = bc_.end();
        for (BoundaryMapIterator it = bc_.begin(); it != endit; ++it) {
          delete it->second;
        }
        bc_.clear();

        endit = other.bc_.end();
        for (BoundaryMapIterator it = other.bc_.begin(); it != endit; ++it) {
          bc_.insert(std::make_pair(it->first, it->second->clone()));
        }
      }
      return *this;
    }

    //- Public methods
    //! Add boundary condition object
    void addBoundaryCondition(int id, const BoundaryType& bc) {
      std::pair<BoundaryMapIterator, bool> bp =
        bc_.insert(std::pair<int, BoundaryType*>(id, bc.clone()));
      assert(bp.second); // Make sure no boundary condition is overwritten
    }

    //! \brief Get boundary condition object for a particular part of the
    //! boundary
    //! \note Make sure that you store the boundary conditons for any boundary
    //! segment you want to access or specify a default boundary condition,
    //! otherwise this method will fail!
    const BoundaryType& getBoundaryCondition(int id) const {
      BoundaryMapIterator bit = bc_.find(id);
      if (bit != bc_.end()) {
        return *(bit->second);
      } else {
        assert(defaultBoundary_);
        return *defaultBoundary_;
      }
    }

    //! Return boundary type of boundary object
    typename BoundaryType::Type boundaryType(int id) const {
      return getBoundaryCondition()->boundaryType();
    }

  private:
    //- Local typedefs
    typedef std::map<int, BoundaryType*> BoundaryMap;
    typedef typename BoundaryMap::const_iterator BoundaryMapIterator;
    typedef BoundaryManager<FunctionSpaceT> ThisType;

    //- Data members
    BoundaryMap bc_;

    BoundaryType* defaultBoundary_;
  };


} // end namespace Adi

#endif
