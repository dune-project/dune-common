// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRIDDISPLAY_HH
#define DUNE_GRIDDISPLAY_HH

#include <stdarg.h>
#include <assert.h>
#include <string.h>

#include "grapewrapper.hh"
#include "grapecommon.hh"

enum { MAX_EL_DOF  = 8 };
enum { MAX_EL_FACE = 6 };

namespace GrapeInterface_two_two
{
#define GRAPE_DIM 2
#define GRAPE_DIMWORLD 2
#include "grapehmesh.hh"
}

namespace GrapeInterface_two_three
{
#define GRAPE_DIM 2
#define GRAPE_DIMWORLD 3
#include "grapehmesh.hh"
}

namespace GrapeInterface_three_three
{
#define GRAPE_DIM 3
#define GRAPE_DIMWORLD 3
#include "grapehmesh.hh"
}

namespace Dune
{

  static int __MaxPartition = 1;
  // the interface to dune
  template <int dim, int dimworld>
  struct GrapeInterface
  {
    static int called;
    typedef GrapeInterface_two_two::DUNE_ELEM DUNE_ELEM;
    typedef GrapeInterface_two_two::DUNE_FDATA DUNE_FDATA;
    typedef GrapeInterface_two_two::DUNE_DAT DUNE_DAT;

    inline static void init()
    {
      GrapeInterface_two_two::grape_add_remove_methods();
    }

    inline static void setThread(int t)
    {}

    inline static void handleMesh (void *hmesh)
    {
      GrapeInterface_two_two::handleMesh(hmesh);
    }
    inline static void addDataToHmesh(void  *hmesh, DUNE_FDATA * fe,
                                      void (* const func_real) (DUNE_ELEM *, DUNE_FDATA*, int ind, const
                                                                double *, double *)  )
    {
      GrapeInterface_two_two::addDataToHmesh(hmesh,fe,func_real);
    }

    inline static void *hmesh(
      void (* const func_real) (DUNE_ELEM *, DUNE_FDATA*, int ind, const double *coord,  double *),
      const int noe, const int nov, const int maxlev,
      DUNE_FDATA * fe, DUNE_DAT * dune )
    {
      return GrapeInterface_two_two::hmesh(
               func_real,noe,nov,maxlev,fe,dune);
    }

    inline static void addHmeshToTimeScene(void * timescene, double time, void  *hmesh , int proc)
    {
      GrapeInterface_two_two::addHmeshToTimeScene(timescene,time,hmesh,proc);
    }

    inline static void addHmeshToGlobalTimeScene(double time, void  *hmesh , int proc)
    {
      GrapeInterface_two_two::addHmeshToGlobalTimeScene(time,hmesh,proc);
    }

    inline static void colorBarMinMax(const double min, const double max)
    {
      GrapeInterface_two_two::colorBarMinMax(min,max);
    }
  };

  // not tested yet
  /*
     // the interface to dune
     template <>
     struct GrapeInterface<2,3>
     {
     typedef GrapeInterface_two_three::DUNE_ELEM  DUNE_ELEM;
     typedef GrapeInterface_two_three::DUNE_FDATA DUNE_FDATA;

     static void handleMesh (void *hmesh)
     {
      GrapeInterface_two_three::handleMesh(hmesh);
     }
     static void addDataToHmesh(void  *hmesh, DUNE_FDATA * fe,
           void (* const func_real) (DUNE_ELEM *, DUNE_FDATA*, int ind, const
            double *, double *)  )
     {
      GrapeInterface_two_three::addDataToHmesh(hmesh,fe,func_real);
     }

     static void *hmesh(
            int (* const f_leaf)(DUNE_ELEM *), int (* const n_leaf)(DUNE_ELEM *),
            int (* const f_mac)(DUNE_ELEM *), int (* const n_mac)(DUNE_ELEM *),
            int (* const f_chi)(DUNE_ELEM *), int (* const n_chi)(DUNE_ELEM *),
            void * (* const cp)(const void *),
            int  (* const check_inside)(DUNE_ELEM *, const double *),
            int  (* const wtoc)(DUNE_ELEM *, const double *, double *),
            void (* const ctow)(DUNE_ELEM *, const double *, double *),
            void (* const func_real) (DUNE_ELEM *, DUNE_FDATA*, int ind, const double *coord,  double *),
            const int noe, const int nov, const int maxlev,int partition,
            DUNE_ELEM *he , DUNE_FDATA * fe)
     {
      return  GrapeInterface_two_three::hmesh(f_leaf,n_leaf,f_mac,n_mac,f_chi,n_chi,
            cp,check_inside,wtoc,ctow,func_real,noe,nov,maxlev,partition,he,fe);
     }
     };
   */

  // the interface to dune for dim = dimworld = 3
  template <>
  struct GrapeInterface<3,3>
  {
    typedef GrapeInterface_three_three::DUNE_ELEM DUNE_ELEM;
    typedef GrapeInterface_three_three::DUNE_FDATA DUNE_FDATA;
    typedef GrapeInterface_three_three::DUNE_DAT DUNE_DAT;

    inline static void init()
    {
      GrapeInterface_three_three::initPartitionDisp(__MaxPartition);
      GrapeInterface_three_three::grape_add_remove_methods();
      //if((!leafButton) || (!maxlevelButton))
      //GrapeInterface_two_two::setupLeafButton(mgr,tsc,1);
    }

    inline static void setThread(int t)
    {
      GrapeInterface_three_three::setThread(t);
    }

    inline static void handleMesh (void *hmesh)
    {
      GrapeInterface_three_three::handleMesh(hmesh);
    }
    inline static void addDataToHmesh(void  *hmesh, DUNE_FDATA * fe,
                                      void (* const func_real) (DUNE_ELEM *, DUNE_FDATA*, int ind, const
                                                                double *, double *)  )
    {
      GrapeInterface_three_three::addDataToHmesh(hmesh,fe,func_real);
    }

    inline static void *hmesh(
      void (* const func_real) (DUNE_ELEM *, DUNE_FDATA*, int ind, const double *coord,  double *),
      const int noe, const int nov, const int maxlev,
      DUNE_FDATA * fe, DUNE_DAT * dune )
    {
      return GrapeInterface_three_three::
             hmesh(func_real,noe,nov,maxlev,fe,dune);
    }

    inline static void addHmeshToTimeScene(void * timescene, double time, void  *hmesh , int proc)
    {
      GrapeInterface_three_three::addHmeshToTimeScene(timescene,time,hmesh,proc);
    }
    inline static void addHmeshToGlobalTimeScene(double time, void  *hmesh , int proc)
    {
      GrapeInterface_three_three::addHmeshToGlobalTimeScene(time,hmesh,proc);
    }

    inline static void colorBarMinMax(const double min, const double max)
    {
      GrapeInterface_three_three::colorBarMinMax(min,max);
    }
  };


} // end namespace Dune

#include "grapecommon.cc"

#endif
