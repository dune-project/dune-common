// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <mpi.h>

#include <math.h>
#include <map>
#include <sstream>

#include "../pmgsolver.hh"

#include <dump.hh>

using namespace Dune;

template <class T>
T* realloc(T* & pointer, int size) {
  if (size==0) {
    std::cerr << "Warning request for realloc with size=0\n";
    size = 1;
  }
  pointer =
    static_cast<T*>(realloc(static_cast<void*>(pointer), size*sizeof(T)));
  if (pointer == 0)
    throw std::string("Bad realloc");
  return pointer;
}

template <class T>
T* malloc(int size) {
  return static_cast<T*>(malloc(size*sizeof(T)));
}

#undef NODUMP
#define NODUMP

#ifdef NODUMP
#define dump2D(a,b,c,d,e) void()
#else
#define dump2D dump3D
#endif

/**
    Ein Gauss-Seidel-Schritt als Glaettungsschritt
 */
template <class GRID>
class loopstubGaussSeidel {
  enum { DIM = GRID::griddim };
  double * x;
  double * b;
  GRID & g;
  coefflist & cl;
  discrete<GRID> &mydiscrete;
  array<DIM> add;
public:
  loopstubGaussSeidel(double * _x, double * _b, GRID & _g,
                      int l, coefflist & _cl, discrete<GRID> &d) :
    x(_x), b(_b), g(_g), cl(_cl), mydiscrete(d), add(0) {
    add = g.init_add(l);
  }
  void evaluate(int l, const array<DIM> & coord, int i) {
    /* Coeffs */
    /*
       coefflist & coeffs(coefflist &cl,
       mygrid &g, typename mygrid::level l,
       const array<DIM> &coord, int id_i) const
     */
    mydiscrete.coeffs(cl, g, l, add, coord, i);

    x[i]=b[i];
    for (int n=0; n<cl.size; n++) {
      int j = cl.j[n];
      x[i] -= cl.aij[n] * x[j];
    }
    x[i] /= cl.aii;
  }
};

template <class GRID>
class loopstubGaussSeidelBorder {
  enum { DIM = GRID::griddim };
  double * x;
  double * b;
  GRID & g;
  coefflist & cl;
  discrete<GRID> &mydiscrete;
  array<DIM> add;
public:
  loopstubGaussSeidelBorder(double * _x, double * _b, GRID & _g,
                            int l, coefflist & _cl, discrete<GRID> &d) :
    x(_x), b(_b), g(_g), cl(_cl), mydiscrete(d), add(0) {
    add = g.init_add(l);
  }
  void evaluate(int l, const array<DIM> & coord, int i) {
    /* Coeffs */
    /*
       coefflist & coeffs(coefflist &cl,
       mygrid &g, typename mygrid::level l,
       const array<DIM> &coord, int id_i) const
     */
    mydiscrete.coeffs(cl, g, l, add, coord, i);

    x[i]=b[i];
    for (int n=0; n<cl.size; n++) {
      int j = cl.j[n];
      x[i] -= cl.aij[n] * x[j];
    }
    x[i] /= cl.aii;
  }
};

template <class GRID, int SMOOTHER>
void pmgsolver<GRID,SMOOTHER>::
smootherGaussSeidel(level l, vec x, vec b) {
  /* Alle Elemente ohne Rand */
  TIME_SMOOTHER -= MPI_Wtime();
  coefflist cl = mydiscrete.newcoefflist();
  loopstubGaussSeidel<GRID> stub(x,b,g,l,cl,mydiscrete);
  g.loop_owner( l, stub );
  TIME_SMOOTHER += MPI_Wtime();

  exchange(l,x);
}

/**
    Ein Jakobi-Schritt als Glaettungsschritt
 */
template <class GRID, int SMOOTHER>
void pmgsolver<GRID,SMOOTHER>::
smootherJacobi(level l, vec x, vec b) {
  TIME_SMOOTHER -= MPI_Wtime();

  double *x_old = new double[(*(--g.end(l))).id()+1-(*(g.begin(l))).id()];
  int xoff=(*(g.begin(l))).id();

  typename GRID::iterator gEnd=g.end(l);

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
    //    mydiscrete.coeffs(cl, it);

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
}; /* end jakobi() */

/**
   Multi-Grid-Cycle
 */
template <class GRID, int SMOOTHER>
void pmgsolver<GRID,SMOOTHER>::
mgc (level l, vec x, vec b) {
  if (l==0) { //g.roughest()) {
    double my_d = defect(l,x,b);
    double max_d = d*1e-4;
    // alles auf einen Knoten schieben und loesen :-)
    int c=0;
    while (my_d > max_d) {
      smoother(l,x,b);
      my_d = defect(l,x,b);
      c++;
      {
        std::stringstream extention;
        extention << "X Iteration " << c;
        dump2D(g,l,x,"smoothest",(char*)extention.str().c_str());
      }
      {
        std::stringstream extention;
        extention << "D Iteration " << c;
        dump2D(g,l,defect_v,"smoothest",(char*)extention.str().c_str());
      }
      if (c > 500) {
        std::cout << "too many iterations on level 0" << std::endl;
        return;
      }
    };
    std::cout << "roughest: " << c << std::endl;
  }
  else {
    // Vorglaetter
    for (int n=0; n<n1; n++) smoother(l,x,b);
    // x_{l-1} = 0 b_{l-1}=0
    typename GRID::iterator gEnd=g.end(l-1);
    for (typename GRID::iterator i=g.begin(l-1); i != gEnd; ++i) {
      x[i.id()]=0;
      b[i.id()]=0;
    }
#ifndef NODUMP
    char *dumpfile="dumpfile";
#endif
    defect(l,x,b);
    dump2D(g,l,x,dumpfile,"X before restrict");
    dump2D(g,l,defect_v,dumpfile,"D before restrict");
    dump2D(g,l-1,b,dumpfile,"B before restrict");
    // Restriktion d_l -> b_{l-1}
    restrict (l,x,b);
    dump2D(g,l-1,b,dumpfile,"B after restrict");
#ifndef NDEBUG
    for (typename GRID::iterator i=g.begin(l-1); i != gEnd; ++i) {
      assert(x[i.id()]==0);
    }
#endif
    // ein level rauf
    mgc(l-1,x,b);
    // Prologation X_{l-1} -> X_l
    dump2D(g,l-1,x,dumpfile,"X before prolongate");
    dump2D(g,l,x,dumpfile,"X before prolongate");
    prolongate(l,x);
    dump2D(g,l,x,dumpfile,"X after prolongate");
    /* Nachglaetter */
    for (int n=0; n<n2; n++) smoother(l,x,b);
  }
}; /* end mgc() */

/** local defect */
template <class GRID, int SMOOTHER>
double pmgsolver<GRID,SMOOTHER>::
localdefect(level l, const array<DIM> & add, const array<DIM> & coord,
            int i, vec x, vec b, vec d) const {
  double defect=b[i];

  //    discrete... get the coeffs
  static coefflist cl = mydiscrete.newcoefflist();
  mydiscrete.coeffs(cl, g, l, add, coord, i);

  defect -= cl.aii * x[i];
  /*
     std::cout << coord << " -> " << cl.aii << " * x[" << i << "]\n";
     for (int n=0; n<cl.size; n++) {
     std::cout << cl.aij[n] << " * " << "x[" << cl.j[n] << "]\n";
     }
     exit(0);
   */
  for (int n=0; n<cl.size; n++) {
    int j = cl.j[n];
    defect -= cl.aij[n] * x[j];
  }
  if (!finite(defect))
    std::cerr << "DEFECT ERROR Element " << i << coord << std::endl;
  assert(finite(defect));

  if (d!=0)
    d[i]=defect;
  defect_v[i]=defect;

  return defect;
}

template <class GRID, int SMOOTHER>
class loopstubDefect {
  typedef int level;
  enum { DIM = GRID::griddim };
  const pmgsolver<GRID,SMOOTHER> & solver;
  double *x;
  double *b;
  array<DIM> add;
public:
  array<2,double> defect_array;
  loopstubDefect(const pmgsolver<GRID,SMOOTHER> & solver_,
                 double *X, double *B, level l) :
    solver(solver_), x(X), b(B),
    add(solver.g.init_add(l)), defect_array(0)
  {}
  void evaluate(level l, const array<DIM> & coord, int i) {
    /* do something */
    double defect = solver.localdefect(l, add, coord, i, x, b);
    // update process defect
    defect_array[0] += defect*defect;
    defect_array[1] ++;
  }
};

template <class GRID, int SMOOTHER>
double pmgsolver<GRID,SMOOTHER>::
defect(level l, vec x, vec b, vec d) const {
  TIME_DEFECT -= MPI_Wtime();

  // run through lines
  loopstubDefect<GRID,SMOOTHER> stub(*this, x, b, l);
  g.loop_owner(l,stub);

  assert(finite(stub.defect_array[0]));

  // get max defect of all processes
  double defect_array_recv[2];
  MPI_Allreduce(stub.defect_array, defect_array_recv, 2,
                MPI_DOUBLE, MPI_SUM, g.comm());
  stub.defect_array[0]=defect_array_recv[0];
  stub.defect_array[1]=defect_array_recv[1];

  TIME_DEFECT += MPI_Wtime();
  return sqrt(stub.defect_array[0]); // /defect_array[1];
};

/**
   Restriktion d_l -> b_{l-1}
 */
template <class GRID, int SMOOTHER>
inline void
pmgsolver<GRID,SMOOTHER>::
adddefect(double d, vec b, int dir,
          typename GRID::level l, array<DIM> coord,
          array<DIM> coord_shift){
  dir--;

  if (dir<0) {
    int f = g.father_id(l,coord);
    b[f] += d;
    return;
  }

  if (coord[dir]%2==coord_shift[dir]) {
    adddefect(d,b,dir,l,coord,coord_shift);
  }
  else {
    array<DIM> shiftl=coord;
    array<DIM> shiftr=coord;
    shiftl[dir]-=1;
    shiftr[dir]+=1;
    if (! (!g.do_end_share(dir) &&
           coord[dir]==
           g.size(l,dir)+g.end_overlap(l,dir)+g.front_overlap(l,dir)-1)) {
      adddefect(d/2.0,b,dir,l,shiftr,coord_shift);
    }
    if (! (!g.do_front_share(dir) &&
           coord[dir]==0)) {
      adddefect(d/2.0,b,dir,l,shiftl,coord_shift);
    }
  }
}

template <class GRID, int SMOOTHER>
class loopstubRestrict {
  typedef int level;
  enum { DIM = GRID::griddim };
  pmgsolver<GRID,SMOOTHER> & solver;
  array<DIM> coord_shift;
  double * b;
  level L;
public:
  loopstubRestrict(pmgsolver<GRID,SMOOTHER> & solver_, double* b_, level l) :
    solver(solver_), b(b_), L(l)
  {
    for(int d=0; d<DIM; d++)
      coord_shift[d] = solver.g.has_coord_shift(l,d);
  }
  void evaluate(level l, const array<DIM> & coord, int i) {
    solver.adddefect(solver.defect_v[i], b, DIM, l, coord, coord_shift);
  }
};

template <class GRID, int SMOOTHER>
void pmgsolver<GRID,SMOOTHER>::
restrict (level l, vec x, vec b) {
  std::cout << "restrict " << l << std::endl;

  assert(l>0);

  defect(l,x,b);
  exchange(l,defect_v);

  TIME_REST -= MPI_Wtime();
  loopstubRestrict<GRID,SMOOTHER> stub(*this,b,l);
  g.loop_all( l, stub );
  TIME_REST += MPI_Wtime();

  // ABGLEICH vector b Level l-1
  exchange(l-1,b);
  std::cout << "restrict done" << std::endl;
}; /* end restrict() */

/**
    Prolongation x_{l-1} -> x_l
 */
template <class GRID, int SMOOTHER>
inline
double pmgsolver<GRID,SMOOTHER>::
correction(vec x, int dir,
           typename GRID::level l, array<DIM> coord,
           array<DIM> coord_shift){

  dir--;

  if (dir<0) {
    int f=g.father_id(l, coord);
    return x[f];
  }

  if (coord[dir]%2==coord_shift[dir])
    return correction(x,dir,l,coord,coord_shift);
  else {
    array<DIM> shiftl=coord;
    array<DIM> shiftr=coord;
    shiftl[dir]-=1;
    shiftr[dir]+=1;
    return 0.5*correction(x,dir,l,shiftl,coord_shift) +
           0.5*correction(x,dir,l,shiftr,coord_shift);
  }
};

template <class GRID, int SMOOTHER>
class loopstubProlongate {
  typedef int level;
  enum { DIM = GRID::griddim };
  pmgsolver<GRID,SMOOTHER> & solver;
  array<DIM> coord_shift;
  double * x;
public:
  loopstubProlongate(pmgsolver<GRID,SMOOTHER> & solver_, double* x_, level l) :
    solver(solver_), x(x_)
  {
    for(int d=0; d<DIM; d++)
      coord_shift[d] = solver.g.has_coord_shift(l,d);
  }
  void evaluate(level l, const array<DIM> & coord, int i) {
    x[i] += solver.correction(x, DIM, l, coord, coord_shift);
  }
};

template <class GRID, int SMOOTHER>
void pmgsolver<GRID,SMOOTHER>::
prolongate(level l, vec x) {
  assert(l>0);

  TIME_PROL -= MPI_Wtime();
  loopstubProlongate<GRID,SMOOTHER> stub(*this,x,l);
  g.loop_owner( l, stub );
  TIME_PROL += MPI_Wtime();

  // ABGLEICH Level l
  exchange(l,x);
};

template <class GRID, int SMOOTHER>
class loopstubInitExchange {
  typedef int level;
  enum { DIM = GRID::griddim };
  pmgsolver<GRID,SMOOTHER> & solver;
public:
  loopstubInitExchange(pmgsolver<GRID,SMOOTHER> & solver_) :
    solver(solver_) {}
  void evaluate(level l, const array<DIM> & coord, int id) {
    typename GRID::iterator it(id,solver.g);
    typename GRID::index i=*it;
    typename GRID::remotelist remote=i.remote();
    /* if i own the data, I'll search all processes to receive the data */
    if(i.owner()) {
      for (int r=0; r<remote.size; r++) {
        int & size = solver.exchange_data_to[l][remote.list[r].process()].size;
        realloc<int>(solver.exchange_data_to[l][remote.list[r].process()].id,
                     size + 1);
        solver.exchange_data_to[l][remote.list[r].process()].id[size]
          = it.id();
        size ++;
      }
    }
    /* if I share the data, find the owner-processes */
    else {
      for (int r=0; r<remote.size; r++) {
        if (remote.list[r].owner()) {
          int & size = solver.exchange_data_from[l][remote.list[r].process()].size;
          realloc<int>(solver.exchange_data_from[l][remote.list[r].process()].id,
                       size + 1);
          solver.exchange_data_from[l][remote.list[r].process()].id[size]
            = it.id();
          size ++;
          continue;
        }
      }
    }
  }
};

/**
    Datenabgleich auf Level l
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
    loopstubInitExchange<GRID,SMOOTHER> stub(*this);
    g.loop_overlap(l, stub);
  }
};

/**
    Datenabgleich auf Level l
 */
template <class GRID, int SMOOTHER>
void pmgsolver<GRID,SMOOTHER>::
exchange(level l, vec ex) {
  TIME_EX -= MPI_Wtime();

  //#define TALKALOT
#ifdef TALKALOT
  int P;
  MPI_Comm_size(g.comm(), &P); // Number of Processors
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
}; /* end exchange() */

template <class GRID>
class loopstubInitIterator {
  enum { DIM = GRID::griddim };
  double * b;
  double * x;
  discrete<GRID> & mydiscrete;
  GRID & g;
public:
  loopstubInitIterator(double *B, double * X, discrete<GRID> & D, GRID & G) :
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
init(vec x) {

  std::cout << rank << " initalizing exchange data structures..."
            << std::flush;

  /* Initialize the exchange data structures */
  initExchange();
  std::cout << " done\n" << std::flush;


  /* build rhs and x-values at the border on smoothest level */
  typename GRID::level lvl = g.smoothest();
  int size_all = (--g.end(lvl)).id()+1;
  b=new double[size_all];
  bzero(b, sizeof(double) * size_all);
  defect_v=new double[size_all];
  bzero(defect_v, sizeof(double) * size_all);
#ifndef NDEBUG
  for (int i = 0; i < size_all; i++) {
    b[i]=0;
    defect_v[i]=0;
  }
#else
  for (int i = g.begin(lvl).id(); i < size_all; i++) {
    b[i]=0;
    defect_v[i]=0;
  }
#endif

  loopstubInitIterator<GRID> stub(b,x,mydiscrete,g);
  g.loop_border(lvl, stub);

  std::cout << rank << " Exchanging x and b\n" << std::flush;

  exchange(g.smoothest(),x);
  exchange(g.smoothest(),b);

  MPI_Barrier(g.comm());
}

template <class GRID, int SMOOTHER>
void pmgsolver<GRID,SMOOTHER>::
solve(vec x) {
  typename GRID::level lvl = g.smoothest();

  init(x);

  // multigrid cycle
  double mydefect=defect(lvl,x,b);
  double maxdefect=mydefect*d;
  int cycle=0;
  if (rank==0) std::cout << "MGC-Cycle " << cycle
                         << " " << mydefect << " " << 0 << std::endl;
  cycle ++;

  double lastdefect=mydefect;
  array<2> coord;
  coord[0]=7;
  coord[1]=0;
  double starttime, endtime;
  while (mydefect > maxdefect)
  {
    TIME_SMOOTHER = 0;
    TIME_PROL = 0;
    TIME_REST = 0;
    TIME_DEFECT = 0;

    starttime = MPI_Wtime();
#ifndef NODUMP
    dump2D(g,lvl,x,"jakobi","X");
    //    dump2D(g,lvl,b,"jakobi","B");
    dump2D(g,lvl,defect_v,"jakobi","D");
#endif
    mgc(lvl,x,b);
    //smoother(lvl,x,b);
    //smootherJacobi(lvl,x,b);
    mydefect=defect(lvl,x,b);
    if (rank==0) std::cout << "MGC-Cycle " << cycle << " " << mydefect
                           << " " << mydefect/lastdefect
                           << std::endl;
    lastdefect = mydefect;
    endtime = MPI_Wtime();
    if (rank==0)
      std::cout << "0: That took " << endtime-starttime
                << " seconds" << std::endl;
    if (rank==0)
      std::cout << "Time in smoother:" << TIME_SMOOTHER << std::endl
                << "Time in prolongate:" << TIME_PROL << std::endl
                << "Time in restrict:" << TIME_REST << std::endl
                << "Time in exchange:" << TIME_EX << std::endl
                << "Time in defect:" << TIME_DEFECT << std::endl;
    //if (cycle==10) return;
    cycle ++;
  };
};
