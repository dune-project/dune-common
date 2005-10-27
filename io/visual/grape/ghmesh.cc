// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __GRAPE_HMESH_C__
#define __GRAPE_HMESH_C__

#include <string.h>

#include "ghmesh.hh"
#include "geldesc.hh"

/*****************************************************************************
* Globale defines                  *                     **
*****************************************************************************/

static const double EPSILON  = 1.0e-2;
#define STA(el) ((STACKENTRY *) (el))
static const double INFTY = 999999.;
#define FVAL_ON_NODE(b) (jump_data[b])

#define MINIMUM(a,b) (((a) > (b)) ? (b) : (a))
#define MAXIMUM(a,b) (((a) < (b)) ? (b) : (a))


GENMESHnD *genmesh3d_switch_part_light_model_on_off();
GENMESHnD *genmesh3d_switch_part_displaybar_on_off();
GENMESHnD *genmesh3d_partition_disp();

/*****************************************************************************
* Verwendete Strukturen                *                     **
*****************************************************************************/

/***************************/
typedef struct stackentry
{
  HELEMENT hel;

  struct stackentry *next;
  int ref_flag ;

  double hmax;

} STACKENTRY;


/**********************************************************************
* storage of the dune data ( discrete functions )
*
* normaly stored as function_data in the F_DATA pointer
**********************************************************************/
typedef struct dune_func
{
  const char * name;
  /* the function to evaluate */
  void (* func_real)(DUNE_ELEM *he, DUNE_FDATA *fe, int ind,
                     double G_CONST *coord, double *val);
  /* struct storing the pointer to the disrete function */
  DUNE_FDATA * all;
} DUNE_FUNC;

/* definition of dune_dat in g_eldesc.h */
/* stored as user_data in the mesh pointer */


/*****************************************************************************
* Statische Variablen                *                     **
*****************************************************************************/

static STACKENTRY *stackfree = NULL;

/**************************************************************************
*
* froward declarations
*
* ************************************************************************/

/* switch from leaf to level iteration for mesh */
int switchMethods( GENMESHnD *actHmesh);

/* add Button which can switch between LevelIteration and LeafIteration */
//void setupLeafButton(MANAGER *mgr, void *sc, int yesTimeScene);

/***************************************************************************/
inline static void swapQuadrilateral( double ** vertex, double (* vp)[3])
{
  vertex[2] = vp[3];
  vertex[3] = vp[2];
}

#if GRAPE_DIM == 3
inline static void swapHexahedron( double ** vertex, DUNE_ELEM * el)
{
  swapQuadrilateral(vertex,el->vpointer);

  vertex[6] = el->vpointer[7];
  vertex[7] = el->vpointer[6];
}
#endif

/*****************************************************************************
******************************************************************************
**                      **
**  Memory Management Routinen              **
**                      **
******************************************************************************
*****************************************************************************/
inline static DUNE_ELEM * getNewDuneElem ()
{
  DUNE_ELEM * elem = (DUNE_ELEM *) malloc(sizeof(DUNE_ELEM));
  assert(elem);

  elem->type = gr_unknown;

  for(int i=0 ; i<MAX_EL_FACE; i++) elem->bnd[i] = -1;
  elem->eindex = -1;
  elem->level = -1;
  elem->level_of_interest = -1;;
  elem->has_children = 0;;

  elem->mesh = 0;
  elem->isLeafIterator = 1;;

  for(int i=0; i<MAX_EL_DOF; i++)
  {
    elem->vindex [i] = -i;
    for(int j=0; j<3; j++)
    {
      elem->vpointer[i][j] = 0.0;
    }
  }
  elem->display = NULL;
  elem->liter = NULL;
  elem->hiter = NULL;
  elem->actElement = NULL;
  return elem;
}

inline static HELEMENT *get_stackentry()
{
  STACKENTRY *stel;
  DUNE_ELEM * elem = 0;

  if ( stackfree)
  {
    stel = stackfree;
    stackfree = stackfree->next;
  }
  else
  {
    stel = (STACKENTRY *)malloc(sizeof(STACKENTRY));
    elem = getNewDuneElem ();
    assert( elem );
    ((HELEMENT *)stel)->user_data = (void *)elem;
  }
  return( (HELEMENT *)stel);
}


inline static void free_stackentry(HELEMENT *stel)
{
  ((STACKENTRY *)stel)->next = stackfree ;
  stackfree = (STACKENTRY*)stel;
}


inline static void gFreeElement(ELEMENT *el)
{
  if (el)
  {
    free_stackentry((HELEMENT *)el);
  }
  return ;
}

/*****************************************************************************
* little help routines
*****************************************************************************/
inline static double dist(const double *x,  const double *y)
{
  double dist=0.0;
  int i;

  for(i=0; i<GRAPE_DIMWORLD; i++)
  {
    dist += (x[i]-y[i])*(x[i]-y[i]);
  }
  return (sqrt(dist));
}

inline static double calc_hmax(HELEMENT *el)
{
  return ( dist(el->vertex[0], el->vertex[1]) );
}

/*****************************************************************************
******************************************************************************
**
**  The functions to which each HMesh has a pointer, i.e. first_macro
**  here first_macro, next_macro, first_child, next_child
**
******************************************************************************
*****************************************************************************/
inline static void helementUpdate( DUNE_ELEM *elem, HELEMENT *grapeEl )
{
  grapeEl->vindex       = elem->vindex ;
  grapeEl->eindex       = elem->eindex ;
  grapeEl->level        = elem->level;
  grapeEl->has_children = elem->has_children;
  grapeEl->user_data    = (void *)elem ;
}

inline static HELEMENT * first_macro (GENMESHnD *mesh, MESH_ELEMENT_FLAGS flag)
{
  int i ;
  HELEMENT * el = get_stackentry();
  static double * vertex[MAX_EL_DOF] = { 0,0,0,0,0,0,0,0 };
  DUNE_ELEM * elem = (DUNE_ELEM *) el->user_data;

  assert(mesh);
  assert(el);
  assert(elem);

  /* store level of interest for LeafIterator */
  if(maxlevelButton->on_off == OFF) /* dont know why wrong, but it works */
    elem->level_of_interest = -1;
  else
    elem->level_of_interest = mesh->level_of_interest;

  elem->isLeafIterator = switchMethods(mesh);

  el->present = hefAll;
  el->parent = NULL;

  elem->display = ((struct dune_dat *)mesh->user_data)->all->display;
  elem->hiter = NULL;

  {
    /* call first macro and check for first element */
    int test = (*(((struct dune_dat *)(mesh->user_data))->first_macro))(elem) ;
    // means no element exits at all
    if(!test) return NULL;
  }

  el->level = 0;
  el->mesh  = (GENMESHnD *)mesh ;

  for(i = 0 ; i < MAX_EL_DOF; i++)
  {
    vertex[i] = (double *)elem->vpointer[i];
  }

  el->vertex = (double G_CONST*G_CONST*)vertex;

  helementUpdate(elem,el);
  ((STACKENTRY *)el)->hmax = calc_hmax(el);

  // reset element description
  el->descr = 0;

  /********************************************************/
  /*   dim == 3 */
#if GRAPE_DIM == 2
  if (elem->type == gr_triangle)
  {
    el->descr           = (ELEMENT_DESCRIPTION *)&triangle_description ;
    el->vinh    = NULL ;
    ((STACKENTRY *)el)->ref_flag   = -1;
  }
  else if (elem->type == gr_quadrilateral)
  {
    swapQuadrilateral( vertex, elem->vpointer);

    el->descr           = (ELEMENT_DESCRIPTION *)&quadrilateral_description ;
    el->vinh    = NULL ;
    ((STACKENTRY *)el)->ref_flag   = -1;
  }
#endif

  /********************************************************/
  /*   dim == 3 */
#if GRAPE_DIM==3
  if (elem->type == gr_tetrahedron)
  {
    el->descr           = (ELEMENT_DESCRIPTION *) &tetra_description ;
    el->vinh    = NULL ;
    ((STACKENTRY *)el)->ref_flag   = -1;
  }
  else if (elem->type == gr_hexahedron)
  {
    swapHexahedron(vertex,elem);

    el->descr   = (ELEMENT_DESCRIPTION *)&cube_description ;
    el->vinh    = NULL ;
    ((STACKENTRY *)el)->ref_flag   = -1;
  }
#endif
  /***************************************************************/
  // is this assertion is thrown then something with the geometry types is
  // wrong
  assert( el->descr != 0 );

  return(el);
}

/* go next macro element */
inline static HELEMENT * next_macro(HELEMENT * el, MESH_ELEMENT_FLAGS flag)
{
  int mflag=0;
  assert(el) ;

  el->present = (MESH_ELEMENT_FLAGS) (hefAll & ! hefVinh);
  mflag = (*(((struct dune_dat *)(el->mesh->user_data))->next_macro))((DUNE_ELEM *)el->user_data);
  if(mflag)
  {
    helementUpdate(((DUNE_ELEM *)el->user_data),el);
    ((STACKENTRY *)el)->hmax = calc_hmax(el);

    return(el) ;
  }
  else
  { /*printf("next macro: bin draussen flag = %i \n",mflag);*/
    gFreeElement((ELEMENT *)el) ; return NULL ;
  }
}

/***********************************************************/
/* first_child, go to first child of current element */
/************************************************************/
inline static HELEMENT * first_child (HELEMENT * ael, MESH_ELEMENT_FLAGS flag)
{
  HELEMENT * el;
  DUNE_ELEM * elem;
  static double  * vertex [MAX_EL_DOF] = {0,0,0,0,0,0,0,0};
  int i , actlevel = ael->level;

  if ( actlevel < ((HMESH *)ael->mesh)->level_of_interest )
  {
    el = get_stackentry();
    assert(el);

    elem = (DUNE_ELEM *)el->user_data;
    assert(elem);

    el->present = (MESH_ELEMENT_FLAGS) (hefAll & !hefVinh);

    elem->display = ((DUNE_ELEM *)ael->user_data)->display;
    elem->liter = ((DUNE_ELEM *)ael->user_data)->liter;
    elem->hiter = ((DUNE_ELEM *)ael->user_data)->hiter;

    /* call the dune method */
    if((*(((struct dune_dat *)(ael->mesh->user_data))->first_child))(elem))
    {
      el->level =  actlevel+1;
      el->mesh              = ael->mesh ;

      for(i = 0 ; i < MAX_EL_DOF; i++)
        vertex[i] = (double *)elem->vpointer[i];

      el->vertex = (double G_CONST*G_CONST*)vertex;

      helementUpdate(elem,el);
      el->parent    = ael;
      ((STACKENTRY *)el)->hmax = ((STACKENTRY *)ael)->hmax *0.5;

      el->descr = 0;

#if GRAPE_DIM == 2
      /*************************************************/
      /* dim == 2 */
      if (elem->type == gr_triangle)
      {
        el->descr = (ELEMENT_DESCRIPTION *)&triangle_description ;
        el->vinh  = NULL ;
        ((STACKENTRY *)el)->ref_flag   = -1;
      }
      else if (elem->type == gr_quadrilateral)
      {
        /* change point 2 and 3 because grape is different compared to dune */
        swapQuadrilateral( vertex, elem->vpointer);

        el->descr = (ELEMENT_DESCRIPTION *)&quadrilateral_description ;
        el->vinh    = NULL ;
        ((STACKENTRY *)el)->ref_flag   = -1;
      }
#endif
      /*************************************************/
      /* dim == 3 */
#if GRAPE_DIM == 3
      if (elem->type == gr_tetrahedron)
      {
        el->descr   = (ELEMENT_DESCRIPTION *)&tetra_description ;
        el->vinh    = NULL ;
        ((STACKENTRY *)el)->ref_flag   = -1;
      }
      else if (elem->type == gr_hexahedron)
      {
        swapHexahedron(vertex,elem);

        el->descr   = (ELEMENT_DESCRIPTION *)&cube_description ;
        el->vinh    = NULL ;
        ((STACKENTRY *)el)->ref_flag   = -1;
      }
#endif
      /****************************************************/
      // is this assertion is thrown then something with the geometry types is
      // wrong
      assert( el->descr != 0 );
      return(el);
    }
    else
    {
      gFreeElement((ELEMENT *)el) ; return NULL ;
    }
  }
  else { return NULL ; }
}

/* go to next child of the current element */
inline static HELEMENT * next_child(HELEMENT * el, MESH_ELEMENT_FLAGS flag)
{
  assert(el) ;
  el->present = (MESH_ELEMENT_FLAGS) (hefAll & !hefVinh);

  if((*(((struct dune_dat *)(el->mesh->user_data))->next_child))((DUNE_ELEM *)el->user_data)) {
    ((STACKENTRY *)el)->ref_flag++;
    helementUpdate(((DUNE_ELEM *)el->user_data),el);

    if  (((DUNE_ELEM *)el->user_data)->type == 3)
    {
      el->vinh = NULL;
    }

    return(el) ;
  }

  else { gFreeElement((ELEMENT *)el) ; return NULL ; }

}

/* fake that no children exixst */
inline static HELEMENT * fake_child (HELEMENT * ael, MESH_ELEMENT_FLAGS flag)
{
  return NULL;
}

/* first_child, go to first child of current element */
inline static HELEMENT * select_child (HELEMENT * ael, double *parent_coord,
                                       double *child_coord, MESH_ELEMENT_FLAGS flag)
{
  HELEMENT *child = NULL;
  HMESH *mesh = (HMESH *) ael->mesh;
  const ELEMENT_DESCRIPTION *descr = ael->descr;
  double coord [GRAPE_DIMWORLD];
  int inside = 0;

  /* in child_ccord the local coordinates of the point in child can be
   * stored */
  descr->coord_to_world(ael,parent_coord,(double *) &coord);

  if( mesh->first_child )
    child = mesh->first_child(ael, flag);
  if( !child ) return NULL;

  /* tranform to local coords of child */
  descr->world_to_coord(child, (double *) &coord, child_coord);

  while ( inside != -1 )
  {
    inside = descr->check_inside( child , child_coord );
    if( inside == -1)
    {
      return child;
    }

    child = mesh->next_child( child , flag);
    if ( ! child ) return NULL;
    descr->world_to_coord(child, (double *) &coord, child_coord);
  }
  return NULL;
}

inline static ELEMENT * first_element (GRAPEMESH *mesh, MESH_ELEMENT_FLAGS flag)
{
  return first_macro((GENMESHnD * )mesh,flag);
}

/* go next macro element */
inline static ELEMENT * next_element(ELEMENT * el, MESH_ELEMENT_FLAGS flag)
{
  return next_macro(el,flag);
}
/***************************************************************************
*
*  f_data function
*
***************************************************************************/
inline void f_bounds(HELEMENT *el, double* min, double* max,
                     void *function_data)
{
  (*min) =  1.0E+308;
  (*max) = -1.0E+308;
  return;
}
/****************************************************************************/
inline void grape_get_vertex_estimate(HELEMENT *el, double *value,
                                      void *function_data)
{
  *value = 1.0E+308;
  return;
}

/****************************************************************************/

inline double grape_get_element_estimate(HELEMENT *el, void *function_data)
{
  return 1.0E+308;
}

/***************************************************************************/
inline void f_real_el_info(HELEMENT *el, F_EL_INFO *f_el_info,
                           void *function_data)
{
  f_el_info->polynomial_degree = ((DUNE_FDATA *) function_data)->polyOrd;
  return;
}

/***************************************************************************/

/* print DUNE_FUNC STRUCT */
inline void printfFdata(DUNE_FUNC *df)
{
  DUNE_FDATA *fem = df->all;
  printf("Dune Func %p | Dune Fdata %p \n",df,fem);
  printf("comp %d      | DiscFunc   %p \n",fem->comp[0],fem->discFunc);
  printf("-------------------------------------------\n");
}
inline void printDuneFunc(DUNE_FDATA *df)
{
  printf("DUNE_FDATA %p \n",df);
  printf("discFunc %p \n",df->discFunc);
  //printf("lf       %p \n",df->lf);
  printf("comp     %d \n",df->comp[0]);
}


inline void f_real(HELEMENT *el, int ind, double G_CONST *coord,
                   double *val, void *function_data)
{
  assert(el);
  DUNE_ELEM * elem = (DUNE_ELEM *)el->user_data;
  DUNE_FUNC * df = (DUNE_FUNC *) function_data;
  DUNE_FDATA *fem = df->all;

  /*
     if((!fem->allLevels) && levelButton->on_off == OFF)
     printf("Warning: data only on leaf level! \n");
   */

  assert(elem != NULL);
  assert(fem != NULL);
  assert(fem->discFunc != NULL);

  df->func_real(elem,fem,ind,coord,val);
  return;
}

/* function displaying the level of the element */
inline void f_level(HELEMENT *el, int ind, double G_CONST *coord,
                    double *val, void *function_data)
{
  assert(el);
  val[0] = (double) el->level;
  return;
}

/***************************************************************************/
inline void grapeInitScalarData(GRAPEMESH *grape_mesh, DUNE_FUNC * dfunc)
{
  F_DATA *f_data = NULL;
  char * name = NULL;
  int compName = 0;
  const char * vecName;

  if (!grape_mesh)
  {
    fprintf(stderr,"ERROR: no grape_mesh in grapeInitScalarData! file = %s, line = %d \n",__FILE__,__LINE__);
    exit(1);
    return;
  }

  if (dfunc)
  {
    if (!f_data)
    {
      f_data = (F_DATA *) malloc(sizeof(F_DATA));
      assert( f_data != NULL );

      f_data->next = NULL;
      f_data->last = NULL;

      /* little hack to cover a grape bug */
      compName = ((DUNE_FDATA *) dfunc->all)->compName;
      vecName  = ((DUNE_FDATA *) dfunc->all)->name;
      int length = (strlen(vecName));
      length += 5;
      name = (char *) malloc(length*sizeof(char));
      assert (name);
      if(compName < 0)
        sprintf(name,"%s",vecName);
      else
        sprintf(name,"%s [%d]",vecName,compName);

      printf("generate data for discrete function '%s'!\n",name);

      f_data->name = name;
      dfunc->name = name;
      f_data->dimension_of_value = dfunc->all->dimVal;

      f_data->continuous_data = dfunc->all->continuous;

      f_data->f                   = f_real;
      f_data->f_el_info           = f_real_el_info;

      f_data->function_data = (void *) dfunc;

      f_data->get_bounds      = f_bounds;
      f_data->get_vertex_estimate   = grape_get_vertex_estimate;
      f_data->get_element_estimate  = grape_get_element_estimate;
      f_data->threshold     = 0.0;
#if GRAPE_DIM == 3
      f_data->geometry_threshold     = 0.0;
#endif
      f_data->hp_threshold    = 0.0;
      f_data->hp_maxlevel     = grape_mesh->max_level;

      grape_mesh = (GRAPEMESH *) GRAPE(grape_mesh,"add-function") (f_data);
    }
    else if (grape_mesh->f_data != (GENMESH_FDATA *)f_data)
    {
      printf("select f_data for \n");
      grape_mesh->f_data = (GENMESH_FDATA *)f_data;
    }
  }
  else
  {
    printf("no dfunc, or no vec\n");
  }

  return;
}

/***************************************************************************/
/* function info for level display */
/* the variables are only needed once, therefore static */
static char * level_name = "level";
static DUNE_FUNC level_func = {level_name,NULL,NULL};

/* generates the function to display the level of an element */
inline void grapeAddLevelFunction(GRAPEMESH *grape_mesh)
{
  F_DATA *f_data = NULL;

  if (!grape_mesh)
  {
    fprintf(stderr,"ERROR: no grape_mesh in grapeInitScalarData! file = %s, line = %d \n",__FILE__,__LINE__);
    exit(1);
    return;
  }

  if (!f_data)
  {
    f_data = (F_DATA *) malloc(sizeof(F_DATA));
    assert( f_data != NULL );

    f_data->next = NULL;
    f_data->last = NULL;

    f_data->name = level_name;
    f_data->dimension_of_value = 1;
    f_data->continuous_data    = 0;

    f_data->f                   = f_level;
    f_data->f_el_info           = f_real_el_info;

    f_data->function_data = (void *) &level_func;

    f_data->get_bounds      = f_bounds;
    f_data->get_vertex_estimate   = grape_get_vertex_estimate;
    f_data->get_element_estimate  = grape_get_element_estimate;
    f_data->threshold     = 0.0;
#if GRAPE_DIM == 3
    f_data->geometry_threshold     = 0.0;
#endif
    f_data->hp_threshold    = 0.0;
    f_data->hp_maxlevel     = grape_mesh->max_level;

    grape_mesh = (GRAPEMESH *) GRAPE(grape_mesh,"add-function") (f_data);
  }
  else if (grape_mesh->f_data != (GENMESH_FDATA *)f_data)
  {
    printf("select f_data for \n");
    grape_mesh->f_data = (GENMESH_FDATA *)f_data;
  }

  return;
}


/*****************************************************************************
******************************************************************************
**                      **
**  Die MESH Routinen "copy_element"                              **
**                      **
******************************************************************************
*****************************************************************************/

inline static ELEMENT * copy_element(ELEMENT *el, MESH_ELEMENT_FLAGS flag)
{
  HELEMENT * cel = get_stackentry();

  DUNE_ELEM * hexa_elem, * chexa_elem ;

  assert(el) ;
  assert(cel) ;

  hexa_elem = (DUNE_ELEM *)el->user_data;
  chexa_elem = (DUNE_ELEM *)
               (*((struct dune_dat *)el->mesh->user_data)->copy)(hexa_elem) ;
  assert(chexa_elem) ;
  cel->mesh              = el->mesh ;
  cel->vertex            = (double G_CONST*G_CONST*)chexa_elem->vpointer;
  cel->vindex            = el->vindex ;
  cel->eindex            = el->eindex ;
  cel->descr             = el->descr ;
  cel->parent             = ((HELEMENT *)el)->parent ;
  cel->user_data         = (void *)chexa_elem ;
  cel->level = el->level;

  return ( (ELEMENT *)cel );
}

inline static void get_geometry_vertex_estimate(HELEMENT* helement, double* results)
{
  /* planar mesh -> all geometry-estimates 0*/
  int i;
  for(i=0; i<3; i++)
    results[i] = 1e5;
  return;
}


inline static double get_geometry_element_estimate(HELEMENT* helement)
{
  /*planar mesh -> estimators 0*/
  return(1e5);
}

/* method to get partition number from mesh */
inline HMESH * get_partition_number (int * partition)
{
  HMESH * hmesh = (HMESH *) START_METHOD (G_INSTANCE);
  assert(hmesh != 0);
  DUNE_DAT * dunedata = (DUNE_DAT *) hmesh->user_data;
  assert(dunedata != 0);
  *partition = dunedata->partition;

  END_METHOD(hmesh);
}

/*****************************************************************************
******************************************************************************
**                      **
**  Die Hautroutine zum Initialisieren und Aufrufen eines HMESH"      **
**                      **
**  --setupHmesh
**  --hmesh
**
******************************************************************************
*****************************************************************************/
inline void * hmesh(int (* const f_leaf) (DUNE_ELEM *), int (* const n_leaf) (DUNE_ELEM *),
                    int (* const f_mac) (DUNE_ELEM *), int (* const n_mac) (DUNE_ELEM *),
                    int (* const f_chi) (DUNE_ELEM *), int (* const n_chi) (DUNE_ELEM *),
                    void * (* const cp)(const void *),
                    int (* const check_inside) (DUNE_ELEM *, const double *),
                    int (* const wtoc) (DUNE_ELEM *, const double *, double *),
                    void (* const ctow) (DUNE_ELEM *, const double *, double *),
                    void (* const func_real) (DUNE_ELEM *, DUNE_FDATA*, int ind, const double *, double *),
                    const int noe, const int nov, const int maxlev, int partition,
                    DUNE_ELEM *he , DUNE_FDATA * fe)
{
  DUNE_DAT *dune   = (DUNE_DAT *) malloc(sizeof(DUNE_DAT));
  assert(dune != NULL);

  GRAPEMESH * mesh = (GRAPEMESH *) GRAPE(GrapeMesh,"new-instance") ("Dune Mesh");
  assert(mesh != NULL);

  dune->fst_leaf = f_leaf;
  dune->nxt_leaf = n_leaf;
  dune->fst_macro = f_mac;
  dune->nxt_macro = n_mac;

  /* we start with leaf */
  dune->first_macro = f_leaf;
  dune->next_macro  = n_leaf;

  dune->first_child = f_chi;
  dune->next_child  = n_chi;

  dune->copy         = cp;
  dune->wtoc         = wtoc;
  dune->ctow         = ctow;
  dune->check_inside = check_inside;
  dune->all          = he;
  dune->partition    = partition;

  mesh->first_macro = first_macro ;
  mesh->next_macro  = next_macro ;

  mesh->first_child = &fake_child;
  mesh->next_child  = &fake_child;
  mesh->select_child  = select_child;

  mesh->copy_element  = copy_element ;
  mesh->free_element  = gFreeElement ;

  mesh->first_element = first_element;
  mesh->next_element  = next_element;

  mesh->max_number_of_vertices = MAX_EL_DOF ;
  mesh->max_eindex = noe ;
  mesh->max_vindex = nov ;

#if GRAPE_DIM==2
  mesh->dimension_of_world = GRAPE_DIMWORLD;
#endif
  mesh->max_dimension_of_coord = GRAPE_DIMWORLD;
  mesh->max_dindex = 20;

  mesh->max_level = maxlev;
  mesh->level_of_interest = maxlev;

  mesh->get_geometry_vertex_estimate  = get_geometry_vertex_estimate;
  mesh->get_geometry_element_estimate = get_geometry_element_estimate;
  mesh->get_lens_element_estimate = NULL;
  mesh->threshold              = 1.0;

  mesh->user_data = (void *) dune;

  mesh->set_time = NULL;
  mesh->get_time = NULL;
  mesh->f_data   = NULL;

  grapeAddLevelFunction(mesh);

  if(fe)
  {
    /* setup dune data */
    if(fe->discFunc)
    {
      DUNE_FUNC *dfunc = (DUNE_FUNC *) malloc(sizeof(DUNE_FUNC));
      dfunc->func_real = func_real;
      dfunc->all = fe;
      grapeInitScalarData (mesh, dfunc );
    }
    else
    {
      mesh->f_data = NULL;
    }
  }
  /* fill the reference elements */
  setupReferenceElements();

  return ((void *) mesh);
}

/* forward declaration */
static void grape_add_remove_methods(void);

inline void handleMesh(void *hmesh)
{
  GRAPEMESH *mesh = (GRAPEMESH *) hmesh;
  assert(mesh != NULL);

  MANAGER * mgr = (MANAGER *)GRAPE(Manager,"get-stdmgr") ();
  SCENE  *sc = (SCENE *)GRAPE(Scene,"new-instance") ("dune hmesh");

  char p_name[32];
  sprintf(p_name,"uif-m%d",GRAPE_DIM);
  g_project_add(p_name);

  if(!mesh->f_data)
  {
    /* if no data then switch to grid mode */
    GRAPHICDEVICE *grdev;

    grdev = (GRAPHICDEVICE *)GRAPE(GraphicDevice,"get-stddev") ();
    grdev->clear();
    if (grdev && (grdev->grid_patch != G_GRID)) {
      GRAPE(grdev,"grid-patch") (G_GRID);
    }
  }

  sc->object = (TREEOBJECT *)mesh;

  if((!leafButton) || (!maxlevelButton))
    setupLeafButton(mgr,sc,0);

  grape_add_remove_methods();

  GRAPE(mgr,"handle") (sc);  // grape display call
  return ;
}

/* setup TimeScene Tree  */
inline void addDataToHmesh(void  *hmesh, DUNE_FDATA * fe,
                           void (* const func_real) (DUNE_ELEM *, DUNE_FDATA*, int ind, const double *, double *)  )
{
  GRAPEMESH *mesh = (GRAPEMESH *) hmesh;
  assert(mesh != NULL);

  if(fe)
  {
    DUNE_FUNC *dfunc = (DUNE_FUNC *) malloc (sizeof(DUNE_FUNC));
    assert(dfunc != NULL);

    /* setup dune data */
    if(fe->discFunc)
    {
      dfunc->func_real = func_real;
      dfunc->all = fe;

      grapeInitScalarData (mesh, dfunc );
    }
  }
  else
  {
    fprintf(stderr,"ERROR: no function data for setup in addDataToHmesh! \n");
    mesh->f_data = NULL;
  }
}

/*
 * setup TimeScene Tree  */
inline void addHmeshToTimeScene(void * timescene, double time, void  *hmesh, int proc)
{
  TIMESCENE *tsc = (TIMESCENE *) timescene;
  GRAPEMESH *mesh = (GRAPEMESH *) hmesh;
  int i=0;
  assert(tsc != NULL); assert ( mesh != NULL);

  for(i=0; i<proc; i++)
  {
    tsc = (TIMESCENE *) tsc->next_scene;
  }
  assert(tsc);

  if(tsc->dynamic)
  {
    tsc->dynamic = (G_SCENE_OBJECT *)GRAPE(tsc->dynamic,"put") (mesh, mesh, time);
  }
  else
    tsc->dynamic = (G_SCENE_OBJECT *)GRAPE(TimeStep,"put") (mesh, mesh, time);

  return;
}

/*
 * setup TimeScene Tree  */
inline void addHmeshToGlobalTimeScene(double time, void  *hmesh, int proc)
{
  TIMESCENE *tsc = globalTsc;
  GRAPEMESH *mesh = (GRAPEMESH *) hmesh;
  assert(tsc  != NULL);
  assert(mesh != NULL);

  if(tsc->dynamic)
  {
    tsc->dynamic = (G_SCENE_OBJECT *)GRAPE(tsc->dynamic,"put") (mesh, mesh, time);
  }
  else
    tsc->dynamic = (G_SCENE_OBJECT *)GRAPE(TimeStep,"put") (mesh, mesh, time);

  return;
}

inline DUNE_FDATA * extractData ( void * hmesh , int num )
{
  HMESH *mesh = (HMESH *) hmesh;
  int count = 0;
  assert ( mesh != NULL );

  printf("actual mesh %p \n",mesh);

  GENMESH_FDATA *next_data = mesh->f_data;
  while ( (count < num) && (next_data != NULL) )
  {
    next_data = next_data->next;
    count++;
  }

  if( next_data )
  {
    DUNE_FDATA * df = ((DUNE_FUNC *) next_data->function_data)->all;
    printf("df->name %s \n",df->name);
    return df;
  }

  return NULL;
}


/* copy function data */
inline void copyFdata(F_DATA *copy, F_DATA *org)
{
  copy->name = org->name;
  copy->last = org->last;
  copy->next = org->next;

  copy->dimension_of_value  = org->dimension_of_value;
  copy->continuous_data = org->continuous_data;
  copy->function_data  = org->function_data;

  copy->f = org->f;
  copy->f_el_info = org->f_el_info;

  copy->get_bounds  = org->get_bounds;
  copy->get_vertex_estimate = org->get_vertex_estimate;
  copy->get_element_estimate  = org->get_element_estimate;

  copy->threshold = org->threshold;
#if GRAPE_DIM == 3
  copy->geometry_threshold = org->geometry_threshold;
#endif
  copy->hp_threshold = org->hp_threshold;
  copy->hp_maxlevel = org->hp_maxlevel;
}

/* interpol method for timescence, just constant interpolation */
inline static GRAPEMESH *grape_mesh_interpol(GRAPEMESH *mesh1, GRAPEMESH *mesh2,
                                             double factor)
{
  GRAPEMESH *self=NULL;
  GRAPEMESH *newMesh =NULL;

  self = (GRAPEMESH *)START_METHOD(G_INSTANCE);
  ASSURE (self, "No HMESH in method interpol! \n", END_METHOD (NULL));

  if (factor < 0.5)
    newMesh = mesh1;
  else
    newMesh = mesh2;

  if( (!self->f_data) && (newMesh->f_data) )
  {
    self->level_of_interest = newMesh->level_of_interest;
    F_DATA *next_data = (F_DATA *) newMesh->f_data;
    /* to keep the same order we have to go backward */
    while (next_data)
    {
      if(next_data->next) next_data = (F_DATA *) next_data->next;
      else break;
    }
    while (next_data)
    {
      F_DATA * f_data = (F_DATA *) malloc(sizeof(F_DATA));
      assert(f_data != NULL);
      copyFdata(f_data,next_data);

      self = (GRAPEMESH *) GRAPE(self,"add-function") (f_data);
      next_data = (F_DATA *) next_data->last;
    }
  }

  self->max_dimension_of_coord = newMesh->max_dimension_of_coord;
  self->max_eindex = newMesh->max_eindex;
  self->max_vindex = newMesh->max_vindex;
  self->max_dindex = newMesh->max_dindex;
  self->max_number_of_vertices = newMesh->max_number_of_vertices;

  self->access_mode = newMesh->access_mode;
  self->access_capability = newMesh->access_capability;

  /* we have to do that, GRAPE sucks  */
  /* set other function_data pointers */
  if(newMesh->f_data)
  {
    GENMESH_FDATA * sf = self->f_data;
    while(sf != NULL)
    {
      const char * sfname = ((DUNE_FUNC *) sf->function_data)->name;
      GENMESH_FDATA * nf = newMesh->f_data;
      int length = strlen(sfname);
      while( (nf != NULL) )
      {
        /* compare the real function name, ha, not with me */
        const char * nfname = ((DUNE_FUNC *) nf->function_data)->name;
        if( strncmp(sfname,nfname,length) == 0 )
        {
          sf->function_data = nf->function_data;
          break;
        }

        /* GRAPE sucks, sucks, sucks
         * Robert after debugin' for this shit more than one day */
        if( nf != nf->last )
          nf = nf->next;
        else
          break;
      }

      /* go next f_data */
      if( sf != sf->last )
        sf = sf->next;
    }
  }

  /* copy current function selections to newMesh */
  self = (GRAPEMESH *) GRAPE(self, "copy-function-selector") (newMesh);

  self->user_data = newMesh->user_data;

  self->copy_element = newMesh->copy_element;
  self->free_element = newMesh->free_element;

  self->complete_element = newMesh->complete_element;
  self->set_time = newMesh->set_time;
  self->get_time = newMesh->get_time;

  self->first_macro = newMesh->first_macro;
  self->next_macro = newMesh->next_macro;
  self->first_child = newMesh->first_child;
  self->next_child = newMesh->next_child;
  self->select_child = newMesh->select_child;

  self->first_element = newMesh->first_element;
  self->next_element  = newMesh->next_element;

  self->max_level = newMesh->max_level;
  self->level_of_interest = newMesh->level_of_interest;
  /* do not set level_of_interest, because is set by user during run time */

  self->get_geometry_vertex_estimate  = newMesh->get_geometry_vertex_estimate;
  self->get_geometry_element_estimate = newMesh->get_geometry_element_estimate;
  self->get_lens_element_estimate     = newMesh->get_lens_element_estimate;
  self->threshold                     = newMesh->threshold;

#if GRAPE_DIM==2
  self->dimension_of_world = newMesh->dimension_of_world;
#endif

  END_METHOD(self);
}

/****************************************************************************/
/* handling of multiple functions (selection by next/last)                  */
/****************************************************************************/

inline static HMESH *next_f_data_send(void)
{
  HMESH *self;
  printf("next_f_data_send called! \n");

  self = (HMESH *)START_METHOD(G_INSTANCE);
  ASSURE(self, "", END_METHOD(NULL));

  if (self->f_data && self->f_data->next)
  {
    self->f_data->next->last = self->f_data;  /*only to be sure...*/
    self->f_data = self->f_data->next;
  }
  if (self->f_data)
    printf("new f_data is: %s\n", self->f_data->name);

  END_METHOD(self);
}

inline static HMESH *prev_f_data_send(void)
{
  HMESH *self;
  printf("prev_f_data_send called! \n");

  self = (HMESH *)START_METHOD(G_INSTANCE);
  ASSURE(self, "", END_METHOD(NULL));

  if (self->f_data && self->f_data->last)
  {
    self->f_data->last->next = self->f_data;  /*only to be sure...*/
    self->f_data = self->f_data->last;
  }
  if (self->f_data)
    printf("new f_data is: %s\n", self->f_data->name);

  END_METHOD(self);
}

inline SCENE* scene_leaf_button_on_off ()
{
  SCENE*   sc = (SCENE*) START_METHOD (G_INSTANCE);
  ALERT (sc, "level-button-on-off: No hmesh!", END_METHOD(NULL));
  assert(leafButton);

  if( leafButton->on_off == ON )
  {
    GRAPE(leafButton,"set-state") (UNPRESSED);
    leafButton->on_off = OFF;
  }
  else
  {
    GRAPE(leafButton,"set-state") (PRESSED);
    leafButton->on_off = ON;
  }
  END_METHOD (sc);
}

inline SCENE* scene_maxlevel_on_off ()
{
  SCENE* sc = (SCENE*) START_METHOD (G_INSTANCE);
  ALERT( sc, "maxlevel-on-off: No hmesh!", END_METHOD(NULL));

  if( maxlevelButton->on_off == ON )
  {
    GRAPE(maxlevelButton,"set-state") (UNPRESSED);
    maxlevelButton->on_off = OFF;
  }
  else
  {
    GRAPE(maxlevelButton,"set-state") (PRESSED);
    maxlevelButton->on_off = ON;
  }
  END_METHOD (sc);
}


//HMESH3D * new_hmesh3d_clip(double (*f)(),
//        void *var, void (*draw_clip)() , void *scal);

GENMESH3D * genmesh3d_switch_iterateLeafs_on_off();

static int calledAddMethods = 0;

/* add some usefull methods */
inline static void grape_add_remove_methods(void)
{
  if(!calledAddMethods)
  {
    printf("Add Method 'next-f-data-send' on HMesh%dd!\n",GRAPE_DIM);
    GRAPE(HMesh,"add-method") ("next-f-data-send",&next_f_data_send);
    printf("Add Method 'prev-f-data-send' on HMesh%dd!\n",GRAPE_DIM);
    GRAPE(HMesh,"add-method") ("prev-f-data-send",&prev_f_data_send);
    GRAPE(GrapeMesh,"add-method") ("interpol", &grape_mesh_interpol);

#if GRAPE_DIM == 3
    //printf("Remove Method  'clip' on GenMesh3d!\n");
    //GRAPE(GenMesh3d,"delete-method")("clip");
    //printf("Add new Method 'clip' on GenMesh3d!\n");
    //GRAPE(GenMesh3d,"add-method")("clip", & new_hmesh3d_clip);
    //printf("\n");
    GRAPE(GenMesh3d,"add-method") ("get-partition-number",get_partition_number);

    printf("Remove Method 'clip-isoline-disp' on GenMesh3d!\n");
    GRAPE(GenMesh3d,"delete-method") ("clip-isoline-disp");
    printf("Remove Method 'clip-isoline-select-disp' on GenMesh3d!\n");
    GRAPE(GenMesh3d,"delete-method") ("clip-isoline-select-disp");
    printf("\n");
#endif
    if( ! (GRAPE(Scene,"find-method") ("leaf-button-on-off")) )
      GRAPE(Scene,"add-method") ("leaf-button-on-off",scene_leaf_button_on_off);
    if( ! (GRAPE(Scene,"find-method") ("maxlevel-on-off")) )
      GRAPE(Scene,"add-method") ("maxlevel-on-off",scene_maxlevel_on_off);

    {
      char p_name[32];
      sprintf(p_name,"uif-m%d",GRAPE_DIM);
      g_project_add(p_name);
    }

    calledAddMethods = 1;
  }
}

/* switch methods from LevelIterator to LeafIterator */
inline int switchMethods(GENMESHnD *actHmesh)
{
  DUNE_DAT * dune = (DUNE_DAT *) actHmesh->user_data;
  GENMESH_FDATA *fd = NULL;
  int isLeaf = 0;
  assert(dune != NULL);

  /* check if data on all Levels or not */
  fd = actHmesh->f_data;
  /*
     if(fd)
     {
     if(( !((DUNE_FDATA *) fd->function_data)->allLevels)
         && (leafButton->on_off == OFF))
       printf("Warning: Only data on leaf level, use LeafIterator! \n");
     }
   */

  // this marks the state before
  if(leafButton->on_off == OFF) // off means it is going to be on
  {
    /* the button is pressed */
    dune->first_macro = dune->fst_leaf;
    dune->next_macro  = dune->nxt_leaf;
    actHmesh->first_child = &fake_child;
    actHmesh->next_child = &fake_child;
    isLeaf = 1;
    /*printf("Leaf is on \n");*/
  }
  else
  {
    /* the button is not pressed */
    dune->first_macro = dune->fst_macro;
    dune->next_macro  = dune->nxt_macro;
    actHmesh->first_child = &first_child;
    actHmesh->next_child = &next_child;
    /*printf("Leaf is off \n");*/
  }

  return isLeaf;
}


/* action function for the levelButton */
/* switch button on or off */
inline GENMESH3D * genmesh3d_switch_iterateLeafs_on_off()
{
  GENMESH3D * self = (GENMESH3D *) START_METHOD(G_INSTANCE);
  assert(self!=NULL);

  if(leafButton->on_off == ON)
  {
    /* the button is not pressed */
    GRAPE(leafButton,"set-state") (UNPRESSED);
  }
  else
  {
    /* the button is pressed */
    GRAPE(leafButton,"set-state") (PRESSED);
  }
  END_METHOD(self);
}

#endif
