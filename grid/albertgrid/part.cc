// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
/****************************************************************************/
/* ALBERT:   an Adaptive multi Level finite element toolbox using           */
/*           Bisectioning refinement and Error control by Residual          */
/*           Techniques                                                     */
/*                                                                          */
/* file:     2d/macro.c                                                     */
/*           !!! includes Common/macro_common.c  !!!                        */
/*                                                                          */
/* description: tools for reading and writing macro triangulations          */
/*                                                                          */
/*                                                                          */
/* history:                                                                 */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*        authors: Alfred Schmidt and Kunibert G. Siebert                   */
/*                 Institut f"ur Angewandte Mathematik                      */
/*                 Albert-Ludwigs-Universit"at Freiburg                     */
/*                 Hermann-Herder-Str. 10                                   */
/*                 79104 Freiburg                                           */
/*                 Germany                                                  */
/*                                                                          */
/*        email:   alfred,kunibert@mathematik.uni-freiburg.de               */
/*                                                                          */
/*     (c) by A. Schmidt und K.G. Siebert (1996)                            */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*  modifications: vector oriented storage of macro triangulation data      */
/*  in structure MACRO_DATA; data is then converted into ALBERT-macro       */
/*  Author: Daniel Koester (2001)                                           */
/****************************************************************************/
#include <ctype.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <algorithm>

#if LINKEN
#include <albert.h>

namespace Albert
{
#endif

/****************************************************************************/
/* these definitions are necessary at the moment for the macros "MEM_ALLOC" */
/* and "MEM_FREE" to work properly                                          */
/****************************************************************************/
typedef int INT_NVERT[N_VERTICES];
typedef int INT_NNEIGH[N_NEIGH];
typedef S_CHAR SCHAR_NNEIGH[N_NEIGH];
typedef U_CHAR UCHAR_NNEIGH[N_NEIGH];



/****************************************************************************/
/* default_boundary(): This procedure returns a constant pointer to a       */
/* BOUNDARY structure corresponding to a standard polygonal boundary.       */
/* NOTE: The total number of possible boundary types is unknown during the  */
/* first calls of this procedure. It therefore has to allocate a new        */
/* BOUNDARY structure for each new argument "bound" passed to it; the       */
/* address of this new BOUNDARY is stored in a dynamic array default_bounds.*/
/****************************************************************************/

const BOUNDARY *default_boundary(MESH *mesh, int bound)
{
  FUNCNAME("default_boundary");

  typedef BOUNDARY *BOUND_PTR;

  static BOUND_PTR *default_bounds = nil;
  static unsigned int i = 0, size = 1;

  if(!bound) {
    WARNING("tried to assign a BOUNDARY * with bound == 0!\n");
    return(nil);
  }

  if(!default_bounds) {
    default_bounds = MEM_ALLOC(size, BOUND_PTR);
    default_bounds[0] = MEM_ALLOC(1, BOUNDARY);

    default_bounds[0]->param_bound = nil;
    default_bounds[0]->bound = bound;

    return((const BOUNDARY *) default_bounds[0]);
  }

  for(i = 0; i < size; i++)
    if (default_bounds[i]->bound == bound)
      return((const BOUNDARY *) default_bounds[i]);

  if(i == size) {
    default_bounds = MEM_REALLOC(default_bounds, size, size + 1, BOUND_PTR);

    default_bounds[size] = MEM_ALLOC(1, BOUNDARY);

    default_bounds[size]->param_bound = nil;
    default_bounds[size]->bound = bound;
    size++;
  }
  return((const BOUNDARY *) default_bounds[size - 1]);
}


/****************************************************************************/
/*  fill_more_boundary_dofs():                                              */
/*  adds dof's at the edges of a given macro triangulation and calculates   */
/*  the number of edges                                                     */
/*  function pointer bdry only used in 3d!                                  */
/****************************************************************************/

static void fill_more_boundary_dofs(MESH *mesh,
                                    const BOUNDARY *(*bdry)(MESH *, int))
{
  int i, n_edges = 0, node = mesh->node[EDGE];
  MACRO_EL   *mel;
  DOF        *dof;

  for (mel = mesh->first_macro_el; mel; mel = mel->next)
  {
    for (i = 0; i < N_NEIGH; i++)
    {
      if (!mel->neigh[i] || (mel->neigh[i]->index < mel->index))
      {
        n_edges++;
        if (mesh->n_dof[EDGE])
        {
          dof = mel->el->dof[node+i] = get_dof(mesh, EDGE);
          if (mel->neigh[i])
            mel->neigh[i]->el->dof[node+mel->opp_vertex[i]] = dof;
        }
      }
    }
  }

  mesh->n_edges = n_edges;

  return;
}

/****************************************************************************/
/*  fill_bound_info():                                                      */
/*  fills boundary information for the vertices of the macro triangulation  */
/*  The type of a boundary vertex is equal to the highest type of all       */
/*  adjoining boundary edges. If there are no boundary edges containing the */
/*  vertex, it is assumed to be an interior vertex.                         */
/****************************************************************************/

static void fill_bound_info(MESH *mesh, MACRO_DATA *data,
                            const BOUNDARY *(*bdry)(MESH *, int))
{
  FUNCNAME("fill_bound_info");
  MACRO_EL   *mel = mesh->first_macro_el;
  int i, j, ne = mesh->n_elements, nv = mesh->n_vertices;

  S_CHAR     *bound = MEM_ALLOC(nv, S_CHAR);

  for(i = 0; i < data->n_macro_elements; i++) {
    for(j = 0; j < N_NEIGH; j++) {
      if (data->boundary[i][j] != INTERIOR)
        mel[i].boundary[j] = bdry(mesh, data->boundary[i][j]);
      else
        mel[i].boundary[j] = nil;
    }
  }

  for (i = 0; i < nv; i++)
    bound[i] = INTERIOR;

  for (i = 0; i < ne; i++)
  {
    for (j = 0; j < N_NEIGH; j++)
    {
      if (mel[i].boundary[j])
      {
        if (mel[i].boundary[j]->bound >= DIRICHLET)
        {
          int j1 = data->mel_vertices[i][(j+1)%3];
          int j2 = data->mel_vertices[i][(j+2)%3];

          bound[j1] = std::max(bound[j1], mel[i].boundary[j]->bound);
          bound[j2] = std::max(bound[j2], mel[i].boundary[j]->bound);
        }
        else if (mel[i].boundary[j]->bound <= NEUMANN)
        {
          int j1 = data->mel_vertices[i][(j+1)%3];
          int j2 = data->mel_vertices[i][(j+2)%3];

          if (bound[j1] != INTERIOR)
            bound[j1] = std::max(bound[j1], mel[i].boundary[j]->bound);
          else
            bound[j1] = mel[i].boundary[j]->bound;

          if (bound[j2] != INTERIOR)
            bound[j2] = std::max(bound[j2], mel[i].boundary[j]->bound);
          else
            bound[j2] = mel[i].boundary[j]->bound;
        }
      }
    }
  }

  for (i = 0; i < ne; i++)
    for (j = 0; j < N_VERTICES; j++)
      mel[i].bound[j] = bound[data->mel_vertices[i][j]];

  MEM_FREE(bound, nv, S_CHAR);

  return;
}


/****************************************************************************/
/* fill_best_edges(): The main job of this routine is to fill the arrays    */
/* best_edges[] and neighs[] below. best_edges[elem] is best explained with */
/* some examples:                                                           */
/* best_edges[elem] == {2, 3, 3}: one longest edge, namely 2                */
/* best_edges[elem] == {0, 1, 3}: two longest edges, namely 0 and 1         */
/* best_edges[elem] == {2, 0, 1}: three longest edges, namely 2, 0, 1       */
/* neighs[elem] contains the global indices of the neighbour edges ordered  */
/* by length to match best_edges[elem].                                     */
/****************************************************************************/

static void fill_best_edges(MACRO_DATA *data, int elem, UCHAR_NNEIGH edge,
                            INT_NNEIGH neighs)
{
  static U_CHAR i;
  static REAL l[3];

  for(i = 0; i < N_EDGES; i++) {
    l[i] = DIST_DOW(data->coords[data->mel_vertices[elem][(i + 1) % N_EDGES]],
                    data->coords[data->mel_vertices[elem][(i + 2) % N_EDGES]]);
    edge[i] = i;
  }

  for (i = 0; i < N_EDGES; i++) {
    if (l[i] > l[edge[0]]) edge[0] = i;
    if (l[i] < l[edge[2]]) edge[2] = i;
  }

  edge[1] = N_EDGES - edge[0] - edge[2];

  for(i = 0; i < N_NEIGH; i++)
    neighs[i] = data->neigh[elem][edge[i]];

  for (i = 1; i < N_EDGES; i++)
    if ( (l[edge[i - 1]] - l[edge[i]]) > REAL_EPSILON * l[edge[i]] )
      break;

  for (; i < N_EDGES; i++)
    edge[i]=N_EDGES;
}


/****************************************************************************/
/* new_refine_edge(): change the local indices of vertices, neighbours and  */
/* boundaries to fit the choice of the new refinement edge.                 */
/* NOTE: the element's orientation is unimportant for this routine.         */
/****************************************************************************/

static void new_refine_edge(MACRO_DATA *data, int elem, U_CHAR new_edge)
{
  static int buffer_vertex, buffer_neigh;
  static S_CHAR buffer_boundary;

  if(new_edge != 2) {

    buffer_vertex   = data->mel_vertices[elem][0];
    buffer_neigh    = data->neigh[elem][0];
    buffer_boundary = data->boundary[elem][0];

    switch (new_edge) {
    case 0 :

      data->mel_vertices[elem][0] = data->mel_vertices[elem][1];
      data->mel_vertices[elem][1] = data->mel_vertices[elem][2];
      data->mel_vertices[elem][2] = buffer_vertex;
      data->neigh[elem][0] = data->neigh[elem][1];
      data->neigh[elem][1] = data->neigh[elem][2];
      data->neigh[elem][2] = buffer_neigh;
      data->boundary[elem][0] = data->boundary[elem][1];
      data->boundary[elem][1] = data->boundary[elem][2];
      data->boundary[elem][2] = buffer_boundary;
      break;
    case 1 :

      data->mel_vertices[elem][0] = data->mel_vertices[elem][2];
      data->mel_vertices[elem][2] = data->mel_vertices[elem][1];
      data->mel_vertices[elem][1] = buffer_vertex;
      data->neigh[elem][0] = data->neigh[elem][2];
      data->neigh[elem][2] = data->neigh[elem][1];
      data->neigh[elem][1] = buffer_neigh;
      data->boundary[elem][0] = data->boundary[elem][2];
      data->boundary[elem][2] = data->boundary[elem][1];
      data->boundary[elem][1] = buffer_boundary;
    }
  }
}


/****************************************************************************/
/* reorder(): the main routine for correcting cyles                         */
/*                                                                          */
/* Refinement edges are chosen using the following priority:                */
/*      0. If the current element elem only has one longest edge then       */
/*         choose that edge as refinement edge.                             */
/*      1. If possible, choose the refinement edge as one of the longest    */
/*         edges along the boundary.                                        */
/*      2. Otherwise chose the refinement edge as one of the longest edges  */
/*         whose corresponding neighbour's edge is also one of its longest  */
/*         ones (thus creating compatibly divisible pairs of elements)      */
/*      3. Choose a longest edge towards an already tested element.         */
/*      4. If all else fails, choose an arbitrary longest edge.             */
/****************************************************************************/

static void reorder(MACRO_DATA *data, U_CHAR *test, int elem,
                    INT_NNEIGH *neighs, UCHAR_NNEIGH *best_edges)
{
  FUNCNAME("reorder");

  static U_CHAR j, k;

  MSG("Current elem: %d, best_edges: %d %d %d\n", elem, best_edges[elem][0], best_edges[elem][1], best_edges[elem][2]);

  test[elem] = true;

  if (best_edges[elem][1] == N_EDGES) {
    new_refine_edge(data, elem, best_edges[elem][0]);
    return;
  }

  for (j = 0; best_edges[elem][j] < N_EDGES; j++) {
    MSG("Looking at best_edges[%d][%d]...\n", elem, j);
    if (neighs[elem][j] < 0) {
      MSG("It is a border edge! Selecting it...\n");
      new_refine_edge(data, elem, best_edges[elem][j]);
      return;
    }
    if (!test[neighs[elem][j]]) {
      for (k = 0; best_edges[neighs[elem][j]][k] < N_EDGES; k++)
        if(neighs[neighs[elem][j]][k] == elem) {
          MSG("Found compatibly divisible neighbour %d!\n", neighs[elem][j]);
          test[neighs[elem][j]] = true;
          new_refine_edge(data, elem, best_edges[elem][j]);
          new_refine_edge(data,neighs[elem][j],best_edges[neighs[elem][j]][k]);
          return;
        }
    }
  }

  MSG("No immediate patch found - trying to select an edge towards tested elements.\n");

  for (j = 0; best_edges[elem][j] < N_EDGES; j++) {
    MSG("Looking at best_edges[%d][%d]...\n", elem, j);
    if (test[neighs[elem][j]]) {
      MSG("Found tested neighbour on edge %d.", j);
      new_refine_edge(data, elem, best_edges[elem][j]);
      return;
    }
  }
  MSG("Finally resorted to selecting edge %d.\n", best_edges[elem][0]);
  new_refine_edge(data, elem, best_edges[elem][0]);
  return;
}


/****************************************************************************/
/* correct_cycles(): Correct refinement cycles using reorder()              */
/****************************************************************************/

static void correct_cycles(MACRO_DATA *data)
{
  FUNCNAME("correct_cycles");

  int elem;
  U_CHAR *test;

  INT_NNEIGH *neighs = MEM_ALLOC(data->n_macro_elements, INT_NNEIGH);
  UCHAR_NNEIGH *best_edges = MEM_ALLOC(data->n_macro_elements, UCHAR_NNEIGH);

  test = MEM_CALLOC(data->n_macro_elements, U_CHAR);

  for(elem = 0; elem < data->n_macro_elements; elem++)
    fill_best_edges(data, elem, best_edges[elem], neighs[elem]);

  for(elem = 0; elem < data->n_macro_elements; elem++)
    if(!test[elem]) reorder(data,test,elem, neighs, best_edges);

  MEM_FREE(test, data->n_macro_elements, U_CHAR);
  MEM_FREE(neighs, data->n_macro_elements, INT_NNEIGH);
  MEM_FREE(best_edges, data->n_macro_elements, UCHAR_NNEIGH);
}


/****************************************************************************/
/* orientation(): checks and corrects whether the element is oriented       */
/* counterclockwise.                                                        */
/****************************************************************************/

U_CHAR orientation(MACRO_DATA *data)
{
  int i, vert_buffer, neigh_buffer;
  REAL_D e1, e2;
  REAL det, *a0;
  S_CHAR bound_buffer;
  U_CHAR result = false;



  for(i = 0; i < data->n_macro_elements; i++) {
    a0 = data->coords[data->mel_vertices[i][0]];

    e1[0] = data->coords[data->mel_vertices[i][1]][0] - a0[0];
    e1[1] = data->coords[data->mel_vertices[i][1]][1] - a0[1];
    e2[0] = data->coords[data->mel_vertices[i][2]][0] - a0[0];
    e2[1] = data->coords[data->mel_vertices[i][2]][1] - a0[1];

    det = e1[0]*e2[1] - e1[1]*e2[0];

    if(det < 0) {
      result = true;

      vert_buffer = data->mel_vertices[i][0];
      data->mel_vertices[i][0] = data->mel_vertices[i][1];
      data->mel_vertices[i][1] = vert_buffer;

      neigh_buffer = data->neigh[i][0];
      data->neigh[i][0] = data->neigh[i][1];
      data->neigh[i][1] = neigh_buffer;

      bound_buffer = data->boundary[i][0];
      data->boundary[i][0] = data->boundary[i][1];
      data->boundary[i][1] = bound_buffer;
    }
  }

  return(result);
}


/****************************************************************************/
/* macro_test(): Check data for potential cycles during refinement          */
/* At the moment, a correction (and subsequent writing of a correct macro   */
/* data file) can only be done in 2D                                        */
/* Additionally, the element orientation is checked and corrected.          */
/****************************************************************************/

static int cycles(MACRO_DATA *);

extern void macro_test(MACRO_DATA *data, const char *nameneu)
{
  FUNCNAME("macro_test");

  U_CHAR error_found = false;
  int i = -1;

  i = cycles(data);

  if (i >= 0)
  {
    error_found = true;
    WARNING("There is a cycle beginning in macro element %d.\n", i);
    MSG("Correcting refinement edges....\n");
    correct_cycles(data);
  }

#if DIM_OF_WORLD == 2
  if(orientation(data)) {
    error_found = true;
    WARNING("Element orientation was corrected for some elements.\n");
  }
#endif

  if (error_found && nameneu) {
    MSG("Attempting to write corrected macro data to file %s...\n", nameneu);
    //write_macro_data(data, nameneu);
  }

  return;
}

#include "partialGrid.cc"

#if LINKEN
} // namespace Albert
#endif
