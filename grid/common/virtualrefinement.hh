// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_COMMON_VIRTUALREFINEMENT_HH
#define DUNE_GRID_COMMON_VIRTUALREFINEMENT_HH

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

/*! @addtogroup VirtualRefinement Virtual Refinement
   \ingroup Refinement
    <!--WWWWWWWWWWWWWWWWWWWW-->

   Please have a look on the non-virtual @link Refinement
   Refinement@endlink documentation.

   @section General
   <!--=========-->

   @link Refinement Refinement@endlink can only be used when you know
   the geometryType of your entities at compile time.  You could
   circumvent this by using a switch(geometryType), but each case would
   look very much the same.  If you have many such switch() statements,
   or each case contains lots of code, or you simply have many possible
   geometryTypes, this can be quiet annoying.

   VirtualRefinement does all of this switch() statements for you.  It
   defines a common virtual base class per dimension, and derives one
   class for each geometryType and coerceTo from that class.  The
   derived classes simply wrap the non-virtual classes from @link
   Refinement Refinement@endlink.  This makes it possible to treat each
   geometryType (of a given dimension) the same, and thus eleminates
   the many repetitions of lots of code.

   But the case statements are not totally gone yet.  VirtualRefinement
   does these statements once and for all by wrapping them into the
   buildRefinement() function.

   @section User_interface The user Interface
   <!--===================================-->

   @subsection VirtualRefinement The VirtualRefinement class
   <!------------------------------------------------------>

   VirtualRefinement is not a set of unrelated specialisations of the
   same template class.  VirtualRefinement is a base class with several
   virtual methods, which are overloaded by the concrete
   VirtualRefinement implementation classes.  Each implementation class
   wraps one of the non-virtual @link Refinement Refinement@endlink
   classes.

   The user interface is modelled closely after the the @link
   Refinement Refinement@endlink interface.  The main differences are:

   - VirtualRefinement is not a static class, but a singleton.  Thus
    each VirtualRefinement implementation has to be instanciated
    before use.  This is done with the template function
    buildRefinement (see below).
   - Since the methods of VirtualRefinement are virtual (or use virtual
    methods themself) they have to be called like
    @code
   refinementInstace.nElements(level);
    @endcode
    instead of
    @code
   RefinementTypedef::nElements(level);
    @endcode

   - IndexVector is a std::vector instead of a FieldVector since the
    number of corners of different geometry types may be different at
    runtime.  The user is responsible to always pass the same coerceTo
    parameter to buildRefinement() so he always gets the same number
    of corners.

   @code
   template<int dimension>
   class VirtualRefinement
   {
   public:
    template<int Codimension>
    struct Codim {
      class SubEntityIterator;
    };
    typedef VertexIterator; // These are aliases for Codim<codim>::SubEntityIterator
    typedef ElementIterator;

    typedef IndexVector; // This is a std::vector
    typedef CoordVector; // This is a FieldVector

    virtual int nVertices(int level) const;
    VertexIterator vBegin(int level) const;
    VertexIterator vEnd(int level) const;
    virtual int nElements(int level) const;
    ElementIterator eBegin(int level) const;
    ElementIterator eEnd(int level) const;
   };
   @endcode

   The iterators have the same interface as the @link Refinement
   Refinement@endlink iterators except that IndexVector is a
   std::vector instead of a FieldVector (see above).  Also the
   restriction that the Iterators are not derefencable applies.

   @code
   template<int dimension>
   class VertexIterator
   {
   public:
    typedef VirtualRefinement<dimension> Refinement;

    int index() const;
    Refinement::CoordVector coords() const;
   };

   template<int dimension>
   class ElementIterator
   {
   public:
    typedef VirtualRefinement<dimension> Refinement;

    int index() const;
    Refinement::IndexVector vertexIndices() const;
   };
   @endcode

   @subsection buildRefinement buildRefinement()
   <!------------------------------------------>

   The declaration for buildRefinement is

   @code
   template<int dimension, class CoordType>
   VirtualRefinement<dimension, CoordType> &buildRefinement(GeometryType geometryType, GeometryType coerceTo);
   @endcode

   It is expected that you know the dimension and the coordinate type
   of the elements you want to refine at compile time.

   The simple case is that you want to refine, say, quadrilaterals and
   the subentities should look like quadrilaterals as well.  In that
   case you would call buildRefinement() like

   @code
   VirtualRefinement<2, CoordType> &refinement = buildRefinement<2, CoordType>(quadrilateral, quadrilateral);
   @endcode

   The more complicated case is that your entity is a quadrilateral,
   but the subentities should look like triangles.  In this case call
   buildRefinement() like

   @code
   VirtualRefinement<2, CoordType> &refinement = buildRefinement<2, CoordType>(quadrilateral, triangle);
   @endcode

   Summary: geometryType is the geometry type of the entity you want to
   refine, while coerceTo is the geometry type of the subentities.

   @section Implementing Implementing a new Refinement type
   <!--=================================================-->

   When you write a Refinement implementation for a new combination of
   geometryType and coerceTo, you have to tell buildRefinement() about
   it.

   - First, you have to implement the non-virtual part in @link
    Refinement Refinement@endlink, if you have not done so yet.
   - Second, visit the end of refinementvirtual.cc, and look for the
    specialisations of template<int dimension, class CoordType> class
    RefinementBuilder.  There is one specialisation for each
    dimension, containing the single method build().
   - The build() contains two levels of switch statements, the outer
    for geomentryType and the inner for coerceTo.  Each case will
    either return the correct VirtualRefinement or fall throught to
    the end of the method and throw an error.  Insert the cases for
    your refinement.

   Everything else has been done for you automatically.

   @subsection Namespaces
   <!------------------->

   VirtualRefinement does not use a complicated namespace scheme like
   @link Refinement Refinement@endlink.  The complete VirtualRefinement
   stuff simply lives directly in namespace Dune.

   @subsection Layers Conceptual layers
   <!--------------------------------->

   VirtualRefinement adds to more layers to the ones already defined in
   @link Refinement Refinement@endlink:

   - <strong>Layer 3</strong> makes it easy to use several Refinement
    implementations in the same code, when you only know at run-time,
    which Refinement implementation you need.  It wraps class
    Refinement and it's iterators into a Proxy class, retaining it's
    interface but all deriving from a virtual base class
    VirtualRefinement<dimension, CoordType>.  This is located in
    refinementvirtual.cc.
   - <strong>Layer 4</strong> defines function
    buildRefinement(geometryType, coerceTo), which returns the right
    refinement for a runtime-determined GeometryType.  It is also
    located in refinementvirtual.cc

   @section Implementation
   <!--================-->

   The interface is defined by the template class VirtualRefinement.
   It simply defines the CoordVectors and IndexVectors appropriate for
   this dimension and CoordType, defines which iterators to use, and
   provides some proxy or pure virtual functions.

   For each class Refinement<geometryType, CoordType, coercTo, dim> we
   provide a class VirtualRefinementImp<geometryType, CoordType,
   coercTo, dim>, which wraps the matching class
   Refinement<geometryType, CoordType, coercTo, dim> and derives from
   the matching base class VirtualRefinement<dimension, CoordType>.
   Each VirtualRefinementImp is a singleton and has a static instance()
   method which will return this instance as a reference to the base
   class VirtualRefinement.  All this is done in a single template
   class.

   @subsection Iterators The iterators
   <!-------------------------------->

   We can't do the same thing with the iterators as we do with class
   VirtualRefinement.  Since they are polymorph we cannot simply pass
   them into user code.  They are not singletons, so we also cannot
   pass references to them.  Passing pointers to iterators would work,
   but then the programmer has to remember to explecitely delete them.
   Also, it is uncommon for iterators to be handled by their pointers.

   What we do instead is having a wrapper class which conforms to the
   iterator interface and is the same for all
   VirtualRefinementIterators of a given dimension.  This class
   contains a pointer to a polymorph backend object implementing the
   iterator.  The various VirtualRefinementImps then derive from the
   abstract backend class and pass a pointer to a concrete backend
   object when instantiating an iterator.

   @subsection buildRefinement buildRefinement()
   <!------------------------------------------>

   The template function buildRefinement() has to be specialized for
   each dimension.  It makes no sense to test for
   geometryType==GeometryType::prism when dimension==2.  But this
   way we run into a limitation of C++: we can't do partial function
   specialisation.

   The workaround is to create a class RefinementBuilder with a lone
   static method build() and to call that from buildRefinement().
   Since RefinementBuilder is a class and not a function we can do
   partial specialisations.

   It is probably possible to automatically generate the switch
   statements with linked lists of template structs, functions
   implementing the cases, and a recursive template function that will
   iterate over the list, but it is probably not worth the effort, as
   long as buildRefinement() is enough for the job.
 */

#include <vector>
#include "../../common/geometrytype.hh"
#include "../../common/fvector.hh"
#include "refinement.hh"

namespace Dune {

  // //////////////////////////////////////////
  //
  // The virtual base class and its iterators
  //

  //
  // Refinement
  //

  /*! @brief VirtualRefinement base class

     @param dimension The dimension of the element to refine
     @param CoordType The C++ type of the coordinates
   */
  template<int dimension, class CoordType>
  class VirtualRefinement
  {
  public:
    template<int codimension>
    struct Codim;
    //! The VertexIterator of the VirtualRefinement.
    typedef typename Codim<dimension>::SubEntityIterator VertexIterator;
    //! The ElementIterator of the VirtualRefinement
    typedef typename Codim<0>::SubEntityIterator ElementIterator;

    /*! @brief The CoordVector of the VirtualRefinement

       This is always a typedef to a FieldVector
     */
    typedef FieldVector<CoordType, dimension> CoordVector;
    /*! @brief The IndexVector of the VirtualRefinement

       This is always a typedef to a std::vector
     */
    typedef std::vector<int> IndexVector;

    template<int codimension>
    class SubEntityIteratorBack;
    typedef SubEntityIteratorBack<dimension> VertexIteratorBack;
    typedef SubEntityIteratorBack<0> ElementIteratorBack;

    //! Get the number of Vertices
    virtual int nVertices(int level) const = 0;
    //! Get a VertexIterator
    VertexIterator vBegin(int level) const;
    //! Get a VertexIterator
    VertexIterator vEnd(int level) const;

    //! Get the number of Elements
    virtual int nElements(int level) const = 0;
    //! Get an ElementIterator
    ElementIterator eBegin(int level) const;
    //! Get an ElementIterator
    ElementIterator eEnd(int level) const;

    //! Destructor
    virtual ~VirtualRefinement()
    {}

  protected:
    virtual VertexIteratorBack *vBeginBack(int level) const = 0;
    virtual VertexIteratorBack *vEndBack(int level) const = 0;
    virtual ElementIteratorBack *eBeginBack(int level) const = 0;
    virtual ElementIteratorBack *eEndBack(int level) const = 0;
  };

  //! codim database of VirtualRefinement
  template<int dimension, class CoordType>
  template<int codimension>
  struct VirtualRefinement<dimension, CoordType>::Codim
  {
    class SubEntityIterator;
  };

  // ////////////////////////
  //
  // The refinement builder
  //

  template<int dimension, class CoordType>
  VirtualRefinement<dimension, CoordType> &
  buildRefinement(GeometryType geometryType,
                  GeometryType coerceTo);

} // namespace Dune

#include "virtualrefinement.cc"

#endif //DUNE_GRID_COMMON_VIRTUALREFINEMENT_HH
