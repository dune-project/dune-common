// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
/****************************************************************************/
/* ALBERT:   an Adaptive multi Level finite element toolbox using           */
/*           Bisectioning refinement and Error control by Residual          */
/*           Techniques                                                     */
/*                                                                          */
/* file:     macro_common.c                                                 */
/*     !!!   included by macro.c's  !!!                                     */
/*                                                                          */
/* description: reading of macro triangulations; file is included by        */
/*              1d/macro.c, 2d/macro.c, 3d/macro.c which contain the        */
/*              dimension-depending parts                                   */
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
/*     (c) by A. Schmidt and K.G. Siebert (1996-2001)                       */
/*                                                                          */
/****************************************************************************/
/* The following code is new development using the structure MACRO_DATA.    */
/*                                                  Daniel Koester, 2001    */
/****************************************************************************/


/****************************************************************************/
/*  cycles(): check "data" for potential cycles during refinement           */
/*  returns -1 if data is OK else the global index of an element where a    */
/*  cycle is found                                                          */
/****************************************************************************/
//typedef int INT_NVERT[N_VERTICES];
//typedef int INT_NNEIGH[N_NEIGH];
//typedef S_CHAR SCHAR_NNEIGH[N_NEIGH];
//typedef U_CHAR UCHAR_NNEIGH[N_NEIGH];
//


static int cycles(MACRO_DATA *data)
{
  FUNCNAME("cycles");

  int zykstart, i, elem, elemlfd;
  U_CHAR *test, *zykl, flg;

  test=MEM_CALLOC(data->n_macro_elements, U_CHAR);
  zykl=MEM_ALLOC(data->n_macro_elements, U_CHAR);

  zykstart = -1;

  for(elemlfd = 0; elemlfd < data->n_macro_elements; elemlfd++)
  {
    if (!test[elemlfd]) {
      for (i = 0; i < data->n_macro_elements; i++)
        zykl[i]=0;

      elem = elemlfd;

      flg=2;
      do
      {
        if (zykl[elem] == 1)
        {
          flg=0;
          zykstart=elem;
        }
        else
        {
          zykl[elem]=1;

          if (test[elem]==1)
          {
            flg=1;
          }
          else if (data->neigh[elem][2] < 0)
          {
            flg=1;
            test[elem]=1;
          }
          else if (elem == data->neigh[data->neigh[elem][2]][2])
          {
            flg=1;
            test[elem]=1;
            test[data->neigh[elem][2]]=1;
          }
          else
          {
            elem=data->neigh[elem][2];
          }
        }

      } while(flg == 2);

      if (flg != 1) break;
    }
  }

  MEM_FREE(zykl, data->n_macro_elements, U_CHAR);
  MEM_FREE(test, data->n_macro_elements, U_CHAR);

  return(zykstart);
}


/****************************************************************************/
/* opp_vertex_fast() checks whether the vertex/edge/face with vertices      */
/* test[0],..,test[DIM-1] is part of mel's boundary. It returns the         */
/* opposite vertex if true else -1.                                         */
/****************************************************************************/

static S_CHAR opp_vertex(INT_NVERT mel_vert, int *test)
{
  int i, j, nv = 0, ov = 0;

  for (i = 0; i < N_VERTICES; i++)
  {
    if (nv < i-1) return(-1);

    for (j = 0; j < DIM; j++)
    {
      if (mel_vert[i] == test[j])
      {
        /****************************************************************************/
        /* i is a common vertex                                                     */
        /****************************************************************************/
        ov += i;
        nv++;
        break;
      }
    }

  }
  if (nv != DIM) return(-1);
  /****************************************************************************/
  /*  the opposite vertex is 1/3/6 - (sum of indices of common vertices) in   */
  /*  1d/2d/3d                                                                */
  /****************************************************************************/
#if DIM == 1
  return(1-ov);
#endif

#if DIM == 2
  return(3-ov);
#endif

#if DIM == 3
  return(6-ov);
#endif
}

#if 0
/****************************************************************************/
/*  compute_neighbours() fills neighbour and opp_vertex information if such */
/*  information was not supplied in the macro triangulation file. This is   */
/*  done by looping over all elements, and the looking for another element  */
/*  which shares a common face                                              */
/****************************************************************************/

static void compute_neighbours(MACRO_DATA *data)
{
  FUNCNAME("compute_neighbours");
  int i, j, k, vertices[DIM], info=0;
  S_CHAR l;

  data->neigh = MEM_ALLOC(data->n_macro_elements, INT_NNEIGH);

  /****************************************************************************/
  /* first initialize elements  (-2 as "undefined")                           */
  /****************************************************************************/

  for (i = 0; i < data->n_macro_elements; i++)
    for (j = 0; j < N_NEIGH; j++)
      data->neigh[i][j] = -2;

  /****************************************************************************/
  /* now loop through all elements and look for neighbours                    */
  /****************************************************************************/

  for (i = 0; i < data->n_macro_elements; i++)
  {
    INFO(info,4) ("Current element %d\n",i);
    INFO(info,6) ("with vertices: ");

    for(j = 0; j < N_VERTICES; j++)
      PRINT_INFO(info,6) ("%d ", data->mel_vertices[i][j]);
    PRINT_INFO(info,6) ("\n");

    for (j = 0; j < N_NEIGH; j++)
    {
      if (data->neigh[i][j] == -2)
      {
        INFO(info,8) ("looking for neighbour no %d\n", j);

#if DIM == 1
        vertices[0] = data->mel_vertices[i][1-j];
#else
        for (k = 0; k < DIM; k++)
          vertices[k] = data->mel_vertices[i][(j+k+1)%(DIM+1)];
#endif

        for (k = i+1; k < data->n_macro_elements; k++)
        {
          if ((l = opp_vertex(data->mel_vertices[k], vertices)) != -1) {
            data->neigh[i][j] = k;
            data->neigh[k][l] = i;
            INFO(info,8) ("found element %d as neighbour...\n", k);
            break;
          }
        }

        if(k == data->n_macro_elements) {
          INFO(info,8)
            ("no neighbour %d of element %d found: Assuming a boundary...\n", j, i);

          data->neigh[i][j] = -1;
        }
      }
    }
  }

  return;
}
#endif

/****************************************************************************/
/*  compute_neigh_fast() is an algorithm meant to speed up the task of      */
/*  computing neighbours. It has the same function as the above routine,    */
/*  but does not use an N^2-algorithm.                                      */
/*  The idea is to link vertices to elements sharing them to make the       */
/*  search for neighbours more efficient -  at the cost of some additional  */
/*  temporary memory usage.                                 Daniel Koester  */
/****************************************************************************/

static void compute_neigh_fast(MACRO_DATA *data)
{
  FUNCNAME("compute_neigh_fast");
  int i, j, index, vertices[DIM], info=0;

#if DIM>1
  int k;
#endif

  S_CHAR l;

  struct vert2elem {
    struct vert2elem *next;
    int mel;
  };

  typedef struct vert2elem VERT2ELEM;

  VERT2ELEM *buffer, *buffer2;
  VERT2ELEM **list = MEM_CALLOC(data->n_total_vertices, VERT2ELEM *);

  data->neigh     = MEM_ALLOC(data->n_macro_elements, INT_NNEIGH);

  /****************************************************************************/
  /* first initialize elements  (-2 as "undefined")                           */
  /****************************************************************************/

  for (i = 0; i < data->n_macro_elements; i++)
    for (j = 0; j < N_NEIGH; j++)
      data->neigh[i][j] = -2;

  /****************************************************************************/
  /* fill the array "list" of linked lists                                    */
  /****************************************************************************/

  for(i = 0; i < data->n_macro_elements; i++) {
    for(j = 0; j < N_VERTICES; j++) {
      buffer = list[(index=data->mel_vertices[i][j])];

      list[index] = MEM_ALLOC(1, VERT2ELEM);

      list[index]->next = buffer;

      list[index]->mel = i;
    }
  }

  /****************************************************************************/
  /* here comes the actual checking...                                        */
  /****************************************************************************/

  for (i = 0; i < data->n_macro_elements; i++)
  {
    INFO(info,4) ("Current element %d\n",i);
    INFO(info,6) ("with vertices: ");

    for(j = 0; j < N_VERTICES; j++)
      PRINT_INFO(info,6) ("%d ", data->mel_vertices[i][j]);
    PRINT_INFO(info,6) ("\n");

    for (j = 0; j < N_NEIGH; j++)
    {
      if (data->neigh[i][j] == -2)
      {
        INFO(info,8) ("looking for neighbour no %d\n", j);

#if DIM == 1
        vertices[0] = data->mel_vertices[i][1-j];
#else
        for (k = 0; k < DIM; k++)
          vertices[k] = data->mel_vertices[i][(j+k+1)%(DIM+1)];
#endif
        buffer = list[vertices[0]];

        while(buffer) {
          if(buffer->mel != i) {
            if ((l = opp_vertex(data->mel_vertices[buffer->mel], vertices)) != -1) {
              data->neigh[i][j] = buffer->mel;
              data->neigh[buffer->mel][l] = i;
              INFO(info,8) ("found element %d as neighbour...\n", buffer->mel);
              break;
            }
          }

          buffer = buffer->next;
        }

        if(buffer == nil) {
          INFO(info,8)
            ("no neighbour %d of element %d found: Assuming a boundary...\n", j, i);

          data->neigh[i][j] = -1;
        }
      }
    }
  }

  /****************************************************************************/
  /* now is the time to clean up                                              */
  /****************************************************************************/


  for(i = 0; i < data->n_total_vertices; i++) {
    buffer = list[i];

    while(buffer) {
      buffer2 = buffer->next;
      MEM_FREE(buffer, 1, VERT2ELEM);

      buffer = buffer2;
    }
  }

  MEM_FREE(list, data->n_total_vertices, VERT2ELEM *);

  return;
}


/****************************************************************************/
/*  sets the boundary of all edges without neigbour to DIRICHLET boundary   */
/*  type                                                                    */
/****************************************************************************/

static void dirichlet_boundary(MACRO_DATA *data)
{
  int i, j;

  data->boundary = MEM_ALLOC(data->n_macro_elements, SCHAR_NNEIGH);

  for (i = 0; i < data->n_macro_elements; i++)
    for (j = 0; j < N_NEIGH; j++)
      data->boundary[i][j] = data->neigh[i][j]>=0 ? INTERIOR : DIRICHLET;

  return;
}


/****************************************************************************/
/*  initialize and clear macro data structures                              */
/****************************************************************************/

static void init_macro_data(MACRO_DATA *data)
{
  FUNCNAME("init_macro_data");

  data->coords       = MEM_ALLOC(data->n_total_vertices, REAL_D);
  data->mel_vertices = MEM_ALLOC(data->n_macro_elements, INT_NVERT);
  data->neigh        = nil;
  data->boundary     = nil;
#if DIM == 3
  data->el_type      = nil;
#endif
  return;
}

#ifndef _ALBERTA_
static void free_macro_data(MACRO_DATA *data)
{
  int ne = data->n_macro_elements, nv = data->n_total_vertices;

  MEM_FREE(data->coords, nv, REAL_D);
  MEM_FREE(data->mel_vertices, ne, INT_NVERT);

  if(data->neigh) {
    MEM_FREE(data->neigh, ne, INT_NNEIGH);
    data->neigh = nil;
  }

  if(data->boundary) {
    MEM_FREE(data->boundary, ne, SCHAR_NNEIGH);
    data->boundary = nil;
  }

#if DIM == 3
  if(data->el_type) {
    MEM_FREE(data->el_type, ne, U_CHAR);
    data->el_type = nil;
  }
#endif

  return;
}
#endif

/****************************************************************************/
/* read data->neigh into mel[].neigh[]                                      */
/* fill opp_vertex values and do a check on neighbour relations             */
/****************************************************************************/

static void fill_neigh_info(MACRO_EL *mel, MACRO_DATA *data)
{
  FUNCNAME("fill_neigh_info");

  MACRO_EL      *neigh;
  int i, j, k, index;

  for (i = 0; i < data->n_macro_elements; i++)
  {
    for (j = 0; j < N_NEIGH; j++)
    {
      mel[i].neigh[j] =
        ((index=data->neigh[i][j]) >= 0) ? (mel+index) : nil;
#if NEIGH_IN_EL
      mel[i].el->neigh[j] = (index >= 0) ? mel[index].el : nil;
#endif
    }
  }

  for (i = 0; i < data->n_macro_elements; i++)
  {
    for (j = 0; j < N_NEIGH; j++)
    {
      if ((neigh = mel[i].neigh[j]))
      {
        for (k = 0; k < N_NEIGH; k++)
          if (neigh->neigh[k] == mel+i) break;

        TEST_EXIT(k < N_NEIGH) ("el %d is no neighbour of neighbour %d!\n",
                                mel[i].index, neigh->index);
        mel[i].opp_vertex[j] = k;
#if NEIGH_IN_EL
        mel[i].el->opp_vertex[j] = k;
#endif
      }
      else
      {
        // minus 1 is not allowed
        mel[i].opp_vertex[j] = 128;
#if NEIGH_IN_EL
        mel[i].el->opp_vertex[j] = -1;
#endif
      }
    }
  }
  return;
}


/****************************************************************************/
/* domain size                                                              */
/****************************************************************************/

static void calculate_size(MESH *mesh, MACRO_DATA *data)
{
  int i,j;
  REAL_D x_min, x_max;

  for (j = 0; j < DIM_OF_WORLD; j++)
  {
    x_min[j] =  1.E30;
    x_max[j] = -1.E30;
  }

  for (i = 0; i < mesh->n_vertices; i++)
  {
    for (j = 0; j < DIM_OF_WORLD; j++)
    {
      x_min[j] = MIN(x_min[j], data->coords[i][j]);
      x_max[j] = MAX(x_max[j], data->coords[i][j]);
    }
  }

  for (j = 0; j < DIM_OF_WORLD; j++)
    mesh->diam[j] = x_max[j] - x_min[j];

}

/***************************************************************************/
/*  macro_data2mesh():                                                     */
/*  copy macro data to the MESH structure "mesh" provided:                 */
/*  1) set most entries in "mesh"                                          */
/*  2) allocate macro elements and link them to "mesh"                     */
/*  3) assign DOFs                                                         */
/*  4) calculate the mesh size for "mesh->diam"                            */
/*                                                                         */
/*  Note: the entry "data->coords" is copied to a newly allocated array    */
/*  (the entire MACRO_DATA structure can thus be freed after use!)         */
/***************************************************************************/

static void macro_data2mesh(MESH *mesh, MACRO_DATA *data,
                            const BOUNDARY *(*bdry)(MESH *, int))
{
  FUNCNAME("macro_data2mesh");
  int i,j;
  MACRO_EL   *mel;
  DOF       **newdofs;
  REAL_D     *newcoords;

  TEST_EXIT(mesh) ("no mesh, mesh is nil pointer!\n");

  mesh->n_elements = mesh->n_hier_elements = mesh->n_macro_el = data->n_macro_elements;
  mesh->n_vertices = data->n_total_vertices;

  mel = mesh->first_macro_el = MEM_ALLOC(data->n_macro_elements, MACRO_EL);

  newdofs = MEM_ALLOC(data->n_total_vertices, DOF *);
  newcoords = MEM_ALLOC(data->n_total_vertices, REAL_D);

  for(i = 0; i < data->n_total_vertices; i++) {
    newdofs[i] = get_dof(mesh, VERTEX);

    for(j = 0; j < DIM_OF_WORLD; j++)
      newcoords[i][j] = data->coords[i][j];
  }

  ((MESH_MEM_INFO *)mesh->mem_info)->count = data->n_total_vertices;
  ((MESH_MEM_INFO *)mesh->mem_info)->coords = newcoords;

  for(i = 0; i < data->n_macro_elements; i++) {
    mel[i].el = get_element(mesh);

    mel[i].index = i;
    mel[i].el->mark = 0;

    for(j = 0; j < N_VERTICES; j++) {
      mel[i].coord[j] = newcoords[data->mel_vertices[i][j]];
      mel[i].el->dof[j] = newdofs[data->mel_vertices[i][j]];
    }
#if DIM > 1
    mel[i].el->new_coord = nil;
#endif
#if EL_INDEX
    mel[i].el->index = i;
#endif
#if DIM == 3
    mel[i].el_type = data->el_type ? data->el_type[i] : 0;
#if NEIGH_IN_EL
    mel[i].el->el_type = data->el_type ? data->el_type[i] : 0;
#endif
#endif
    if (i > 0) mel[i].last = &(mel[i-1]);
    if (i < mesh->n_macro_el - 1) mel[i].next = &(mel[i+1]);
  }
  mel[0].last = nil;
  mel[data->n_macro_elements-1].next = nil;

  calculate_size(mesh, data);

#if DIM > 1
  if(!bdry) bdry = default_boundary;
#endif

  fill_neigh_info(mel, data);
  fill_bound_info(mesh, data, bdry);
#if DIM > 1
  fill_more_boundary_dofs(mesh, bdry);
#endif
  /*
     for (i = 0; i < mesh->n_elements; i++)
     {
     MSG("vertex/edge bounds on element %2d: ", i);
      for (j = 0; j < N_VERTICES; j++)
     print_msg("%d%s", mel[i].bound[j], j < N_VERTICES-1 ? "," : " / ");
      for (j = 0; j < N_EDGES; j++)
     print_msg("%d%s", GET_BOUND(mel[i].boundary[N_FACES+j]),
                  j<N_EDGES-1?"," : "\n");
     }
   */

  if (mesh->n_dof[CENTER])
  {
    for (i = 0; i < mesh->n_macro_el; i++)
    {
      mel[i].el->dof[mesh->node[CENTER]] = get_dof(mesh, CENTER);
    }
  }

  MEM_FREE(newdofs, data->n_total_vertices, DOF *);

  return;
}


/****************************************************************************/
/*  read_indices()  reads DIM+1 indices from  file  into  id[0-DIM],        */
/*    returns true if DIM+1 inputs arguments could be read successfully by  */
/*    fscanf(), else false                                                  */
/****************************************************************************/

#if 0
static int read_indices(FILE *file, int id[])
{
  int i;

  for (i = 0; i <= DIM; i++)
    if (fscanf(file, "%d", id+i) != 1)
      return(false);
  return(true);
}
#endif

#define N_KEYS      9
#define N_MIN_KEYS  6
#if 0
static const char *keys[N_KEYS] = {"DIM",                 /*  0  */
                                   "DIM_OF_WORLD",        /*  1  */
                                   "number of vertices",  /*  2  */
                                   "number of elements",  /*  3  */
                                   "vertex coordinates",  /*  4  */
                                   "element vertices",    /*  5  */
                                   "element boundaries",  /*  6  */
                                   "element neighbours",  /*  7  */
                                   "element type"};       /*  8  */
#endif

#if 0
static int get_key_no(const char *key)
{
  int i;

  for (i = 0; i < N_KEYS; i++)
    if (!strcmp(keys[i], key)) return(i);

  return(-1);
}

static const char *read_key(const char *line)
{
  static char key[100];
  char         *k = key;

  while(isspace(*line)) line++;
  while((*k++ = *line++) != ':') ;
  *--k = '\0';

  return((const char *) key);
}

#endif

/****************************************************************************/
/*  read_macro_data():                                                      */
/*    read macro triangulation from ascii file in ALBERT format             */
/*    fills macro_data structure                                            */
/*    called by read_macro()                                                */
/****************************************************************************/
#if 0
static void read_macro_data(MACRO_DATA *macro_data, const char *filename)
{
  FUNCNAME("read_macro_data");
  FILE       *file;
  int dim, dow, nv, ne, i, j, ind[DIM+1];
  REAL dbl;
  char name[128], line[256];
  int line_no, n_keys, i_key, sort_key[N_KEYS], nv_key, ne_key;
  int key_def[N_KEYS] = {0,0,0,0,0,0,0,0,0};
  const char *key;

  TEST_EXIT(filename) ("no file specified; filename nil pointer\n");
  TEST_EXIT(strlen(filename) < (unsigned int) 127)
    ("can only handle filenames up to 127 characters\n");

  TEST_EXIT((file=fopen(filename,"r"))) ("cannot open file %s\n",filename);
  strncpy(name, filename, 127);

  /****************************************************************************/
  /*  looking for all keys in the macro file ...                              */
  /****************************************************************************/

  line_no = n_keys = 0;
  while (fgets(line, 255, file))
  {
    line_no++;
    if (!strchr(line, ':')) continue;
    key = read_key(line);
    i_key = get_key_no(key);
    TEST_EXIT(i_key >= 0)
      ("file %s: must not contain key %s on line %d\n",
      name, key, line_no);
    TEST_EXIT(!key_def[i_key])
      ("file %s: key %s defined second time on line %d\n", name, key, line_no);

    sort_key[n_keys++] = i_key;
    key_def[i_key] = true;
  }
  fclose(file);

  for (i_key = 0; i_key < N_MIN_KEYS; i_key++)
  {
    for (j = 0; j < n_keys; j++)
      if (sort_key[j] == i_key) break;
    TEST_EXIT(j < n_keys)
      ("file %s: You do not have specified data for %s in %s\n",
      name, keys[i_key]);

    for (j = 0; j < n_keys; j++)
      if (sort_key[j] == 2) break;
    nv_key = j;
    for (j = 0; j < n_keys; j++)
      if (sort_key[j] == 3) break;
    ne_key = j;

    switch(i_key)
    {
    case 0 :
    case 1 :
      TEST_EXIT(sort_key[i_key] < 2)
        ("file %s: You have to specify DIM or DIM_OF_WORLD before all other data\n",
        name);
      break;
    case 4 :
      TEST_EXIT(nv_key < i_key)
        ("file %s: Before reading data for %s, you have to specify the %s\n",
        name, keys[4], keys[2]);
      break;
    case 5 :
      TEST_EXIT(nv_key < i_key  &&  ne_key < i_key)
        ("file %s: Before reading data for %s, you have to specify the %s and %s\n",
        name, keys[5], keys[3], keys[2]);
    case 6 :
    case 7 :
    case 8 :
      TEST_EXIT(ne_key < i_key)
        ("file %s: Before reading data for %s, you have to specify the %s\n",
        name, keys[i_key], keys[3]);
    }
  }

  for (i_key = 0; i_key < N_KEYS; i_key++)
    key_def[i_key] = false;

  /****************************************************************************/
  /*  and now, reading data ...                                               */
  /****************************************************************************/

  TEST_EXIT((file=fopen(name,"r"))) ("cannot open file %s\n",name);

  for (i_key = 0; i_key < n_keys; i_key++)
  {
    switch(sort_key[i_key])
    {
    case 0 :
      TEST_EXIT(fscanf(file, "%*s %d", &dim) == 1)
        ("file %s: can not read DIM correctly\n", name);
      TEST_EXIT(dim == DIM) ("file %s: dimension = %d != DIM = %d\n", name, dim, DIM);
      key_def[0] = true;
      break;
    case 1 :
      TEST_EXIT(fscanf(file, "%*s %d", &dow) == 1)
        ("file %s: can not read DIM_OF_WORLD correctly\n", name);
      TEST_EXIT(dow == DIM_OF_WORLD)
        ("file %s: dimension of world = %d != DIM_OF_WORLD = %d\n", name, dow, DIM_OF_WORLD);

      key_def[1] = true;
      break;
    case 2 :
      TEST_EXIT(fscanf(file, "%*s %*s %*s %d", &nv) == 1)
        ("file %s: can not read number of vertices correctly\n", name);
      TEST_EXIT(nv > 0)
        ("file %s: number of vertices = %d must be bigger than 0\n", name, nv);

      macro_data->n_total_vertices = nv;
      key_def[2] = true;

      if(key_def[3])
        init_macro_data(macro_data);

      break;
    case 3 :
      TEST_EXIT(fscanf(file, "%*s %*s %*s %d", &ne) == 1)
        ("file %s: can not read number of elements correctly\n", name);
      TEST_EXIT(ne > 0)
        ("file %s: number of elements = %d must be bigger than 0\n", name, ne);

      macro_data->n_macro_elements = ne;
      key_def[3] = true;

      if(key_def[2])
        init_macro_data(macro_data);

      break;
    case 4 :
      fscanf(file, "%*s %*s");
      for (i = 0; i < nv; i++)
      {
        for (j = 0; j < DIM_OF_WORLD; j++)
        {
          TEST_EXIT(fscanf(file, "%lf", &dbl) == 1)
            ("file %s: error while reading coordinates, check file\n", name);

          macro_data->coords[i][j] = dbl;
        }
      }

      key_def[4] = true;
      break;
    case 5 :
      fscanf(file, "%*s %*s");
      /****************************************************************************/
      /* global index of vertices for each single element                         */
      /****************************************************************************/

      for (i = 0; i < ne; i++)
      {
        TEST_EXIT(read_indices(file, ind))
          ("file %s: can not read vertex indices of element %d\n",
          name, i);

        for (j = 0; j < N_VERTICES; j++)
          macro_data->mel_vertices[i][j] = ind[j];
      }

      key_def[5] = true;
      break;
    case 6 :
      fscanf(file, "%*s %*s");
      /****************************************************************************/
      /* read boundary type of each vertex/edge/face (in 1d/2d/3d)                */
      /****************************************************************************/

      macro_data->boundary = MEM_ALLOC(ne, SCHAR_NNEIGH);

      for (i = 0; i < ne; i++) {
        TEST_EXIT(read_indices(file, ind)) ("file %s: can not read boundary types of element %d\n", name, i);

        for(j = 0; j < N_NEIGH; j++)
          macro_data->boundary[i][j] = (S_CHAR) ind[j];
      }

      key_def[6] = true;
      break;
    case 7 :
      fscanf(file, "%*s %*s");
      /****************************************************************************/
      /* read neighbour indices:                                                  */
      /****************************************************************************/

      macro_data->neigh = MEM_ALLOC(ne, INT_NNEIGH);

      for (i = 0; i < ne; i++) {
        TEST_EXIT(read_indices(file, ind)) ("file %s: can not read neighbour info of element %d\n", name, i);

        for(j = 0; j < N_NEIGH; j++)
          macro_data->neigh[i][j] = ind[j];

      }

      key_def[7] = true;
      break;
    case 8 :
      fscanf(file, "%*s %*s");
      /****************************************************************************/
      /* el_type is handled just like bound and neigh above                       */
      /****************************************************************************/

#if DIM < 3
      WARNING("File %s: element type only used in 3d; will ignore data for el_type\n", name);
#endif
#if DIM == 3
      macro_data->el_type = MEM_ALLOC(ne, U_CHAR);

      for (i = 0; i < ne; i++)
      {
        TEST_EXIT(fscanf(file, "%d", &j) == 1) ("file %s: can not read el_type of element %d\n", name, i);

        macro_data->el_type[i] = (U_CHAR) j;
      }

      key_def[8] = true;
#endif
      break;
    }
  }
  fclose(file);

  return;
}

/****************************************************************************/
/* read macro triangulation from file "filename" into macro_data data in    */
/* native binary format                                                     */
/****************************************************************************/

static void read_macro_data_bin(MACRO_DATA *macro_data, const char *name)
{
  FUNCNAME("read_macro_data_bin");

  FILE *file;
  int i,length;
  char *s;
  char record_written;


  TEST_EXIT(file = fopen(name, "rb")) ("cannot open file %s\n", name);

  length = MAX(strlen(ALBERT_VERSION)+1, 21);
  s = MEM_ALLOC(length, char);

  fread(s, sizeof(char), length, file);
  TEST_EXIT(!strncmp(s, "ALBERT", 6)) ("file %s: unknown file id:\"%s\"\n",name,s);

  MEM_FREE(s, length, char);

  fread(&i, sizeof(int), 1, file);
  TEST_EXIT(i == sizeof(REAL))("file %s: wrong sizeof(REAL) %d\n", name, i);

            fread(&i, sizeof(int), 1, file);
  TEST_EXIT(i == DIM) ("file %s: dimension = %d != DIM = %d\n", name, i, DIM);

  fread(&i, sizeof(int), 1, file);
  TEST_EXIT(i == DIM_OF_WORLD)
    ("file %s: dimension of world = %d != DIM_OF_WORLD = %d\n", name, i, DIM_OF_WORLD);

  fread(&(macro_data->n_total_vertices), sizeof(int), 1, file);
  TEST_EXIT(macro_data->n_total_vertices > 0)
    ("file %s: number of vertices = %d must be bigger than 0\n", name, macro_data->n_total_vertices);

  fread(&(macro_data->n_macro_elements), sizeof(int), 1, file);
  TEST_EXIT(macro_data->n_macro_elements > 0)
    ("file %s: number of elements = %d must be bigger than 0\n", name, macro_data->n_macro_elements);

  init_macro_data(macro_data);

  fread(macro_data->coords, sizeof(REAL_D), macro_data->n_total_vertices, file);
  fread(macro_data->mel_vertices, sizeof(INT_NVERT), macro_data->n_macro_elements, file);

  fread(&record_written, sizeof(char), 1, file);
  if(record_written) {
    macro_data->boundary = MEM_ALLOC(macro_data->n_macro_elements, SCHAR_NNEIGH);
    fread(macro_data->boundary, sizeof(SCHAR_NNEIGH), macro_data->n_macro_elements, file);
  }

  fread(&record_written, sizeof(char), 1, file);
  if(record_written) {
    macro_data->neigh = MEM_ALLOC(macro_data->n_macro_elements, INT_NNEIGH);
    fread(macro_data->neigh, sizeof(INT_NNEIGH), macro_data->n_macro_elements, file);
  }

#if DIM == 3
  fread(&record_written, sizeof(char), 1, file);
  if(record_written ) {
    macro_data->el_type = MEM_ALLOC(macro_data->n_macro_elements, U_CHAR);
    fread(macro_data->el_type, sizeof(U_CHAR), macro_data->n_macro_elements, file);
  }
#endif


  s = MEM_ALLOC(5, char);

  TEST_EXIT(fread(s, sizeof(char), 4, file) == 4)
    ("file %s: problem while reading FILE END MARK\n", name);

  TEST_EXIT(!strncmp(s, "EOF.", 4)) ("file %s: no FILE END MARK\n", name);
  MEM_FREE(s, 5, char);

  fclose(file);
}


/****************************************************************************/
/* Some routines needed for interaction with xdr-files                      */
/* WARNING: These will need to be adapted if ALBERT data types REAL, REAL_D */
/* ,etc. change!                                                            */
/****************************************************************************/
#endif

#if 0
static bool_t xdr_REAL(XDR *xdr, REAL *rp)
{
  return (xdr_double(xdr,rp));
}

static bool_t xdr_U_CHAR(XDR *xdr, U_CHAR *ucp)
{
  return (xdr_u_char(xdr,ucp));
}

static bool_t xdr_S_CHAR(XDR *xdr, S_CHAR *cp)
{
  return (xdr_char(xdr,(char *)cp));
}

static bool_t xdr_REAL_D(XDR *xdr, REAL_D *dp)
{
  return (xdr_vector(xdr, (char *)dp, DIM_OF_WORLD, sizeof(REAL), (xdrproc_t) xdr_REAL));
}

static bool_t xdr_INT_NVERT(XDR *xdr, INT_NVERT *dp)
{
  return (xdr_vector(xdr, (char *)dp, N_VERTICES, sizeof(int), (xdrproc_t) xdr_int));
}

static bool_t xdr_INT_NNEIGH(XDR *xdr, INT_NNEIGH *dp)
{
  return (xdr_vector(xdr, (char *)dp, N_NEIGH, sizeof(int), (xdrproc_t) xdr_int));
}

static bool_t xdr_SCHAR_NNEIGH(XDR *xdr, SCHAR_NNEIGH *dp)
{
  return (xdr_vector(xdr, (char *)dp, N_NEIGH, sizeof(S_CHAR), (xdrproc_t) xdr_S_CHAR));
}

static int read_xdr_file(void *xdr_file, void *buffer, u_int size)
{
  return ((int)fread(buffer, 1, (size_t)size, (FILE *)xdr_file));
}

static int write_xdr_file(void *xdr_file, void *buffer, u_int size)
{
  return (fwrite(buffer, (size_t)size, 1, (FILE *)xdr_file) == 1 ? (int)size : 0);
}
#endif

#if 0
static XDR *xdr_open_file(const char *filename, enum xdr_op mode)
{
  XDR *xdr;
  FILE *xdr_file;

  if (!(xdr = MEM_ALLOC(1,XDR)))
  {
    ERROR("can't allocate memory for xdr pointer.\n");

    return NULL;
  }

  if ((xdr_file = fopen(filename, mode == XDR_DECODE ? "r" : "w")))
  {
    xdrrec_create(xdr, 65536, 65536, (char *)xdr_file,
                  (int (*)(char*, char* , int ))read_xdr_file,
                  (int (*)(char*, char* , int ))write_xdr_file);

    xdr->x_op = mode;
    xdr->x_public = (caddr_t)xdr_file;

    if (mode == XDR_DECODE)
      xdrrec_skiprecord(xdr);

    return xdr;
  }
  else
  {
    ERROR("error opening xdr file.\n");

    MEM_FREE(xdr,1,XDR);

    return NULL;
  }
}


static int xdr_close_file(XDR *xdr)
{
  if (!xdr)
  {
    ERROR("NULL xdr pointer.\n");
    return 0;
  }

  if (xdr->x_op == XDR_ENCODE)
    xdrrec_endofrecord(xdr, 1);

  if (fclose((FILE *) xdr->x_public))
    ERROR("error closing file.\n");

  xdr_destroy(xdr);

  MEM_FREE(xdr,1,XDR);

  return 1;
}


/****************************************************************************/
/*  read_macro_data_xdr():                                                  */
/*    read macro triangulation from file in xdr-format                      */
/*    fills macro_data structure                                            */
/*    called by ....?                                                       */
/****************************************************************************/

static void read_macro_data_xdr(MACRO_DATA *macro_data, const char *name)
{
  FUNCNAME("read_macro_data_xdr");

  XDR     *xdrp;
  int length, dim, dow, nv, ne;
  char    *s;

  bool_t record_written;

  caddr_t array_loc;


  TEST_EXIT(name) ("no file specified; filename nil pointer\n");

  if (!(xdrp = xdr_open_file(name, XDR_DECODE)))
    ERROR_EXIT("cannot open file %s\n",name);

  length = MAX(strlen(ALBERT_VERSION)+1,21);     /* length with terminating \0 */
  s = MEM_ALLOC(length, char);

  TEST_EXIT(xdr_string(xdrp, &s, length)) ("file %s: could not read file id\n", name);
  TEST_EXIT(!strncmp(s, "ALBERT", 6)) ("file %s: unknown file id: \"%s\"\n",name,s);

  MEM_FREE(s, length, char);

  TEST_EXIT(xdr_int(xdrp, &dim)) ("file %s: could not read dimension correctly\n",name);
  TEST_EXIT(dim == DIM) ("file %s: dimension = %d != DIM = %d\n", name, dim, DIM);


  TEST_EXIT(xdr_int(xdrp, &dow)) ("file %s: could not read dimension of world correctly\n",name);
  TEST_EXIT(dow == DIM_OF_WORLD)
    ("file %s: dimension of world = %d != DIM_OF_WORLD = %d\n", name, dow, DIM_OF_WORLD);

  TEST_EXIT(xdr_int(xdrp, &nv))
    ("file %s: can not read number of vertices correctly\n", name);
  TEST_EXIT(nv > 0)
    ("file %s: number of vertices = %d must be bigger than 0\n", name, nv);
  macro_data->n_total_vertices = nv;


  TEST_EXIT(xdr_int(xdrp, &ne))
    ("file %s: can not read number of elements correctly\n", name);
  TEST_EXIT(ne > 0)
    ("file %s: number of elements = %d must be bigger than 0\n", name, ne);
  macro_data->n_macro_elements = ne;

  init_macro_data(macro_data);

  array_loc=(caddr_t) macro_data->coords;
  TEST_EXIT(xdr_array(xdrp, &array_loc, (u_int *) &(macro_data->n_total_vertices), (u_int) macro_data->n_total_vertices, sizeof(REAL_D), (xdrproc_t) xdr_REAL_D))
    ("file %s: error while reading coordinates, check file\n", name);

  array_loc=(caddr_t) macro_data->mel_vertices;
  TEST_EXIT(xdr_array(xdrp, &array_loc, (u_int *) &(macro_data->n_macro_elements), (u_int) macro_data->n_macro_elements, sizeof(INT_NVERT), (xdrproc_t) xdr_INT_NVERT))
    ("file %s: can not read vertex indices\n", name);

  TEST_EXIT(xdr_bool(xdrp, &record_written))
    ("file %s: could not determine whether to allocate memory for boundaries\n", name);
  if(record_written) {
    macro_data->boundary = MEM_ALLOC(ne, SCHAR_NNEIGH);

    array_loc=(caddr_t) macro_data->boundary;
    TEST_EXIT(xdr_array(xdrp, &array_loc, (u_int *) &(macro_data->n_macro_elements), (u_int) macro_data->n_macro_elements, sizeof(SCHAR_NNEIGH), (xdrproc_t) xdr_SCHAR_NNEIGH))
      ("file %s: could not read boundary types\n",name);
  }

  TEST_EXIT(xdr_bool(xdrp, &record_written))
    ("file %s: could not determine whether to allocate memory for neighbours\n", name);
  if(record_written) {
    macro_data->neigh = MEM_ALLOC(ne, INT_NNEIGH);

    array_loc=(caddr_t) macro_data->neigh;
    TEST_EXIT(xdr_array(xdrp, &array_loc, (u_int *) &(macro_data->n_macro_elements), (u_int) macro_data->n_macro_elements, sizeof(INT_NNEIGH), (xdrproc_t) xdr_INT_NNEIGH))
      ("file %s: could not read neighbor info\n",name);
  }

#if DIM == 3
  TEST_EXIT(xdr_bool(xdrp, &record_written))
    ("file %s: could not determine whether to allocate memory for element types\n", name);
  if(record_written) {
    macro_data->el_type = MEM_ALLOC(ne, U_CHAR);

    array_loc=(caddr_t) macro_data->el_type;
    TEST_EXIT(xdr_array(xdrp, &array_loc, (u_int *) &(macro_data->n_macro_elements), (u_int) macro_data->n_macro_elements, sizeof(U_CHAR), (xdrproc_t) xdr_U_CHAR))
      ("file %s: can not read element types\n", name);
  }
#endif

  xdr_close_file(xdrp);

  return;
}


/****************************************************************************/
/* write raw macro triangulation in "data" to "filename" in standard ALBERT */
/* key format                                                               */
/****************************************************************************/

int write_macro_data(MACRO_DATA *data, const char *filename)
{
  FUNCNAME("write_macro_data");

  FILE *macro_file;
  int i,j;

  if (!(macro_file = fopen(filename, "w")))
  {
    ERROR("could not open file %s for writing\n", filename);
    return(0);
  }

  fprintf(macro_file, "DIM: %d\n", DIM);
  fprintf(macro_file, "DIM_OF_WORLD: %d\n\n", DIM_OF_WORLD);

  fprintf(macro_file, "number of vertices: %d\n", data->n_total_vertices);
  fprintf(macro_file, "number of elements: %d\n\n", data->n_macro_elements);

  fprintf(macro_file, "vertex coordinates:\n");
  for(i = 0; i < data->n_total_vertices; i++)
    for (j = 0; j < DIM_OF_WORLD; j++)
      fprintf(macro_file, "%17.10e%s", data->coords[i][j],
              j < DIM_OF_WORLD-1 ? " " : "\n");

  fprintf(macro_file, "\nelement vertices:\n");
  for(i = 0; i < data->n_macro_elements; i++)
    for (j = 0; j < N_VERTICES; j++)
      fprintf(macro_file, "%5d%s", data->mel_vertices[i][j],
              j < N_VERTICES-1 ? " " : "\n");

  fprintf(macro_file, "\nelement boundaries:\n");
  for(i = 0; i < data->n_macro_elements; i++)
    for (j = 0; j < N_NEIGH; j++)
      fprintf(macro_file, "%4d%s", data->boundary[i][j],
              j < N_NEIGH-1 ? " " : "\n");

#if DIM == 3
  if (data->el_type)
  {
    fprintf(macro_file, "\nelement type:\n");
    for(i = 0; i < data->n_macro_elements; i++)
      fprintf(macro_file, "%d%s", data->el_type[i],  ((i+1)%20) ? " " : "\n");
  }
#endif

  if (!(i%20))
    fprintf(macro_file, "\n");

  fclose(macro_file);

  INFO(2,2) ("wrote macro file %s\n", filename);

  return(1);
}


/****************************************************************************/
/* write raw macro triangulation in "data" to "filename" in native binary   */
/* format                                                                   */
/****************************************************************************/

extern int write_macro_data_bin(MACRO_DATA *data, const char *filename)
{
  FUNCNAME("write_macro_data_bin");

  FILE *file;
  int i;
  char record_written=1;
  char record_not_written=0;


  if(!data) {
    ERROR("no data - no file created\n");
    return(0);
  }

  if (!(file = fopen(filename, "wb")))
  {
    ERROR("cannot open file %s\n",filename);
    return(0);
  }

  fwrite(ALBERT_VERSION, sizeof(char), strlen(ALBERT_VERSION)+1, file);

  i = sizeof(REAL);
  fwrite(&i, sizeof(int), 1, file);

  i = DIM;
  fwrite(&i, sizeof(int), 1, file);

  i = DIM_OF_WORLD;
  fwrite(&i, sizeof(int), 1, file);

  fwrite(&(data->n_total_vertices), sizeof(int), 1, file);
  fwrite(&(data->n_macro_elements), sizeof(int), 1, file);

  fwrite(data->coords, sizeof(REAL_D), data->n_total_vertices, file);
  fwrite(data->mel_vertices, sizeof(INT_NVERT), data->n_macro_elements, file);

  if(data->boundary) {
    fwrite(&record_written, sizeof(char), 1, file);
    fwrite(data->boundary, sizeof(SCHAR_NNEIGH), data->n_macro_elements, file);
  }
  else fwrite(&record_not_written, sizeof(char), 1, file);

  if(data->neigh) {
    fwrite(&record_written, sizeof(char), 1, file);
    fwrite(data->neigh, sizeof(INT_NNEIGH), data->n_macro_elements, file);
  }
  else fwrite(&record_not_written, sizeof(char), 1, file);

#if DIM == 3
  if (data->el_type) {
    fwrite(&record_written, sizeof(char), 1, file);
    fwrite(data->el_type, sizeof(U_CHAR), data->n_macro_elements, file);
  }
  else fwrite(&record_not_written, sizeof(char), 1, file);
#endif

  fwrite("EOF.", sizeof(char), 4, file);
  fclose(file);

  INFO(2,2) ("wrote macro binary-file %s\n", filename);

  return(1);
}


/****************************************************************************/
/* write raw macro triangulation in "data" to "filename" in xdr format      */
/****************************************************************************/

extern int write_macro_data_xdr(MACRO_DATA *data, const char *filename)
{
  FUNCNAME("write_macro_data_xdr");

  XDR *xdrp;
  int i, length;
  char *s;
  bool_t record_written=1;
  bool_t record_not_written=0;

  caddr_t array_loc;


  if(!data) {
    ERROR("no data - no file created\n");
    return(0);
  }

  if (!(xdrp = xdr_open_file(filename, XDR_ENCODE)))
  {
    ERROR("cannot open file %s\n",filename);
    return(0);
  }

  length = MAX(strlen(ALBERT_VERSION) + 1, 5);  /* length with terminating \0 */
  s=MEM_ALLOC(length, char);
  strcpy(s, ALBERT_VERSION);
  xdr_string(xdrp, &s, length);
  MEM_FREE(s, length, char);

  i = DIM;
  xdr_int(xdrp, &i);

  i = DIM_OF_WORLD;
  xdr_int(xdrp, &i);

  xdr_int(xdrp, &(data->n_total_vertices));
  xdr_int(xdrp, &(data->n_macro_elements));

  array_loc=(caddr_t) data->coords;
  xdr_array(xdrp, &array_loc, (u_int *) &(data->n_total_vertices), (u_int) data->n_total_vertices, sizeof(REAL_D), (xdrproc_t) xdr_REAL_D);

  array_loc=(caddr_t) data->mel_vertices;
  xdr_array(xdrp, &array_loc, (u_int *) &(data->n_macro_elements), (u_int) data->n_macro_elements, sizeof(INT_NVERT), (xdrproc_t) xdr_INT_NVERT);

  if(data->boundary) {
    xdr_bool(xdrp, &record_written);
    array_loc=(caddr_t) data->boundary;
    xdr_array(xdrp, &array_loc, (u_int *) &(data->n_macro_elements), (u_int) data->n_macro_elements, sizeof(SCHAR_NNEIGH), (xdrproc_t) xdr_SCHAR_NNEIGH);
  }
  else xdr_bool(xdrp, &record_not_written);

  if(data->neigh) {
    xdr_bool(xdrp, &record_written);
    array_loc=(caddr_t) data->neigh;
    xdr_array(xdrp, &array_loc, (u_int *) &(data->n_macro_elements), (u_int) data->n_macro_elements, sizeof(INT_NNEIGH), (xdrproc_t) xdr_INT_NNEIGH);
  }
  else xdr_bool(xdrp, &record_not_written);

#if DIM == 3
  if (data->el_type) {
    xdr_bool(xdrp, &record_written);
    array_loc=(caddr_t) data->el_type;
    xdr_array(xdrp, &array_loc, (u_int *) &(data->n_macro_elements), (u_int) data->n_macro_elements, sizeof(U_CHAR), (xdrproc_t) xdr_U_CHAR);
  }
  else xdr_bool(xdrp, &record_not_written);
#endif

  xdr_close_file(xdrp);

  INFO(2,2) ("wrote macro xdr-file %s\n", filename);

  return(1);
}


/****************************************************************************/
/* supported file types for macro data files:                               */
/****************************************************************************/

//typedef enum {ascii_format, binary_format, xdr_format} macro_format;


/****************************************************************************/
/* read macro triangulation from file "filename"                            */
/****************************************************************************/

static void read_macro_master(MESH *mesh, const char *filename,
                              const BOUNDARY *(*bdry)(MESH *, int), macro_format format)
{
  FUNCNAME("read_macro_master");
  MACRO_DATA macro_data[1];

  TEST_EXIT(filename) ("no file specified; filename nil pointer\n");
  TEST_EXIT(mesh) ("no mesh specified; mesh nil pointer\n");

  switch(format) {
  case ascii_format :
    read_macro_data(macro_data, filename);
    break;
  case binary_format :
    read_macro_data_bin(macro_data, filename);
    break;
  case xdr_format :
    read_macro_data_xdr(macro_data, filename);
  }

  if(!macro_data->neigh) compute_neigh_fast(macro_data);
  if(!macro_data->boundary) dirichlet_boundary(macro_data);

  {
    char filenew[128];
    strncpy(filenew, filename, 128); filenew[127] = 0;
    strncat(filenew, ".new", 128);   filenew[127] = 0;
    macro_test(macro_data, filenew);
  }

  macro_data2mesh(mesh, macro_data, bdry);
  check_mesh(mesh);

  free_macro_data(macro_data);

  return;
}

#endif

static void cleanup_write_macro(MESH *mesh, MACRO_DATA *data,
                                DOF_INT_VEC *dof_vert_ind,
                                TRAVERSE_STACK *stack)
{
  free_macro_data(data);
  free_dof_int_vec(dof_vert_ind);
  free_traverse_stack(stack);
}


/****************************************************************************/
/* attempt to find a DOF_ADMIN structure which stores vertex DOFs           */
/****************************************************************************/

static const DOF_ADMIN *get_vertex_admin(DOF_ADMIN **admins, int n_admin)
{
  int i;
  const DOF_ADMIN *admin = nil;

  for (i = 0; i < n_admin; i++)
  {
    if (admins[i]->n_dof[VERTEX])
    {
      if (!admin)
        admin = admins[i];
      else if (admins[i]->size < admin->size)
        admin = admins[i];
    }
  }
  return(admin);
}


/****************************************************************************/
/* mesh2macro_data(): counterpart to macro_data2mesh above. This routine    */
/* converts the information stored in the leaf elements of mesh to the raw  */
/* data type MACRO_DATA.                                                    */
/****************************************************************************/

static int mesh2macro_data(MESH *mesh, MACRO_DATA *data)
{
  FUNCNAME("mesh2macro_data");

  TRAVERSE_STACK  *stack;
  FLAGS fill_flag = CALL_LEAF_EL|FILL_COORDS|FILL_BOUND|FILL_NEIGH;
  const DOF_ADMIN *admin;
  FE_SPACE fe_space = {"write fe_space", nil, nil};
  EL_INFO         *el_info;
  DOF_INT_VEC     *dof_vert_ind;
  int n0, ne, nv, i, j, *vert_ind = nil;
#if DIM == 3
  U_CHAR write_el_type;
#endif

  if (!(admin = get_vertex_admin(mesh->dof_admin, mesh->n_dof_admin)))
  {
    ERROR("mesh %s: no dof admin for vertices\n", mesh->name);
    return(0);
  }
  n0 = admin->n0_dof[VERTEX];
  fe_space.admin = admin;

  dof_vert_ind = get_dof_int_vec("vertex indices", &fe_space);
  GET_DOF_VEC(vert_ind, dof_vert_ind);
  FOR_ALL_DOFS(admin, vert_ind[dof] = -1);

  data->n_macro_elements = mesh->n_elements;
  data->n_total_vertices = mesh->n_vertices;
  init_macro_data(data);

  nv = ne = 0;
#if DIM == 3
  write_el_type = false;
#endif

  stack = get_traverse_stack();

  /****************************************************************************/
  /* The first pass counts elements and vertices, checks these against the    */
  /* entries of mesh->n_elements, mesh->n_vertices, and fills data->coords.   */
  /* A check on whether an element has nonzero el_type is also done.          */
  /****************************************************************************/
  for(el_info = traverse_first(stack, mesh, -1, CALL_LEAF_EL | FILL_COORDS);
      el_info;
      el_info = traverse_next(stack, el_info))
  {
    for (i = 0; i < N_VERTICES; i++)
    {
      if (vert_ind[el_info->el->dof[i][n0]] == -1)
      {
        /****************************************************************************/
        /* assign a global index to each vertex                                     */
        /****************************************************************************/
        vert_ind[el_info->el->dof[i][n0]] = nv;

        for (j = 0; j < DIM_OF_WORLD; j++)
          data->coords[nv][j] = el_info->coord[i][j];

        nv++;

        if(nv > mesh->n_vertices) {
          cleanup_write_macro(mesh, data, dof_vert_ind, stack);
          ERROR("mesh %s: n_vertices (==%d) is too small! Writing aborted\n",
                mesh->name, mesh->n_vertices);
          return(0);
        }
      }
    }

    ne++;

    if(ne > mesh->n_elements) {
      cleanup_write_macro(mesh, data, dof_vert_ind, stack);
      ERROR("mesh %s: n_elements (==%d) is too small! Writing aborted\n",
            mesh->name, mesh->n_elements);
      return(0);
    }
#if DIM == 3
    if (EL_TYPE(el_info->el, el_info)) write_el_type = true;
#endif
  } // end traverse next

  if(ne < mesh->n_elements) {
    cleanup_write_macro(mesh, data, dof_vert_ind, stack);
    ERROR("mesh %s: n_elements (==%d) is too large: only %d leaf elements counted -- writing aborted\n",
          mesh->name, mesh->n_elements, ne);
    return(0);
  }
  if(nv < mesh->n_vertices) {
    cleanup_write_macro(mesh, data, dof_vert_ind, stack);
    ERROR("mesh %s: n_vertices (==%d) is too large: only %d vertices counted --  writing of mesh aborted\n",
          mesh->name, mesh->n_vertices, nv);
    return(0);
  }

  data->boundary = MEM_ALLOC(ne, SCHAR_NNEIGH);
#if DIM == 3
  if(write_el_type) data->el_type = MEM_ALLOC(ne, U_CHAR);
#endif

  ne = 0;

  /****************************************************************************/
  /* The second pass assigns mel_vertices, boundary, and if necessary el_type */
  /****************************************************************************/
  for(el_info = traverse_first(stack, mesh, -1, fill_flag);
      el_info;
      el_info = traverse_next(stack, el_info)) {

    for (i = 0; i < N_VERTICES; i++)
      data->mel_vertices[ne][i] = vert_ind[el_info->el->dof[i][n0]];

    for (i = 0; i < N_NEIGH; i++)
#if DIM == 1
      data->boundary[ne][i] = el_info->bound[i];
#else
      data->boundary[ne][i] = GET_BOUND(el_info->boundary[i]);
#endif

#if DIM == 3
    if(write_el_type) data->el_type[ne] = EL_TYPE(el_info->el, el_info);
#endif

    ++ne;
  }

  free_dof_int_vec(dof_vert_ind);
  free_traverse_stack(stack);

  return(1);
}

/****************************************************************************/
/* write_macro() writes the current mesh (at the level of leaf elements) as */
/* a macro triangulation to the specified file                              */
/****************************************************************************/
#if 0
static int write_macro_master(MESH *mesh, const char *filename,
                              macro_format format)
{
  FUNCNAME("write_macro_master");

  int result=0;
  MACRO_DATA data[1];

  if (!filename)
  {
    ERROR("no filename specified, filename is nil pointer\n");
    return(0);
  }

  if (!mesh)
  {
    ERROR("no mesh specified, mesh is nil pointer\n");
    return(0);
  }

  if(!mesh2macro_data(mesh, data)) return(0);

  switch(format) {
  case ascii_format :
    result=write_macro_data(data, filename);
    break;
  case binary_format :
    result=write_macro_data_bin(data, filename);
    break;
  case xdr_format :
    result=write_macro_data_xdr(data, filename);
  }

  free_macro_data(data);

  return(result);
}


/****************************************************************************/
/* These routines are available to the user:                                */
/****************************************************************************/

extern void read_macro(MESH *mesh, const char *filename,
                       const BOUNDARY *(*bdry)(MESH *, int))
{
  read_macro_master(mesh, filename, bdry, ascii_format);
}

extern void read_macro_bin(MESH *mesh, const char *filename,
                           const BOUNDARY *(*bdry)(MESH *, int))
{
  read_macro_master(mesh, filename, bdry, binary_format);
}

extern void read_macro_xdr(MESH *mesh, const char *filename,
                           const BOUNDARY *(*bdry)(MESH *, int))
{
  read_macro_master(mesh, filename, bdry, xdr_format);
}


extern int write_macro(MESH *mesh, const char *filename)
{
  return(write_macro_master(mesh, filename, ascii_format));
}

extern int write_macro_bin(MESH *mesh, const char *filename)
{
  return(write_macro_master(mesh, filename, binary_format));
}

extern int write_macro_xdr(MESH *mesh, const char *filename)
{
  return(write_macro_master(mesh, filename, xdr_format));
}


extern MESH *check_and_get_mesh(int dim, int dow, int neigh, int index,
                                const char *version, const char *name,
                                void (*init_dof_admins)(MESH *),
                                void (*init_leaf_data)(LEAF_DATA_INFO *))
{
  FUNCNAME("check_and_get_mesh");
  int error = 0;

  if (dim != DIM)
  {
    ERROR("%s = %d, but you are using a lib with %s = %d\n",
          "DIM", dim, "DIM", DIM);
    error++;
  }
  if (dow != DIM_OF_WORLD)
  {
    ERROR("%s = %d, but you are using a lib with %s = %d\n",
          "DIM_OF_WORLD", dow, "DIM_OF_WORLD", DIM_OF_WORLD);
    error++;
  }
  if (neigh != NEIGH_IN_EL)
  {
    ERROR("%s = %d, but you are using a lib with %s = %d\n",
          "NEIGH_IN_EL", neigh, "NEIGH_IN_EL", NEIGH_IN_EL);
    error++;
  }
  if (index != EL_INDEX)
  {
    ERROR("%s = %d, but you are using a lib with %s = %d\n",
          "EL_INDEX", index, "EL_INDEX", EL_INDEX);
    error++;
  }
  if (strcmp(version,ALBERT_VERSION))
  {
    ERROR("you are using %s but a lib with %s\n", version, ALBERT_VERSION);
    error++;
  }
  if (error) exit(1);

  return(get_mesh(name, init_dof_admins, init_leaf_data));
}

#endif
