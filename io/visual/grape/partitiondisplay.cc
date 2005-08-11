// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
/*
 * grape-module "level_disp.c"
 *
 * Description       : shrink-display-method on hmesh3d,
 *                     uses displays every partition in different color
 *
 * Author            : Dirk Trescher, Robert K.
 *
 */
#ifndef __GRAPE_PARTITIONDISPLAY_HH_
#define __GRAPE_PARTITIONDISPLAY_HH_

#include "grapewrapper.hh"
#include <assert.h>

/*  ------------------------------------------------------------------  */

static int hm3_partition_draw_element   (HELEMENT3D*, HM3_GENERAL*, void*, void*);
static int hm3_simpl_projection      (HELEMENT3D*, HM3_GENERAL*, void*, void*);
static int hm3_simpl_test_if_proceed (HELEMENT3D*, HM3_GENERAL*, void*);

/*  ------------------------------------------------------------------  */

static double shrink_size = 0.8;
static double color_intensity = .6;
static int light_on = TRUE;
static int bar_on = TRUE;
static int diag_on = FALSE;

#define NUM_OF_COLORS 32

static VEC3 color[NUM_OF_COLORS];

static BUTTON*  light_ctl = NULL;
static BUTTON*  bar_ctl = NULL;
static BUTTON*  diagnostic_ctl = NULL;

static int last_element;  /*  merkt sich das letzte El in der display-Routine  */
static int last_partition;  /*  true wenn wir auf der letzten partition arbeiten */
static int max_partition = 1;  /* max number of partitions */
static int thread = 0;  /* actual partition number */

static SUPROP_DEV suprop;

/*  ------------------------------------------------------------------  */


inline GENMESHnD* genmesh3d_switch_part_light_model_on_off ()
{
  GENMESHnD*          hmesh = (GENMESHnD*) START_METHOD (G_INSTANCE);
  ALERT (hmesh, "genmesh3d-switch-part-light-model-on-off: No hmesh!", END_METHOD(NULL));

  if( light_on )
  { light_on = FALSE;
    GRAPE(light_ctl,"set-state") (PRESSED);}
  else
  { light_on = TRUE;
    GRAPE(light_ctl,"set-state") (UNPRESSED);}

  END_METHOD (hmesh);
}

inline GENMESHnD* genmesh3d_switch_part_diagnostic_on_off ()
{
  GENMESHnD*   hmesh = (GENMESHnD*) START_METHOD (G_INSTANCE);
  ALERT (hmesh, "genmesh3d-switch-part-diagnostic-on-off: No hmesh!", END_METHOD(NULL));

  if( diag_on )
  { diag_on = FALSE;
    GRAPE(diagnostic_ctl,"set-state") (PRESSED);}
  else
  { diag_on = TRUE;
    GRAPE(diagnostic_ctl,"set-state") (UNPRESSED);}
  END_METHOD (hmesh);
}

inline GENMESHnD* genmesh3d_switch_part_displaybar_on_off ()
{
  GENMESHnD*   hmesh = (GENMESHnD*) START_METHOD (G_INSTANCE);
  ALERT (hmesh, "genmesh3d-switch-part-displaybar-on-off: No hmesh!", END_METHOD(NULL));

  if( bar_on )
  { bar_on = FALSE;
    GRAPE(bar_ctl,"set-state") (PRESSED);}
  else
  { bar_on = TRUE;
    GRAPE(bar_ctl,"set-state") (UNPRESSED);}
  END_METHOD (hmesh);
}

static int * globalNumberOfElements = 0 ;
static double globalTime = 0.0;

//static double minmax[15][2] = {{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},
//                        {0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1}};
//static double ymin=-1.0;
//static double ymax=1.0;
//static int norm_data = 1;
//static int show_sc1_item = 0;
//static int show_sc2_item = 0;
//static int linear_item = 0;
//static int which_part_sc1_trans = -1;
//static int which_part_sc2_trans = -1;

static double maxnoElements = 8000000;
static double maxTime = 200;

static int * globalLastStep = 0 ;

static GRAPHICDEVICE * dev = 0 ;
static double colorbarHeight = 1.8, colorbarWidth = 0.07,
              colorbarBoundingScale = 0.1 ;
static double colorbarX = -0.75, colorbarY = -0.9 , colorbarZ = 0.9 ;
static void (*patch_normal)(const double *),
(*patch_color)(const double *),
(*patch_vertex)(const double *),
(*begin_patch)(), (*end_patch)(),
(*move)(const double *),
(*draw)(const double *),
(*text)(const double*, const char*);

static int initColorBarDone = 0;

static void set_colors (int currProcs);

inline static void get_graphicdevice()
{
  dev = (GRAPHICDEVICE *)GRAPE(GraphicDevice,"get-stddev") ();

  patch_normal = dev->patch_normal;
  patch_color  = dev->patch_color;
  patch_vertex = dev->patch_vertex;
  begin_patch  = dev->begin_patch;
  end_patch    = dev->end_patch;
  move         = dev->move;
  draw         = dev->draw;
  text         = dev->text;
}

static void setThread( int t )
{
  thread = t;
}

inline void initColorBarDisp()
{
  int i ;
  assert (max_partition >= 0) ;
  printf("Init ColorBar for %d partitions!\n", max_partition );
  if(!globalNumberOfElements) globalNumberOfElements = (int *) malloc (sizeof(int) * max_partition) ;
  if(!globalLastStep) globalLastStep = (int *) malloc (sizeof(int) * max_partition) ;
  for (i = 0 ; i < max_partition ; i ++) globalLastStep [i] = -1 ;
  for (i = 0 ; i < max_partition ; i ++) globalNumberOfElements [i] = 0 ;
  set_colors (max_partition) ;
  get_graphicdevice() ;
  initColorBarDone = 1;
}


inline static void colorbar_display(int nProcs)
{
  int i, light_model, off = OFF,totalnoElements;
  VEC3 xyz, xyz_g, back_color = { 0.3, 0.3, 0.3 };
  MATRIX44 matp, matv, matm, mat;

  dev->attribute(G_MODE_GET,G_LIGHT_MODEL,&light_model);
  dev->attribute(G_MODE_SET,G_LIGHT_MODEL,&off);

  /* Ausschalten der Model- und Viewing Matrix */
  dev->transform(G_MODE_GET,G_MATRIX_MODEL,matm);
  dev->transform(G_MODE_GET,G_MATRIX_VIEW,matv);
  dev->transform(G_MODE_GET,G_MATRIX_PROJECT,matp);
  /*dev->attribute(G_MODE_GET, G_FONT, oldfont);
     printf("%s\n",oldfont);*/

  g_matrix44_set_identity (mat);
  dev->transform(G_MODE_SET,G_MATRIX_PROJECT,mat);
  dev->transform(G_MODE_SET,G_MATRIX_VIEW,mat);
  dev->transform(G_MODE_SET,G_MATRIX_MODEL,mat);

  xyz[1] = colorbarX - colorbarWidth * 1.75
           - colorbarBoundingScale * colorbarWidth - 0.000001 ;
  xyz[0] = colorbarY
           - colorbarBoundingScale * colorbarWidth - 0.000001;
  xyz[2] = colorbarZ - 0.00001;
  begin_patch();
  patch_color(back_color);
  patch_vertex(xyz);
  xyz[0] = colorbarY
           + colorbarHeight + colorbarBoundingScale * colorbarWidth + 0.000001;
  patch_vertex(xyz);
  xyz[1] = colorbarX
           + colorbarWidth + colorbarBoundingScale * colorbarWidth + 0.000001;
  patch_vertex(xyz);
  xyz[0] = colorbarY - colorbarBoundingScale * colorbarWidth - 0.000001;
  patch_vertex(xyz);
  end_patch();

  xyz[1] = colorbarX;
  xyz[0] = colorbarY + colorbarHeight + colorbarBoundingScale * colorbarWidth + 0.000001 ;
  xyz[2] = colorbarZ;
  xyz_g[2] = colorbarZ;

  totalnoElements = 0;

  for (i=0; i<nProcs; i++)
  {
    totalnoElements += globalNumberOfElements [i] ;
    if(diag_on)
      printf("On partition %d : no. elements = %d \n",i,globalNumberOfElements [i]);
  }
  if(diag_on)
    printf("Total number of elements = %d \n\n",totalnoElements);

  xyz[0] = colorbarY;
  xyz_g[0] = colorbarY;
  for (i=0; i<nProcs; i++) {
    xyz[1] = colorbarX;
    begin_patch();
    patch_color(color[i%NUM_OF_COLORS]);
    patch_vertex(xyz);
    xyz[1] += colorbarWidth;
    patch_vertex(xyz);
    xyz[0] += colorbarHeight * globalNumberOfElements[i] / totalnoElements;
    patch_vertex(xyz);
    xyz[1] = colorbarX;
    patch_vertex(xyz);
    end_patch();
    /* Gittergr"osse */
    begin_patch();
    patch_color(color[0]);
    xyz_g[1] = colorbarX-colorbarWidth*0.5;
    patch_vertex(xyz_g);
    xyz_g[1] -= colorbarWidth*0.5;
    patch_vertex(xyz_g);
    xyz_g[0] += colorbarHeight*globalNumberOfElements[i] / maxnoElements;
    patch_vertex(xyz_g);
    xyz_g[1] = colorbarX-colorbarWidth*0.5;
    patch_vertex(xyz_g);
    end_patch();
  }

  /**/
  /* Zeit */
  begin_patch();
  patch_color(color[3]);
  xyz[0] = colorbarY;
  xyz[1] = colorbarX-colorbarWidth*1.25;
  patch_vertex(xyz);
  xyz[0] += colorbarHeight*globalTime / maxTime;
  patch_vertex(xyz);
  xyz[1] -= colorbarWidth*0.5;
  patch_vertex(xyz);
  xyz[0] = colorbarY;
  patch_vertex(xyz);
  end_patch();
  /**/

  dev->attribute(G_MODE_SET,G_LIGHT_MODEL,&light_model);
  /* zuruecksetzen der Matrizen */
  dev->transform(G_MODE_SET,G_MATRIX_PROJECT,matp);
  dev->transform(G_MODE_SET,G_MATRIX_VIEW,matv);
  dev->transform(G_MODE_SET,G_MATRIX_MODEL,matm);

  /* set globalNumbers to zero for new display */
  for (i = 0 ; i < max_partition ; i ++) globalNumberOfElements [i] = 0 ;

  return;
}

inline static void hsv_to_rgb (VEC3 rgb, VEC3 hsv)
{
  double h, s, v, f, p, q, t;
  int i;

  h = hsv[0];
  s = hsv[1];
  v = hsv[2];

  if (s == 0.0) {
    rgb[0] = rgb[1] = rgb[2] = v;
  }
  else {
    if (h < 0.)
      h += 360.;
    if (h >= 360.)
      h -= 360.;
    h /= 60.;
    i = (int) (g_floor(h) + 0.1);
    f = h-i;
    p = v*(1.-s);
    q = v*(1.-(s*f));
    t = v*(1.-(s*(1.-f)));
    switch (i) {
    case 0 :
      g_vec3_set (rgb, v,t,p);
      break;
    case 1 :
      g_vec3_set (rgb, q,v,p);
      break;
    case 2 :
      g_vec3_set (rgb, p,v,t);
      break;
    case 3 :
      g_vec3_set (rgb, p,q,v);
      break;
    case 4 :
      g_vec3_set (rgb, t,p,v);
      break;
    case 5 :
      g_vec3_set (rgb, v,p,q);
      break;
    default :
      g_vec3_set (rgb, 1.0, 1.0, 1.0);
      break;
    }
  }
}

/* set partition colors */
inline static void set_colors (int currProcs)
{
  /*
     for( i=0; i<max_partition+1; ++i )
     {
     color[i][0] = color_intensity * G_MAX( G_MIN((-3.*i)/max_partition + 2., 1.), 0. );
     color[i][1] = color_intensity * G_MIN( G_MIN(( 3.*i)/max_partition, 1.), (-6.*i)/max_partition + 6. );
     color[i][2] = color_intensity * G_MIN( G_MAX(( 6.*i)/max_partition - 4., 0.), 1. );
     }
   */

  if (currProcs == 1) {
    color [0][0] = color [0][1] = color [0][2] = 1.;
  } else {
    int i, size = G_MIN(NUM_OF_COLORS,currProcs), mod = 1;
    double frac = 1.;
    VEC3 hsv, diff = {0., .3, .5};

    if (size >= 16)
      mod = 3;
    else if (size > 12) {
      mod = 2;
      diff[1] = 0.4;
    }
    if (size > 0)
      frac = 300./(double)(size - 1 - (size-1)%mod);
    hsv[1] = hsv[2] = 1.;
    for (i=0; i<size; i++) {
      hsv[0] = (i - i%mod)*frac;
      if (size >= 16)
        hsv[2] = 1.-diff[i%3];
      else if (size > 7)
        hsv[2] = 1.-diff[i%2];

      hsv_to_rgb (color[i], hsv) ;
    }
  }
  return ;
}

/* forward declaration */
GENMESHnD* genmesh3d_partition_disp ();

static int initialized = 0;
/* init max_partition and declare partition methods */
inline void initPartitionDisp (int mp)
{
  if(!initialized)
  {
    max_partition = mp;

    printf("Add Method 'switch-part-light-model-on-off' on GenMesh3d!\n");
    GRAPE(GenMesh3d,"add-method") ("switch-part-light-model-on-off",genmesh3d_switch_part_light_model_on_off);

    printf("Add Method 'switch-part-displaybar-on-off' on GenMesh3d!\n");
    GRAPE(GenMesh3d,"add-method") ("switch-part-displaybar-on-off",genmesh3d_switch_part_displaybar_on_off);
    GRAPE(GenMesh3d,"add-method") ("switch-part-diagnostic-on-off",genmesh3d_switch_part_diagnostic_on_off);
    printf("Add Method 'partition-disp' on GenMesh3d!\n");
    GRAPE(GenMesh3d,"add-method") ("partition-disp",genmesh3d_partition_disp);
    printf("\n");
    initialized = 1;
  }
}

inline GENMESHnD* genmesh3d_partition_disp ()
{
  MANAGER*             mgr;
  GRAPHICDEVICE*       dev;
  GENMESHnD*           hmesh;
  F_HDATA3D*           fhdata;

  HM3_GENERAL*         general;
  HM3_COORD_DATA**     hm3_partition_data;

  int i, max;

  VEC3 save_color;
  double transparency;

  if(!initColorBarDone) initColorBarDisp();

  static GROUP*     group                     = NULL;
  static FUNCTION_SLIDER* shrink_size_ctl     = NULL;
  static FUNCTION_SLIDER* color_intensity_ctl = NULL;
  /* get hmesh */

  hmesh = (GENMESHnD*) START_METHOD (G_INSTANCE);
  ALERT (hmesh, "genmesh3d-partition: No hmesh!", END_METHOD(NULL));

  fhdata = (F_HDATA3D*)GRAPE(hmesh,"get-function")
             ("scalar","scalar","vector","default", NULL);

  set_colors( max_partition );

  /* GRAPE preliminaries */
  mgr = (MANAGER *)      GRAPE (Manager,       "get-stdmgr") ();
  dev = (GRAPHICDEVICE*) GRAPE (GraphicDevice, "get-stddev") ();


  /* get interactives */
  if (!shrink_size_ctl) {
    shrink_size_ctl = (FUNCTION_SLIDER*) new_item (Function_Slider,
                                                   I_Label,   "shrink-factor",
                                                   I_Var,     &shrink_size, dfDouble,
                                                   I_MinMax,  0.01, 1.0,
                                                   I_Scale,   1.0,
                                                   I_Offset,  0.0,
                                                   I_RSizeX,  1.0, I_SizeY, 1.0,
                                                   I_End);
  }
  if (!color_intensity_ctl) {
    color_intensity_ctl = (FUNCTION_SLIDER*) new_item (Function_Slider,
                                                       I_Label,   "color intensity",
                                                       I_Var,     &color_intensity, dfDouble,
                                                       I_MinMax,  0.0, 1.0,
                                                       I_Scale,   1.0,
                                                       I_Offset,  0.0,
                                                       I_RSizeX,  1.0, I_SizeY, 1.0,
                                                       I_End);
  }
  if (!light_ctl) {
    light_ctl = (BUTTON*) new_item (Button,
                                    I_Label,   "light model",
                                    I_State,   PRESSED,
                                    I_Instance, hmesh,
                                    I_Method,  "switch-part-light-model-on-off",
                                    I_RSizeX,  1.0, I_SizeY, 1.0,
                                    I_End);
  }

  if (!bar_ctl) {
    bar_ctl = (BUTTON*) new_item (Button,
                                  I_Label,   "display partition bar",
                                  I_State,   PRESSED,
                                  I_Instance, hmesh,
                                  I_Method,  "switch-part-displaybar-on-off",
                                  I_RSizeX,  1.0, I_SizeY, 1.0,
                                  I_End);
  }

  if (!diagnostic_ctl) {
    diagnostic_ctl = (BUTTON*) new_item (Button,
                                         I_Label,   "diagnostics ",
                                         I_State,   UNPRESSED,
                                         I_Instance, hmesh,
                                         I_Method,  "switch-part-diagnostic-on-off",
                                         I_RSizeX,  1.0, I_SizeY, 1.0,
                                         I_End);
  }

  if (!group) {
    group = (GROUP*) new_item (Group,
                               I_Border,  bfBorder|bfTitle,
                               I_Name,    "genmesh3d-partition",
                               I_Size,    12.0, 1.1+5.*1.07,
                               I_Item,    light_ctl,
                               I_Item,    bar_ctl,
                               I_Item,    diagnostic_ctl,
                               I_Item,    color_intensity_ctl,
                               I_Item,    shrink_size_ctl,
                               I_End);
  }

  if (GRAPE (mgr,"new-handle") (genmesh3d_partition_disp,1)) {
    GRAPE (mgr,"add-inter") (group);
  }

  last_element = -1;
  last_partition = 0;

  GRAPE(hmesh,"get-partition-number") (&thread);
  if(thread == max_partition-1) last_partition = 1;

  /*  Surface-Properties holen  */
  dev->attribute (G_MODE_GET, G_PATCH_SUPROP, &suprop);

  /* prepare mesh-traverse */
  max = hmesh->level_of_interest + 1;

  general = (HM3_GENERAL*) GRAPE (hmesh, "general-alloc") (TRUE);
  general->f_data3d = fhdata;

  G_MEM_ALLOC (hm3_partition_data, max);
  for (i=0; i<max; i++) { G_MEM_ALLOC(hm3_partition_data[i],1); }

  save_color[0] = suprop.emission[0];
  save_color[1] = suprop.emission[1];
  save_color[2] = suprop.emission[2];
  transparency = suprop.transparency;

  /* traverse the mesh */
#if GRAPE_DIM == 3
  g_traverse_mesh3d (hmesh,
                     general,
                     G_LEAVES,
                     hmesh->level_of_interest,
                     hm3_simpl_projection,
                     hm3_simpl_test_if_proceed,
                     hm3_partition_draw_element,
                     (void**) hm3_partition_data,
                     NULL,
                     hefAll);
#endif

  for(i=0; i<max; i++) { G_MEM_FREE(hm3_partition_data[i],1); }

  suprop.emission[0] = save_color[0];
  suprop.emission[1] = save_color[1];
  suprop.emission[2] = save_color[2];
  suprop.transparency = transparency;
  dev->attribute (G_MODE_SET, G_PATCH_SUPROP, &suprop);

  if(last_partition && bar_on )
    colorbar_display(max_partition);
  else if (last_partition)
    for(i=0; i<max_partition; i++) globalNumberOfElements [i] = 0;

  G_MEM_FREE (hm3_partition_data,max);
  END_METHOD (hmesh);
}

inline static int hm3_partition_draw_element (HELEMENT3D* helement,
                                              HM3_GENERAL* general, void* el_data, void* action_arg)
{
  GRAPHICDEVICE*          dev   = general->dev;
  HELEMENT3D_DESCRIPTION* descr = (HELEMENT3D_DESCRIPTION*) helement->descr;
  HM3_COORD_DATA*  element_data = (HM3_COORD_DATA*)         el_data;

  VEC3* vertex_coord = element_data->vertex_coord;
  VEC3 shrinked_coord [MESH3D_MAX_VERTEX];
  VEC3 normal;
  VEC3 center;

  int h, i;

  /* count elements of partition */
  globalNumberOfElements [thread]++;

  g_vec3_set_zero (center);

  for (i=0; i<descr->number_of_vertices; i++ ) {
    center[0] += vertex_coord[i][0];
    center[1] += vertex_coord[i][1];
    center[2] += vertex_coord[i][2];
  }

  center[0] /= descr->number_of_vertices;
  center[1] /= descr->number_of_vertices;
  center[2] /= descr->number_of_vertices;

  /* draw each face of the element seperately */
  for (h=0; h<descr->number_of_polygons; h++) {

    /* calculate shrinked coordinates */
    for (i=0; i<descr->polygon_length[h]; i++) {
      shrinked_coord[i][0] = (1.0 - shrink_size) * center[0]
                             + shrink_size * vertex_coord [descr->polygon_vertex[h][i]] [0];
      shrinked_coord[i][1] = (1.0 - shrink_size) * center[1]
                             + shrink_size * vertex_coord [descr->polygon_vertex[h][i]] [1];
      shrinked_coord[i][2] = (1.0 - shrink_size) * center[2]
                             + shrink_size * vertex_coord [descr->polygon_vertex[h][i]] [2];
    }

    /* draw one face as grip or patch */
    if (dev->grid_patch == G_GRID) {
      VEC3 linecolor;

      dev->attribute (G_MODE_GET, G_LINE_COLOR, linecolor);
      dev->attribute (G_MODE_SET, G_LINE_COLOR, color[thread]);

      dev->move (shrinked_coord[descr->polygon_length[h]-1]);

      for (i=0; i<descr->polygon_length[h]; i++) {
        dev->draw (shrinked_coord[i]);
      }
      dev->attribute (G_MODE_SET, G_LINE_COLOR, linecolor);
    }
    else {
      int light_model, off = 0;

      g_vec3_get_normal_to_plane (normal,
                                  shrinked_coord[0], shrinked_coord[1], shrinked_coord[2]);

      if( light_on )
      { if( thread != last_element )
        {
          suprop.emission[0] = color[thread][0];
          suprop.emission[1] = color[thread][1];
          suprop.emission[2] = color[thread][2];
          suprop.transparency = 1.;
          dev->attribute (G_MODE_SET, G_PATCH_SUPROP, &suprop);

          last_element = thread;
        }}
      else
      { dev->attribute (G_MODE_GET, G_LIGHT_MODEL, &light_model);
        dev->attribute (G_MODE_SET, G_LIGHT_MODEL, &off);}

      dev->begin_patch  ();
      if( !light_on )
        dev->patch_color (color[thread]);
      dev->patch_normal (normal);

      for (i=0; i<descr->polygon_length[h]; i++) {
        dev->patch_vertex (shrinked_coord[i]);
      }

      dev->end_patch ();

      if( !light_on )
        dev->attribute (G_MODE_SET, G_LIGHT_MODEL, &light_model);
    }
  }
  return TRUE;
}


inline int hm3_simpl_projection (HELEMENT3D*  helement,
                                 HM3_GENERAL* general,
                                 void*        el_data,
                                 void*        pa_data)
{
  HM3_COORD_DATA* element_data = (HM3_COORD_DATA*)el_data;
  HM3_COORD_DATA* parent_data  = (HM3_COORD_DATA*)pa_data;

  VEC3* vertex_coord = element_data->vertex_coord;

  MAKE_SIMPL_PROJECT (3, helement,
                      parent_data,
                      {
                        g_vec3_assign (vertex_coord[local_node_number],
                                       helement->vertex[local_node_number]);
                      },
                      {
                        g_vec3_hm_interpol (helement,
                                            vertex_coord[local_node_number],
                                            parent_data->vertex_coord,
                                            local_node_number);
                      },
                      {
                        g_vec3_assign (vertex_coord[local_node_number],
                                       parent_data->vertex_coord[helement->
                                                                 vinh[local_node_number].pindex[0]]);
                      },
                      TRUE, return FALSE);
  return TRUE;
}

inline int hm3_simpl_test_if_proceed  (HELEMENT3D*  helement,
                                       HM3_GENERAL* general,
                                       void*        el_data)
{
  int result;

  HM_TEST_IF_PROCEED(3,result, helement, general);

  return result;
}

#endif
