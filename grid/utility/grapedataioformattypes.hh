// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRAPEDATAIOFORMATTYPE_HH
#define DUNE_GRAPEDATAIOFORMATTYPE_HH

//- system includes
#include <string>

namespace Dune {

  /*!
     Specify the format to store grid and vector data
   */
  enum GrapeIOFileFormatType
  { ascii = 0 ,     //!< store data in a human readable form
    xdr = 1   ,     //!< store data in SUN's library routines
                    //!< for external data representation (xdr)
    pgm = 2 };      //!< store data in portable graymap file format


  typedef std::string GrapeIOStringType;

  /** \brief convert type to string
   */
  template <typename T>
  inline GrapeIOStringType typeIdentifier ()
  {
    GrapeIOStringType tmp = "unknown";
    return tmp;
  }

  template <>
  inline GrapeIOStringType typeIdentifier<float> ()
  {
    GrapeIOStringType tmp = "float";
    return tmp;
  }

  template <>
  inline GrapeIOStringType typeIdentifier<int> ()
  {
    GrapeIOStringType tmp = "int";
    return tmp;
  }

  template <>
  inline GrapeIOStringType typeIdentifier<double> ()
  {
    GrapeIOStringType tmp = "double";
    return tmp;
  }

} // end namespace Dune

#endif
