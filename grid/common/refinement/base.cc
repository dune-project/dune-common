// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_COMON_REFINEMENT_BASE_CC
#define DUNE_GRID_COMON_REFINEMENT_BASE_CC

// This file is part of DUNE, a Distributed and Unified Numerics Environment
// This file is copyright (C) 2005 Jorrit Fahlke <jorrit@jorrit.de>
// It is distributed under the terms of the GNU Lesser General Public License version 2.1
// See COPYING at the top of the source tree for the full licence.

/*! @file

   @brief This file contains the parts independent of a particular @ref
         Refinement implementation.

   @verbatim
   $Id$
   @endverbatim
 */

#include "../../../common/geometrytype.hh"

namespace Dune {

  //! @brief This namespace contains the implementation of @ref
  //!        Refinement.
  namespace RefinementImp {

    // /////////////////////////////////
    //
    // Declaration of RefinementImp::Traits
    //

#ifdef DOXYGEN
    // This is just for Doxygen
    /*!
       @brief Mapping from geometryType, CoordType and coerceTo to a
              particular @ref Refinement implementation.

       @param geometryType The GeometryType::BasicType of the element to refine
       @param CoordType    The C++ type of the coordinates
       @param coerceTo     The GeometryType::BasicType of the subelements
       @param dimension    The dimension of the refinement.

       Each @ref Refinement implementation has to define one or more
       specialisations of this struct to declare what it implements.
       Template class Refinement uses this struct to know which
       implementation it should inherit from.

       Each specialisation should contain a single member typedef Imp,
       e.g.:
       @code
       template<class CoordType>
       struct Traits<GeometryType::sphere, CoordType, GeometryType::cube, 2>
       {
       typedef SquaringTheCircle::Refinement Imp;
       };
       @endcode
     */
    template<GeometryType::BasicType geometryType, class CoordType, GeometryType::BasicType coerceTo, int dimension>
    struct Traits
    {
      //! The implementation this specialisation maps to
      typedef SquaringTheCircle::Refinement Imp;
    };
#else // !DOXYGEN
      // Doxygen won't see this
    template<GeometryType::BasicType geometryType, class CoordType, GeometryType::BasicType coerceTo, int dimension>
    struct Traits;
#endif // !DOXYGEN
  } // namespace RefinementImp


  /////////////////
  ///
  ///  Refinement
  ///

  /*! @brief Wrap each @ref Refinement implementation to get a
             consistent interface

     @param geometryType The GeometryType::BasicType of the element to refine
     @param CoordType    The C++ type of the coordinates
     @param coerceTo     The GeometryType::BasicType of the subelements
     @param dimension    The dimension of the refinement.

     @par Member Structs:

     <dl>
     <dt>template<int codimension> struct @ref Codim</dt>
     <dd>codimension template containing the SubEntityIterator</dd>
     </dl>
   */
  template<GeometryType::BasicType geometryType, class CoordType, GeometryType::BasicType coerceTo, int dimension_>
  class Refinement
    : public RefinementImp::Traits<geometryType, CoordType, coerceTo, dimension_>::Imp
  {
  public:
#ifdef DOXYGEN
    /*! @brief The Codim struct inherited from the @ref Refinement implementation

       @param codimension There is a different struct Codim for each codimension
     */
    template<int codimension>
    struct Codim
    {
      /*! @brief The SubEntityIterator for each codim

         This is @em some sort of type, not necessarily a typedef

       */
      typedef SubEntityIterator;
    };

    //! The VertexIterator of the Refinement
    typedef Codim<dimension>::SubEntityIterator VertexIterator;
    //! The ElementIterator of the Refinement
    typedef Codim<0>::SubEntityIterator ElementIterator;

    /*! @brief The CoordVector of the Refinement

       This is always a typedef to a FieldVector
     */
    typedef CoordVector;
    /*! @brief The IndexVector of the Refinement

       This is always a typedef to a FieldVector
     */
    typedef IndexVector;

    //! Get the number of Vertices
    static int nVertices(int level);
    //! Get a VertexIterator
    static VertexIterator vBegin(int level);
    //! Get a VertexIterator
    static VertexIterator vEnd(int level);

    //! Get the number of Elements
    static int nElements(int level);
    //! Get an ElementIterator
    static ElementIterator eBegin(int level);
    //! Get an ElementIterator
    static ElementIterator eEnd(int level);
#endif //DOXYGEN
    typedef typename RefinementImp::Traits<geometryType, CoordType, coerceTo, dimension_>::Imp RefinementImp;

    using RefinementImp::dimension;

    using RefinementImp::Codim;

    using RefinementImp::VertexIterator;
    using RefinementImp::CoordVector;

    using RefinementImp::ElementIterator;
    using RefinementImp::IndexVector;
  };


} // namespace Dune

#endif //DUNE_GRID_COMON_REFINEMENT_BASE_CC
