// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __ALBERTGRID_ELMEM_CC__
#define __ALBERTGRID_ELMEM_CC__

namespace AlbertHelp {

  // IndexManagerType defined in albertgrid.hh
  static IndexManagerType * tmpIndexStack = 0;

  static void initIndexManager_elmem_cc(IndexManagerType * newIm)
  {
    tmpIndexStack = newIm;
    assert(tmpIndexStack != 0);
  }

  static void removeIndexManager_elmem_cc()
  {
    tmpIndexStack = 0;
  }

  /* return the new element index for el->index */
  static int get_elIndex()
  {
    assert(tmpIndexStack != 0);
    return tmpIndexStack->getIndex();
  }

  /* when element is deleted remember the index */
  static void free_elIndex(int ind)
  {
    assert(tmpIndexStack != 0);
    tmpIndexStack->freeIndex(ind);
  }

} // end namespace AlbertHelp

#endif
