// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __ALBERTGRID_ELMEM_CC__
#define __ALBERTGRID_ELMEM_CC__

namespace AlbertHelp {

  // for vertices not needed now
  // dim = 2 means, we have a vector for element and edge numbering
  enum { numOfElNumVec = DIM };

  // IndexManagerType defined in albertgrid.hh
  static IndexManagerType * tmpIndexStack[numOfElNumVec];

  static void initIndexManager_elmem_cc(IndexManagerType (&newIm)[numOfElNumVec])
  {
    for(int i=0; i<numOfElNumVec; i++)
    {
      tmpIndexStack[i] = &newIm[i];
      assert(tmpIndexStack[i] != 0);
    }
  }

  static void removeIndexManager_elmem_cc()
  {
    for(int i=0; i<numOfElNumVec; i++) tmpIndexStack[i] = 0;
  }

  template <int codim>
  static int getElementIndex()
  {
    assert(tmpIndexStack[codim] != 0);
    return (*tmpIndexStack[codim]).getIndex();
  }

  static int getElementIndexForCodim(int codim)
  {
    assert((codim >= 0) && (codim < DIM+1));
    switch(codim)
    {
    case 0 : return getElementIndex<0> ();
    case 1 : return getElementIndex<1> ();
    case 2 : return getElementIndex<2> ();
    case 3 : return getElementIndex<3> ();
    }
    return -1;
  }

  template <int codim>
  static void freeElementIndex(int idx)
  {
    assert(tmpIndexStack[codim] != 0);
    (*tmpIndexStack[codim]).freeIndex(idx);
  }

  // codim to ALBERTA Dof Type translator
  template <int codim> struct AlbertaDofType { enum { type = VERTEX }; }; // dof located on vertices
  template <> struct AlbertaDofType<0> { enum { type = CENTER }; }; // dofs located inside an element
#if DIM == 2
  template <> struct AlbertaDofType<1> { enum { type = EDGE }; }; // dofs located on edges
#endif
#if DIM == 3
  template <> struct AlbertaDofType<1> { enum { type = FACE }; }; // dofs located on faces
  template <> struct AlbertaDofType<2> { enum { type = EDGE }; }; // dofs located on edges
#endif

  //****************************************************************************
  //
  //  if the grid is refined then the two methods on this class
  //  organize new element numbers for all codima and set them free if an
  //  element is removed
  //
  //****************************************************************************
  template <int codim>
  struct RefineNumbering
  {
    // get element index form stack or new number
    inline static void refineNumbers ( DOF_INT_VEC * drv , RC_LIST_EL *list, int ref)
    {
      const DOF_ADMIN * admin = drv->fe_space->admin;
      const int nv = admin->n0_dof    [AlbertaDofType<codim>::type];
      const int k  = admin->mesh->node[AlbertaDofType<codim>::type];
      int dof;
      int *vec = 0;

      GET_DOF_VEC(vec,drv);

      assert(ref > 0);

      for(int i=0; i<ref; i++)
      {
        EL * el = list[i].el;
        for(int ch=0; ch<2; ch++)
        {
          dof = el->child[ch]->dof[k][nv];

          // get element index from stack or new, see. elmem.cc
          vec[dof] = getElementIndex<codim>();
        }
      }
    }

    // put element index to stack, if element is coarsend
    inline static void coarseNumbers ( DOF_INT_VEC * drv , RC_LIST_EL *list, int ref)
    {
      const DOF_ADMIN * admin = drv->fe_space->admin;
      const int nv = admin->n0_dof    [AlbertaDofType<codim>::type];
      const int k  = admin->mesh->node[AlbertaDofType<codim>::type];
      int dof;
      int *vec = 0;

      GET_DOF_VEC(vec,drv);

      assert(ref > 0);

      for(int i=0; i<ref; i++)
      {
        EL * el = list[i].el;
        for(int ch=0; ch<2; ch++)
        {
          dof = el->child[ch]->dof[k][nv];

          // put element index to stack, see elmem.cc
          freeElementIndex<codim>( vec[dof] );
        }
      }
    }
  };

  // give element numer 1 to first_el
  static void swapElNum (DOF_INT_VEC * elnums, EL * first_el)
  {
    const DOF_ADMIN * admin = elnums->fe_space->admin;
    const int nv = admin->n0_dof    [AlbertaDofType<0>::type];
    const int k  = admin->mesh->node[AlbertaDofType<0>::type];
    int *vec = 0;

    GET_DOF_VEC(vec,elnums);
    assert(vec[first_el->dof[k][nv]] == 0);
    assert(admin->mesh->n_macro_el == 1);

    vec[first_el->dof[k][nv]] = getElementIndex<0>();
    assert(vec[first_el->dof[k][nv]] == 1);
    freeElementIndex<0>( 0 );
  }

  // swap element numbers of first_el and sec_el
  static void swapElNum (DOF_INT_VEC * elnums, EL * first_el, EL * sec_el )
  {
    const DOF_ADMIN * admin = elnums->fe_space->admin;
    const int nv = admin->n0_dof    [AlbertaDofType<0>::type];
    const int k  = admin->mesh->node[AlbertaDofType<0>::type];
    int *vec = 0;
    int swap = -1;

    GET_DOF_VEC(vec,elnums);

    swap = vec[first_el->dof[k][nv]];
    vec[first_el->dof[k][nv]] = vec[sec_el->dof[k][nv]];
    vec[sec_el->dof[k][nv]] = swap;
  }


} // end namespace AlbertHelp

#endif
