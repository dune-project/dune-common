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

#include "uggridelement.cc"
#include "uggridentity.cc"
#include "uggridhieriterator.cc"
#include "uggridleveliterator.cc"


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
      printf("Before format\n");
      UG3d::CreateFormatCmd(4, newformatArgs);
      printf("after format\n");

    }

    numOfUGGrids++;
  }

  /** \bug Actually delete the grid from UG! */
  template < int dim, int dimworld >
  inline UGGrid < dim, dimworld >::~UGGrid()
  {
    numOfUGGrids--;

    // Shut down UG if this was the last existing UGGrid object
    if (numOfUGGrids == 0) {

      UG3d::ExitUg();

    }

  };

  template < int dim, int dimworld >
  inline int UGGrid < dim, dimworld >::maxlevel() const
  {
    return mesh_.topLevel;
  }

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
    //     UG3d::vertex* myvertex = mytarget->myvertex;

    // #define OBJT(p) ReadCW(p, UG3d::OBJ_CE)
    //     while (mytarget && OBJT(myvertex)!= UG3d::IVOBJ) {
    // #undef OBJT

    //         mytarget = mytarget->succ;
    //         if (!mytarget)
    //             break;
    //         myvertex = mytarget->myvertex;

    //     }

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


} // namespace Dune
