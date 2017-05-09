// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_INTERFACES_HH
#define DUNE_INTERFACES_HH

/** @file
   @author Robert Kloefkorn
   @brief Provides interfaces for detection of specific behavior
 */

namespace Dune {

  //! An interface class for dcloneable objects
  struct Cloneable {

    /** \brief Clones the object
     * clone needs to be redefined by an implementation class, with the
     * return type covariantly adapted. Remember to
     * delete the resulting pointer.
     */
    virtual Cloneable* clone() const = 0;

    /** \brief Destructor */
    virtual ~Cloneable()
    {}

  };

} // end namespace Dune
#endif
