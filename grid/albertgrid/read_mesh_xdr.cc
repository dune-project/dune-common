// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
/****************************************************************************/
/* ALBERT:   an Adaptive multi Level finite element toolbox using           */
/*           Bisectioning refinement and Error control by Residual          */
/*           Techniques                                                     */
/*                                                                          */
/* file:                                                                    */
/*                                                                          */
/*                                                                          */
/* description:                                                             */
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
/*     (c) by A. Schmidt and K.G. Siebert (1996-1999)                       */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/* to_do :  number of dofs depending on node                                */
/* nach read_macro, damit dort pointer auf mel, v vorhanden sind!!!         */
/*    (fuers naechste Schreiben)                                            */
/****************************************************************************/


#include <string.h>
#include <rpc/types.h>
#include <rpc/xdr.h>

namespace AlbertRead {

  static XDR        *xdrp;


  /*
     XDR-Routinen zur Uebertragung von ALBERT-Datentypen

     muessen bei abgeaenderter Definition auch veraendert werden !!!

     akt. Def.:  REAL   = double
                 U_CHAR = unsigned char
                 S_CHAR = signed char
                 DOF    = int
   */

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

  static bool_t xdr_DOF(XDR *xdr, DOF *dp)
  {
    return (xdr_int(xdr,dp));
  }
#else
  static bool_t xdr_REAL(XDR *xdr, double *rp)
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

  static bool_t xdr_DOF(XDR *xdr, DOF* dp)
  {
    return (xdr_int(xdr,dp));
  }
#endif



  static int read_xdr_file(void *file, void *buffer, u_int size)
  {
    return ((int)fread(buffer, 1, (size_t)size, (FILE *)file));
  }

  static int write_xdr_file(void *file, void *buffer, u_int size)
  {
    return (fwrite(buffer, (size_t)size, 1, (FILE *)file) == 1 ? (int)size : 0);
  }


  static FILE *file;

  static XDR *xdr_open_file(const char *fn, enum xdr_op mode)
  {
    XDR *xdr;

    if (!(xdr = MEM_ALLOC(1,XDR)))
    {
      ERROR("can't allocate memory for xdr pointer.\n");

      return NULL;
    }

    if ((file = fopen(fn, mode == XDR_DECODE ? "r" : "w")))
    {
      xdrrec_create(xdr, 65536, 65536, (char *)file,
                    (int (*)(char *, char *, int ))read_xdr_file,
                    (int (*)(char *, char *, int ))write_xdr_file);

      xdr->x_op = mode;
      xdr->x_public = (caddr_t)file;

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

    xdr_destroy(xdr);

    if (fclose(file))
      ERROR("error closing file.\n");

    MEM_FREE(xdr,1,XDR);

    return 1;
  }


  /****************************************************************************/

  static DOF_ADMIN  *admin = nil;
  static MESH       *mesh = nil;
  static U_CHAR preserve_coarse_dofs;

  static int n_vert_dofs;
  static DOF        **vert_dofs;

#if DIM > 1
  static int n_edge_dofs;
  static DOF        **edge_dofs;
#endif

#if DIM == 3
  static int n_face_dofs;
  static DOF        **face_dofs;
#endif

  static EL *read_el_recursive(EL *);
  extern DOF_ADMIN *get_dof_admin(MESH *, const char *, const int [DIM+1]);

  /****************************************************************************/

  static void read_dof_admins_xdr(MESH *mesh)
  {
    FUNCNAME("read_dof_admins_xdr");
    int i, n_dof_admin, iadmin, used_count;
    int n_dof_el, n_dof[DIM+1], n_node_el, node[DIM+1];
    int a_n_dof[DIM+1];
    char     *name;


    xdr_int(xdrp, &n_dof_el);
    xdr_vector(xdrp, (char *)n_dof, DIM+1, sizeof(int), (xdrproc_t) xdr_int);
    xdr_int(xdrp, &n_node_el);
    xdr_vector(xdrp, (char *)node, DIM+1, sizeof(int), (xdrproc_t) xdr_int);
    /* use data later for check */

    xdr_int(xdrp, &n_dof_admin);
    for (iadmin = 0; iadmin < n_dof_admin; iadmin++)
    {
      xdr_vector(xdrp, (char *)a_n_dof, DIM+1, sizeof(int),
                 (xdrproc_t) xdr_int);
      xdr_int(xdrp, &used_count);

      xdr_int(xdrp, &i);              /* length without terminating \0 */
      name = MEM_ALLOC(i+1, char);
      xdr_string(xdrp, &name, i+1);

      admin = get_dof_admin(mesh, name, a_n_dof);
      name = nil;

      if (used_count > 0) enlarge_dof_lists(admin, used_count);
    } /* end for (iadmin) */

    TEST(mesh->n_dof_el == n_dof_el) ("wrong n_dof_el: %d %d\n",
                                      mesh->n_dof_el, n_dof_el);
    for (i=0; i<=DIM; i++)
      TEST(mesh->n_dof[i] == n_dof[i]) ("wrong n_dof[%d]: %d %d\n",
                                        i, mesh->n_dof[i], n_dof[i]);
    TEST(mesh->n_node_el == n_node_el) ("wrong n_node_el: %d %d\n",
                                        mesh->n_node_el, n_node_el);
    for (i=0; i<=DIM; i++)
      TEST(mesh->node[i] == node[i]) ("wrong node[%d]: %d %d\n",
                                      i, mesh->node[i], node[i]);
    return;
  }

  /****************************************************************************/


  MESH *new_read_mesh_xdr(const char *fn, REAL *timeptr,
                          void (*init_leaf_data)(LEAF_DATA_INFO *),
                          const BOUNDARY *(*init_boundary)(MESH *, int),
                          Dune::SerialIndexSet & gIndex)
  {
    FUNCNAME("read_mesh_xdr");
    MACRO_EL       *mel;
    int i, j, n;
    REAL_D         *v, x_min, x_max;
    int neigh_i[N_NEIGH];
#if DIM == 2
    S_CHAR bound_sc[N_EDGES];
#endif
#if DIM == 3
    S_CHAR bound_sc[N_FACES+N_EDGES];
#endif
    char           *name, *s;
    size_t length;
    int iDIM, iDIM_OF_WORLD, ne, nv;
    REAL time, diam[DIM_OF_WORLD];
    int n_vertices, n_elements, n_hier_elements;
#if DIM > 1
    int n_edges;
#endif
    int vert_i[N_VERTICES];
    static int funccount=0;
#if DIM==3
    int n_faces, max_edge_neigh;
#endif

#if DIM > 1
    const BOUNDARY *(*init_bdry)(MESH *, int);
    init_bdry = (init_boundary) ? init_boundary : default_boundary;
#endif

    if (!(xdrp = xdr_open_file(fn, XDR_DECODE)))
    {
      ERROR("cannot open file %s\n",fn);
    }

    length = MAX(strlen(ALBERT_VERSION)+1,5);    /* length with terminating \0 */
    s = MEM_ALLOC(length, char);

    xdr_string(xdrp, &s, length);

    if (strncmp(s, "ALBERT", 6))
    {
      ERROR("unknown file id: \"%s\"\n",s);
      goto error_exit;
    }

    xdr_int(xdrp, &iDIM);
    if (iDIM != DIM)
    {
      ERROR("wrong DIM %d. abort.\n", iDIM);
      goto error_exit;
    }

    xdr_int(xdrp, &iDIM_OF_WORLD);
    if (iDIM_OF_WORLD != DIM_OF_WORLD)
    {
      ERROR("wrong DIM_OF_WORLD %d. abort.\n", iDIM_OF_WORLD);
      goto error_exit;
    }

    xdr_REAL(xdrp, &time);
    if (timeptr) *timeptr = time;



    xdr_int(xdrp, &i);                 /* length without terminating \0 */
    if(i) {
      name = MEM_ALLOC(i+1, char);
      xdr_string(xdrp, &name, i+1);
    }
    else {
      funccount++;
      i=100;
      name = MEM_ALLOC(i+1, char);
      sprintf(name, "READ_MESH%d", funccount);
    }


    xdr_int(xdrp, &n_vertices);
#if DIM > 1
    xdr_int(xdrp, &n_edges);
#endif
    xdr_int(xdrp, &n_elements);
    xdr_int(xdrp, &n_hier_elements);


#if DIM == 3

    xdr_int(xdrp, &n_faces);
    xdr_int(xdrp, &max_edge_neigh);

#endif


    xdr_vector(xdrp, (char *)diam, DIM_OF_WORLD, sizeof(REAL),
               (xdrproc_t) xdr_REAL);

    xdr_U_CHAR(xdrp, &preserve_coarse_dofs);



    mesh = GET_MESH(name, read_dof_admins_xdr, init_leaf_data);



    MEM_FREE(name, i+1, char);
    mesh->preserve_coarse_dofs = preserve_coarse_dofs;
    /* mesh->parametric = nil;     */

    xdr_int(xdrp, &n_vert_dofs);

    if (n_vert_dofs > 0)
    {
      vert_dofs = MEM_ALLOC(n_vert_dofs, DOF *);
      n = mesh->n_dof[VERTEX];
      for (i = 0; i < n_vert_dofs; i++)
      {
        vert_dofs[i] = get_dof(mesh, VERTEX);

        xdr_vector(xdrp, (char *)vert_dofs[i], n, sizeof(DOF),
                   (xdrproc_t) xdr_DOF);
      }
    }

#if DIM > 1
    xdr_int(xdrp, &n_edge_dofs);

    if (n_edge_dofs > 0)
    {
      edge_dofs = MEM_ALLOC(n_edge_dofs, DOF *);
      n = mesh->n_dof[EDGE];
      for (i = 0; i < n_edge_dofs; i++)
      {
        edge_dofs[i] = get_dof(mesh, EDGE);

        xdr_vector(xdrp, (char *)edge_dofs[i], n, sizeof(DOF),
                   (xdrproc_t) xdr_DOF);
      }
    }
#endif

#if DIM==3

    xdr_int(xdrp, &n_face_dofs);

    if (n_face_dofs > 0)
    {
      face_dofs = MEM_ALLOC(n_face_dofs, DOF *);
      n = mesh->n_dof[FACE];
      for (i = 0; i < n_face_dofs; i++)
      {
        face_dofs[i] = get_dof(mesh, FACE);

        xdr_vector(xdrp, (char *)face_dofs[i], n, sizeof(DOF),
                   (xdrproc_t) xdr_DOF);
      }
    }
#endif

    xdr_int(xdrp, &ne);
    xdr_int(xdrp, &nv);

    v = MEM_ALLOC(nv, REAL_D);

    for (i = 0; i < nv; i++)
      xdr_vector(xdrp, (char *)v[i], DIM_OF_WORLD, sizeof(REAL),
                 (xdrproc_t) xdr_REAL);

    for (j = 0; j < DIM_OF_WORLD; j++)
    {
      x_min[j] =  1.E30;
      x_max[j] = -1.E30;
    }

    for (i = 0; i < nv; i++)
      for (j = 0; j < DIM_OF_WORLD; j++)
      {
        x_min[j] = MIN(x_min[j], v[i][j]);
        x_max[j] = MAX(x_max[j], v[i][j]);
      }

    for (j = 0; j < DIM_OF_WORLD; j++)
      mesh->diam[j] = x_max[j] - x_min[j];


    mel  = MEM_ALLOC(ne, MACRO_EL);

    for (i = 0; i < ne-1; i++) mel[i].next = mel+(i+1);
    for (i = 1; i < ne; i++) mel[i].last = mel+(i-1);
    mel[0].last = mel[ne-1].next = nil;

    mesh->n_macro_el = ne;
    mesh->first_macro_el = mel;


    for (n = 0; n < ne; n++)
    {
      mel[n].index = n;

      xdr_vector(xdrp, (char *)vert_i, N_VERTICES, sizeof(int),
                 (xdrproc_t) xdr_int);

      for (i = 0; i < N_VERTICES; i++)
      {
        if ((vert_i[i] >= 0) && (vert_i[i] < nv))
          mel[n].coord[i] = (REAL *)(v + (vert_i[i]));
        else
          mel[n].coord[i] = nil;
      }


      xdr_vector(xdrp, (char *)mel[n].bound, N_VERTICES, sizeof(S_CHAR),
                 (xdrproc_t) xdr_S_CHAR);

#if DIM == 2
      xdr_vector(xdrp, (char *)bound_sc, N_EDGES, sizeof(S_CHAR),
                 (xdrproc_t) xdr_S_CHAR);

      for (i = 0; i < N_EDGES; i++)
      {
        if (bound_sc[i])
          mel[n].boundary[i] = init_bdry(mesh, bound_sc[i]);
        else
          mel[n].boundary[i] = nil;
      }
#endif

#if DIM == 3
      xdr_vector(xdrp, (char *)bound_sc, (N_FACES+N_EDGES),
                 sizeof(S_CHAR), (xdrproc_t) xdr_S_CHAR);

      for (i = 0; i < (N_FACES+N_EDGES); i++)
      {
        if (bound_sc[i])
          mel[n].boundary[i] = init_bdry(mesh, bound_sc[i]);
        else
          mel[n].boundary[i] = nil;
      }
#endif

      xdr_vector(xdrp, (char *)neigh_i, N_NEIGH, sizeof(int),
                 (xdrproc_t) xdr_int);

      for (i = 0; i < N_NEIGH; i++)
      {
        if ((neigh_i[i] >= 0) && (neigh_i[i] < ne))
          mel[n].neigh[i] = mel + (neigh_i[i]);
        else
          mel[n].neigh[i] = nil;
      }

      xdr_vector(xdrp, (char *)mel[n].opp_vertex, N_NEIGH, sizeof(U_CHAR),
                 (xdrproc_t) xdr_U_CHAR);

#if DIM==3
      xdr_U_CHAR(xdrp, &(mel[n].el_type));
#endif
      mel[n].el = read_el_recursive(nil);
    }





    if (n_elements != mesh->n_elements)
    {
      ERROR("n_elements != mesh->n_elements.\n");
      goto error_exit;
    }

    if (n_hier_elements != mesh->n_hier_elements)
    {
      ERROR("n_hier_elements != mesh->n_hier_elements.\n");
      goto error_exit;
    }


    if (mesh->n_dof[VERTEX])
    {
      if (n_vertices != n_vert_dofs)
      {
        ERROR("n_vertices != n_vert_dofs.\n");
        mesh->n_vertices = n_vert_dofs;
        goto error_exit;
      }
    }
    mesh->n_vertices = n_vertices;

#if DIM > 1
    if (mesh->n_dof[EDGE])
    {
      if (n_edges != n_edge_dofs)
      {
        ERROR("n_edges != n_edge_dofs.\n");
        mesh->n_edges = n_edge_dofs;
        goto error_exit;
      }
    }
    mesh->n_edges = n_edges;
#endif

#if DIM == 3
    if (mesh->n_dof[FACE])
    {
      if (n_faces != n_face_dofs)
      {
        ERROR("n_faces != n_face_dofs.\n");
        mesh->n_faces = n_face_dofs;
        goto error_exit;
      }
    }
    mesh->n_faces = n_faces;

    mesh->max_edge_neigh = max_edge_neigh;
#endif

    for (i=0; i<DIM_OF_WORLD; i++)
    {
      if (ABS(mesh->diam[i]-diam[i]) > (mesh->diam[i]/10000.0))
      {
        ERROR("diam[%i] != mesh->diam[%i].\n",i,i);
        goto error_exit;
      }
    }

    // read index set
    gIndex.processXdr(xdrp);

    xdr_string(xdrp, &s, length);
    if (strncmp(s, "EOF.", 4))                  /* file end marker */
    {
      ERROR("no FILE END MARK.\n");
      goto error_exit;
    }
    else
    {
      MSG("File %s read.\n",fn);
    }

#if NEIGH_IN_EL
    ERROR_EXIT("read_mesh for NEIGH_IN_EL=1 not implemented yet!!!\n");
#endif

error_exit:
    MEM_FREE(s, length, char);

    xdr_close_file(xdrp);

    return(mesh);
  }

  /****************************************************************************/

  static EL *read_el_recursive(EL *parent)
  {
    FUNCNAME("read_el_recursive");
    int i, j, n, node0;
    EL     *el;
    U_CHAR uc;
#if DIM > 1
    U_CHAR nc;
#endif

    el = get_element(mesh);
    mesh->n_hier_elements++;

#if EL_INDEX
    //el->index = mesh->n_hier_elements;
    int fake = 0;
    xdr_int(xdrp,&fake);
    el->index = fake;
    //std::cout << "read element " << fake << "\n";
#endif

    xdr_U_CHAR(xdrp, &uc);

#if DIM > 1
    xdr_U_CHAR(xdrp, &nc);
    if (nc)
    {
      el->new_coord = get_real_d(mesh);

      xdr_vector(xdrp, (char *)el->new_coord, DIM_OF_WORLD, sizeof(REAL),
                 (xdrproc_t) xdr_REAL);
    }
    else
    {
      el->new_coord = nil;
    }
#endif

    if (mesh->n_dof[VERTEX] > 0)
    {
      node0 = mesh->node[VERTEX];
      for (i = 0; i < N_VERTICES; i++)
      {
        xdr_int(xdrp, &j);

        TEST_EXIT(j < n_vert_dofs)
          ("vert_dofs index too large: %d >= %d\n", j, n_vert_dofs);
        el->dof[node0 + i] = vert_dofs[j];
      }
    }

    if ((!uc) || preserve_coarse_dofs)
    {
#if DIM > 1
      if (mesh->n_dof[EDGE] > 0)
      {
        node0 = mesh->node[EDGE];
        for (i = 0; i < N_EDGES; i++)
        {
          xdr_int(xdrp, &j);

          TEST_EXIT(j < n_edge_dofs)
            ("edge_dofs index too large: %d >= %d\n", j, n_edge_dofs);
          el->dof[node0 + i] = edge_dofs[j];
        }
      }
#endif

#if DIM == 3
      if ((n = mesh->n_dof[FACE]) > 0)
      {
        node0 = mesh->node[FACE];
        for (i = 0; i < N_FACES; i++)
        {
          xdr_int(xdrp, &j);

          TEST_EXIT(j < n_face_dofs)
            ("face_dofs index too large: %d >= %d\n", j, n_face_dofs);
          el->dof[node0 + i] = face_dofs[j];
        }
      }
#endif

      if ((n = mesh->n_dof[CENTER]) > 0)
      {
        node0 = mesh->node[CENTER];
        el->dof[node0] = get_dof(mesh, CENTER);

        xdr_vector(xdrp, (char *)el->dof[node0], n, sizeof(DOF),
                   (xdrproc_t) xdr_DOF);
      }
    }

#if NEIGH_IN_EL
    for (i = 0; i < N_NEIGH; i++)
    {
      el->neigh[i] = nil;
      el->opp_vertex[i] = 0;
    }
#endif

    if (uc)
    {
      el->child[0] = read_el_recursive(el);
      el->child[1] = read_el_recursive(el);
    }
    else
      mesh->n_elements++;

    return(el);
  }



  /****************************************************************************/
  /* read DOF vectors of various types                                        */
  /****************************************************************************/

  typedef DOF_REAL_VEC DOF_VEC;

  static const DOF_ADMIN *read_dof_vec_xdr(const char *fn, DOF_VEC *dv,
                                           const char *dofvectype,
                                           MESH *mesh, FE_SPACE *fe_space)
  {
    FUNCNAME("read_dof_vec_xdr");
    int i, last;
    int n_dof[DIM+1];
    const DOF_ADMIN *admin = nil;
    const BAS_FCTS  *bas_fcts;
    char            *name, *s;
    size_t length;

    TEST_EXIT(mesh) ("no mesh given\n");

    if (!(xdrp = xdr_open_file(fn, XDR_DECODE)))
    {
      ERROR("cannot open file %s\n",fn);
    }

    length = 17;        /* length of dofvectype with terminating \0 */

    s=MEM_ALLOC(length, char);
    xdr_string(xdrp, &s, length);

    if (strncmp(s, dofvectype, 12))
    {
      ERROR("invalid file id; %s\n", s);
      goto error_exit;
    }

    xdr_int(xdrp, &last);                 /* length without terminating \0 */
    name = MEM_ALLOC(last+1, char);
    xdr_string(xdrp, &name, last+1);
    dv->name = name;
    xdr_vector(xdrp, (char *)n_dof, DIM+1, sizeof(int), (xdrproc_t) xdr_int);

    xdr_int(xdrp, &last);                      /* length of BAS_FCTS name */

    if (last)
    {
      name = MEM_ALLOC(last+1, char);
      xdr_string(xdrp, &name, last+1);

      if (fe_space && (bas_fcts = fe_space->bas_fcts))
      {
        if (strcmp(bas_fcts->name, name))
        {
          ERROR("invalid name %s is not given fe_space->bas_fcts->name %s\n",
                name, bas_fcts->name);
        }
      }
      else
      { /* no given fe_space or no bas_fcts in given fe_space */
        TEST_EXIT(bas_fcts = get_bas_fcts (name))
                  ("cannot get bas_fcts <%s>\n", name);

                  if (fe_space)
                  {                 /* use given fe_space */
                    fe_space->bas_fcts = bas_fcts;
                  }
                  else
                  {                         /* create new fe_space */
                    TEST_EXIT(fe_space =
                                (FE_SPACE *)get_fe_space(mesh, name, n_dof, bas_fcts))
                      ("cannot get fe_space for bas_fcts <%s>\n", name);
                  }
      }

      for (i=0; i<=DIM; i++)
      {
        TEST_EXIT(n_dof[i] == bas_fcts->n_dof[i])
          ("wrong n_dof in bas_fcts <%s>", name);
      }
    }
    else
    { /* no bas_fcts.name in file */
      if (fe_space)
      {                             /* use given fe_space */
        TEST_EXIT(admin = fe_space->admin) ("no fe_space->admin");
        for (i=0; i<=DIM; i++)
        {
          TEST_EXIT(n_dof[i] == admin->n_dof[i])
            ("wrong n_dof in admin <%s>", NAME(admin));
        }
      }
      else
      {                                     /* create new fe_space */
        TEST_EXIT(fe_space =
                    (FE_SPACE *) get_fe_space(mesh, nil, n_dof, nil))
          ("cannot get fe_space for given n_dof\n");
        TEST_EXIT(admin = fe_space->admin) ("no admin in new fe_space\n");
        for (i=0; i<=DIM; i++)
        {
          TEST_EXIT(n_dof[i] == admin->n_dof[i])
            ("wrong n_dof in admin <%s>", NAME(admin));
        }
      }
    }
    TEST_EXIT(fe_space) ("still no fe_space\n");
    dv->fe_space = fe_space;
    TEST_EXIT(admin = fe_space->admin) ("still no admin\n");

    dof_compress(mesh);

    xdr_int(xdrp, &last);

    if (last)
    {
      dv->size = last;

      if (!strncmp(dofvectype, "DOF_REAL_VEC    ", 12))
      {
        dv->vec = ((REAL *) albert_alloc((size_t)(last*sizeof(double)), funcName,
                                         __FILE__, __LINE__));
        xdr_vector(xdrp, (char *)dv->vec, last, sizeof(REAL),
                   (xdrproc_t) xdr_REAL);
      }
      else if (!strncmp(dofvectype, "DOF_REAL_D_VEC  ", 12))
      {
        dv->vec =
          ((REAL *) albert_alloc((size_t)(last*DIM_OF_WORLD*sizeof(REAL)),
                                 funcName, __FILE__, __LINE__));
        xdr_vector(xdrp, (char *)dv->vec, last*DIM_OF_WORLD,
                   sizeof(REAL), (xdrproc_t) xdr_REAL);
      }
      else if (!strncmp(dofvectype, "DOF_INT_VEC     ", 12))
      {
        dv->vec = ((REAL *) albert_alloc((size_t)(last*sizeof(int)), funcName,
                                         __FILE__, __LINE__));
        xdr_vector(xdrp, (char *)dv->vec, last, sizeof(int),
                   (xdrproc_t) xdr_int);
      }
      else if (!strncmp(dofvectype, "DOF_SCHAR_VEC   ", 12))
      {
        dv->vec = ((REAL *) albert_alloc((size_t)(last*sizeof(char)), funcName,
                                         __FILE__, __LINE__));
        xdr_vector(xdrp, (char *)dv->vec, last, sizeof(S_CHAR),
                   (xdrproc_t) xdr_S_CHAR);
      }
      else if (!strncmp(dofvectype, "DOF_UCHAR_VEC   ", 12))
      {
        dv->vec = ((REAL *) albert_alloc((size_t)(last*sizeof(unsigned char)),
                                         funcName, __FILE__, __LINE__));
        xdr_vector(xdrp, (char *)dv->vec, last, sizeof(U_CHAR),
                   (xdrproc_t) xdr_U_CHAR);
      }
      else
        ERROR("ivalid file id %s\n",dofvectype);
    }
    else
    {
      ERROR("empty dof vector\n");
      dv->size = 0;
      dv->vec = nil;
    }

    xdr_string(xdrp, &s, length);
    if (strncmp(s, "EOF.", 4))                  /* file end marker */
    {
      ERROR("no FILE END MARK.\n");
      goto error_exit;
    }
    else
    {
      MSG("File %s read.\n",fn);
    }

error_exit:
    MEM_FREE(s,length,char);

    xdr_close_file(xdrp);

    return(admin);
  }
  /****************************************************************************/

  DOF_REAL_VEC *read_dof_real_vec_xdr(const char *fn, MESH *mesh, FE_SPACE
                                      *fe_space)
  {
    DOF_REAL_VEC    *dv;
    const DOF_ADMIN *admin;
    void add_dof_real_vec_to_admin(DOF_REAL_VEC *dv, DOF_ADMIN *admin);

    dv = get_dof_real_vec(fn, nil);

    admin = read_dof_vec_xdr(fn, (DOF_VEC *)dv, "DOF_REAL_VEC    ", mesh,
                             fe_space);

    if (admin) add_dof_real_vec_to_admin(dv, (DOF_ADMIN *)admin);
    return(dv);
  }

  /****************************************************************************/

  DOF_REAL_D_VEC *read_dof_real_d_vec_xdr(const char *fn, MESH *mesh,
                                          FE_SPACE *fe_space)
  {
    DOF_REAL_D_VEC  *dv;
    const DOF_ADMIN *admin;
    void add_dof_real_d_vec_to_admin(DOF_REAL_D_VEC *dv, DOF_ADMIN *admin);

    dv = get_dof_real_d_vec(fn, nil);

    admin = read_dof_vec_xdr(fn, (DOF_VEC *)dv, "DOF_REAL_D_VEC  ", mesh,
                             fe_space);

    if (admin) add_dof_real_d_vec_to_admin(dv, (DOF_ADMIN *)admin);
    return(dv);
  }

  /****************************************************************************/

  DOF_INT_VEC *read_dof_int_vec_xdr(const char *fn, MESH *mesh,
                                    FE_SPACE *fe_space)
  {
    DOF_INT_VEC     *dv;
    const DOF_ADMIN *admin;
    void add_dof_int_vec_to_admin(DOF_INT_VEC *dv, DOF_ADMIN *admin);

    dv = get_dof_int_vec(fn, nil);

    admin = read_dof_vec_xdr(fn, (DOF_VEC *)dv, "DOF_INT_VEC     ",
                             mesh, fe_space);

    if (admin) add_dof_int_vec_to_admin(dv, (DOF_ADMIN *)admin);
    return(dv);
  }

  /****************************************************************************/

  DOF_SCHAR_VEC *read_dof_schar_vec_xdr(const char *fn, MESH *mesh,
                                        FE_SPACE *fe_space)
  {
    DOF_SCHAR_VEC   *dv;
    const DOF_ADMIN *admin;
    void add_dof_schar_vec_to_admin(DOF_SCHAR_VEC *dv, DOF_ADMIN *admin);

    dv = get_dof_schar_vec(fn, nil);

    admin = read_dof_vec_xdr(fn, (DOF_VEC *)dv, "DOF_SCHAR_VEC   ", mesh,
                             fe_space);

    if (admin) add_dof_schar_vec_to_admin(dv, (DOF_ADMIN *)admin);
    return(dv);
  }

  /****************************************************************************/

  DOF_UCHAR_VEC *read_dof_uchar_vec_xdr(const char *fn, MESH *mesh,
                                        FE_SPACE *fe_space)
  {
    DOF_UCHAR_VEC   *dv;
    const DOF_ADMIN *admin;
    void add_dof_uchar_vec_to_admin(DOF_UCHAR_VEC *dv, DOF_ADMIN *admin);

    dv = get_dof_uchar_vec(fn, nil);

    admin = read_dof_vec_xdr(fn, (DOF_VEC *)dv, "DOF_UCHAR_VEC   ", mesh,
                             fe_space);

    if (admin) add_dof_uchar_vec_to_admin(dv, (DOF_ADMIN *)admin);
    return(dv);
  }

} // end namespace

/****************************************************************************/
