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

    void newSize(int size);

    int component(int combinedIndex) const;
    int containedDof(int combinedIndex) const;

    int combinedDof(int enclosedIndex, int component) const;
  private:
    int size_;
  };

  template <>
  class DofConversionUtility<PointBased> {
  public:
    DofConversionUtility(int numComponents) :
      numComponents_(numComponents)
    {}

    static DofStoragePolicy policy() { return PointBased; }

    void newSize(int size) {} // just do nothing

    int component(int combinedIndex) const {
      return combinedIndex%numComponents_;
    }
    int containedDof(int combinedIndex) const {
      return combinedIndex/numComponents_;
    }

    int combinedDof(int containedIndex, int component) const {
      return containedIndex*numComponents_ + component;
    }

  private:
    const int numComponents_;
  };

  template <>
  class DofConversionUtility<VariableBased> {
  public:
    DofConversionUtility(int size) :
      size_(size)
    {}

    static DofStoragePolicy policy() { return VariableBased; }

    void newSize(int size) { size_ = size; }

    int component(int combinedIndex) const {
      return combinedIndex/size_;
    }
    int containedDof(int combinedIndex) const {
      return combinedIndex%size_;
    }

    int combinedDof(int containedIndex, int component) const {
      return containedIndex + component*size_;
    }

  private:
    int size_;
  };

} // end namespace Dune

#endif
