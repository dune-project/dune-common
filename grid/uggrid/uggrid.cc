// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
//************************************************************************
//
//  Implementation von UGGrid
//
//  namespace Dune
//
//************************************************************************

namespace Dune
{

#if 0
  static ALBERT EL_INFO statElInfo[DIM+1];

  // singleton holding reference elements
  template<int dim> struct AlbertGridReferenceElement
  {
    enum { dimension = dim };

    static AlbertGridElement<dim,dim> refelem;
    static ALBERT EL_INFO elInfo_;

  };


  // initialize static variable with bool constructor
  // (which makes reference element)
  template<int dim>
  AlbertGridElement<dim,dim> AlbertGridReferenceElement<dim>::refelem(true);

#endif


#include "uggridelement.cc"
#include "uggridentity.cc"
#include "uggridhieriterator.cc"
#include "uggridleveliterator.cc"

  //*********************************************************************
  //
  //  AlbertMarkerVertex
  //
  //*********************************************************************
#if 0
  inline void AlbertMarkerVector::makeNewSize(int newNumberOfEntries)
  {
    vec_.resize(newNumberOfEntries);
    for(Array<int>::Iterator it = vec_.begin(); it != vec_.end(); ++it)
      (*it) = -1;
  }

  inline void AlbertMarkerVector::makeSmaller(int newNumberOfEntries)
  {}

  void AlbertMarkerVector::checkMark(ALBERT EL_INFO * elInfo, int localNum)
  {
    if(vec_[elInfo->el->dof[localNum][0]] == -1)
      vec_[elInfo->el->dof[localNum][0]] = elInfo->el->index;
  }


  inline bool AlbertMarkerVector::
  notOnThisElement(ALBERT EL * el, int level, int localNum)
  {
    return (vec_[ numVertex_ * level + el->dof[localNum][0]] != el->index);
  }

  template <class GridType>
  inline void AlbertMarkerVector::markNewVertices(GridType &grid)
  {
    enum { dim      = GridType::dimension };
    enum { dimworld = GridType::dimensionworld };

    ALBERT MESH *mesh_ = grid.getMesh();

    int nvx = mesh_->n_vertices;
    // remember the number of vertices of the mesh
    numVertex_ = nvx;

    int maxlevel = grid.maxlevel();

    int number = (maxlevel+1) * nvx;
    if(vec_.size() < number) vec_.resize( 2 * number );
    for(int i=0; i<vec_.size(); i++) vec_[i] = -1;

    for(int level=0; level <= maxlevel; level++)
    {
      typedef typename GridType::Traits<0>::LevelIterator LevelIteratorType;
      LevelIteratorType endit = grid.template lend<0> (level);
      for(LevelIteratorType it = grid.template lbegin<0> (level); it != endit; ++it)
      {
        for(int local=0; local<dim+1; local++)
        {
          int num = it->getElInfo()->el->dof[local][0];
          if( vec_[level * nvx + num] == -1 )
            vec_[level * nvx + num] = it->globalIndex();
        }
      }
      // remember the number of entity on level and codim = 0
    }
  }

  inline void AlbertMarkerVector::print()
  {
    printf("\nEntries %d \n",vec_.size());
    for(int i=0; i<vec_.size(); i++)
      printf("Konten %d visited on Element %d \n",i,vec_[i]);
  }
#endif
  //***********************************************************************
  //
  // --UGGrid
  // --Grid
  //
  //***********************************************************************

  int UGGrid < 2, 2 >::numOfUGGrids = 0;
  int UGGrid < 3, 3 >::numOfUGGrids = 0;

  template < int dim, int dimworld >
  inline UGGrid < dim, dimworld >::UGGrid()
  {
    if (numOfUGGrids==0) {

      // Init the UG system
      int argc = 1;
      char* arg = {"dune.exe"};
      char** argv = &arg;

      UG3d::InitUg(&argc, &argv);

      // Create a dummy problem
      UG3d::CoeffProcPtr coeffs[1];
      UG3d::UserProcPtr upp[1];

      upp[0] = NULL;
      coeffs[0] = NULL;

      if (UG3d::CreateBoundaryValueProblem("DuneDummyProblem", NULL,
                                           1,coeffs,1,upp) == NULL)
        assert(false);


      // A Dummy new format
      //newformat P1_conform $V n1: nt 9 $M implicit(nt): mt 2 $I n1;
      char* newformatArgs[4] = {"newformat DuneFormat",
                                "V n1: nt 9",
                                "M implicit(nt): mt 2",
                                "I n1"};
      UG3d::CreateFormatCmd(4, newformatArgs);

    }

    numOfUGGrids++;
  }

  template < int dim, int dimworld >
  inline UGGrid < dim, dimworld >::~UGGrid()
  {
    numOfUGGrids--;

    // Shut down UG if this was the last existing UGGrid object
    if (numOfUGGrids == 0) {

      UG3d::ExitUg();

    }

  };

  //template < int dim, int dimworld > template<int codim>
  template <>
#ifndef __GNUC__
  template <>
#endif
  inline UGGridLevelIterator<3,3,3>
  UGGrid < 3, 3 >::lbegin<3> (int level) const
  {
    UG3d::multigrid* theMG = UG3d::GetMultigrid("DuneMG");
    assert(theMG);
    UG3d::grid* theGrid = theMG->grids[level];

    UGGridLevelIterator<3,3,3> it((*const_cast<UGGrid< 3, 3 >* >(this)),level);

    // Choose the first *inner* vertex
    UG3d::node* mytarget = theGrid->firstNode[0];
    UG3d::vertex* myvertex = mytarget->myvertex;

#define OBJT(p) ReadCW(p, UG3d::OBJ_CE)
    while (mytarget && OBJT(myvertex)!= UG3d::IVOBJ) {
#undef OBJT

      mytarget = mytarget->succ;
      if (!mytarget)
        break;
      myvertex = mytarget->myvertex;

    }

    it.setToTarget(mytarget);
    return it;
  }

  template <>
#ifndef __GNUC__
  template <>
#endif
  inline UGGridLevelIterator<0,3,3>
  UGGrid < 3, 3 >::lbegin<0> (int level) const
  {
    UG3d::multigrid* theMG = UG3d::GetMultigrid("DuneMG");
    assert(theMG);
    UG3d::grid* theGrid = theMG->grids[level];

    UGGridLevelIterator<0,3,3> it((*const_cast<UGGrid< 3, 3 >* >(this)),level);
    it.setToTarget(theGrid->elements[0]);
    return it;
  }

  template<int dim, int dimworld> template<int codim>
  inline UGGridLevelIterator<codim, dim, dimworld>
  UGGrid<dim, dimworld>::lbegin (int level) const
  {
    printf("UGGrid<%d, %d>::lbegin<%d> not implemented\n", dim, dimworld, codim);
    UGGridLevelIterator<codim,dim,dimworld> dummy((*const_cast<UGGrid< dim, dimworld >* >(this)),level);
    return dummy;
  }

  template < int dim, int dimworld > template<int codim>
  inline UGGridLevelIterator<codim,dim,dimworld>
  UGGrid < dim, dimworld >::lend (int level) const
  {
    UGGridLevelIterator<codim,dim,dimworld> it((*const_cast<UGGrid< dim, dimworld >* >(this)),level);
    return it;
  }

  template < int dim, int dimworld >
  inline int UGGrid < dim, dimworld >::size (int level, int codim) const
  {
    //     enum { numCodim = dim+1 };
    //     int ind = (level * numCodim) + codim;

    //     if(size_[ind] == -1)
    {
      int numberOfElements = 0;

      if(codim == 0)
      {
        UGGridLevelIterator<0,dim,dimworld> endit = lend<0>(level);
        for(UGGridLevelIterator<0,dim,dimworld> it = lbegin<0>(level);
            it != endit; ++it)
          numberOfElements++;
      }
      if(codim == 1)
      {
        UGGridLevelIterator<1,dim,dimworld> endit = lend<1>(level);
        for(UGGridLevelIterator<1,dim,dimworld> it = lbegin<1>(level);
            it != endit; ++it)
          numberOfElements++;
      }
      if(codim == 2)
      {
        UGGridLevelIterator<2,dim,dimworld> endit = lend<2>(level);
        for(UGGridLevelIterator<2,dim,dimworld> it = lbegin<2>(level);
            it != endit; ++it)
          numberOfElements++;
      }

      if(codim == 3)
      {
        UGGridLevelIterator<3,dim,dimworld> endit = lend<3>(level);
        for(UGGridLevelIterator<3,dim,dimworld> it = lbegin<3>(level);
            it != endit; ++it)
          numberOfElements++;
      }

      //            size_[ind] = numberOfElements;
      return numberOfElements;
    }
    //     else
    //         {
    //             return size_[ind];
    //         }
  }

#if 0


  template < int dim, int dimworld >
  inline void AlbertGrid < dim, dimworld >::calcExtras ()
  {
    if(numberOfEntitys_.size() != dim+1)
      numberOfEntitys_.resize(dim+1);

    for(int i=0; i<dim+1; i++) numberOfEntitys_[i] = NULL;

    numberOfEntitys_[0] = &(mesh_->n_hier_elements);
    numberOfEntitys_[dim] = &(mesh_->n_vertices);
    // determine new maxlevel and mark neighbours
    maxlevel_ = ALBERT AlbertHelp::calcMaxLevelAndMarkNeighbours( mesh_, neighOnLevel_ );

    // mark vertices on elements
    vertexMarker_->markNewVertices(*this);

    // map the indices
    markNew();

    // we have a new grid
    wasChanged_ = true;
  }


  //! Index Mapping
  template < int dim, int dimworld >
  inline void AlbertGrid < dim, dimworld >::
  makeNewSize(Array<int> &a, int newNumberOfEntries)
  {
    a.resize(newNumberOfEntries);
    for(Array<int>::Iterator it = a.begin(); it != a.end(); ++it)
      (*it) = -1;
  }

  template < int dim, int dimworld > template <int codim>
  inline int AlbertGrid < dim, dimworld >::
  indexOnLevel(int globalIndex, int level)
  {
    // level = 0 is not interesting for this implementation
    // +1, because if Entity is Boundary then globalIndex == -1
    // an therefore we add 1 and get Entry 0, which schould be -1
    if (globalIndex < 0)
      return globalIndex;
    else
      return levelIndex_[codim][(level * (*(numberOfEntitys_[codim]))) + globalIndex];
  }

  // create lookup table for indices of the elements
  template < int dim, int dimworld >
  inline void AlbertGrid < dim, dimworld >::markNew()
  {
    // only for gcc, means notin'
    //typedef AlbertGrid < dim ,dimworld > GridType;

    int nElements = mesh_->n_hier_elements;
    int nVertices = mesh_->n_vertices;

    int number = (maxlevel_+1) * nElements;
    if(number > levelIndex_[0].size())
      //makeNewSize(levelIndex_[0], number);
      levelIndex_[0].resize(number);

    // make new size and set all levels to -1 ==> new calc
    if((maxlevel_+1)*(numCodim) > size_.size())
      makeNewSize(size_, 2*((maxlevel_+1)*numCodim));

    // the easiest way, in Albert all elements have unique global element
    // numbers, therefore we make one big array from which we get with the
    // global unique number the local level number
    for(int level=0; level <= maxlevel_; level++)
    {
      typedef AlbertGridLevelIterator<0,dim,dimworld> LevelIterator;
      int num = 0;
      LevelIterator endit = lend<0>(level);
      for(LevelIterator it = lbegin<0> (level); it != endit; ++it)
      {
        int no = it->globalIndex();
        levelIndex_[0][level * nElements + no] = num;
        num++;
      }
      // remember the number of entity on level and codim = 0
      size_[level*numCodim /* +0 */] = num;
    };

    if((maxlevel_+1) * nVertices > levelIndex_[dim].size())
      makeNewSize(levelIndex_[dim], ((maxlevel_+1)* nVertices));

    for(int level=0; level <= maxlevel_; level++)
    {
      //std::cout << level << " " << maxlevel_ << "\n";
      typedef AlbertGridLevelIterator<dim,dim,dimworld> LevelIterator;
      int num = 0;
      LevelIterator endit = lend<dim> (level);
      for(LevelIterator it = lbegin<dim> (level); it != endit; ++it)
      {
        int no = it->globalIndex();
        //std::cout << no << " Glob Num\n";
        levelIndex_[dim][level * nVertices + no] = num;
        num++;
      }
      //   std::cout << "Done LevelIt \n";
      // remember the number of entity on level and codim = 0
      size_[level*numCodim + dim] = num;
    };
  }

  // if defined some debugging test were made that reduce the performance
  // so they were switch off normaly

  //#define DEBUG_FILLELINFO
  //*********************************************************************
  //  fillElInfo 2D
  //*********************************************************************
  template<int dim, int dimworld>
  inline void AlbertGrid<dim,dimworld >::
  fillElInfo(int ichild, int actLevel , const ALBERT EL_INFO *elinfo_old, ALBERT EL_INFO *elinfo, bool hierarchical) const
  {

#if 1
    ALBERT fill_elinfo(ichild,elinfo_old,elinfo);
#else

    ALBERT EL      *nb=NULL;
    ALBERT EL      *el = elinfo_old->el;
    ALBERT FLAGS fill_flag = elinfo_old->fill_flag;
    ALBERT FLAGS fill_opp_coords;

    //ALBERT_TEST_EXIT(el->child[0])("no children?\n");
    // in this implementation we can go down without children
    if(el->child[0])
    {
#ifdef DEBUG_FILLELINFO
      printf("Called fillElInfo with El %d \n",el->index);
#endif

      ALBERT_TEST_EXIT((elinfo->el = el->child[ichild])) ("missing child %d?\n", ichild);

      elinfo->macro_el  = elinfo_old->macro_el;
      elinfo->fill_flag = fill_flag;
      elinfo->mesh      = elinfo_old->mesh;
      elinfo->parent    = el;
      elinfo->level     = elinfo_old->level + 1;

      if (fill_flag & FILL_COORDS)
      {
        if (el->new_coord)
        {
          for (int j = 0; j < dimworld; j++)
            elinfo->coord[2][j] = el->new_coord[j];
        }
        else
        {
          for (int j = 0; j < dimworld; j++)
            elinfo->coord[2][j] =
              0.5 * (elinfo_old->coord[0][j] + elinfo_old->coord[1][j]);
        }

        if (ichild==0)
        {
          for (int j = 0; j < dimworld; j++)
          {
            elinfo->coord[0][j] = elinfo_old->coord[2][j];
            elinfo->coord[1][j] = elinfo_old->coord[0][j];
          }
        }
        else
        {
          for (int j = 0; j < dimworld; j++)
          {
            elinfo->coord[0][j] = elinfo_old->coord[1][j];
            elinfo->coord[1][j] = elinfo_old->coord[2][j];
          }
        }
      }

      /* ! NEIGH_IN_EL */
      // make the neighbour relations

      if (fill_flag & (FILL_NEIGH | FILL_OPP_COORDS))
      {
        fill_opp_coords = (fill_flag & FILL_OPP_COORDS);

        // first child
        if (ichild==0)
        {
          elinfo->opp_vertex[2] = elinfo_old->opp_vertex[1];
          if ((elinfo->neigh[2] = elinfo_old->neigh[1]))
          {
            if (fill_opp_coords)
            {
              for (int j=0; j<dimworld; j++)
                elinfo->opp_coord[2][j] = elinfo_old->opp_coord[1][j];
            }

            if(hierarchical)
            {
              // this is new
              ALBERT EL * nextNb = elinfo->neigh[2]->child[0];
              if(nextNb)
              {
#ifdef DEBUG_FILLELINFO
                ALBERT_TEST_EXIT(elinfo_old->opp_vertex[2] != 2) ("invalid neighbour\n");
#endif
                int oppV = elinfo->opp_vertex[2];
                // if oppV == 0 neighbour must be the child 1
                if( oppV == 0)
                  nextNb = elinfo->neigh[2]->child[1];

                if(neighOnLevel_[nextNb->index] <= actLevel )
                  elinfo->neigh[2] = nextNb;
                // if we go down the opposite vertex now must be 2
                elinfo->opp_vertex[2] = 2;

                if (fill_opp_coords)
                {
                  if(oppV == 0)
                  {
                    for (int k=0; k < dimworld ; k++)
                      elinfo->opp_coord[2][k] = 0.5 *
                                                (elinfo_old->opp_coord[oppV][k] +
                                                 elinfo_old->coord[1][k]);
                  }
                  else
                  {
                    for (int k=0; k < dimworld ; k++)
                      elinfo->opp_coord[2][k] = 0.5 *
                                                (elinfo_old->opp_coord[oppV][k] +
                                                 elinfo_old->coord[0][k]);
                  }
                }
              }
            } // end hierarchical

          } // end new code

          if (el->child[0])
          {
            bool goDownNextChi = false;
            ALBERT EL *chi1 = el->child[1];

            if(chi1->child[0])
            {
              if(neighOnLevel_[chi1->child[1]->index] <= actLevel )
                goDownNextChi = true;
            }

            if(goDownNextChi)
            {
              // set neighbour
              ALBERT_TEST_EXIT((elinfo->neigh[1] = chi1->child[1]))
                ("el->child[1]->child[0]!=nil, but el->child[1]->child[1]=nil\n");

              elinfo->opp_vertex[1] = 2;
              if (fill_opp_coords)
              {
                if (chi1->new_coord)
                {
                  for (int j=0; j<dimworld; j++)
                    elinfo->opp_coord[1][j] = chi1->new_coord[j];
                }
                else
                {
                  for (int j=0; j<dimworld; j++)
                    elinfo->opp_coord[1][j] =
                      0.5 * (elinfo_old->coord[1][j] + elinfo_old->coord[2][j]);
                }
              }
            }
            else
            {
              // set neighbour
              ALBERT_TEST_EXIT((elinfo->neigh[1] = chi1))
                ("el->child[0] != nil, but el->child[1] = nil\n");

              elinfo->opp_vertex[1] = 0;
              if (fill_opp_coords)
              {
                for (int j=0; j<dimworld; j++)
                  elinfo->opp_coord[1][j] = elinfo_old->coord[1][j];
              }
            }
          }
#ifdef DEBUG_FILLELINFO
          else
          {
            ALBERT_TEST_EXIT((el->child[0])) ("No Child\n");
          }
#endif

          if ((nb = elinfo_old->neigh[2]))
          {
            // the neighbour across the refinement edge
#ifdef DEBUG_FILLELINFO
            printf("El  %d , Neigh %d \n",el->index,nb->index);
            printf("El  %d , Neigh %d \n",el->child[0]->index,nb->index);
            //printf("OppVx %d \n",elinfo_old->opp_vertex[2]);
            //printf("El  %d , Neigh %d \n",el->index,nb->index);
            ALBERT_TEST_EXIT(elinfo_old->opp_vertex[2] == 2) ("invalid neighbour\n");
            //ALBERT_TEST_EXIT((nb->child[0]))("missing child[0] of nb2 ?\n");
#endif
            if(nb->child[0])
            {
              ALBERT_TEST_EXIT((nb = nb->child[1])) ("missing child[1]?\n");
            }
            else
            {
              printf("El  %d , Neigh %d \n",el->child[0]->index,nb->index);
              //nb = nb->child[0];
            }

            if (nb->child[0])
            {
              bool goDownNextChi = false;
              if(neighOnLevel_[nb->child[0]->index] <= actLevel )
                goDownNextChi = true;

              if(goDownNextChi)
              {

                elinfo->opp_vertex[0] = 2;
                if (fill_opp_coords)
                {
                  if (nb->new_coord)
                  {
                    for (int j=0; j<dimworld; j++)
                      elinfo->opp_coord[0][j] = nb->new_coord[j];
                  }
                  else
                  {
                    for (int j=0; j<dimworld; j++)
                    {
                      elinfo->opp_coord[0][j] = 0.5*
                                                (elinfo_old->opp_coord[2][j] + elinfo_old->coord[0][j]);
                    }
                  }
                }
                nb = nb->child[0];
              }
              else
              {
                elinfo->opp_vertex[0] = 1;
                if (fill_opp_coords)
                {
                  for (int j=0; j<dimworld; j++)
                    elinfo->opp_coord[0][j] = elinfo_old->opp_coord[2][j];
                }
              }
            }
          }
          elinfo->neigh[0] = nb;
        }
        else /* ichild==1 , second child */
        {
          elinfo->opp_vertex[2] = elinfo_old->opp_vertex[0];
          if ((elinfo->neigh[2] = elinfo_old->neigh[0]))
          {
            if (fill_opp_coords)
            {
              for (int j=0; j<dimworld; j++)
                elinfo->opp_coord[2][j] = elinfo_old->opp_coord[0][j];
            }

            if(hierarchical)
            {
              ALBERT EL * nextNb = elinfo->neigh[2]->child[0];
              if(nextNb)
              {
#ifdef DEBUG_FILLELINFO
                ALBERT_TEST_EXIT(elinfo_old->opp_vertex[2] != 2) ("invalid neighbour\n");
#endif
                int oppV = elinfo->opp_vertex[2];
                // if oppV == 0 neighbour must be the child 1
                if( oppV == 0 )
                  nextNb = elinfo->neigh[2]->child[1];

                if(neighOnLevel_[nextNb->index] <= actLevel )
                  elinfo->neigh[2] = nextNb;
                // if we go down the opposite vertex now must be 2
                elinfo->opp_vertex[2] = 2;

                if (fill_opp_coords)
                {
                  // add new coord here
                  if(oppV == 0)
                  {
                    for (int k=0; k < dimworld ; k++)
                      elinfo->opp_coord[2][k] = 0.5 *
                                                (elinfo_old->opp_coord[oppV][k] +
                                                 elinfo_old->coord[1][k]);
                  }
                  else
                  {
                    for (int k=0; k < dimworld ; k++)
                      elinfo->opp_coord[2][k] = 0.5 *
                                                (elinfo_old->opp_coord[oppV][k] +
                                                 elinfo_old->coord[0][k]);
                  }
                }
              }
            } // end hierarchical
          }

          { // begin chi0
            ALBERT EL *chi0=el->child[0];
            bool goDownChild = false;

            if (chi0->child[0])
            {
              if(neighOnLevel_[chi0->child[0]->index] <= actLevel )
                goDownChild = true;
            }

            if(goDownChild)
            {
              elinfo->neigh[0] = chi0->child[0];
              elinfo->opp_vertex[0] = 2;
              if (fill_opp_coords)
              {
                if (chi0->new_coord)
                {
                  for (int j=0; j<dimworld; j++)
                    elinfo->opp_coord[0][j] = chi0->new_coord[j];
                }
                else
                {
                  for (int j=0; j<dimworld; j++)
                    elinfo->opp_coord[0][j] = 0.5 *
                                              (elinfo_old->coord[0][j] + elinfo_old->coord[2][j]);
                }
              }
            }
            else
            {
              elinfo->neigh[0] = chi0;
              elinfo->opp_vertex[0] = 1;
              if (fill_opp_coords)
              {
                for (int j=0; j<dimworld; j++)
                  elinfo->opp_coord[0][j] = elinfo_old->coord[0][j];
              }
            }
          } // end chi0

          if ((nb = elinfo_old->neigh[2]))
          {
            // the neighbour across the refinement edge
#ifdef DEBUG_FILLELINFO
            //printf("OppVx %d \n",elinfo_old->opp_vertex[2]);
            ALBERT_TEST_EXIT(elinfo_old->opp_vertex[2] == 2) ("invalid neighbour\n");
#endif
            if(nb->child[0])
              ALBERT_TEST_EXIT((nb = nb->child[0])) ("missing child?\n");

            bool goDownChild = false;
            if (nb->child[0])
            {
              if(neighOnLevel_[nb->child[1]->index] <= actLevel )
                goDownChild = true;
            }

            if(goDownChild)
            {
              // we go down, calc new coords
              elinfo->opp_vertex[1] = 2;
              if (fill_opp_coords)
              {
                if (nb->new_coord)
                  for (int j=0; j<dimworld; j++)
                    elinfo->opp_coord[1][j] = nb->new_coord[j];
                else
                  for (int j=0; j<dimworld; j++)
                    elinfo->opp_coord[1][j] = 0.5 *
                                              (elinfo_old->opp_coord[2][j] + elinfo_old->coord[1][j]);
              }
              nb = nb->child[1];
            }
            else
            {
              // we are nto going down, so copy the coords
              elinfo->opp_vertex[1] = 0;
              if (fill_opp_coords)
              {
                for (int j=0; j<dimworld; j++)
                  elinfo->opp_coord[1][j] = elinfo_old->opp_coord[2][j];
              }
            }
          }
          elinfo->neigh[1] = nb;
        }
      }

      if (fill_flag & FILL_BOUND)
      {
        if (elinfo_old->boundary[2])
          elinfo->bound[2] = elinfo_old->boundary[2]->bound;
        else
          elinfo->bound[2] = INTERIOR;

        if (ichild==0)
        {
          elinfo->bound[0] = elinfo_old->bound[2];
          elinfo->bound[1] = elinfo_old->bound[0];
          elinfo->boundary[0] = elinfo_old->boundary[2];
          elinfo->boundary[1] = nil;
          elinfo->boundary[2] = elinfo_old->boundary[1];
        }
        else
        {
          elinfo->bound[0] = elinfo_old->bound[1];
          elinfo->bound[1] = elinfo_old->bound[2];
          elinfo->boundary[0] = nil;
          elinfo->boundary[1] = elinfo_old->boundary[2];
          elinfo->boundary[2] = elinfo_old->boundary[0];
        }
      }

    }
    // no child exists, but go down maxlevel
    // means neighbour may be changed but element itself not
    else
    {
      memcpy(elinfo,elinfo_old,sizeof(ALBERT EL_INFO));
      elinfo->level = (unsigned  char) (elinfo_old->level + 1);

      if (fill_flag & (FILL_NEIGH | FILL_OPP_COORDS))
      {
        fill_opp_coords = (fill_flag & FILL_OPP_COORDS);

        // we use triangles here
        enum { numberOfNeighbors = 3 };
        enum { numOfVx = 3 };
        for(int i=0; i<numberOfNeighbors; i++)
        {
          if(elinfo_old->neigh[i])
          {
            // if children of neighbour
            if(elinfo_old->neigh[i]->child[0])
            {
#ifdef DEBUG_FILLELINFO
              ALBERT_TEST_EXIT(elinfo_old->opp_vertex[2] != 2) ("invalid neighbour\n");
#endif
              int oppV = elinfo_old->opp_vertex[i];
              if(oppV == 0)
              {
                elinfo->neigh[i] = elinfo_old->neigh[i]->child[1];
                elinfo->opp_vertex[i] = 2;

                if( fill_opp_coords )
                  for (int k=0; k < dimworld ; k++)
                    elinfo->opp_coord[i][k] = 0.5 *
                                              (elinfo_old->opp_coord[oppV][k] +
                                               elinfo_old->coord[(i-1)%numOfVx][k]);
              }
              else
              {
                elinfo->neigh[i] = elinfo_old->neigh[i]->child[0];
                elinfo->opp_vertex[i] = 2;
                if( fill_opp_coords )
                  for (int k=0; k < dimworld ; k++)
                    elinfo->opp_coord[i][k] = 0.5 *
                                              (elinfo_old->opp_coord[oppV][k] +
                                               elinfo_old->coord[(i+1)%numOfVx][k]);
              }
            }
          }
        }
      } // end if (fill_flag & (FILL_NEIGH | FILL_OPP_COORDS))
    } //end else

#endif

  } // end Grid::fillElInfo 2D


  //***********************************************************************
  // fillElInfo 3D
  //***********************************************************************
#if DIM == 3
  template <>
  inline void AlbertGrid<3,3>::
  fillElInfo(int ichild, int actLevel , const ALBERT EL_INFO *elinfo_old,
             ALBERT EL_INFO *elinfo, bool hierarchical) const
  {
    enum { dim = 3 };
    enum { dimworld = 3 };

#if 0
    ALBERT fill_elinfo(ichild,elinfo_old,elinfo);
#else
    static S_CHAR child_orientation[3][2] = {{1,1}, {1,-1}, {1,-1}};

    int k;
    int el_type=0;                                   /* el_type in {0,1,2} */
    int ochild=0;                       /* index of other child = 1-ichild */
    int     *cv=nil;                   /* cv = child_vertex[el_type][ichild] */
    int     (*cvg)[4]=nil;                    /* cvg = child_vertex[el_type] */
    int     *ce;                     /* ce = child_edge[el_type][ichild] */
    int iedge;
    EL      *nb, *nbk, **neigh_old;
    EL      *el_old = elinfo_old->el;
    FLAGS fill_flag = elinfo_old->fill_flag;
    DOF    *dof;
#if !NEIGH_IN_EL
    int ov;
    EL      **neigh;
    FLAGS fill_opp_coords;
    U_CHAR  *opp_vertex;
#endif

    TEST_EXIT(el_old->child[0]) ("missing child?\n"); /* Kuni 22.08.96 */

    elinfo->el        = el_old->child[ichild];
    elinfo->macro_el  = elinfo_old->macro_el;
    elinfo->fill_flag = fill_flag;
    elinfo->mesh      = elinfo_old->mesh;
    elinfo->parent    = el_old;
    elinfo->level     = elinfo_old->level + 1;
#if !NEIGH_IN_EL
    elinfo->el_type   = (elinfo_old->el_type + 1) % 3;
#endif

    REAL_D * opp_coord = elinfo->opp_coord;
    REAL_D * coord = elinfo->coord;

    const REAL_D * old_coord = elinfo_old->coord;
    const REAL_D * oldopp_coord = elinfo_old->opp_coord;


    TEST_EXIT(elinfo->el) ("missing child %d?\n", ichild);

    if (fill_flag) {
      el_type = EL_TYPE(elinfo_old->el, elinfo_old);
      cvg = child_vertex[el_type];
      cv = cvg[ichild];
      ochild = 1-ichild;
    }

    if (fill_flag & FILL_COORDS)
    {
      for (int i=0; i<3; i++) {
        for (int j = 0; j < dimworld; j++) {
          coord[i][j] = old_coord[cv[i]][j];
        }
      }
      if (el_old->new_coord)
        for (int j = 0; j < dimworld; j++)
          coord[3][j] = el_old->new_coord[j];
      else
        for (int j = 0; j < dimworld; j++)
          coord[3][j] = 0.5*
                        (old_coord[0][j] + old_coord[1][j]);
    }


#if NEIGH_IN_EL
    if (fill_flag & FILL_OPP_COORDS)
    {
      neigh_old = el_old->neigh;

      /*----- nb[0] is other child -------------------------------------------*/

      /*    if (nb = el_old->child[ochild]) {        old version    */
      if (el_old->child[0]  &&  (nb = el_old->child[ochild])) /* Kuni 22.08.96*/
      {
        if (nb->child[0]) {   /* go down one level for direct neighbour */
          k = cvg[ochild][1];
          if (nb->new_coord)
            for (int j = 0; j < dimworld; j++)
              opp_coord[0][j] = nb->new_coord[j];
          else
            for (int j = 0; j < dimworld; j++)
              opp_coord[0][j] = 0.5*
                                (old_coord[ochild][j] + old_coord[k][j]);
        }
        else {
          for (int j = 0; j < dimworld; j++) {
            opp_coord[0][j] = old_coord[ochild][j];
          }
        }
      }
      else {
        ERROR_EXIT("no other child");
      }


      /*----- nb[1],nb[2] are childs of old neighbours nb_old[cv[i]] ----------*/

      for (int i=1; i<3; i++)
      {
        if (nb = neigh_old[cv[i]])
        {
          TEST_EXIT(nb->child[0]) ("nonconforming triangulation\n");

          for (k=0; k<2; k++) { /* look at both children of old neighbour */

            nbk = nb->child[k];
            if (nbk->dof[0] == el_old->dof[ichild]) {
              dof = nb->dof[el_old->opp_vertex[cv[i]]]; /* opp. vertex */
              if (dof == nbk->dof[1]) {
                if (nbk->child[0]) {
                  if (nbk->new_coord)
                    for (int j = 0; j < dimworld; j++)
                      opp_coord[i][j] = nbk->new_coord[j];
                  else
                    for (int j = 0; j < dimworld; j++)
                      opp_coord[i][j] = 0.5*
                                        (oldopp_coord[cv[i]][j] + old_coord[ichild][j]);
                  break;
                }
              }
              else {
                TEST_EXIT(dof == nbk->dof[2]) ("opp_vertex not found\n");
              }

              for (int j = 0; j < dimworld; j++) {
                opp_coord[i][j] = oldopp_coord[cv[i]][j];
              }
              break;
            }

          } /* end for k */
          TEST_EXIT(k<2) ("child not found with vertex\n");

        }
      } /* end for i */


      /*----- nb[3] is old neighbour neigh_old[ochild] ------------------------*/

      if (neigh_old[ochild]) {
        for (int j = 0; j < dimworld; j++) {
          opp_coord[3][j] = oldopp_coord[ochild][j];
        }
      }

    }

#else  /* ! NEIGH_IN_EL */

    if (fill_flag & (FILL_NEIGH | FILL_OPP_COORDS))
    {
      neigh      = elinfo->neigh;
      neigh_old  = (EL **) elinfo_old->neigh;
      opp_vertex = (U_CHAR *) &(elinfo->opp_vertex);
      fill_opp_coords = (fill_flag & FILL_OPP_COORDS);

      /*----- nb[0] is other child --------------------------------------------*/

      /*    if (nb = el_old->child[ochild])   old version  */
      if (el_old->child[0]  &&  (nb = el_old->child[ochild])) /*Kuni 22.08.96*/
      {
        if (nb->child[0])
        {   /* go down one level for direct neighbour */
          if (fill_opp_coords)
          {
            if (nb->new_coord)
            {
              for (int j = 0; j < dimworld; j++)
                opp_coord[0][j] = nb->new_coord[j];
            }
            else
            {
              k = cvg[ochild][1];
              for (int j = 0; j < dimworld; j++)
                opp_coord[0][j] = 0.5*
                                  (old_coord[ochild][j] + old_coord[k][j]);
            }
          }
          neigh[0]      = nb->child[1];
          opp_vertex[0] = 3;
        }
        else {
          if (fill_opp_coords) {
            for (int j = 0; j < dimworld; j++) {
              opp_coord[0][j] = old_coord[ochild][j];
            }
          }
          neigh[0]      = nb;
          opp_vertex[0] = 0;
        }
      }
      else {
        ERROR_EXIT("no other child");
        neigh[0] = nil;
      }


      /*----- nb[1],nb[2] are childs of old neighbours nb_old[cv[i]] ----------*/

      for (int i=1; i<3; i++)
      {
        if ((nb = neigh_old[cv[i]]))
        {
          TEST_EXIT(nb->child[0]) ("nonconforming triangulation\n");

          for (k=0; k<2; k++) /* look at both childs of old neighbour */
          {
            nbk = nb->child[k];
            if (nbk->dof[0] == el_old->dof[ichild]) {
              dof = nb->dof[elinfo_old->opp_vertex[cv[i]]]; /* opp. vertex */
              if (dof == nbk->dof[1])
              {
                ov = 1;
                if (nbk->child[0])
                {
                  if (fill_opp_coords)
                  {
                    if (nbk->new_coord)
                      for (int j = 0; j < dimworld; j++)
                        opp_coord[i][j] = nbk->new_coord[j];
                    else
                      for (int j = 0; j < dimworld; j++)
                        opp_coord[i][j] = 0.5*
                                          (oldopp_coord[cv[i]][j]
                                           + old_coord[ichild][j]);
                  }
                  neigh[i]      = nbk->child[0];
                  opp_vertex[i] = 3;
                  break;
                }
              }
              else
              {
                TEST_EXIT(dof == nbk->dof[2]) ("opp_vertex not found\n");
                ov = 2;
              }

              if (fill_opp_coords)
              {
                for (int j = 0; j < dimworld; j++)
                {
                  opp_coord[i][j] = oldopp_coord[cv[i]][j];
                }
              }
              neigh[i]      = nbk;
              opp_vertex[i] = ov;
              break;
            }

          } /* end for k */
          TEST_EXIT(k<2) ("child not found with vertex\n");
        }
        else
        {
          neigh[i] = nil;
        }
      } /* end for i */


      /*----- nb[3] is old neighbour neigh_old[ochild] ------------------------*/

      if ((neigh[3] = neigh_old[ochild]))
      {
        opp_vertex[3] = elinfo_old->opp_vertex[ochild];
        if (fill_opp_coords) {
          for (int j = 0; j < dimworld; j++) {
            opp_coord[3][j] = oldopp_coord[ochild][j];
          }
        }
      }
    }
#endif

    if (fill_flag & FILL_BOUND)
    {
      for (int i = 0; i < 3; i++)
      {
        elinfo->bound[i] = elinfo_old->bound[cv[i]];
      }
      elinfo->bound[3] = GET_BOUND(elinfo_old->boundary[N_FACES+0]);

      elinfo->boundary[0] = nil;
      elinfo->boundary[1] = elinfo_old->boundary[cv[1]];
      elinfo->boundary[2] = elinfo_old->boundary[cv[2]];
      elinfo->boundary[3] = elinfo_old->boundary[ochild];

      ce = child_edge[el_type][ichild];
      for (iedge=0; iedge<4; iedge++) {
        elinfo->boundary[N_FACES+iedge] =
          elinfo_old->boundary[N_FACES+ce[iedge]];
      }
      for (iedge=4; iedge<6; iedge++) {
        int i = 5 - cv[iedge-3];              /* old vertex opposite new edge */
        elinfo->boundary[N_FACES+iedge] = elinfo_old->boundary[i];
      }
    }


    if (elinfo->fill_flag & FILL_ORIENTATION) {
      elinfo->orientation =
        elinfo_old->orientation * child_orientation[el_type][ichild];
    }
#endif
  }
#endif

#endif

} // namespace Dune
