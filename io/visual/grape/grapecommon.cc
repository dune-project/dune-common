// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __GRAPE_COMMON_CC__
#define __GRAPE_COMMON_CC__

#define MINIMUM(a,b) (((a) > (b)) ? (b) : (a))
#define MAXIMUM(a,b) (((a) < (b)) ? (b) : (a))

#include "grapecommon.hh"

enum { numberOfPartitionTypes = 6 };
static char *partitionNames[numberOfPartitionTypes] = {
  "Interior_Partition",
  "InteriorBorder_Partition",
  "Overlap_Partition",
  "OverlapFront_Partition",
  "All_Partition",
  "Ghost_Partition"
};

enum { numberOfIterators = 3 };
static char *iteratorNames[numberOfIterators] = {
  "LeafIterator",
  "LevelIterator",
  "Macro + HierarchicIterator",
};

static int actualPartitionType = 0;

static BUTTON *button_set_current_data_item(int pnr)
{
  BUTTON *lbutton = 0;

  lbutton = (BUTTON *)START_METHOD(G_INSTANCE);
  assert( lbutton );
  actualPartitionType = pnr;

  END_METHOD(lbutton);
}

/* add Button which can switch between LevelIteration and LeafIteration */
inline void setupLeafButton(MANAGER *mgr, void *sc, int yesTimeScene)
{
  assert(!iteratorButton);
  assert(!maxlevelButton);


  // partition types
  {
    int num = numberOfPartitionTypes;
    CYCLE_LABEL *clabel;

    clabel = (CYCLE_LABEL *) malloc(sizeof(CYCLE_LABEL) * (num + 1));

    for (int i=0; i<num; i++)
    {
      clabel[i].value = i;
      clabel[i].label = partitionNames[i];
    }
    // last of list is 0,and NULL
    clabel[num].value = 0;
    clabel[num].label = NULL;

    GRAPE(Button,"add-method") ("set-current-data-item",
                                button_set_current_data_item);


    partitionTypeButton = (COMBOBUTTON *)GRAPE(ComboButton,"new-instance")
                            ("set-current-data-item",NULL,"",clabel);
    GRAPE(partitionTypeButton, "set-fill-mode") (MENU_FILL_BOTTOM);

    GRAPE(partitionTypeButton,"set-instance") (partitionTypeButton);
    GRAPE(partitionTypeButton,"set-label") (clabel[0].label);
    GRAPE(partitionTypeButton,"set-pref-size") (12.0,1.0);

    GRAPE(mgr,"add-inter") (partitionTypeButton);
  }

  // iterator types
  {
    int num = numberOfIterators;
    CYCLE_LABEL *clabel;

    clabel = (CYCLE_LABEL *) malloc(sizeof(CYCLE_LABEL) * (num + 1));

    for (int i=0; i<num; ++i)
    {
      clabel[i].value = i;
      clabel[i].label = iteratorNames[i];
    }
    // last of list is 0,and NULL
    clabel[num].value = 0;
    clabel[num].label = NULL;

    //GRAPE(Button,"add-method")("set-current-data-item",
    //                           button_set_current_data_item);

    iteratorButton = (COMBOBUTTON *)GRAPE(ComboButton,"new-instance")
                       ("set-current-data-item",NULL,"",clabel);
    GRAPE(iteratorButton, "set-fill-mode") (MENU_FILL_BOTTOM);

    GRAPE(iteratorButton,"set-instance") (iteratorButton);
    GRAPE(iteratorButton,"set-label") (clabel[0].label);
    GRAPE(iteratorButton,"set-pref-size") (12.0,1.0);

    GRAPE(mgr,"add-inter") (iteratorButton);
  }

  maxlevelButton = (BUTTON *)
                   new_item (Button,
                             I_Label, "use only maxlevel",
                             I_Instance, sc,
                             I_Method, "maxlevel-on-off",
                             I_Size, 12., 1.,
                             I_FillMode, MENU_FILL_BOTTOM,
                             I_End);

  GRAPE(mgr,"add-inter") (maxlevelButton);

  GRAPE(maxlevelButton,"set-state") (PRESSED);
  maxlevelButton->on_off = OFF;
}

#if 0
COLORBAR *colorbar_eval_samples_layer(XLIST *xlist)
{
  COLORBAR *self;

  ASSURE( self=(COLORBAR *)START_METHOD(G_INSTANCE),"", END_METHOD(NULL));

  if( xlist->chosen ) {
    if( samples[xlist->chosen_key].func )
      samples[xlist->chosen_key].func(self);
  }

  /* update the display */

  GRAPE(GRAPE(Manager,"get-stdmgr") (),"set-graphic-redraw") (rfOn);
  g_hide_layer((ITEM *)xlist);

  END_METHOD(self);
}
#endif


inline void timeSceneInit(INFO *info, int n_info, int procs, int time_bar)
{
  int n,p;
  int numProcs = (procs <= 1) ? 1 : (procs-1);

  Dune :: __MaxPartition = numProcs;

  printf("Warning: set proc to 1\n");

  for (n = 0; n < MAXIMUM(1, n_info); n++)
  {
    printf("n = %d, make TimeScene \n",n);
    info[n].tsc = (TIMESCENE *)GRAPE(TimeScene,"new-instance") (info[n].name);
    ((TIMESCENE *)info[n].tsc)->sync = 1;
    if (n > 0)
      ((TIMESCENE *) info[n-1].tsc)->next_scene = (SCENE *) info[n].tsc;
  }

  for (n = 0; n < MAXIMUM(1, n_info); n++)
  {
    {
      TIMESCENE * tsc = (TIMESCENE *) info[n].tsc;
      // > 0 because tsc for proc 0 already exists
      for (p = numProcs-1; p > 0; p--)
      {
        TIMESCENE * newSc = NULL;
        printf("add timescene for proc %d \n",p);
        assert(tsc);
        char * newName = (char *) malloc(strlen(info[n].name) + 5 * sizeof(char));
        assert(newName);
        sprintf(newName,"%s_%d",info[n].name,p);
        newSc = (TIMESCENE *)GRAPE(TimeScene,"new-instance") (newName);
        assert(newSc);

        newSc->sync = 1;
        newSc->next_scene = tsc->next_scene;
        tsc->next_scene = (SCENE *) newSc;
      }
    }

  }

  {
    TIMESCENE * newSc = NULL;
    printf("add timescene for proc combined object\n");
    char * newName = (char *) malloc(12 * sizeof(char));
    assert(newName);
    sprintf(newName,"combo obj");
    newSc = (TIMESCENE *)GRAPE(TimeScene,"new-instance") (newName);
    assert(newSc);

    newSc->sync = 1;
    newSc->next_scene = NULL;
    globalTsc = newSc;
  }
  return;
}

/* add scene with combined object at the end of scene tree */
inline SCENE * combine_scenes_send ()
{
  SCENE* sc = (SCENE*) START_METHOD (G_INSTANCE);
  TIMESCENE * newSc = NULL;
  MANAGER * mgr = NULL;
  ALERT( sc, "combine-scenes: No hmesh!", END_METHOD(NULL));

  newSc = (TIMESCENE *) GRAPE(TimeScene,"new-instance") ("combined Scene");
  assert(newSc);

  while(sc)
  {
    if(sc->next_scene)
      sc = sc->next_scene;
    else
      break;
  }

  mgr = (MANAGER *)GRAPE(Manager,"get-stdmgr") ();
  assert(mgr);
  assert(globalTsc);

  sc->next_scene = (SCENE *) globalTsc;
  GRAPE(mgr, "goto-instance") (globalTsc);

  END_METHOD(sc);
}

/* call handle for a bunch of timescenes */
inline void displayTimeScene(INFO * info, int numberOfProcs )
{
  TIMESCENE *tsc = (TIMESCENE *) info[0].tsc;
  if(tsc)
  {
    MANAGER       *mgr;
#ifdef GRID_MODE
    GRAPHICDEVICE *grdev;

    grdev = (GRAPHICDEVICE *)GRAPE(GraphicDevice,"get-stddev") ();
    grdev->clear();
    if (grdev && (grdev->grid_patch != G_GRID))
    {
      GRAPE(grdev,"grid-patch") (G_GRID);
    }
#endif
    GrapeInterface_two_two::grape_add_remove_methods();
    GrapeInterface_three_three::grape_add_remove_methods();
    GrapeInterface_three_three::initPartitionDisp(numberOfProcs-1);

    /* add combine methods to send method of Scene and TimeScene */
    GRAPE(Scene,"add-method") ("combine-secnes-send",combine_scenes_send);
    GRAPE(TimeScene,"add-method") ("combine-secnes-send",combine_scenes_send);

    mgr = (MANAGER *)GRAPE(Manager,"get-stdmgr") ();

    if(!maxlevelButton && !iteratorButton) setupLeafButton(mgr,tsc,1);

    GRAPE(mgr,"handle") (tsc);
  }
}


#undef MINIMUM
#undef MAXIMUM

#endif
