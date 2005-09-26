// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_COMMON_VIRTUALREFINEMENT_CC
#define DUNE_GRID_COMMON_VIRTUALREFINEMENT_CC

// This file is part of DUNE, a Distributed and Unified Numerics Environment
// This file is copyright (C) 2005 Jorrit Fahlke <jorrit@jorrit.de>
// It is distributed under the terms of the GNU Lesser General Public License version 2.1
// See COPYING at the top of the source tree for the full licence.

/*! @file

   @brief This file contains the virtual wrapper around refinement.

   @verbatim
   $Id$
   @endverbatim
 */

#include "../../common/exceptions.hh"
#include "grid.hh"
#include "../../common/iteratorfacades.hh"
#include "../../common/fvector.hh"
#include "refinement.hh"
#include <dune/common/exceptions.hh>

namespace Dune {

  // //////////////////////////////////////////
  //
  // The virtual base class and its iterators
  //

  //
  // Refinement
  //

  template<int dimension, class CoordType>
  typename VirtualRefinement<dimension, CoordType>::VertexIterator
  VirtualRefinement<dimension, CoordType>::
  vBegin(int level) const
  {
    return VertexIterator(vBeginBack(level));
  }

  template<int dimension, class CoordType>
  typename VirtualRefinement<dimension, CoordType>::VertexIterator
  VirtualRefinement<dimension, CoordType>::
  vEnd(int level) const
  {
    return VertexIterator(vEndBack(level));
  }

  template<int dimension, class CoordType>
  typename VirtualRefinement<dimension, CoordType>::ElementIterator
  VirtualRefinement<dimension, CoordType>::
  eBegin(int level) const
  {
    return ElementIterator(eBeginBack(level));
  }

  template<int dimension, class CoordType>
  typename VirtualRefinement<dimension, CoordType>::ElementIterator
  VirtualRefinement<dimension, CoordType>::
  eEnd(int level) const
  {
    return ElementIterator(eEndBack(level));
  }

  //
  // The iterators
  //

  template<int dimension, class CoordType, int codimension>
  class VirtualRefinementSubEntityIteratorSpecial;

  // The iterator for vertices

  template<int dimension, class CoordType>
  class VirtualRefinementSubEntityIteratorSpecial<dimension, CoordType, dimension>
  {
  public:
    typedef typename VirtualRefinement<dimension, CoordType>::template Codim<dimension>::SubEntityIterator Common;
    typedef VirtualRefinement<dimension, CoordType> Refinement;
    typedef typename Refinement::CoordVector CoordVector;

    CoordVector coords() const;
  };

  template<int dimension, class CoordType>
  typename VirtualRefinementSubEntityIteratorSpecial<dimension, CoordType, dimension>::CoordVector
  VirtualRefinementSubEntityIteratorSpecial<dimension, CoordType, dimension>::
  coords() const
  {
    return static_cast<const Common *>(this)->backend->coords();
  }

  // The iterator for elements

  template<int dimension, class CoordType>
  class VirtualRefinementSubEntityIteratorSpecial<dimension, CoordType, 0>
  {
  public:
    typedef VirtualRefinement<dimension, CoordType> Refinement;
    typedef typename Refinement::template Codim<0>::SubEntityIterator Common;
    typedef typename Refinement::IndexVector IndexVector;

    IndexVector vertexIndices() const;
  };

  template<int dimension, class CoordType>
  typename VirtualRefinementSubEntityIteratorSpecial<dimension, CoordType, 0>::IndexVector
  VirtualRefinementSubEntityIteratorSpecial<dimension, CoordType, 0>::
  vertexIndices() const
  {
    return static_cast<const Common *>(this)->backend->vertexIndices();
  }

  // The iterator common stuff

  template<int dimension, class CoordType>
  template<int codimension>
  class VirtualRefinement<dimension, CoordType>::Codim<codimension>::SubEntityIterator
    : public ForwardIteratorFacade<typename VirtualRefinement<dimension, CoordType>::template Codim<codimension>::SubEntityIterator, int>,
      public VirtualRefinementSubEntityIteratorSpecial<dimension, CoordType, codimension>
  {
  public:
    typedef typename VirtualRefinement<dimension, CoordType>::template Codim<codimension>::SubEntityIterator This;
    typedef typename VirtualRefinement<dimension, CoordType>::template SubEntityIteratorBack<codimension> IteratorBack;

    SubEntityIterator(IteratorBack *backend);
    SubEntityIterator(const This &other);
    ~SubEntityIterator();

    This &operator=(const This &other);

    bool equals(const This &other) const;
    void increment();

    int index() const;
  private:
    friend class VirtualRefinementSubEntityIteratorSpecial<dimension, CoordType, codimension>;
    IteratorBack *backend;
  };

  template<int dimension, class CoordType>
  template<int codimension>
  VirtualRefinement<dimension, CoordType>::Codim<codimension>::SubEntityIterator::
  SubEntityIterator(IteratorBack *backend_)
    : backend(backend_)
  {}

  template<int dimension, class CoordType>
  template<int codimension>
  VirtualRefinement<dimension, CoordType>::Codim<codimension>::SubEntityIterator::
  SubEntityIterator(const This &other)
    : backend(other.backend->clone())
  {}

  template<int dimension, class CoordType>
  template<int codimension>
  VirtualRefinement<dimension, CoordType>::Codim<codimension>::SubEntityIterator::
  ~SubEntityIterator()
  {
    delete backend;
  }

  template<int dimension, class CoordType>
  template<int codimension>
  typename VirtualRefinement<dimension, CoordType>::template Codim<codimension>::SubEntityIterator &
  VirtualRefinement<dimension, CoordType>::Codim<codimension>::SubEntityIterator::
  operator=(const This &other)
  {
    delete backend;
    backend = other.backend->clone();
  }

  template<int dimension, class CoordType>
  template<int codimension>
  bool
  VirtualRefinement<dimension, CoordType>::Codim<codimension>::SubEntityIterator::
  equals(const This &other) const
  { return *backend == *(other.backend); }

  template<int dimension, class CoordType>
  template<int codimension>
  void
  VirtualRefinement<dimension, CoordType>::Codim<codimension>::SubEntityIterator::
  increment()
  {
    ++*backend;
  }

  template<int dimension, class CoordType>
  template<int codimension>
  int
  VirtualRefinement<dimension, CoordType>::Codim<codimension>::SubEntityIterator::
  index() const
  { return backend->index(); }

  //
  // The iterator backend
  //

  template<int dimension, class CoordType, int codimension>
  class VirtualRefinementSubEntityIteratorBackSpecial;

  // The iterator backend for vertices

  template<int dimension, class CoordType>
  class VirtualRefinementSubEntityIteratorBackSpecial<dimension, CoordType, dimension>
  {
  public:
    typedef VirtualRefinement<dimension, CoordType> Refinement;
    typedef typename Refinement::CoordVector CoordVector;

    virtual CoordVector coords() const = 0;

    virtual ~VirtualRefinementSubEntityIteratorBackSpecial()
    {}

  };

  // The iterator backend for elements

  template<int dimension, class CoordType>
  class VirtualRefinementSubEntityIteratorBackSpecial<dimension, CoordType, 0>
  {
  public:
    typedef VirtualRefinement<dimension, CoordType> Refinement;
    typedef typename Refinement::IndexVector IndexVector;

    virtual IndexVector vertexIndices() const = 0;

    virtual ~VirtualRefinementSubEntityIteratorBackSpecial()
    {}

  };

  // The iterator backend common stuff

  template<int dimension, class CoordType>
  template<int codimension>
  class VirtualRefinement<dimension, CoordType>::SubEntityIteratorBack
    : public VirtualRefinementSubEntityIteratorBackSpecial<dimension, CoordType, codimension>
  {
  public:
    typedef typename VirtualRefinement<dimension, CoordType>::template SubEntityIteratorBack<codimension> This;

    virtual ~SubEntityIteratorBack() {}

    virtual This *clone() const = 0;

    virtual bool operator==(const This &other) const = 0;
    virtual This &operator++() = 0;

    virtual int index() const = 0;
  };

  // /////////////////////////////////////////////////
  //
  // The derived classes and their iterator backends
  //

  //
  // The refinement implementation
  //

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo>
  class VirtualRefinementImp
    : public VirtualRefinement<Refinement<geometryType, CoordType, coerceTo>::dimension, CoordType>
  {
  public:
    typedef Refinement<geometryType, CoordType, coerceTo> Refinement;
    enum { dimension = Refinement::dimension };
    typedef VirtualRefinement<dimension, CoordType> VirtualRefinement;

    template<int codimension>
    class SubEntityIteratorBack;

    int nVertices(int level) const;
    int nElements(int level) const;

    static VirtualRefinementImp<geometryType, CoordType, coerceTo> &instance();
  private:
    VirtualRefinementImp() {}
    static VirtualRefinementImp<geometryType, CoordType, coerceTo> *instance_;

    typename VirtualRefinement::VertexIteratorBack *vBeginBack(int level) const;
    typename VirtualRefinement::VertexIteratorBack *vEndBack(int level) const;
    typename VirtualRefinement::ElementIteratorBack *eBeginBack(int level) const;
    typename VirtualRefinement::ElementIteratorBack *eEndBack(int level) const;
  };

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo>
  VirtualRefinementImp<geometryType, CoordType, coerceTo> &
  VirtualRefinementImp<geometryType, CoordType, coerceTo>::instance()
  {
    if(instance_ == 0)
      instance_ = new VirtualRefinementImp<geometryType, CoordType, coerceTo>;
    return *instance_;
  }

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo>
  VirtualRefinementImp<geometryType, CoordType, coerceTo> *
  VirtualRefinementImp<geometryType, CoordType, coerceTo>::instance_ = 0;

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo>
  int
  VirtualRefinementImp<geometryType, CoordType, coerceTo>::
  nVertices(int level) const
  {
    return Refinement::nVertices(level);
  }

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo>
  typename VirtualRefinementImp<geometryType, CoordType, coerceTo>::VirtualRefinement::VertexIteratorBack *
  VirtualRefinementImp<geometryType, CoordType, coerceTo>::
  vBeginBack(int level) const
  { return new SubEntityIteratorBack<dimension>(Refinement::vBegin(level)); }

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo>
  typename VirtualRefinementImp<geometryType, CoordType, coerceTo>::VirtualRefinement::VertexIteratorBack *
  VirtualRefinementImp<geometryType, CoordType, coerceTo>::
  vEndBack(int level) const
  { return new SubEntityIteratorBack<dimension>(Refinement::vEnd(level)); }

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo>
  int
  VirtualRefinementImp<geometryType, CoordType, coerceTo>::
  nElements(int level) const
  {
    return Refinement::nElements(level);
  }

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo>
  typename VirtualRefinementImp<geometryType, CoordType, coerceTo>::VirtualRefinement::ElementIteratorBack *
  VirtualRefinementImp<geometryType, CoordType, coerceTo>::
  eBeginBack(int level) const
  { return new SubEntityIteratorBack<0>(Refinement::eBegin(level)); }

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo>
  typename VirtualRefinementImp<geometryType, CoordType, coerceTo>::VirtualRefinement::ElementIteratorBack *
  VirtualRefinementImp<geometryType, CoordType, coerceTo>::
  eEndBack(int level) const
  { return new SubEntityIteratorBack<0>(Refinement::eEnd(level)); }

  //
  // The iterator backend implementation
  //

  // The iterator backend implementation specialties

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo, int codimension>
  class VirtualRefinementImpSubEntityIteratorBackSpecial;

  // The iterator backend implementation specialties for vertices

  // to avoid "template argument ` Dune::Refinement<geometryType,CoordType,coerceTo>::dimension' involves template parameter(s)"
  // we use the general template for vertices and specialize only for elements.
  // This works because the classes with codimension other than dimension or 0 are undefined anyway.

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo, int dimension>
  class VirtualRefinementImpSubEntityIteratorBackSpecial
    : public VirtualRefinement<dimension, CoordType>::template SubEntityIteratorBack<dimension>
  {
  public:
    typedef typename VirtualRefinementImp<geometryType, CoordType, coerceTo>::template SubEntityIteratorBack<dimension> Common;
    typedef VirtualRefinement<dimension, CoordType> RefinementBase;
    typedef typename RefinementBase::CoordVector CoordVector;

    // construct to abort compilation unless dimension == codimension
    VirtualRefinementImpSubEntityIteratorBackSpecial()
    { IsTrue<dimension == Refinement<geometryType, CoordType, coerceTo>::dimension>::yes(); }

    CoordVector coords() const;
  };

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo, int dimension>
  typename VirtualRefinementImpSubEntityIteratorBackSpecial<geometryType, CoordType, coerceTo, dimension>::CoordVector
  VirtualRefinementImpSubEntityIteratorBackSpecial<geometryType, CoordType, coerceTo, dimension>::
  coords() const
  {
    return static_cast<const Common*>(this)->backend.coords();
  }

  // The iterator backend implementation specialties for elements

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo>
  class VirtualRefinementImpSubEntityIteratorBackSpecial<geometryType, CoordType, coerceTo, 0>
    : public VirtualRefinement<Refinement<geometryType, CoordType, coerceTo>::dimension, CoordType>::template SubEntityIteratorBack<0>
  {
  public:
    enum { dimension = Refinement<geometryType, CoordType, coerceTo>::dimension };
    typedef VirtualRefinementImp<geometryType, CoordType, coerceTo> VirtualRefinementImp;
    typedef typename VirtualRefinementImp::template SubEntityIteratorBack<0> Common;
    typedef typename VirtualRefinementImp::Refinement Refinement;
    typedef VirtualRefinement<dimension, CoordType> RefinementBase;
    typedef typename RefinementBase::IndexVector IndexVector;

    IndexVector vertexIndices() const;
  };

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo>
  typename VirtualRefinementImpSubEntityIteratorBackSpecial<geometryType, CoordType, coerceTo, 0>::IndexVector
  VirtualRefinementImpSubEntityIteratorBackSpecial<geometryType, CoordType, coerceTo, 0>::
  vertexIndices() const
  {
    IndexVector vIndices;
    vIndices.reserve(Refinement::IndexVector::dimension);

    typename Refinement::IndexVector sIndices = static_cast<const Common *>(this)->backend.vertexIndices();
    for(int i = 0; i < Refinement::IndexVector::dimension; ++i)
      vIndices.push_back(sIndices[i]);
    return vIndices;
  }

  // The shared iterator backend implementation

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo>
  template<int codimension>
  class VirtualRefinementImp<geometryType, CoordType, coerceTo>::SubEntityIteratorBack
    : public VirtualRefinementImpSubEntityIteratorBackSpecial<geometryType, CoordType, coerceTo, codimension>
  {
  public:
    typedef typename Refinement::template Codim<codimension>::SubEntityIterator BackendIterator;
    typedef typename VirtualRefinementImp<geometryType, CoordType, coerceTo>::template SubEntityIteratorBack<codimension> This;
    typedef typename VirtualRefinement::template SubEntityIteratorBack<codimension> Base;
    enum { dimension = Refinement::dimension };

    SubEntityIteratorBack(const BackendIterator &backend);
    SubEntityIteratorBack(const This &other);

    Base *clone() const;

    bool operator==(const Base &other) const;
    Base &operator++();

    int index() const;
  private:
    friend class VirtualRefinementImpSubEntityIteratorBackSpecial<geometryType, CoordType, coerceTo, codimension>;
    BackendIterator backend;
  };

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo>
  template<int codimension>
  VirtualRefinementImp<geometryType, CoordType, coerceTo>::SubEntityIteratorBack<codimension>::
  SubEntityIteratorBack(const BackendIterator &backend_)
    : backend(backend_)
  {}

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo>
  template<int codimension>
  VirtualRefinementImp<geometryType, CoordType, coerceTo>::SubEntityIteratorBack<codimension>::
  SubEntityIteratorBack(const This &other)
    : VirtualRefinementImpSubEntityIteratorBackSpecial<geometryType, CoordType, coerceTo, codimension>(other),
      backend(other.backend)
  {}

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo>
  template<int codimension>
  typename VirtualRefinementImp<geometryType, CoordType, coerceTo>::template SubEntityIteratorBack<codimension>::Base *
  VirtualRefinementImp<geometryType, CoordType, coerceTo>::SubEntityIteratorBack<codimension>::
  clone() const
  { return new This(*this); }

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo>
  template<int codimension>
  bool
  VirtualRefinementImp<geometryType, CoordType, coerceTo>::SubEntityIteratorBack<codimension>::
  operator==(const Base &other) const
  {
    try { return backend == dynamic_cast<const This &>(other).backend; }
    catch(std::bad_cast) { return false; }
  }

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo>
  template<int codimension>
  typename VirtualRefinementImp<geometryType, CoordType, coerceTo>::template SubEntityIteratorBack<codimension>::Base &
  VirtualRefinementImp<geometryType, CoordType, coerceTo>::SubEntityIteratorBack<codimension>::
  operator++()
  {
    ++backend;
    return *this;
  }

  template<GeometryType geometryType, class CoordType, GeometryType coerceTo>
  template<int codimension>
  int
  VirtualRefinementImp<geometryType, CoordType, coerceTo>::SubEntityIteratorBack<codimension>::
  index() const
  { return backend.index(); }

  // ////////////////////////
  //
  // The refinement builder
  //

  template<int dimension, class CoordType>
  class RefinementBuilder;

  /*! @brief return a reference to the VirtualRefinement according to
             the parameters

     @param dimension Dimension of the element to refine
     @param CoordType C++ type of the coordinates

     @throws NotImplemented There is no Refinement implementation for
                           the specified parameters.
   */
  template<int dimension, class CoordType>
  VirtualRefinement<dimension, CoordType> &
  buildRefinement( //! geometry type of the refined element
    GeometryType geometryType,
    //! geometry type of the subelements
    GeometryType coerceTo)
  {
    return RefinementBuilder<dimension, CoordType>::build(geometryType, coerceTo);
  }

  template<class CoordType>
  class RefinementBuilder<2, CoordType>
  {
  public:
    static
    VirtualRefinement<2, CoordType> &
    build(GeometryType geometryType, GeometryType coerceTo)
    {
      switch(geometryType) {
      case triangle :
        switch(coerceTo) {
        case triangle :
          return VirtualRefinementImp<triangle, CoordType, triangle>::instance();
        default :
          break;
        }
        break;
      case cube :
        switch(coerceTo) {
        case triangle :
          return VirtualRefinementImp<quadrilateral, CoordType, triangle>::instance();
        case quadrilateral :
          return VirtualRefinementImp<quadrilateral, CoordType, quadrilateral>::instance();
        case cube :
          return VirtualRefinementImp<quadrilateral, CoordType, quadrilateral>::instance();
        default :
          break;
        }
        break;
      case quadrilateral :
        switch(coerceTo) {
        case triangle :
          return VirtualRefinementImp<quadrilateral, CoordType, triangle>::instance();
        case quadrilateral :
          return VirtualRefinementImp<quadrilateral, CoordType, quadrilateral>::instance();
        default :
          break;
        }
        break;
      default :
        break;
      }
      DUNE_THROW(NotImplemented,
                 "No Refinement<" << geometryType << ", CoordType, "
                                  << coerceTo << " >.");
    }
  };

  template<class CoordType>
  class RefinementBuilder<3, CoordType>
  {
  public:
    static
    VirtualRefinement<3, CoordType> &
    build(GeometryType geometryType, GeometryType coerceTo)
    {
      switch(geometryType) {
      case tetrahedron :
        switch(coerceTo) {
        case tetrahedron :
          return VirtualRefinementImp<tetrahedron, CoordType, tetrahedron>::instance();
        default :
          break;
        }
        break;
      case simplex :
        switch(coerceTo) {
        case tetrahedron :
          return VirtualRefinementImp<tetrahedron, CoordType, tetrahedron>::instance();
        case simplex :
          return VirtualRefinementImp<tetrahedron, CoordType, tetrahedron>::instance();
        default :
          break;
        }
        break;
      case hexahedron :
        switch(coerceTo) {
        case tetrahedron :
          return VirtualRefinementImp<hexahedron, CoordType, tetrahedron>::instance();
        case hexahedron :
          return VirtualRefinementImp<hexahedron, CoordType, hexahedron>::instance();
        default :
          break;
        }
        break;
      case cube :
        switch(coerceTo) {
        case tetrahedron :
          return VirtualRefinementImp<hexahedron, CoordType, tetrahedron>::instance();
        case hexahedron :
          return VirtualRefinementImp<hexahedron, CoordType, hexahedron>::instance();
        case cube :
          return VirtualRefinementImp<hexahedron, CoordType, hexahedron>::instance();
        default :
          break;
        }
        break;
      default :
        break;
      }
      DUNE_THROW(NotImplemented,
                 "No Refinement<" << geometryType << ", CoordType, "
                                  << coerceTo << " >.");
    }
  };

} // namespace Dune

#endif //DUNE_GRID_COMMON_VIRTUALREFINEMENT_CC
