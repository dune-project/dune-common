// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <dump.hh>

#include <sstream>
#include <malloc.h>

namespace Dune {

#undef NODUMP
#define NODUMP

#ifdef NODUMP
#define dump2D(a,b,c,d,e) void()
#else
#define dump2D dump3D
#endif

  /**
      one Gauss-Seidel step
   */
  template <class GRID, int SMOOTHER>
  void pmgsolver<GRID,SMOOTHER>::
  smootherGaussSeidel(level l) {
    /* Alle Elemente ohne Rand */
    TIME_SMOOTHER -= MPI_Wtime();
    coefflist cl = mydiscrete.newcoefflist();
    PMGStubs::GaussSeidel<GRID> stub(x,b,g,l,cl,mydiscrete);
    g.loop_owner( l, stub );
    TIME_SMOOTHER += MPI_Wtime();

    exchange(l,x);
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

    array<DIM> add = g.init_add(l);

    // run through lines
    coefflist cl = mydiscrete.newcoefflist();

    for (typename GRID::iterator it=g.begin(l); it != gEnd; ++it) {
      if (! (*it).owner() ) continue;
      if ( mydiscrete.isdirichlet(it) ) continue;

      int i=it.id();

      //    discrete... get the coeffs
      mydiscrete.coeffs(cl, g, l, add, it.coord(), it.id());

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
    };
    exchange(l,x);
    delete[] x_old;

    TIME_SMOOTHER += MPI_Wtime();
  };   /* end jakobi() */

  /**
     Multi-Grid-Cycle
   */
  template <class GRID, int SMOOTHER>
  void pmgsolver<GRID,SMOOTHER>::
  mgc (level l) {
    if (l==0) {    //g.roughest()) {
      double my_d = defect(l);
      double max_d = my_d*reduction;
      // alles auf einen Knoten schieben und loesen :-)
      int c=0;
      while (my_d > max_d) {
        smoother(l);
        my_d = defect(l);
        c++;
        {
          std::stringstream extention;
          extention << "X Iteration " << c;
          dump2D(g,l,x,"smoothest",(char*)extention.str().c_str());
        }
        {
          std::stringstream extention;
          extention << "D Iteration " << c;
          dump2D(g,l,d,"smoothest",(char*)extention.str().c_str());
        }
        if (c > 500) {
          std::cout << "too many iterations on level 0" << std::endl;
          return;
        }
      };
    }
    else {
      // Vorglaetter
      for (int n=0; n<n1; n++) smoother(l);
      // x_{l-1} = 0 b_{l-1}=0
      typename GRID::iterator gEnd=g.end(l-1);
      for (typename GRID::iterator i=g.begin(l-1); i != gEnd; ++i) {
        x[i.id()]=0;
        b[i.id()]=0;
      }
#ifndef NODUMP
      char *dumpfile="dumpfile";
#endif
      defect(l);
      dump2D(g,l,x,dumpfile,"X before restrict");
      dump2D(g,l,d,dumpfile,"D before restrict");
      dump2D(g,l-1,b,dumpfile,"B before restrict");
      // Restriktion d_l -> b_{l-1}
      restrict (l);
      dump2D(g,l-1,b,dumpfile,"B after restrict");
#ifndef NDEBUG
      for (typename GRID::iterator i=g.begin(l-1); i != gEnd; ++i) {
        assert(x[i.id()]==0);
      }
#endif
      // ein level rauf
      mgc(l-1);
      // Prologation X_{l-1} -> X_l
      dump2D(g,l-1,x,dumpfile,"X before prolongate");
      dump2D(g,l,x,dumpfile,"X before prolongate");
      prolongate(l);
      dump2D(g,l,x,dumpfile,"X after prolongate");
      /* Nachglaetter */
      for (int n=0; n<n2; n++) smoother(l);
    }
  };   /* end mgc() */

  template <class GRID, int SMOOTHER>
  double pmgsolver<GRID,SMOOTHER>::
  defect(level l) const {
    TIME_DEFECT -= MPI_Wtime();

    // run through lines
    PMGStubs::Defect<GRID,SMOOTHER> stub(*this, x, b, l);
    g.loop_owner(l,stub);

    assert(finite(stub.defect_array[0]));

    // get max defect of all processes
    double defect_array_recv[2];
    MPI_Allreduce(stub.defect_array, defect_array_recv, 2,
                  MPI_DOUBLE, MPI_SUM, g.comm());
    stub.defect_array[0]=defect_array_recv[0];
    stub.defect_array[1]=defect_array_recv[1];

    TIME_DEFECT += MPI_Wtime();
    return sqrt(stub.defect_array[0]);    // /defect_array[1];
  };

  /**
     Restriktion d_l -> b_{l-1}
   */
  template <class GRID, int SMOOTHER>
  void pmgsolver<GRID,SMOOTHER>::
  restrict (level l) {
    assert(l>0);

    defect(l);
    exchange(l,d);

    TIME_REST -= MPI_Wtime();
    PMGStubs::Restrict<GRID,SMOOTHER> stub(*this,l);
    g.loop_all( l, stub );
    TIME_REST += MPI_Wtime();

    // ABGLEICH vector b Level l-1
    exchange(l-1,b);
  };   /* end restrict() */

  /**
      Prolongation x_{l-1} -> x_l
   */
  template <class GRID, int SMOOTHER>
  void pmgsolver<GRID,SMOOTHER>::
  prolongate(level l) {
    assert(l>0);

    TIME_PROL -= MPI_Wtime();
    PMGStubs::Prolongate<GRID,SMOOTHER> stub(*this,l);
    g.loop_owner( l, stub );
    TIME_PROL += MPI_Wtime();

    // ABGLEICH Level l
    exchange(l,x);
  };

  /**
      Datenabgleich auf Level l vorbereiten
   */
  template <class GRID, int SMOOTHER>
  void pmgsolver<GRID,SMOOTHER>::
  initExchange() {
    int P;
    /* Size of Communicator */
    MPI_Comm_size(g.comm(), &P);
    exchange_data_from = new exchange_data*[g.smoothest()+1];
    exchange_data_to = new exchange_data*[g.smoothest()+1];
    for (level l=g.roughest(); l<=g.smoothest(); l++) {
      exchange_data_from[l] = new exchange_data[P];
      exchange_data_to[l] = new exchange_data[P];
      for (int p=0; p<P; p++) {
        exchange_data_from[l][p].size = 0;
        exchange_data_from[l][p].id = malloc<int>(1);
        exchange_data_to[l][p].size = 0;
        exchange_data_to[l][p].id = malloc<int>(1);
      }
      PMGStubs::InitExchange<GRID,SMOOTHER> stub(*this);
      g.loop_overlap(l, stub);
    }
  };

  /**
      Datenabgleich auf Level l
   */
  template <class GRID, int SMOOTHER>
  void pmgsolver<GRID,SMOOTHER>::
  exchange(level l, Vector<GRID> & ex) {
    TIME_EX -= MPI_Wtime();

    //#define TALKALOT
#ifdef TALKALOT
    int P;
    MPI_Comm_size(g.comm(), &P);     // Number of Processors
    for (int p=0; p<P; p++) {
      if (rank==p) {
        cout << "Rank " << rank << " " << g.process_
             << " ProcessorArrangement: " << g.dim_ << std::endl;
        for (int d=0; d<DIM; d++) {
          cout << d << " do_front_share=" << g.do_front_share(d)
               << " do_end_share=" << g.do_end_share(d) << std::endl;
        }
        std::cout << std::flush;
      }
      MPI_Barrier(g.comm());
    }
#endif

    for (int d=0; d<DIM; d++) {
      for (int s=-1; s<=2; s+=2) {
        /* remote rank */
        array<DIM> remote_process = g.process();
        int shift;
        if ( g.process()[d] % 2 == 0 ) {
          shift = s;
        }
        else {
          shift = -s;
        }
        // calc neighbour coord
        remote_process[d] += shift;

        // check cart boundries
#ifdef TALKALOT
        std::cout << rank << g.process_ << " checking "
                  << remote_process << "(d=" << d
                  << ",shift=" << shift << ")" << std::endl << std::flush;
#endif
        if (shift==-1) {
          if (! g.do_front_share(d) ) continue;
        }
        else {
          if (! g.do_end_share(d) ) continue;
        }

        int remote_rank;
#ifdef TALKALOT
        std::cout << rank << g.process_ << "exchange with coord "
                  << remote_process << std::endl << std::flush;
#endif
        MPI_Cart_rank(g.comm(), remote_process, &remote_rank);
        if (remote_rank < 0 )
          continue;

        /* data buffers and ids */
        int* & id_from =
          exchange_data_from[l][remote_rank].id;
        int* & id_to =
          exchange_data_to[l][remote_rank].id;
        int size_from = exchange_data_from[l][remote_rank].size;
        int size_to = exchange_data_to[l][remote_rank].size;
        double* data_from = new double[size_from];
        double* data_to = new double[size_to];

        /* collect data */
        for (int i=0; i<size_to; i++)
          data_to[i] = ex[id_to[i]];

        /* the real exchange */
        if ( g.process()[d] % 2 == 0 ) {
#ifndef NDEBUG
          /*
             int debug_size = size_to;
             MPI_Send( &debug_size, 1, MPI_INT, remoteprocess,
             exchange_tag, g.comm());
           */
#endif
#ifdef VERBOSE_EXCHANGE
          std::cout << "Process " << rank << " sending " << size_to
                    << " to " << remote_rank
                    << std::endl << std::flush;
#endif
          MPI_Send( data_to, size_to, MPI_DOUBLE, remote_rank,
                    exchange_tag, g.comm());
#ifdef VERBOSE_EXCHANGE
          std::cout << "Process " << rank << " receiving " << size_from
                    << " from " << remote_rank
                    << std::endl << std::flush;
#endif
          MPI_Recv( data_from, size_from, MPI_DOUBLE, remote_rank,
                    exchange_tag, g.comm(), &mpi_status);
        }
        else {
#ifdef VERBOSE_EXCHANGE
          std::cout << "Process " << rank << " receiving " << size_from
                    << " from " << remote_rank
                    << std::endl << std::flush;
#endif
          MPI_Recv( data_from, size_from, MPI_DOUBLE, remote_rank,
                    exchange_tag, g.comm(), &mpi_status);
#ifdef VERBOSE_EXCHANGE
          std::cout << "Process " << rank << " sending " << size_to
                    << " to " << remote_rank
                    << std::endl << std::flush;
#endif
          MPI_Send( data_to, size_to, MPI_DOUBLE, remote_rank,
                    exchange_tag, g.comm());
        }

        /* store data */
        for (int i=0; i<size_from; i++)
          ex[id_from[i]] = data_from[i];

        /* clean up */
        delete[] data_from;
        delete[] data_to;
      }
    }
    TIME_EX += MPI_Wtime();
  };   /* end exchange() */

  template <class GRID>
  class loopstubInitIterator {
    enum { DIM = GRID::griddim };
    Dune::Vector<GRID> & b;
    Dune::Vector<GRID> & x;
    discrete<GRID> & mydiscrete;
    GRID & g;
  public:
    loopstubInitIterator(Dune::Vector<GRID> & B, Dune::Vector<GRID> & X,
                         discrete<GRID> & D, GRID & G) :
      b(B), x(X), mydiscrete(D), g(G) {};
    void evaluate(int l, const array<DIM> & coord, int i) {
      typename GRID::iterator it(i,g);
      b[i] = mydiscrete.rhs(it);
      x[i] = 0;
      if (mydiscrete.isdirichlet(it))
        for (int d=0; d<DIM; d++) {
          for (int s = Dune::left; s != Dune::end; s++) {
            Boundry bd = mydiscrete.boundry(it,d,(Dune::side)s);
            if (bd.typ == dirichlet) x[i]=bd.value;
          }
        }
    }
  };

  /** solve the problem for a certain perm an store the result in x */
  template <class GRID, int SMOOTHER>
  void pmgsolver<GRID,SMOOTHER>::
  init() {

    std::cout << rank << " initalizing exchange data structures..."
              << std::flush;

    /* Initialize the exchange data structures */
    initExchange();
    std::cout << " done\n" << std::flush;

    /* build rhs and x-values at the border on smoothest level */
    typename GRID::level lvl = g.smoothest();
    b = 0;
    d = 0;

    loopstubInitIterator<GRID> stub(b,x,mydiscrete,g);
    g.loop_border(lvl, stub);

    std::cout << rank << " Exchanging x and b\n" << std::flush;

    exchange(g.smoothest(),x);
    exchange(g.smoothest(),b);

    MPI_Barrier(g.comm());
  }

  template <class GRID, int SMOOTHER>
  void pmgsolver<GRID,SMOOTHER>::
  solve() {
    typename GRID::level lvl = g.smoothest();

    init();

    // multigrid cycle
    double mydefect=defect(lvl);
    double maxdefect=mydefect*reduction;
    int cycle=0;
    if (rank==0) std::cout << "MGC-Cycle " << cycle
                           << " " << mydefect << " " << 0 << std::endl;
    cycle ++;

    double lastdefect=mydefect;
    array<2> coord;
    coord[0]=7;
    coord[1]=0;
    TIME_SMOOTHER = 0;
    TIME_PROL = 0;
    TIME_REST = 0;
    TIME_DEFECT = 0;
    while (mydefect > maxdefect)
    {
#ifndef NODUMP
      dump2D(g,lvl,x,"jakobi","X");
      //    dump2D(g,lvl,b,"jakobi","B");
      dump2D(g,lvl,d,"jakobi","D");
#endif
      mgc(lvl);
      //smoother(lvl);
      //smootherJacobi(lvl);
      mydefect=defect(lvl);
      if (rank==0) std::cout << "MGC-Cycle " << cycle << " " << mydefect
                             << " " << mydefect/lastdefect
                             << std::endl;
      lastdefect = mydefect;
      //if (cycle==10) return;
      cycle ++;
    };
    if (rank==0)
      std::cout << "Time in smoother:" << TIME_SMOOTHER << std::endl
                << "Time in prolongate:" << TIME_PROL << std::endl
                << "Time in restrict:" << TIME_REST << std::endl
                << "Time in exchange:" << TIME_EX << std::endl
                << "Time in defect:" << TIME_DEFECT << std::endl;
  };

} // namespace Dune
