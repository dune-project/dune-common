// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $ Id $
#ifndef DUNE_LOCICTRAITS_H
#define DUNE_LOCICTRAITS_H

namespace Dune {
  /** @addtogroup Common
   *
   * @{
   */
  /** \deprecated This class will be removed after version 1.0 */
  template<bool T1, bool T2>
  struct And
  {
    const static bool value=T1 && T2;
  } DUNE_DEPRECATED;

  /** \deprecated This class will be removed after version 1.0 */
  template<bool T1, bool T2>
  struct Or
  {
    const static bool value=T1 || T2;
  } DUNE_DEPRECATED;

  /** @} */
}
#endif
