// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
/****************************************************************************/
/* ALBERT:   an Adaptive multi Level finite element toolbox using           */
/*           Bisectioning refinement and Error control by Residual          */
/*           Techniques                                                     */
/*                                                                          */
/* file:     Common/write_mesh_xdr.c                                        */
/*                                                                          */
/*                                                                          */
/* description:  functions for writing meshes and DOF vectors               */
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
/****************************************************************************/

#include <rpc/types.h>
#include <rpc/xdr.h>

namespace AlbertWrite
{

  static XDR *xdrp;


  /*
     XDR-Routinen zur Uebertragung von ALBERT-Datentypen

     muessen bei abgeaenderter Definition auch veraendert werden !!!

     akt. Def.:  REAL   = double
                 U_CHAR = unsigned char
                 S_CHAR = signed char
                 DOF    = int
   */

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




  static int read_xdr_file(void *file, void *buffer, u_int size)
  {
    return ((int)fread(buffer, 1, (size_t)size, (FILE *)file));
  }

  static int write_xdr_file(void *file, void *buffer, u_int size)
  {
    return (fwrite(buffer, (size_t)size, 1, (FILE *)file) == 1 ? (int)size : 0);
  }


  static FILE *file;

  static XDR *xdr_open_file(const char *filename, enum xdr_op mode)
  {
    XDR *xdr;

    if (!(xdr = MEM_ALLOC(1,XDR)))
    {
      ERROR("can't allocate memory for xdr pointer.\n");

      return NULL;
    }

    if ((file = fopen(filename, mode == XDR_DECODE ? "r" : "w")))
    {
      xdrrec_create(xdr, 65536, 65536, (char *)file,
                    (int (*)(char *, char *, int))read_xdr_file,
                    (int (*)(char *, char *, int))write_xdr_file);

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

  static int  *n_dof, *node;
  static U_CHAR preserve_coarse_dofs;

  static void write_el_recursive(EL *el);

  static int n_vert_ptrs;
  static DOF         *vert_dofs, **vert_ptrs;
  static DOF_ADMIN   *vert_admin;

#if DIM > 1
  static int n_edge_ptrs;
  static DOF         *edge_dofs, **edge_ptrs;
  static DOF_ADMIN   *edge_admin;
#endif

#if DIM == 3
  static int n_face_ptrs;
  static DOF         *face_dofs, **face_ptrs;
  static DOF_ADMIN   *face_admin;
#endif

  static int         *mesh_n_dof, *mesh_node;

  /****************************************************************************/
#define OFFSET 10      /* must be > MAX(VERTEX,EDGE,FACE,CENTER) */

  static void fill_dofs_fct(const EL_INFO *elinfo)
  {
    EL   *el = elinfo->el;
    int i, n0, n1;
    DOF  *p;

    if (IS_LEAF_EL(el) || preserve_coarse_dofs)
    {
      if (vert_admin)
      {
        n0 = mesh_node[VERTEX];
        n1 = vert_admin->n0_dof[VERTEX];
        for (i = 0; i < N_VERTICES; i++)
        {
          p = el->dof[n0+i];
          vert_dofs[p[n1]] = VERTEX-OFFSET;
          vert_ptrs[p[n1]] = p;
        }
      }
#if DIM > 1
      if (edge_admin)
      {
        n0 = mesh_node[EDGE];
        n1 = edge_admin->n0_dof[EDGE];
        for (i = 0; i < N_EDGES; i++)
        {
          p = el->dof[n0+i];
          edge_dofs[p[n1]] = EDGE-OFFSET;
          edge_ptrs[p[n1]] = p;
        }
      }
#endif

#if DIM == 3
      if (face_admin)
      {
        n0 = mesh_node[FACE];
        n1 = face_admin->n0_dof[FACE];
        for (i = 0; i < N_FACES; i++)
        {
          p = el->dof[n0+i];
          edge_dofs[p[n1]] = FACE-OFFSET;
          edge_ptrs[p[n1]] = p;
        }
      }
#endif
    }
    return;
  }

  /****************************************************************************/


  int new_write_mesh_xdr(MESH *mesh, const char *filename, REAL time,
                         Dune::SerialIndexSet &gIndex)
  {
    FUNCNAME("write_mesh_xdr");
    MACRO_EL   *mel, *meln;
    DOF_ADMIN  *admin;
    int i, j, n, m, iadmin;
    int neigh_i[N_NEIGH];
#if DIM == 2
    S_CHAR bound_sc[N_EDGES];
#endif
#if DIM == 3
    S_CHAR bound_sc[N_FACES+N_EDGES];
#endif
    char *s;
    int strileng;


    if (!mesh)
    {
      ERROR("no mesh - no file created\n");
      return(1);
    }

    dof_compress(mesh);

    n_dof = mesh->n_dof;
    node  = mesh->node;


    if (!(xdrp = xdr_open_file(filename, XDR_ENCODE)))
    {
      ERROR("cannot open file %s\n",filename);
      return(1);
    }

    strileng = MAX(strlen(ALBERT_VERSION)+1,5);    /* length with terminating \0 */


    s=MEM_ALLOC(strileng, char);

    strcpy(s,ALBERT_VERSION);                                   /* file marker */
    xdr_string(xdrp, &s, strileng);


    i = DIM;
    xdr_int(xdrp, &i);

    i = DIM_OF_WORLD;
    xdr_int(xdrp, &i);

    xdr_REAL(xdrp, &time);



    if (mesh->name)
    {
      i = strlen(mesh->name);            /* length without terminating \0 */
      if ((i+1) > strileng)
      {
        strileng = i+1;
        s=MEM_REALLOC(s, (strileng-i-1), strileng, char);
      }
      strcpy(s,mesh->name);
    }
    else
    {
      i = 0;
      strcpy(s,"");
    }
    xdr_int(xdrp, &i);
    xdr_string(xdrp, &s, strileng);




    xdr_int(xdrp, &mesh->n_vertices);
#if DIM > 1
    xdr_int(xdrp, &mesh->n_edges);
#endif
    xdr_int(xdrp, &mesh->n_elements);
    xdr_int(xdrp, &mesh->n_hier_elements);


#if DIM == 3
    xdr_int(xdrp, &mesh->n_faces);
    xdr_int(xdrp, &mesh->max_edge_neigh);
#endif

    xdr_vector(xdrp, (char *)mesh->diam, DIM_OF_WORLD, sizeof(REAL),
               (xdrproc_t) xdr_REAL);

    xdr_U_CHAR(xdrp, &mesh->preserve_coarse_dofs);
    preserve_coarse_dofs = mesh->preserve_coarse_dofs;

    xdr_int(xdrp, &(mesh->n_dof_el));
    xdr_vector(xdrp, (char *)mesh->n_dof, DIM+1, sizeof(int),
               (xdrproc_t) xdr_int);
    xdr_int(xdrp, &(mesh->n_node_el));
    xdr_vector(xdrp, (char *)mesh->node, DIM+1, sizeof(int),
               (xdrproc_t) xdr_int);



    xdr_int(xdrp, &(mesh->n_dof_admin));
    for (iadmin = 0; iadmin < mesh->n_dof_admin; iadmin++)
    {
      admin = mesh->dof_admin[iadmin];
      if (admin)
      {
        xdr_vector(xdrp, (char *)admin->n_dof, DIM+1, sizeof(int),
                   (xdrproc_t) xdr_int);
        xdr_int(xdrp, &(admin->used_count));
        /* after dof_compress(), no more information is required */


        if (admin->name)
        {
          i = strlen(admin->name); /* length without terminating \0 */ if ((i+1) > strileng)
          {
            strileng = i+1;
            s=MEM_REALLOC(s, (strileng-i-1), strileng, char);
          }
          strcpy(s,admin->name);
        }
        else
        {
          i = 0;
          strcpy(s,"");
        }
        xdr_int(xdrp, &i);
        xdr_string(xdrp, &s, strileng);

      }
      else
      {
        i = 0;
        for (j = 0; j < DIM+3; j++)
          xdr_int(xdrp, &i);

        strcpy(s,"");
        xdr_string(xdrp, &s, strileng);
      }
    }




#if DIM==3
    face_admin = nil;
    if (mesh->n_dof[FACE] > 0)
    {
      m = 99999;
      for (iadmin = 0; iadmin < mesh->n_dof_admin; iadmin++)
      {
        if ((admin = mesh->dof_admin[iadmin]))
        {
          if (admin->n_dof[FACE] > 0  && admin->n_dof[FACE] < m)
          {
            face_admin = admin;
            m = admin->n_dof[FACE];
          }
        }
      }
      TEST_EXIT(face_admin) ("no admin with face dofs?\n");
      face_dofs = MEM_ALLOC(face_admin->used_count, DOF);
      for (i=0; i<face_admin->used_count; i++) face_dofs[i] = -1;
      face_ptrs = MEM_ALLOC(face_admin->used_count, DOF*);
      for (i=0; i<face_admin->used_count; i++) face_ptrs[i] = 0;
    }
#endif

#if DIM > 1
    edge_admin = nil;
    if (mesh->n_dof[EDGE] > 0)
    {
#if DIM==3
      if (face_admin && (face_admin->n_dof[EDGE] > 0))
      {
        edge_admin = face_admin;
        edge_dofs = face_dofs;
        edge_ptrs = face_ptrs;
      }
      else
#endif
      {
        m = 99999;
        for (iadmin = 0; iadmin < mesh->n_dof_admin; iadmin++)
        {
          if ((admin = mesh->dof_admin[iadmin]))
          {
            if (admin->n_dof[EDGE] > 0  && admin->n_dof[EDGE] < m)
            {
              edge_admin = admin;
              m = admin->n_dof[EDGE];
            }
          }
        }
        TEST_EXIT(edge_admin) ("no admin with edge dofs?\n");
        edge_dofs = MEM_ALLOC(edge_admin->used_count, DOF);
        for (i=0; i<edge_admin->used_count; i++) edge_dofs[i] = -1;
        edge_ptrs = MEM_ALLOC(edge_admin->used_count, DOF*);
        for (i=0; i<edge_admin->used_count; i++) edge_ptrs[i] = 0;
      }
    }
#endif

    vert_admin = nil;
    if (mesh->n_dof[VERTEX] > 0)
    {
#if DIM == 3
      if (face_admin && (face_admin->n_dof[VERTEX] > 0))
      {
        vert_admin = face_admin;
        vert_dofs = face_dofs;
        vert_ptrs = face_ptrs;
      }
      else
#endif
      {
#if DIM > 1
        if (edge_admin && (edge_admin->n_dof[VERTEX] > 0))
        {
          vert_admin = edge_admin;
          vert_dofs = edge_dofs;
          vert_ptrs = edge_ptrs;
        }
        else
#endif
        {
          m = 99999;
          for (iadmin = 0; iadmin < mesh->n_dof_admin; iadmin++)
          {
            if ((admin = mesh->dof_admin[iadmin]))
            {
              if (admin->n_dof[VERTEX] > 0  && admin->n_dof[VERTEX] < m)
              {
                vert_admin = admin;
                m = admin->n_dof[VERTEX];
              }
            }
          }
          TEST_EXIT(vert_admin) ("no admin with vertex dofs?\n");
          vert_dofs = MEM_ALLOC(vert_admin->used_count, DOF);
          for (i=0; i<vert_admin->used_count; i++) vert_dofs[i] = -1;
          vert_ptrs = MEM_ALLOC(vert_admin->used_count, DOF*);
          for (i=0; i<vert_admin->used_count; i++) vert_ptrs[i] = 0;
        }
      }
    }

    mesh_n_dof = mesh->n_dof;
    mesh_node  = mesh->node;
    mesh_traverse(mesh, 0, CALL_EVERY_EL_PREORDER|FILL_NOTHING, fill_dofs_fct);

    if (vert_admin)
    {
      n_vert_ptrs = 0;
      for (i = 0; i < vert_admin->used_count; i++)
        if (vert_dofs[i] == VERTEX-OFFSET)
          n_vert_ptrs++;
    }

#if DIM > 1
    if (edge_admin)
    {
      n_edge_ptrs = 0;
      for (i = 0; i < edge_admin->used_count; i++)
        if (edge_dofs[i] == EDGE-OFFSET)
          n_edge_ptrs++;
    }
#endif

#if DIM == 3
    if (face_admin)
    {
      n_face_ptrs = 0;
      for (i = 0; i < face_admin->used_count; i++)
        if (face_dofs[i] == FACE-OFFSET)
          n_face_ptrs++;
    }
#endif

    xdr_int(xdrp, &n_vert_ptrs);

    if (n_vert_ptrs)
    {
      j = 0;
      n = mesh_n_dof[VERTEX];
      for (i = 0; i < vert_admin->used_count; i++)
      {
        if (vert_dofs[i] == VERTEX-OFFSET)
        {
          vert_dofs[i] = j++;

          xdr_vector(xdrp, (char *)vert_ptrs[i], n, sizeof(DOF),
                     (xdrproc_t) xdr_DOF);
        }
      }
      TEST_EXIT(j==n_vert_ptrs) ("j != n_vert_ptrs\n");
    }

#if DIM > 1
    xdr_int(xdrp, &n_edge_ptrs);

    if (n_edge_ptrs)
    {
      j = 0;
      n = mesh_n_dof[EDGE];
      for (i = 0; i < edge_admin->used_count; i++)
      {
        if (edge_dofs[i] == EDGE-OFFSET)
        {
          edge_dofs[i] = j++;

          xdr_vector(xdrp, (char *)edge_ptrs[i], n, sizeof(DOF),
                     (xdrproc_t) xdr_DOF);
        }
      }
      TEST_EXIT(j==n_edge_ptrs) ("j != n_edge_ptrs\n");
    }
#endif

#if DIM == 3
    xdr_int(xdrp, &n_face_ptrs);

    if (n_face_ptrs)
    {
      j = 0;
      n = mesh_n_dof[FACE];
      for (i = 0; i < face_admin->used_count; i++)
      {
        if (face_dofs[i] == FACE-OFFSET)
        {
          face_dofs[i] = j++;

          xdr_vector(xdrp, (char *)face_ptrs[i], n, sizeof(DOF),
                     (xdrproc_t) xdr_DOF);
        }
      }
      TEST_EXIT(j==n_face_ptrs) ("j != n_face_ptrs\n");
    }
#endif


    /****************************************************************************/
    /* gather info about macro elements (vertices, ...)                         */
    /****************************************************************************/
    {
      typedef int intNV[N_VERTICES];
      int    (*mcindex)[N_VERTICES] = MEM_ALLOC(mesh->n_macro_el, intNV);
      REAL_D   *mccoord;
      int mccount, m;

      mccount = ((MESH_MEM_INFO *)(mesh->mem_info))->count;
      mccoord  = ((MESH_MEM_INFO *)(mesh->mem_info))->coords;

      TEST_EXIT(mcindex) ("could not allocate mcindex\n");

      for (m = 0, mel = mesh->first_macro_el;  mel;  m++, mel = mel->next)
      {
        for (i = 0; i < N_VERTICES; i++)
          mcindex[m][i] =
            ((char *)(mel->coord[i]) - (char *)mccoord)/(sizeof(char)*sizeof(REAL_D));

        if (mel->index != m) mel->index = m;
      }
      TEST(m == mesh->n_macro_el) (" mesh->n_macro_el = %d  !=  %d ???\n");

      xdr_int(xdrp, &(mesh->n_macro_el));
      xdr_int(xdrp, &mccount);                 /* number of macro coords */

      for (i = 0; i < mccount; i++)
        xdr_vector(xdrp, (char *)mccoord[i], DIM_OF_WORLD,
                   sizeof(REAL), (xdrproc_t) xdr_REAL);



      for (m = 0, mel = mesh->first_macro_el;  mel;  m++, mel = mel->next)
      {
        xdr_vector(xdrp, (char *)mcindex[m], N_VERTICES, sizeof(int),
                   (xdrproc_t)  xdr_int);
        xdr_vector(xdrp, (char *)mel->bound, N_VERTICES,
                   sizeof(S_CHAR), (xdrproc_t) xdr_S_CHAR);


#if DIM == 2
        for (i = 0; i < N_EDGES; i++)
          bound_sc[i] = GET_BOUND(mel->boundary[i]);

        xdr_vector(xdrp, (char *)bound_sc, N_EDGES, sizeof(S_CHAR),
                   (xdrproc_t)  xdr_S_CHAR);
#endif

#if DIM == 3
        for (i = 0; i < (N_FACES+N_EDGES); i++)
          bound_sc[i] = GET_BOUND(mel->boundary[i]);

        xdr_vector(xdrp, (char *)bound_sc, (N_FACES+N_EDGES),
                   sizeof(S_CHAR), (xdrproc_t)  xdr_S_CHAR);
#endif

        for (i = 0; i < N_NEIGH; i++)
        {
          if ((meln = mel->neigh[i]))
            neigh_i[i] = meln->index;
          else
            neigh_i[i] = -1;
        }

        xdr_vector(xdrp, (char *)neigh_i, N_NEIGH, sizeof(int),
                   (xdrproc_t)  xdr_int);
        xdr_vector(xdrp, (char *)mel->opp_vertex, N_NEIGH,
                   sizeof(U_CHAR), (xdrproc_t)  xdr_U_CHAR);


#if DIM==3
        xdr_U_CHAR(xdrp, &(mel->el_type));
#endif


        write_el_recursive(mel->el);
      }
      TEST_EXIT(m == mesh->n_macro_el)
        ("m %d != n_macro_el %d\n", m, mesh->n_macro_el);


      MEM_FREE(mcindex, mesh->n_macro_el, intNV);
    }

    // write index set
    gIndex.processXdr(xdrp);

    strcpy(s,"EOF.");                              /* file end marker */
    xdr_string(xdrp, &s, strileng);



#if DIM == 3
    if (face_admin)
    {
      MEM_FREE(face_ptrs, face_admin->used_count, DOF*);
      MEM_FREE(face_dofs, face_admin->used_count, DOF);
      if (edge_admin == face_admin) edge_admin = nil;
      if (vert_admin == face_admin) vert_admin = nil;
      face_admin = nil;
    }
#endif

#if DIM > 1
    if (edge_admin)
    {
      MEM_FREE(edge_ptrs, edge_admin->used_count, DOF*);
      MEM_FREE(edge_dofs, edge_admin->used_count, DOF);
      if (vert_admin == edge_admin) vert_admin = nil;
      edge_admin = nil;
    }
#endif

    if (vert_admin)
    {
      MEM_FREE(vert_ptrs, vert_admin->used_count, DOF*);
      MEM_FREE(vert_dofs, vert_admin->used_count, DOF);
      vert_admin = nil;
    }

    MEM_FREE(s,strileng,char);
    xdr_close_file(xdrp);

    return(0);
  }

  /****************************************************************************/

  static void write_el_recursive(EL *el)
  {
    FUNCNAME("write_el_recursive");
    int i, n, node0, n1;
    static U_CHAR uc_no = 0, uc_yes = 1;

    TEST_EXIT(el) ("el == nil\n");

    //std::cout << "write element " << el->index << "\n";
    xdr_int(xdrp,& (el->index));

    if (el->child[0])
    {
      TEST_EXIT(el->child[1]) ("child 0 but no child 1\n");

      xdr_U_CHAR(xdrp, &uc_yes);
    }
    else
    {
      xdr_U_CHAR(xdrp, &uc_no);
    }

#if DIM > 1
    if (el->new_coord)
    {
      xdr_U_CHAR(xdrp, &uc_yes);
      xdr_vector(xdrp, (char *)el->new_coord, DIM_OF_WORLD,
                 sizeof(REAL), (xdrproc_t)  xdr_REAL);
    }
    else
    {
      xdr_U_CHAR(xdrp, &uc_no);
    }
#endif

    if (n_dof[VERTEX] > 0)
    {
      node0 = node[VERTEX];
      n1 = vert_admin->n0_dof[VERTEX];

      for (i = 0; i < N_VERTICES; i++)
        xdr_int(xdrp, &vert_dofs[el->dof[node0 + i][n1]]);
    }

    if (IS_LEAF_EL(el) || preserve_coarse_dofs)
    {
#if DIM > 1
      if (n_dof[EDGE] > 0)
      {
        node0 = node[EDGE];
        n1 = edge_admin->n0_dof[EDGE];

        for (i = 0; i < N_EDGES; i++)
          xdr_int(xdrp, &edge_dofs[el->dof[node0 + i][n1]]);
      }
#endif

#if DIM == 3
      if (n_dof[FACE] > 0)
      {
        node0 = node[FACE];
        n1 = face_admin->n0_dof[FACE];

        for (i = 0; i < N_FACES; i++)
          xdr_int(xdrp, &face_dofs[el->dof[node0 + i][n1]]);
      }
#endif

      if ((n = n_dof[CENTER]) > 0)
      {
        node0 = node[CENTER];

        xdr_vector(xdrp, (char *)el->dof[node0], n, sizeof(DOF),
                   (xdrproc_t)  xdr_DOF);
      }
    }

    if (el->child[0])
    {
      for (i = 0; i < 2; i++)
        write_el_recursive(el->child[i]);
    }

    return;
  }

  /****************************************************************************/
  /* write DOF vectors of various types                                       */
  /****************************************************************************/
  typedef DOF_REAL_VEC DOF_VEC;
  static int write_dof_vec_xdr(const DOF_VEC *dv, const char *filename,
                               const char *dofvectype)
  {
    FUNCNAME("write_dof_vec_xdr");
    const FE_SPACE  *fe_space;
    const DOF_ADMIN *admin;
    MESH            *mesh;
    int i, iadmin, last;
    char *s;
    int strileng;


    if (!dv || !(fe_space = dv->fe_space))
    {
      ERROR("no %s or fe_space - no file created\n", dofvectype);
      return(1);
    }
    if (!(admin = fe_space->admin) || !(mesh = admin->mesh))
    {
      ERROR("no dof_admin or dof_admin->mesh - no file created\n");
      return(1);
    }

    dof_compress(mesh);

    iadmin = -1;
    for (i = 0; i < mesh->n_dof_admin; i++)
      if (mesh->dof_admin[i] == admin)
      {
        iadmin = i;
        break;
      }
    if (iadmin < 0)
    {
      ERROR("vec->admin not in mesh->dof_admin[] - no file created\n");
      return(1);
    }

    last = admin->used_count;
    TEST_EXIT(last <= dv->size)
      ("dof_vec->size %d < admin->size_used %d\n", dv->size, last);



    if (!(xdrp = xdr_open_file(filename, XDR_ENCODE)))
    {
      ERROR("cannot open file %s\n",filename);
      return(1);
    }




    strileng = 17;         /* length of dofvectype with terminating \0 */

    s=MEM_ALLOC(strileng, char);


    strcpy(s,dofvectype);
    xdr_string(xdrp, &s, strileng);


    if (dv->name)
    {
      i = strlen(dv->name);       /* length without terminating \0 */
      if ((i+1) > strileng)
      {
        strileng = i+1;
        s=MEM_REALLOC(s, (strileng-i-1), strileng, char);
      }
      strcpy(s,dv->name);
    }
    else
    {
      i = 0;
      strcpy(s,"");
    }
    xdr_int(xdrp, &i);
    xdr_string(xdrp, &s, strileng);


    xdr_vector(xdrp, (char *)admin->n_dof, DIM+1, sizeof(int),
               (xdrproc_t)  xdr_int);

    if (fe_space->bas_fcts)
    {
      if (fe_space->bas_fcts->name)
      {
        i = strlen(fe_space->bas_fcts->name); /* length without terminating \0 */
        if ((i+1) > strileng)
        {
          strileng = i+1;
          s=MEM_REALLOC(s, (strileng-i-1), strileng, char);
        }
      }
      else
        i = 0;

      xdr_int(xdrp, &i);

      if (i)
      {
        strcpy(s,fe_space->bas_fcts->name);
        xdr_string(xdrp, &s, strileng);
      }
    }
    else
    {
      i = 0;
      xdr_int(xdrp, &i);
    }



    xdr_int(xdrp, &last);

    if (last)
    {
      if (!strncmp(dofvectype, "DOF_REAL_VEC    ", 12))
        xdr_vector(xdrp, (char *)dv->vec, last, sizeof(REAL),
                   (xdrproc_t)  xdr_REAL);
      else if (!strncmp(dofvectype, "DOF_REAL_D_VEC  ", 12))
        xdr_vector(xdrp, (char *)dv->vec, last*DIM_OF_WORLD,
                   sizeof(REAL), (xdrproc_t)  xdr_REAL);
      else if (!strncmp(dofvectype, "DOF_INT_VEC     ", 12))
        xdr_vector(xdrp, (char *)dv->vec, last, sizeof(int),
                   (xdrproc_t)  xdr_int);
      else if (!strncmp(dofvectype, "DOF_SCHAR_VEC   ", 12))
        xdr_vector(xdrp, (char *)dv->vec, last, sizeof(S_CHAR),
                   (xdrproc_t)  xdr_S_CHAR);
      else if (!strncmp(dofvectype, "DOF_UCHAR_VEC   ", 12))
        xdr_vector(xdrp, (char *)dv->vec, last, sizeof(U_CHAR),
                   (xdrproc_t)  xdr_U_CHAR);
      else
        ERROR("ivalid file id %s\n",dofvectype);
    }




    strcpy(s,"EOF.");                                    /* file end marker */
    xdr_string(xdrp, &s, strileng);


    MEM_FREE(s,strileng,char);


    xdr_close_file(xdrp);

    return(0);
  }
  /****************************************************************************/

  int write_dof_real_vec_xdr(const DOF_REAL_VEC *dv, const char *fn)
  {
    return(write_dof_vec_xdr((const DOF_VEC *)dv, fn, "DOF_REAL_VEC    "));
  }

  int write_dof_real_d_vec_xdr(const DOF_REAL_D_VEC *dv, const char *fn)
  {
    return(write_dof_vec_xdr((const DOF_VEC *)dv, fn, "DOF_REAL_D_VEC  "));
  }

  int write_dof_int_vec_xdr(const DOF_INT_VEC *dv, const char *fn)
  {
    return(write_dof_vec_xdr((const DOF_VEC *)dv, fn, "DOF_INT_VEC     "));
  }

  int write_dof_schar_vec_xdr(const DOF_SCHAR_VEC *dv, const char *fn)
  {
    return(write_dof_vec_xdr((const DOF_VEC *)dv, fn, "DOF_SCHAR_VEC   "));
  }

  int write_dof_uchar_vec_xdr(const DOF_UCHAR_VEC *dv, const char *fn)
  {
    return(write_dof_vec_xdr((const DOF_VEC *)dv, fn, "DOF_UCHAR_VEC   "));
  }
  /****************************************************************************/

} // end namespace
