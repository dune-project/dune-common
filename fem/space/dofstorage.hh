// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DOFSTORAGE_HH
#define DUNE_DOFSTORAGE_HH

namespace Dune {

  //! Indicates how the dofs shall be stored in the discrete functions
  //! Point based means that all dofs belonging to one local degree in a
  //! contained spaced are stored consecutively, whereas in the variable based
  //! approach all dofs belonging to one subspace are stored consecutively
  enum DofStoragePolicy { PointBased, VariableBased };

  //! Utility class that helps in the transformation between dofs in the
  //! combined space and its enclosed spaces
  template <DofStoragePolicy p>
  class DofConversionUtility {
  public:
    DofConversionUtility(int size);

    //! Sets new size (in case the number of dofs changes)
    void newSize(int size);

    //! Component which the actual base function index gives a contribution
    //! \return is in range {0, dimRange-1}
    int component(int combinedIndex) const;
    //! Number of the (scalar) base function belonging to base function index
    int containedDof(int combinedIndex) const;

    //! Reverse operation of containedDof, component
    //! i == combinedDof(containedDof(i), component(i))
    int combinedDof(int enclosedIndex, int component) const;
  private:
    int size_;
  };

  //! Specialisation for PointBased approach
  template <>
  class DofConversionUtility<PointBased> {
  public:
    //! Constructor
    //! \param numComponents Number of components in range vector (==dimRange).
    DofConversionUtility(int numComponents) :
      numComponents_(numComponents)
    {}

    //! Find out what type of policy this is.
    static DofStoragePolicy policy() { return PointBased; }

    //! The size of the range vector cannot change, hence this method does
    //! nothing. (In fact, this method is only here so that you can treat
    //! all DofStorageUtility objects identically without knowing whether they
    //! are PointBased or VariableBased.)
    void newSize(int size) {} // just do nothing

    //! Component which the actual base function index gives a contribution
    //! \return is in range {0, dimRange-1}
    int component(int combinedIndex) const {
      return combinedIndex%numComponents_;
    }
    //! Number of the (scalar) base function belonging to base function index
    int containedDof(int combinedIndex) const {
      return combinedIndex/numComponents_;
    }

    //! Reverse operation of containedDof, component
    //! i == combinedDof(containedDof(i), component(i))
    int combinedDof(int containedIndex, int component) const {
      return containedIndex*numComponents_ + component;
    }

  private:
    const int numComponents_;
  };

  //! Specialisation for VariableBased approach
  template <>
  class DofConversionUtility<VariableBased> {
  public:
    //! Constructor
    //! \param size Number of global dofs per component.
    DofConversionUtility(int size) :
      size_(size)
    {}

    //! Find out what type of policy this is.
    static DofStoragePolicy policy() { return VariableBased; }

    //! Set new size after adaptation.
    void newSize(int size) { size_ = size; }

    //! Component which the actual base function index gives a contribution
    //! \return is in range {0, dimRange-1}
    int component(int combinedIndex) const {
      return combinedIndex/size_;
    }

    //! Number of the (scalar) base function belonging to base function index
    int containedDof(int combinedIndex) const {
      return combinedIndex%size_;
    }

    //! Reverse operation of containedDof, component
    //! i == combinedDof(containedDof(i), component(i))
    int combinedDof(int containedIndex, int component) const {
      return containedIndex + component*size_;
    }

  private:
    int size_;
  };

} // end namespace Dune

#endif
