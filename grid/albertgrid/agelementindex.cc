// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __ALBERTGRID_ELMEM_CC__
#define __ALBERTGRID_ELMEM_CC__

namespace AlbertHelp {

  typedef struct elnum_str ELNUM_STR;
  struct elnum_str
  {
    ELNUM_STR * next;
    int * elNumVec;
    int count;
  };

  typedef struct index_manager INDEX_MANAGER;
  struct index_manager
  {
    ELNUM_STR * free;
    ELNUM_STR * actual;
    int el_index;
  };

  static ELNUM_STR * getNewElNumMem();

  INDEX_MANAGER * get_index_manager ()
  {
    INDEX_MANAGER * im = (INDEX_MANAGER *) malloc(sizeof(INDEX_MANAGER));
    assert(im != NULL);

    im->free = NULL;
    im->actual = getNewElNumMem();
    im->el_index = 0;
    return im;
  }

  static INDEX_MANAGER * Albert_global_im = NULL;

  static void initIndexManager_elmem_cc(INDEX_MANAGER * newIm)
  {
    Albert_global_im = newIm;
    assert(Albert_global_im != NULL);
  }

  static void removeIndexManager_elmem_cc()
  {
    Albert_global_im = NULL;
  }

  static const int newMemSize = 1000;

  static ELNUM_STR * getNewElNumMem()
  {
    //printf("Calling getNewElNumMem \n");
    ELNUM_STR * newMem = (ELNUM_STR *) malloc( sizeof(ELNUM_STR) );
    assert(newMem != NULL);
    newMem->elNumVec = (int *) malloc(newMemSize * sizeof(int));
    assert(newMem->elNumVec != NULL);

    newMem->count = 0;
    newMem->next=NULL;
    return newMem;
  }

  /* return the new element index for el->index */
  static int get_elIndex()
  {
    /* the case of reading the mesh from file */
    if(!Albert_global_im) return -1;

    ELNUM_STR * actNum = Albert_global_im->actual;

    if(actNum->count <= 0)
    {
      actNum = actNum->next;
      if(!actNum)
      {
        int ind = Albert_global_im->el_index;
        Albert_global_im->el_index++;
        return ind;
      }
    }

    //printf("get Num from Stack \n");
    actNum->count--;
    assert((actNum->count >=0) && (actNum->count < newMemSize));

    return actNum->elNumVec[actNum->count];
  }

  /* when element is deleted remember the index */
  static void free_elIndex(int ind)
  {
    assert(Albert_global_im != NULL);

    ELNUM_STR * actNum = Albert_global_im->actual;

    if(actNum->count >= newMemSize)
    {
      ELNUM_STR * newNum = getNewElNumMem();
      newNum->next = actNum;
      Albert_global_im->actual = newNum;
      actNum = newNum;
    }

    actNum->elNumVec[actNum->count] = ind;
    actNum->count++;
    return;
  }

} // end namespace AlbertHelp

#endif
