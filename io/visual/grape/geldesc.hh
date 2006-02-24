// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __GRAPE_ELDESC_H__
#define __GRAPE_ELDESC_H__

#include <assert.h>
#include <string.h>

#if GRAPE_DIM==3
typedef HELEMENT3D HELEMENT;
typedef ELEMENT3D ELEMENT;
typedef HMESH3D HMESH;
typedef MESH3D GRAPEMESH;
typedef GENMESH3D GENMESHnD;
typedef HELEMENT3D_DESCRIPTION H_ELEMENT_DESCRIPTION;
typedef ELEMENT3D_DESCRIPTION ELEMENT_DESCRIPTION;
typedef F_HDATA3D F_DATA;
typedef F_HEL_INFO3D F_EL_INFO;
#define HMesh       HMesh3d
#define GenMesh     GenMesh3d
#define GrapeMesh   Mesh3d
#else
typedef HELEMENT2D HELEMENT;
typedef ELEMENT2D ELEMENT;
typedef HMESH2D HMESH;
typedef GENMESH2D GENMESHnD;
typedef MESH2D GRAPEMESH;
typedef HELEMENT2D_DESCRIPTION H_ELEMENT_DESCRIPTION;
typedef ELEMENT2D_DESCRIPTION ELEMENT_DESCRIPTION;
typedef F_HDATA2D F_DATA;
typedef F_HEL_INFO2D F_EL_INFO;
#define HMesh       HMesh2d
#define GenMesh     GenMesh2d
#define GrapeMesh   Mesh2d
#endif

/**************************************************************************/
/*  element types, see dune/grid/common/grid.hh and grapegriddisplay.hh */
enum GR_ElementType
{gr_vertex=6,gr_line=7,
 // here consecutive numbering from zero that this numbers can be used
 // in an array starting from 0
 gr_triangle=0, gr_quadrilateral=1,gr_tetrahedron=2,
 gr_pyramid=3, gr_prism=4, gr_hexahedron=5,
 gr_iso_triangle=8, gr_iso_quadrilateral=9,
 gr_unknown=127};
enum { numberOfUsedGrapeElementTypes = 6 };

/**************************************************************************/



typedef struct dune_dat DUNE_DAT;

struct dune_dat
{
  /* first and next macro for LeafIterator */
  int (* fst_leaf)(DUNE_ELEM *);
  int (* nxt_leaf)(DUNE_ELEM *);

  /* first and next macro for LevelIterator */
  int (* fst_macro)(DUNE_ELEM *) ;
  int (* nxt_macro)(DUNE_ELEM *) ;

  /* the actual first and next macro for LevelIterator */
  int (* first_macro)(DUNE_ELEM *) ;
  int (* next_macro)(DUNE_ELEM *) ;

  /* first and next macro for HierarchicIterator */
  int (* first_child)(DUNE_ELEM *) ;
  int (* next_child)(DUNE_ELEM *) ;

  void * (* copy)(const void *) ;

  int (* check_inside)(DUNE_ELEM *, const double * ) ;
  int (* wtoc)(DUNE_ELEM *, const double *, double * ) ;
  void (* ctow)(DUNE_ELEM *, const double *, double * ) ;

  /* to which processor partition the element belongs */
  int partition;

  DUNE_ELEM * all;

};



/*****************************************************************************
* HELEMENT2D_DESCRIPTION for Triangles               *
*****************************************************************************/


static HELEMENT2D_DESCRIPTION triangle_description;

static double triangle_local_coordinate_vector_0[3] = {0.,0.,0.};
static double triangle_local_coordinate_vector_1[3] = {1.,0.,0.};
static double triangle_local_coordinate_vector_2[3] = {0.,1.,0.};

static G_CONST double *triangle_local_coordinate_system[3] = {triangle_local_coordinate_vector_0,
                                                              triangle_local_coordinate_vector_1,
                                                              triangle_local_coordinate_vector_2};

/*  inheritance-rules:
     2                           0 1
    /\                          /| |\
   /  \                     C0 / | | \  C1
   /    \           =>         /  | |  \
   /      \                    /   | |   \
   0--------1                  1----2 2----0
 */

// NOTE: To be revised
static VINHERIT inheritance_rule_in_child_0[3];
static VINHERIT inheritance_rule_in_child_1[3];

static double pweight_point_0_or_1[1] = {1.0};

static int pindex_point_0_in_child_0[1] = {2};
static VINHERIT vinherit_point_0_in_child_0    = {1,
                                                  pindex_point_0_in_child_0,
                                                  pweight_point_0_or_1};


static int pindex_point_1_in_child_0[1] = {0};
static VINHERIT vinherit_point_1_in_child_0    = {1,
                                                  pindex_point_1_in_child_0,
                                                  pweight_point_0_or_1};

static int pindex_point_0_in_child_1[1] = {1};
static VINHERIT vinherit_point_0_in_child_1    = {1,
                                                  pindex_point_0_in_child_1,
                                                  pweight_point_0_or_1};

static int pindex_point_1_in_child_1[1] = {2};
static VINHERIT vinherit_point_1_in_child_1    = {1,
                                                  pindex_point_1_in_child_1,
                                                  pweight_point_0_or_1};

static int pindex_point_2[2] = {0  ,1  };
static double pweight_point_2[2] = {0.5,0.5};
static VINHERIT vinherit_point_2    = {2,pindex_point_2,pweight_point_2};


/*****************************************************************************
******************************************************************************
**                      **
**  Die HEL_DESCR Routinen "neighbour, boundary, check_inside,        **
**              world2coord, coord2world"       **
**                      **
******************************************************************************
*****************************************************************************/

inline static HELEMENT2D * triangle_neighbour(HELEMENT2D *el, int np, int flag,

                                              double * coord, double * xyz, MESH_ELEMENT_FLAGS p) {
  printf(" neighbour nicht implementiert \n");
  return el ;
}

inline int triangle_boundary(HELEMENT2D * el, int np) {
  return ((DUNE_ELEM *)el->user_data)->bnd[np] ;
}

/***********************************************************************
*
* the functions check_inside, world2coord and coord2world
* work for all types of elements
*
***********************************************************************/

/* the 2d versions */
inline int el_check_inside(HELEMENT2D * e, const double * coord)
{
  DUNE_DAT * dat = (DUNE_DAT *) ((HMESH2D *) e->mesh)->user_data;
  return dat->check_inside((DUNE_ELEM *) e->user_data, coord);
}

inline int world2coord(HELEMENT2D * e, const double * xyz,double * coord)
{
  DUNE_DAT * dat = (DUNE_DAT *) ((HMESH2D *) e->mesh)->user_data;
  return dat->wtoc((DUNE_ELEM *) e->user_data, xyz, coord);
}

inline void coord2world(HELEMENT2D * e, const double * coord,double * xyz)
{
  DUNE_DAT * dat = (DUNE_DAT *) ((HMESH2D *) e->mesh)->user_data;
  dat->ctow((DUNE_ELEM *) e->user_data, coord, xyz);
}

/* the 3d versions */
inline int el_check_inside_3d(HELEMENT3D * e, double * coord)
{
  DUNE_DAT * dat = (DUNE_DAT *) ((HMESH3D *) e->mesh)->user_data;
  return dat->check_inside((DUNE_ELEM *) e->user_data, coord);
}

inline static int world2coord_3d(HELEMENT3D * e, const double * xyz,
                                 double * coord)
{
  DUNE_DAT * dat = (DUNE_DAT *) ((HMESH3D *) e->mesh)->user_data;
  return dat->wtoc((DUNE_ELEM *) e->user_data, xyz, coord);
}

inline static void coord2world_3d(HELEMENT3D * e, const double * coord,
                                  double * xyz)
{
  DUNE_DAT * dat = (DUNE_DAT *) ((HMESH3D *) e->mesh)->user_data;
  dat->ctow((DUNE_ELEM *) e->user_data, coord, xyz);
}

/*****************************************************************************
* HELEMENT2D_DESCRIPTION for Quadrilaterals            *
*****************************************************************************/

/****************************************************************************/
/*  Eckpunkte :
 *
 *
 *   (0,1)  3---------2  (1,1)
 *          |         |
 *          |         |
 *          |         |
 *          |         |
 *          |         |
 *   (0,0)  0---------1  (1,0)
 *
 *  this is the dune local coordinate system
 ***************************************************************************/


static HELEMENT2D_DESCRIPTION quadrilateral_description;

static double quadrilateral_local_coordinate_vector_0[3] = {0.,0.,0.};
static double quadrilateral_local_coordinate_vector_1[3] = {1.,0.,0.};
static double quadrilateral_local_coordinate_vector_2[3] = {1.,1.,0.};
static double quadrilateral_local_coordinate_vector_3[3] = {0.,1.,0.};

static G_CONST double *quadrilateral_local_coordinate_system[4] = {quadrilateral_local_coordinate_vector_0,
                                                                   quadrilateral_local_coordinate_vector_1,quadrilateral_local_coordinate_vector_2,
                                                                   quadrilateral_local_coordinate_vector_3};



/*****************************************************************************
*  HELEMENT3D_DESCRIPTION for Tetrahedra              *
*****************************************************************************/

static HELEMENT3D_DESCRIPTION tetra_description;

/* vertex indices of the polygons (faces) for a tetrahedron           */
static int t_v0_e[3] = {1,3,2},   t_v1_e[3] = {0,2,3};
static int t_v2_e[3] = {0,3,1},   t_v3_e[3] = {0,1,2};

/* polygon adjacencies  for a tetrahedron                     */
static int t_p0_e[3] = {2,1,3},   t_p1_e[3] = {3,0,2};
static int t_p2_e[3] = {1,0,3},   t_p3_e[3] = {2,0,1};

/* local coordinates of the vertices for a tetrahedron  in barycentric
 * coords */
//static double t_c0[4] = {1.,0.,0.,0.}, t_c1[4] = {0.,1.,0.,0.};
//static double t_c2[4] = {0.,0.,1.,0.}, t_c3[4] = {0.,0.,0.,1.};
/* local coordinates of the vertices for a tetrahedron        */
static double t_c0[3] = {0.,0.,0.}, t_c1[3] = {1.,0.,0.};
static double t_c2[3] = {0.,1.,0.}, t_c3[3] = {0.,0.,1.};

static int tetra_polygon_length[4]          = {3, 3, 3, 3};
static G_CONST int    *tetra_vertex_e[4]       = {t_v0_e,t_v1_e,t_v2_e,t_v3_e};
static G_CONST int    *tetra_next_polygon_e[4] = {t_p0_e,t_p1_e,t_p2_e,t_p3_e};
static G_CONST double *tetra_local_coordinate_system[4] =  {t_c0,t_c1,t_c2,t_c3};

/*****************************************************************************
******************************************************************************
**                      **
**  Die HEL_DESCR Routinen "neighbour, boundary, check_inside,        **
**              world2coord, coord2world"       **
**                      **
******************************************************************************
*****************************************************************************/

inline static HELEMENT3D * dummy_neighbour(HELEMENT3D *el, int np, int flag,

                                           double * coord, double * xyz, MESH_ELEMENT_FLAGS p) {

  printf(" neighbour nicht implementiert \n");
  return el ;

}

inline static int wrap_boundary(HELEMENT3D * el, int np)
{
  return ((DUNE_ELEM *)el->user_data)->bnd[np] ;
}

/*****************************************************************************
*  HELEMENT3D_DESCRIPTION for Hexahedra               *
*****************************************************************************/
/****************************************************************************/
/*  Eckpunkte und Seitenflaechen in GRAPE:
 *              7---------6
 *             /.        /|
 *            / .  1    / |
 *           /  .      /  |
 *          4---------5   | <-- 4 (hinten)
 *    5 --> |   .     | 3 |
 *          |   3.....|...2
 *          |  .      |  /
 *          | .   2   | / <-- 0 (unten)
 *          |.        |/
 *          0---------1
 *
 *  this is the GRAPE local coordinate system
 *
 ***************************************************************************
 *
 *  Eckpunkte und Seitenflaechen in DUNE:
 *
 *              6---------7
 *             /.        /|
 *            / .  5    / |
 *           /  .      /  |
 *          4---------5   | <-- 3 (hinten)
 *    0 --> |   .     | 1 |
 *          |   2.....|...3
 *          |  .      |  /
 *          | .   2   | / <-- 4 (unten)
 *          |.        |/
 *          0---------1
 *
 *  this is the DUNE local coordinate system
 ***************************************************************************/
static HELEMENT3D_DESCRIPTION cube_description;


static VEC3 cc1={0.,0.,0.},cc2={1.,0.,0.},cc3={1.,1.,0.},cc4={0.,1.,0.},
            cc5={0.,0.,1.},cc6={1.,0.,1.},cc7={1.,1.,1.},cc8={0.,1.,1.};
static G_CONST double *cube_local_coordinate_system[8] = {cc1,cc2,cc3,cc4,cc5,cc6,cc7,cc8};
// how many polygons on which face
static int cube_polygon_length[6] = {4,4,4,4,4,4};
// vertices of the faces
static int cv1[4]={0,3,2,1},cv2[4]={4,5,6,7},cv3[4]={0,1,5,4},
           cv4[4]={1,2,6,5},cv5[4]={2,3,7,6},cv6[4]={0,4,7,3};
static G_CONST int *cube_polygon_vertex[6] = {cv1,cv2,cv3,cv4,cv5,cv6};
static int cn1[4]={5,4,3,2},cn2[4]={2,3,4,5},cn3[4]={0,3,1,5},
           cn4[4]={0,4,1,2},cn5[4]={0,5,1,3},cn6[4]={2,1,4,0};
static G_CONST int *cube_polygon_neighbour[6] = {cn1,cn2,cn3,cn4,cn5,cn6};


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************
*  HELEMENT3D_DESCRIPTION for Pyramids             *
*****************************************************************************/
static HELEMENT3D_DESCRIPTION pyra_description;

static VEC3 pyc1={0.,0.,0.},pyc2={1.,0.,0.},pyc3={1.,1.,0.},
            pyc4={0.,1.,0.},pyc5={0.,0.,1.};
static G_CONST double *pyra_local_coordinate_system[5] = {pyc1,pyc2,pyc3,pyc4,pyc5};

static int pyra_polygon_length[5] = {4,3,3,3,3};
static int pyv1[4]={0,1,2,3},pyv2[3]={0,4,1},pyv3[3]={1,4,2},
           pyv4[3]={2,4,3}  ,pyv5[3]={0,3,4};
static G_CONST int *pyra_polygon_vertex[5] = {pyv1,pyv2,pyv3,pyv4,pyv5};

static int pyn1[4]={5,4,3,2},pyn2[3]={0,2,4},pyn3[3]={0,3,1},
           pyn4[3]={0,4,2}  ,pyn5[3]={0,1,3};
static G_CONST int *pyra_polygon_neighbour[5] = {pyn1,pyn2,pyn3,pyn4,pyn5};

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************
*  HELEMENT3D_DESCRIPTION for Prism             *
*****************************************************************************/
static HELEMENT3D_DESCRIPTION prism_description;

static VEC3 prc1={0.,0.,0.},prc2={1.,0.,0.},prc3={0.,1.,0.},
            prc4={0.,0.,1.},prc5={1.,0.,1.},prc6={0.,1.,1.};
static G_CONST double *prism_local_coordinate_system[6] = {prc1,prc2,prc3,prc4,prc5,prc6};

// how many polygons on which face
static int prism_polygon_length[5] = {3,4,4,4,3};

// vertices of the faces
static int prv1[3]={0,1,2},  prv2[4]={0,1,4,3},prv3[4]={1,2,5,4},
           prv4[4]={2,0,3,5},prv5[3]={3,4,5};

static G_CONST int *prism_polygon_vertex[5] = {prv1,prv2,prv3,prv4,prv5};

static int prn1[4]={5,4,3,2},prn2[4]={2,3,4,5},prn3[4]={0,3,1,5},
           prn4[4]={0,4,1,2},prn5[4]={0,5,1,3},prn6[4]={2,1,4,0};
static G_CONST int *prism_polygon_neighbour[6] = {prn1,prn2,prn3,prn4,prn5,prn6};

/*  Standard description                */
/****************************************************************************/

/*static int  cube_world2coord();
   static void cube_coord2world();
   static int cube_check_inside();
   static HELEMENT3D *cube_neighbour();
   static int cube_boundary();
 */

#if 0
static void  cube_coord2world(HELEMENT3D *cube,
                              const double *coord, double *xyz)
{
#if 0
  int j;
  const double **v = cube->vertex;

  for(j=0; j<3; j++)
    xyz[j]=(1.0-coord[2])*((1.0-coord[1])*((1.0-coord[0])*v[0][j]+coord[0]*v[1][j])
                           + coord[1]    *((1.0-coord[0])*v[3][j]+coord[0]*v[2][j]))
            + coord[2]   *((1.0-coord[1])*((1.0-coord[0])*v[4][j]+coord[0]*v[5][j])
                           + coord[1]    *((1.0-coord[0])*v[7][j]+coord[0]*v[6][j]));

  return;
#endif
}

static int cube_boundary(HELEMENT3D *el,int pn);
#if 0
{ int i;
  STACKENTRY *stel;
  stel = (STACKENTRY *)el;
  switch(pn) {
  case 0 : return(stel->l[2] == inp->l[2]);
  case 1 : return(stel->r[2] == inp->r[2]);
  case 2 : return(stel->l[1] == inp->l[1]);
  case 3 : return(stel->r[0] == inp->r[0]);
  case 4 : return(stel->r[1] == inp->r[1]);
  case 5 : return(stel->l[0] == inp->l[0]);
  default : fprintf(stderr,"%s l. %s: unknown case in 'cube_boundary'\n",
                    __FILE__, __LINE__); return(-1);
  }}
#endif
static int cube_check_inside(HELEMENT3D *el,double *coord)
{
  int j;
  static int pp[] = {5,2,0};
  static int pm[] = {3,4,1};

  for(j=0; j<3; j++) {
    if(coord[j]<0.) return(pm[j]);
    if(coord[j]>1.) return(pp[j]);
  }
  return(INSIDE);


}


static int  cube_world2coord(HELEMENT3D *cube,
                             const double *xyz, double *coord)

{
#if 0
  /* MATRIX33 a; VEC3 b; */
  VEC3 p, q;
  double **v;
  int j,k,count,merke[3];

  /********************************************/
  /* ONLY if the elements are parallelepipeds */
  /********************************************/

  v = cube->vertex;

  /*
     for(i=0;i<3;i++) {
      b[i] = xyz[i]-v[0][i];
      for(j=0;j<3;j++) a[i][j] = v[help[i]][j]-v[0][j];
     }
     g_solve3(a,b,coord);
   */

  /****************************************************************/
  /* ONLY if solely the normal in one coord. dir. is not constant */
  /****************************************************************/

  for(k=0,count = 0; k<3; k++) {
    if(test_normal(cube,xyz,k,coord)) merke[count++] = k;
    if(count == 2) {
      merke[2] = (merke[0]==0) ? ((merke[1]==1) ? 2 : 1) : 0;
      for(j=0; j<3; j++) {
        p[j] = v[0][j]+
               coord[merke[0]]*(v[help[merke[0]]][j]-v[0][j]) +
               coord[merke[1]]*(v[help[merke[1]]][j]-v[0][j]);
        q[j] = v[help[merke[2]]][j] +
               coord[merke[0]]*(v[help2[merke[2]][0]][j]-v[help[merke[2]]][j]) +
               coord[merke[1]]*(v[help2[merke[2]][1]][j]-v[help[merke[2]]][j])
               -p[j];
        p[j] = xyz[j]-p[j];
      }
      coord[merke[2]] = g_vec3_skp(p,q)/g_vec3_skp(q,q);
      break;
    }
  }
  ASSURE((count==2),"world to coord failed",return (INSIDE));

  return(cube_check_inside(cube,coord));
#endif
  return 1;
}
#endif

/* fill the upper reference elements */
inline void setupReferenceElements()
{
  /* fill the helement description in 2D*/

  triangle_description.dindex             = 0;   // index of description
  triangle_description.number_of_vertices = 3;
  /* dimension of local coords */
  triangle_description.dimension_of_coord = GRAPE_DIM;
  triangle_description.coord              = triangle_local_coordinate_system;
  triangle_description.parametric_degree  = 1;
  triangle_description.world_to_coord     = world2coord;
  triangle_description.coord_to_world     = coord2world;
  triangle_description.check_inside       = el_check_inside;
  triangle_description.neighbour          = triangle_neighbour;
  triangle_description.boundary           = triangle_boundary;


  quadrilateral_description.dindex             = 1;   // index of description
  quadrilateral_description.number_of_vertices = 4;
  quadrilateral_description.dimension_of_coord = GRAPE_DIM;
  quadrilateral_description.coord              = quadrilateral_local_coordinate_system;
  quadrilateral_description.parametric_degree  = 1;
  quadrilateral_description.world_to_coord     = world2coord;
  quadrilateral_description.coord_to_world     = coord2world;
  quadrilateral_description.check_inside       = el_check_inside;
  quadrilateral_description.neighbour          = triangle_neighbour;
  quadrilateral_description.boundary           = triangle_boundary;

  /* fill the helement description in 3D*/

  tetra_description.dindex             = 2;   // index of description
  tetra_description.number_of_vertices = 4;
  tetra_description.number_of_polygons = 4;   // i.e. number of faces
  tetra_description.polygon_length = tetra_polygon_length;
  tetra_description.polygon_vertex  = tetra_vertex_e;
  tetra_description.polygon_neighbour  = tetra_next_polygon_e;
  tetra_description.dimension_of_coord = 3;   // GRAPE_DIM
  tetra_description.coord              = tetra_local_coordinate_system;
  tetra_description.parametric_degree  = 1;
  tetra_description.world_to_coord     = world2coord_3d;
  tetra_description.coord_to_world     = coord2world_3d;
  tetra_description.check_inside       = el_check_inside_3d;
  tetra_description.neighbour          = dummy_neighbour;
  tetra_description.boundary           = wrap_boundary;
  tetra_description.get_boundary_vertex_estimate = NULL;
  tetra_description.get_boundary_face_estimate   = NULL;
  tetra_description.coord_of_parent              = NULL;

  /* pyramid */
  pyra_description.dindex             = 3;   // index of description , see element type
  pyra_description.number_of_vertices = 5;
  pyra_description.number_of_polygons = 5;   // i.e. number of faces
  pyra_description.polygon_length     = pyra_polygon_length;
  pyra_description.polygon_vertex     = pyra_polygon_vertex;
  pyra_description.polygon_neighbour  = pyra_polygon_neighbour;
  pyra_description.dimension_of_coord = 3;   // GRAPE_DIM
  pyra_description.coord              = pyra_local_coordinate_system;
  pyra_description.parametric_degree  = 1;
  pyra_description.world_to_coord     = world2coord_3d;
  pyra_description.coord_to_world     = coord2world_3d;
  pyra_description.check_inside       = el_check_inside_3d;
  pyra_description.neighbour          = dummy_neighbour;
  pyra_description.boundary           = wrap_boundary;
  pyra_description.get_boundary_vertex_estimate = NULL;
  pyra_description.get_boundary_face_estimate   = NULL;
  pyra_description.coord_of_parent              = NULL;


  /* prism */
  prism_description.dindex             = 4;   // index of description
  prism_description.number_of_vertices = 6;
  prism_description.number_of_polygons = 5;   // i.e. number of faces
  prism_description.polygon_length     = prism_polygon_length;
  prism_description.polygon_vertex     = prism_polygon_vertex;
  prism_description.polygon_neighbour  = prism_polygon_neighbour;
  prism_description.dimension_of_coord = 3;   // GRAPE_DIM
  prism_description.coord              = prism_local_coordinate_system;
  prism_description.parametric_degree  = 1;
  prism_description.world_to_coord     = world2coord_3d;
  prism_description.coord_to_world     = coord2world_3d;
  prism_description.check_inside       = el_check_inside_3d;
  prism_description.neighbour          = dummy_neighbour;
  prism_description.boundary           = wrap_boundary;
  prism_description.get_boundary_vertex_estimate = NULL;
  prism_description.get_boundary_face_estimate   = NULL;
  prism_description.coord_of_parent              = NULL;

  /* Hexahedrons */
  cube_description.dindex             = 5;   // index of description
  cube_description.number_of_vertices = 8;
  cube_description.number_of_polygons = 6;   // i.e. number of faces
  cube_description.polygon_length     = cube_polygon_length;
  cube_description.polygon_vertex     = cube_polygon_vertex;
  cube_description.polygon_neighbour  = cube_polygon_neighbour;
  cube_description.dimension_of_coord = 3;   // GRAPE_DIM
  cube_description.coord              = cube_local_coordinate_system;
  cube_description.parametric_degree  = 1;
  cube_description.world_to_coord     = world2coord_3d;
  cube_description.coord_to_world     = coord2world_3d;
  cube_description.check_inside       = el_check_inside_3d;
  cube_description.neighbour          = dummy_neighbour;
  cube_description.boundary           = wrap_boundary;
  cube_description.get_boundary_vertex_estimate = NULL;
  cube_description.get_boundary_face_estimate = NULL;
  cube_description.coord_of_parent = NULL;


  /* inheritance rules */
  inheritance_rule_in_child_0[0] = vinherit_point_0_in_child_0;
  inheritance_rule_in_child_0[1] = vinherit_point_1_in_child_0;
  inheritance_rule_in_child_0[2] = vinherit_point_2;

  inheritance_rule_in_child_1[0] = vinherit_point_0_in_child_1;
  inheritance_rule_in_child_1[1] = vinherit_point_1_in_child_1;
  inheritance_rule_in_child_1[2] = vinherit_point_2;
}

//vector holding the descriptions enumerated after it's index
static H_ELEMENT_DESCRIPTION *
elementDescriptions[numberOfUsedGrapeElementTypes] = {
  (H_ELEMENT_DESCRIPTION *)&triangle_description,
  (H_ELEMENT_DESCRIPTION *)&quadrilateral_description,
  (H_ELEMENT_DESCRIPTION *)&tetra_description,
  (H_ELEMENT_DESCRIPTION *)&pyra_description,
  (H_ELEMENT_DESCRIPTION *)&prism_description,
  (H_ELEMENT_DESCRIPTION *)&cube_description
};

// the mapping of the reference elements
static const int dune2GrapeDefaultMap[MAX_EL_DOF] = {0,1,2,3,4,5,6,7};
static const int * dune2GrapeTriangle      = dune2GrapeDefaultMap;
static const int * dune2GrapeTetrahedron   = dune2GrapeDefaultMap;
static const int * dune2GrapePyramid       = dune2GrapeDefaultMap;
static const int * dune2GrapePrism         = dune2GrapeDefaultMap;

// for quads the vertices 2,3 are swaped
static const int dune2GrapeQuadrilateral[MAX_EL_DOF] = {0,1,3,2,4,5,6,7};
// for hexas the vertices 2,3 and 6,7 are swaped
static const int dune2GrapeHexahedron[MAX_EL_DOF] = {0,1,3,2,4,5,7,6};

// mapping from dune to grape
static const int *
dune2GrapeVertex[numberOfUsedGrapeElementTypes] = {
  dune2GrapeTriangle ,
  dune2GrapeQuadrilateral ,
  dune2GrapeTetrahedron,
  dune2GrapePyramid ,
  dune2GrapePrism ,
  dune2GrapeHexahedron
};

static H_ELEMENT_DESCRIPTION * getElementDescription( int type )
{
  assert( type >= 0 );
  assert( type <  numberOfUsedGrapeElementTypes );
  return elementDescriptions[type];
}
#endif
