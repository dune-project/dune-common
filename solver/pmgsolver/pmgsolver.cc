// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <sstream>
#include <malloc.h>

#ifndef FORCE_DUMP
#define NODUMP
#endif

namespace Dune {

  /**
      one Gauss-Seidel step
   */
  template <class GRID, int SMOOTHER>
  void pmgsolver<GRID,SMOOTHER>::
  smootherGaussSeidel(level l) {
    TIME_SMOOTHER -= MPI_Wtime();

    coefflist cl = discrete.newcoefflist();

    // loop inner owner / border
    array<DIM> beginI;
    array<DIM> beginB;
    array<DIM> endI;
    array<DIM> endB;
    for (int d=0; d<DIM; d++) {
      beginB[d] = g.front_overlap(l,d);
      beginI[d] = beginB[d]+1;
      endB[d] = g.size(l,d) + g.front_overlap(l,d);
      endI[d] = endB[d] - 1;
      if (beginI[d] > endB[d])
        beginI[d] = endB[d];
      if (beginI[d] > endI[d])
        endI[d] = beginI[d];
    }

    PMGStubs::GaussSeidel<GRID,SMOOTHER,PMGStubs::Inner>
    stub_inner(*this, l);
    PMGStubs::GaussSeidel<GRID,SMOOTHER,PMGStubs::Border>
    stub_border(*this, l);
    g.loop3D(l,beginI,endI,endI,endI,stub_inner); // inner
    g.loop3D(l,beginB,beginI,endI,endB,stub_border); // border
    relocate_array[0] = stub_inner.Sum() + stub_border.Sum();
    relocate_array[1] = stub_inner.Nr() + stub_border.Nr();

    TIME_SMOOTHER += MPI_Wtime();
    g.exchange(l,x);
  }

  /**
      One Jakobi step
   */
  template <class GRID, int SMOOTHER>
  void pmgsolver<GRID,SMOOTHER>::
  smootherJacobi(level l) {
    TIME_SMOOTHER -= MPI_Wtime();

    double *x_old = new double[g.end(l).id()-g.begin(l).id()];
    int xoff=g.begin(l).id();

    typename GRID::iterator gEnd=g.end(l);

    // copy the old solution
    for (typename GRID::iterator it=g.begin(l); it != gEnd; ++it) {
      int i=it.id();
      x_old[i-xoff]=x[i];
    }

    // run through lines
    coefflist cl = discrete.newcoefflist();

    for (typename GRID::iterator it=g.begin(l); it != gEnd; ++it) {
      if (! (*it).owner() ) continue;
      if ( discrete.bc.isdirichlet(it) ) continue;

      int i=it.id();

      //    discrete... get the coeffs
      discrete.coeffs(cl, it);

      // recalc x[i]
      // x[i] = 1/aii { b[i] - sum(j!=i){ aij*x[j] } }
      x[i]=b[i];
      for (int n=0; n<cl.size; n++) {
        int j = cl.j[n];
#ifndef NDEBUG
        if (!finite(x_old[j-xoff])) {
          typename GRID::iterator errit(j,g);
          std::cerr << "FINITE ERROR Element "
                    << j << errit.coord() << std::endl;
        }
        assert(finite(x_old[j-xoff]));
#endif
        x[i] -= cl.aij[n] * x_old[j-xoff];
      }
      x[i] /= cl.aii;
      double omega=0.5;
      x[i] = (1-omega) * x[i] + omega * x_old[i-xoff];
      relocate_array[0] += x[i];
      relocate_array[1] ++;
    };
    g.exchange(l,x);
    delete[] x_old;

    TIME_SMOOTHER += MPI_Wtime();
  }; /* end jakobi() */

  /**
     Multi-Grid-Cycle
   */
  template <class GRID, int SMOOTHER>
  void pmgsolver<GRID,SMOOTHER>::
  mgc (level l) {
    if (l==0) { //g.roughest()) {
      double my_d = defect(l);
      double max_d = my_d*1e-3;
      // alles auf einen Knoten schieben und loesen :-)
      int c=0;
#ifndef NODUMP
      {
        std::stringstream extention;
        extention << "B Iteration " << c;
        dump(g,l,b,"smoothest",(char*)extention.str().c_str());
      }
      {
        std::stringstream extention;
        extention << "X Iteration " << c;
        dump(g,l,x,"smoothest",(char*)extention.str().c_str());
      }
      {
        std::stringstream extention;
        extention << "D Iteration " << c;
        dump(g,l,d,"smoothest",(char*)extention.str().c_str());
      }
#endif
      while (my_d > max_d) {
        smoother(l);
        double last = my_d;
        my_d = defect(l);
        c++;
#ifndef NODUMP
        {
          std::stringstream extention;
          extention << "X - Iteration " << c;
          dump(g,l,x,"smoothest",(char*)extention.str().c_str());
        }
        {
          std::stringstream extention;
          extention << "D Iteration " << c;
          dump(g,l,d,"smoothest",(char*)extention.str().c_str());
        }
#endif
        /*
                std::cout << "level 0 defect = " << my_d
                          << " reduction = " << my_d/last << std::endl;
         */
        if (my_d < 1e-14)
          return;
        if (c > 500) {
          std::cout << "too many iterations on level 0" << std::endl;
          return;
        }
      }
    }
    else {
      // Vorglaetter
      //      double last = defect(l);
      for (int n=0; n<n1; n++) {
        smoother(l);
        /*
                double my_d = defect(l);
                std::cout << "Vorglätter level " << l
                          << " reduction = " << my_d / last << std::endl;
                last = my_d;
         */
      }
      // x_{l-1} = 0 b_{l-1}=0
      typename GRID::iterator gEnd=g.end(l-1);
      for (typename GRID::iterator i=g.begin(l-1); i != gEnd; ++i) {
        x[i.id()]=0;
        b[i.id()]=0;
      }
      defect(l);
#ifndef NODUMP
      char *dumpfile="dumpfile";
      dump(g,l,x,dumpfile,"X before restrict");
      dump(g,l,d,dumpfile,"D before restrict");
      dump(g,l-1,b,dumpfile,"B before restrict");
#endif
      // Restriktion d_l -> b_{l-1}
      restrict (l);
#ifndef NODUMP
      dump(g,l-1,b,dumpfile,"B after restrict");
#endif
#ifndef NDEBUG
      for (typename GRID::iterator i=g.begin(l-1); i != gEnd; ++i) {
        assert(x[i.id()]==0);
      }
#endif
      // ein level rauf
      mgc(l-1);
      // Prologation X_{l-1} -> X_l
#ifndef NODUMP
      dump(g,l-1,x,dumpfile,"X before prolongate");
      dump(g,l,x,dumpfile,"X before prolongate");
#endif
      prolongate(l);
#ifndef NODUMP
      dump(g,l,x,dumpfile,"X after prolongate");
#endif
      /* Nachglaetter */
      for (int n=0; n<n2; n++) smoother(l);
    }
  }; /* end mgc() */

  template <class GRID, int SMOOTHER>
  double pmgsolver<GRID,SMOOTHER>::
  defect(level l) const {
    TIME_DEFECT -= MPI_Wtime();

    // run through lines
    PMGStubs::Defect<GRID,SMOOTHER,PMGStubs::Inner> stub_inner(*this, l);
    PMGStubs::Defect<GRID,SMOOTHER,PMGStubs::Border> stub_border(*this, l);

    // loop inner owner / border
    array<DIM> beginI;
    array<DIM> beginB;
    array<DIM> endI;
    array<DIM> endB;
    {
      for (int d=0; d<DIM; d++) {
        beginB[d] = g.front_overlap(l,d);
        beginI[d] = beginB[d]+1;
        endB[d] = g.size(l,d) + g.front_overlap(l,d);
        endI[d] = endB[d] - 1;
        if (beginI[d] > endB[d])
          beginI[d] = endB[d];
        if (beginI[d] > endI[d])
          endI[d] = beginI[d];
      }
    }

    g.loop3D(l,beginI,endI,endI,endI,stub_inner); // inner
    g.loop3D(l,beginB,beginI,endI,endB,stub_border); // border

    stub_inner.defect_array[0] += stub_border.defect_array[0];
    stub_inner.defect_array[1] += stub_border.defect_array[1];

    //    assert(finite(stub_inner.defect_array[0]));

    // get max defect of all processes
    double defect_array_recv[2];
    MPI_Allreduce(stub_inner.defect_array, defect_array_recv, 2,
                  MPI_DOUBLE, MPI_SUM, g.comm());
    stub_inner.defect_array[0]=defect_array_recv[0];
    stub_inner.defect_array[1]=defect_array_recv[1];

    TIME_DEFECT += MPI_Wtime();

    return sqrt(stub_inner.defect_array[0]); // /defect_array[1];
  };

  /**
     Restriktion d_l -> b_{l-1}
   */
  template <class GRID, int SMOOTHER>
  void pmgsolver<GRID,SMOOTHER>::
  restrict (level l) {
    assert(l>0);

    // defect(l);
    // We also need the defect of out neighbours
    g.exchange(l,d);

    TIME_REST -= MPI_Wtime();

    // loop inner owner / border
    array<DIM> beginI;
    array<DIM> beginB;
    array<DIM> endI;
    array<DIM> endB;
    for (int d=0; d<DIM; d++) {
      beginB[d] = g.front_overlap(l,d);
      beginI[d] = beginB[d]+1;
      endB[d] = g.size(l,d) + g.front_overlap(l,d);
      endI[d] = endB[d] - 1;
      beginB[d] = 0;
      beginI[d] = beginB[d]+1;
      endB[d] = g.size(l,d) + g.front_overlap(l,d) + g.end_overlap(l,d);
      endI[d] = endB[d] - 1;
      if (beginI[d] > endB[d])
        beginI[d] = endB[d];
      if (beginI[d] > endI[d])
        endI[d] = beginI[d];
    }
    PMGStubs::Restrict<GRID,SMOOTHER,PMGStubs::Inner> stub_inner(*this,l);
    PMGStubs::Restrict<GRID,SMOOTHER,PMGStubs::Border> stub_border(*this,l);

    g.loop3D(l,beginI,endI,endI,endI,stub_inner); // inner
    g.loop3D(l,beginB,beginI,endI,endB,stub_border); // border

    TIME_REST += MPI_Wtime();

    // ABGLEICH vector b Level l-1
    g.exchange(l-1,b);
  }; /* end restrict() */

  /**
      Prolongation x_{l-1} -> x_l
   */
  template <class GRID, int SMOOTHER>
  void pmgsolver<GRID,SMOOTHER>::
  prolongate(level l) {
    assert(l>0);

    TIME_PROL -= MPI_Wtime();

    // loop inner owner / border
    array<DIM> beginI;
    array<DIM> beginB;
    array<DIM> endI;
    array<DIM> endB;
    for (int d=0; d<DIM; d++) {
      beginB[d] = 0;
      beginI[d] = beginB[d]+1;
      endB[d] = g.size(l,d) + g.front_overlap(l,d) + g.end_overlap(l,d);
      endI[d] = endB[d] - 1;
      if (beginI[d] > endB[d])
        beginI[d] = endB[d];
      if (beginI[d] > endI[d])
        endI[d] = beginI[d];
    }

    PMGStubs::Prolongate<GRID,SMOOTHER,PMGStubs::Inner> stub_inner(*this,l);
    PMGStubs::Prolongate<GRID,SMOOTHER,PMGStubs::Border> stub_border(*this,l);

    g.loop3D(l,beginI,endI,endI,endI,stub_inner); // inner
    g.loop3D(l,beginB,beginI,endI,endB,stub_border); // border

    TIME_PROL += MPI_Wtime();

    // ABGLEICH Level l
    g.exchange(l,x);
  };

  /** relocate the problem around 0 (only if we haven't any fixed point) */
  template <class GRID, int SMOOTHER>
  void pmgsolver<GRID,SMOOTHER>::
  relocate(level lvl) {
    if (! need_relocate) return;

    // get max defect of all processes
    double relocate_array_recv[2];
    MPI_Allreduce(relocate_array, relocate_array_recv, 2,
                  MPI_DOUBLE, MPI_SUM, g.comm());
    relocate_array[0] = relocate_array_recv[0] / relocate_array_recv[1];

    PMGStubs::RelocateIterator<GRID> stub(x,relocate_array[0]);
    g.loop_all(lvl, stub);
  }

  /** solve the problem for a certain perm an store the result in x */
  template <class GRID, int SMOOTHER>
  void pmgsolver<GRID,SMOOTHER>::
  init(level lvl) {

    need_relocate = true;
    {
      for (int d=0; d<DIM; d++) {
        if (discrete.bc.bd(d,Dune::left) == dirichlet)
          need_relocate = false;
        if (discrete.bc.bd(d,Dune::right) == dirichlet)
          need_relocate = false;
      }
    }

    /* build rhs and x-values at the border */
    b = 0;
    d = 0;

    PMGStubs::InitIterator<GRID> stub(b,x,discrete,g);
    g.loop_border(lvl, stub);

    g.exchange(lvl,x);
    g.exchange(lvl,b);

    MPI_Barrier(g.comm());
  }

  template <class GRID, int SMOOTHER>
  void pmgsolver<GRID,SMOOTHER>::
  solve(int maxCycles, level lvl=-9999) {
    if (lvl == -9999) lvl = g.smoothest();
    init(lvl);

    // multigrid cycle
    double mydefect=defect(lvl);
    double maxdefect=mydefect*reduction;
    int cycle=0;
    if (rank==0) std::cout << "MGC-Cycle " << cycle
                           << " " << mydefect << " " << 0 << std::endl;
    double lastdefect=mydefect;

    TIME_SMOOTHER = 0;
    TIME_PROL = 0;
    TIME_REST = 0;
    TIME_DEFECT = 0;

#ifdef SOLVER_DUMPDX
    char dname[156];
    char cname[256];
    sprintf(dname,"defect-t%04i",cycle);
    sprintf(cname,"calpres-t%04i",cycle);
    dumpdx(g,g.smoothest(),d,dname,"pressure in the end");
    dumpdx(g,g.smoothest(),x,cname,"pressure in the end");
#endif

    cycle ++;

    while (mydefect > maxdefect)
    {
      if (cycle > maxCycles) break;
#ifndef NODUMP
      dump(g,lvl,x,"jakobi","X");
      dump(g,lvl,b,"jakobi","B");
      dump(g,lvl,d,"jakobi","D");

      int comm_size;
      MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
      char fname[256];
      sprintf(fname,"jakobi-p%d",comm_size);
      std::ofstream xfile;

      xfile.open(fname, std::ios_base::out |
                 std::ios_base::app | std::ios_base::ate);
      if (rank == 0)
        xfile << "----------------------"
              << "Iteration " << cycle << " "
              << "----------------------\n";
      xfile.close();
#endif
      mgc(lvl);
      //smoother(lvl);
      //smootherJacobi(lvl);
      mydefect=defect(lvl);
      if (rank==0) std::cout << "MGC-Cycle " << cycle << " " << mydefect
                             << " " << mydefect/lastdefect
                             << std::endl;
      lastdefect = mydefect;

#ifdef SOLVER_DUMPDX
      char dname[156];
      char cname[256];
      sprintf(dname,"defect-t%04i",cycle);
      sprintf(cname,"calpres-t%04i",cycle);
      dumpdx(g,g.smoothest(),d,dname,"pressure in the end");
      dumpdx(g,g.smoothest(),x,cname,"pressure in the end");
#endif

      cycle ++;

    };
    if (rank==0)
      std::cout << "Time in smoother:" << TIME_SMOOTHER << std::endl
                << "Time in prolongate:" << TIME_PROL << std::endl
                << "Time in restrict:" << TIME_REST << std::endl
                << "Time in exchange:" << TIME_EX << std::endl
                << "Time in defect:" << TIME_DEFECT << std::endl;
  };

  template <class GRID, int SMOOTHER>
  void pmgsolver<GRID,SMOOTHER>::
  solveNested() {
    level lvl=g.roughest();
    init(lvl);

    TIME_SMOOTHER = 0;
    TIME_PROL = 0;
    TIME_REST = 0;
    TIME_EX = 0;
    TIME_DEFECT = 0;

    // solve roughest level excactly
    double mydefect=defect(lvl);
    double maxdefect=1e-15; //mydefect*reduction;
    int cycle=0;

#ifdef SOLVER_DUMPDX
    char defecttxt[256];
    char calprestxt[256];
    int run=0;
    sprintf(defecttxt,"defect-run%d",run);
    sprintf(calprestxt,"calpres-run%d",run);
    dumpdx(g,g.roughest(),d,defecttxt,"pressure in the end");
    dumpdx(g,g.roughest(),x,calprestxt,"pressure in the end");
    run++;
#endif

    while (mydefect > maxdefect) {
      smoother(lvl);
      mydefect=defect(lvl);
      cycle++;
      if (mydefect < 1e-16)
        break;
      if (cycle > 5000) {
        if (rank==0)
          std::cout << "too many iterations on level 0" << std::endl;
        break;
      }
    }
    if (rank==0)
      std::cout << "excact solution on level 0 took " << cycle
                << " iterations -> defect=" << mydefect << std::endl;

#ifdef SOLVER_DUMPDX
    sprintf(defecttxt,"defect-run%d",run);
    sprintf(calprestxt,"calpres-run%d",run);
    dumpdx(g,g.roughest(),d,defecttxt,"pressure in the end");
    dumpdx(g,g.roughest(),x,calprestxt,"pressure in the end");
    run++;
#endif
    // prolongate the solution on the next level an run mgc
    // until we have a reduction of 0.1
    for (lvl++; lvl<=g.smoothest(); lvl++) {
      init(lvl);
      // Prologation X_{l-1} -> X_l
      typename GRID::iterator gEnd=g.end(lvl);
      for (typename GRID::iterator i=g.begin(lvl); i != gEnd; ++i) {
        x[i.id()]=0;
      }
      prolongate(lvl);
      if (rank==0) std::cout << "Iteration on level " << lvl
                             << std::endl;
      // multigrid cycle
      mydefect=defect(lvl);
      maxdefect=mydefect*reduction;
      cycle=0;
      if (rank==0) std::cout << "\tMGC-Cycle " << cycle
                             << " " << mydefect << " " << 0 << std::endl;
      cycle ++;
      double lastdefect=mydefect;

#ifdef SOLVER_DUMPDX
      sprintf(defecttxt,"defect-run%d",run);
      sprintf(calprestxt,"calpres-run%d",run);
      dumpdx(g,lvl,d,defecttxt,"pressure in the end");
      dumpdx(g,lvl,x,calprestxt,"pressure in the end");
      run++;
#endif

      while (mydefect > maxdefect)
      {
        if (cycle > 50) break;
        mgc(lvl);
#ifdef SOLVER_DUMPDX
        sprintf(defecttxt,"defect-run%d",run);
        sprintf(calprestxt,"calpres-run%d",run);
        dumpdx(g,lvl,d,defecttxt,"pressure in the end");
        dumpdx(g,lvl,x,calprestxt,"pressure in the end");
        run++;
#endif
        mydefect=defect(lvl);
        if (rank==0) std::cout << "\tMGC-Cycle " << cycle << " " << mydefect
                               << " " << mydefect/lastdefect
                               << std::endl;
        lastdefect = mydefect;
        cycle ++;
        if (mydefect < 1e-16)
          break;
      }
    }
#ifdef SOLVER_DUMPDX
    sprintf(defecttxt,"defect-run%d",run);
    sprintf(calprestxt,"calpres-run%d",run);
    dumpdx(g,g.smoothest(),d,defecttxt,"pressure in the end");
    dumpdx(g,g.smoothest(),x,calprestxt,"pressure in the end");
    run++;
#endif
    if (rank==0)
      std::cout << "Time in smoother:" << TIME_SMOOTHER << std::endl
                << "Time in prolongate:" << TIME_PROL << std::endl
                << "Time in restrict:" << TIME_REST << std::endl
                << "Time in exchange:" << TIME_EX << std::endl
                << "Time in defect:" << TIME_DEFECT << std::endl;
  }

} // namespace Dune
