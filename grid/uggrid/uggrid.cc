// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
//************************************************************************
//
//  Implementation von UGGrid
//
//  namespace Dune
//
//************************************************************************

#include <stdlib.h>

// *********************************************************************
//
//  -- UGGridLevelIteratorFactory
//
// *********************************************************************

template <int codim, PartitionIteratorType PiType, class GridImp>
class UGGridLevelIteratorFactory
{
public:
  static inline
  UGGridLevelIterator<codim,PiType,GridImp> getIterator(typename UGTypes<GridImp::dimension>::GridType* theGrid, int level) {
    std::cout << "LevelIteratorFactory default implementation" << std::endl;
  }

};


#ifdef _2
template <class GridImp>
class UGGridLevelIteratorFactory<2,All_Partition,GridImp>
{
public:
  static inline
  UGGridLevelIterator<2,All_Partition,GridImp> getIterator(typename UGTypes<GridImp::dimension>::GridType* theGrid, int level) {

    UGGridLevelIterator<2,All_Partition,GridImp> it(level);
    it.setToTarget(UG_NS<2>::PFirstNode(theGrid), level);
    return it;
  }

};

template <class GridImp>
class UGGridLevelIteratorFactory<0,All_Partition,GridImp>
{
public:
  static inline
  UGGridLevelIterator<0,All_Partition,GridImp> getIterator(typename UGTypes<GridImp::dimension>::GridType* theGrid, int level) {

    UGGridLevelIterator<0,All_Partition,GridImp> it(level);
    it.setToTarget(UG_NS<GridImp::dimension>::PFirstElement(theGrid), level);
    return it;
  }

};


template <PartitionIteratorType PiType, class GridImp>
class UGGridLevelIteratorFactory<2,PiType,GridImp>
{
public:
  static inline
  UGGridLevelIterator<2,PiType,GridImp> getIterator(UGTypes<2>::GridType* theGrid, int level) {
    std::cout << "Simulating a parallel LevelIterator using a sequential one!" << std::endl;
    UGGridLevelIterator<2,PiType,GridImp> it(level);
    it.setToTarget(UG_NS<2>(theGrid), level);
    return it;
  }

};

template <PartitionIteratorType PiType,class GridImp>
class UGGridLevelIteratorFactory<0,PiType,GridImp>
{
public:
  static inline
  UGGridLevelIterator<0,PiType,GridImp> getIterator(UGTypes<2>::GridType* theGrid, int level) {

    std::cout << "Simulating a parallel LevelIterator using a sequential one!" << std::endl;
    UGGridLevelIterator<0,PiType,GridImp> it(level);
    it.setToTarget(UG_NS<2>::PFirstElement(theGrid), level);
    return it;
  }

};
#endif

#ifdef _3
template <class GridImp>
class UGGridLevelIteratorFactory<3,All_Partition,GridImp>
{
public:
  static inline
  UGGridLevelIterator<3,All_Partition,GridImp> getIterator(typename UGTypes<GridImp::dimension>::GridType* theGrid, int level) {

    UGGridLevelIterator<3,All_Partition,GridImp> it(level);

    it.setToTarget(UG_NS<3>::FirstNode(theGrid), level);
    return it;
  }
};

template <class GridImp>
class UGGridLevelIteratorFactory<0,All_Partition,GridImp>
{
public:
  static inline
  UGGridLevelIterator<0,All_Partition,GridImp> getIterator(UGTypes<3>::GridType* theGrid, int level) {

    UGGridLevelIterator<0,All_Partition,GridImp> it(level);

    it.setToTarget(UG_NS<3>::FirstElement(theGrid), level);
    return it;
  }
};
#endif

//***********************************************************************
//
// --UGGrid
// --Grid
//
//***********************************************************************

template<> int UGGrid < 2, 2 >::numOfUGGrids = 0;
template<> int UGGrid < 3, 3 >::numOfUGGrids = 0;

template < int dim, int dimworld >
inline UGGrid < dim, dimworld >::UGGrid() : multigrid_(NULL), refinementType_(COPY), omitGreenClosure_(false)
{
  init(500, 10);
}

template < int dim, int dimworld >
inline UGGrid < dim, dimworld >::UGGrid(unsigned int heapSize, unsigned envHeapSize)
  : multigrid_(NULL), refinementType_(COPY), omitGreenClosure_(false)
{
  init(heapSize, envHeapSize);
}

template < int dim, int dimworld >
inline void UGGrid < dim, dimworld >::init(unsigned int heapSize, unsigned envHeapSize)
{
  heapsize = heapSize;

  if (numOfUGGrids==0) {

    useExistingDefaultsFile = false;

    if (access("defaults", F_OK) == 0) {

      std::cout << "Using existing UG defaults file" << std::endl;
      useExistingDefaultsFile = true;

    } else {

      // Pass the explicitly given environment heap size
      // This is only possible by passing a pseudo 'defaults'-file
      FILE* fp = fopen("defaults", "w");
      fprintf(fp, "envmemory  %d000000\n", envHeapSize);
      fclose(fp);

    }

    // Init the UG system
    int argc = 1;
    char* arg = {"dune.exe"};
    char** argv = &arg;


    UG_NS<dimworld>::InitUg(&argc, &argv);

  }

  // Create a dummy problem
#ifdef _3
  UG3d::CoeffProcPtr coeffs[1];
  UG3d::UserProcPtr upp[1];
#else
  UG2d::CoeffProcPtr coeffs[1];
  UG2d::UserProcPtr upp[1];
#endif

  upp[0] = NULL;
  coeffs[0] = NULL;

  // Create unique problem name
  static unsigned int nameCounter = 0;
  std::stringstream numberAsAscii;
  numberAsAscii << nameCounter;
  name_ = "DuneUGGrid_" + numberAsAscii.str();

  std::string problemName = name_ + "_Problem";

  if (UG_NS<dimworld>::CreateBoundaryValueProblem(problemName.c_str(), 1,coeffs,1,upp) == NULL)
    DUNE_THROW(GridError, "UG?d::CreateBoundaryValueProblem() returned and error code!");

  if (numOfUGGrids==0) {

    // A Dummy new format
    // We need to pass the parameters in this complicated way, because
    // UG writes into one of the strings, and code compiled by some
    // compilers (gcc, for example) crashes on this.
    //newformat P1_conform $V n1: nt 9 $M implicit(nt): mt 2 $I n1;
    /** \todo Use a smaller format in order to save memory */
    //         for (int i=0; i<4; i++)
    //             newformatArgs[i] = (char*)::malloc(50*sizeof(char));

    //         sprintf(newformatArgs[0], "newformat DuneFormat");
    //sprintf(newformatArgs[1], "V n1: nt 0");
    //sprintf(newformatArgs[2], "M implicit(nt): mt 0");
    //sprintf(newformatArgs[3], "I n1");

    char* nfarg = "newformat DuneFormat";
#ifdef _3
    UG3d::CreateFormatCmd(1, &nfarg);
    //UG3d::CreateFormatCmd(1, newformatArgs);
#else
    UG2d::CreateFormatCmd(1, &nfarg);
    //UG2d::CreateFormatCmd(1, newformatArgs);
#endif
  }

  numOfUGGrids++;

  extra_boundary_data_ = 0;

  std::cout << "UGGrid<" << dim << "," << dimworld <<"> with name "
            << name_ << " created!" << std::endl;

  nameCounter++;
}

template < int dim, int dimworld >
inline UGGrid < dim, dimworld >::~UGGrid()
{
  if (extra_boundary_data_)
    free(extra_boundary_data_);

  if (multigrid_) {
    // Set UG's currBVP variable to the BVP corresponding to this
    // grid.  This is necessary if we have more than one UGGrid in use.
    // DisposeMultiGrid will crash if we don't do this
    std::string BVPName = name() + "_Problem";
#ifdef _3
    void* thisBVP = UG3d::BVP_GetByName(BVPName.c_str());
#else
    void* thisBVP = UG2d::BVP_GetByName(BVPName.c_str());
#endif

    if (thisBVP == NULL)
      DUNE_THROW(GridError, "Couldn't find grid's own boundary value problem!");

#ifdef _3
    UG3d::Set_Current_BVP((void**)thisBVP);
    UG3d::DisposeMultiGrid(multigrid_);
#else
    UG2d::Set_Current_BVP((void**)thisBVP);
    UG2d::DisposeMultiGrid(multigrid_);
#endif
  }

  numOfUGGrids--;

  // Shut down UG if this was the last existing UGGrid object
  if (numOfUGGrids == 0) {

#ifdef _3
    UG3d::ExitUg();
#else
    UG2d::ExitUg();
#endif

    // remove defaults file, if we wrote one on startup
    if (!useExistingDefaultsFile)
      system("rm defaults");

  }

};

template < int dim, int dimworld >
inline int UGGrid < dim, dimworld >::maxlevel() const
{
  assert(multigrid_);
  return multigrid_->topLevel;
}



template<int dim, int dimworld>
template<int codim>
typename UGGrid<dim,dimworld>::Traits::template codim<codim>::LevelIterator
UGGrid<dim, dimworld>::lbegin (int level) const
{
  assert(multigrid_);
  typename UGTypes<dim>::GridType* theGrid = multigrid_->grids[level];

  if (!theGrid)
    DUNE_THROW(GridError, "LevelIterator in nonexisting level " << level << " requested!");

  return UGGridLevelIteratorFactory<codim,All_Partition, const UGGrid<dim,dimworld> >::getIterator(theGrid, level);
}

template<int dim, int dimworld>
template<int codim, PartitionIteratorType PiType>
inline UGGridLevelIterator<codim,PiType,UGGrid<dim,dimworld> >
UGGrid<dim, dimworld>::lbegin (int level) const
{
  assert(multigrid_);
  typename UGTypes<dim>::GridType* theGrid = multigrid_->grids[level];

  if (!theGrid)
    DUNE_THROW(GridError, "LevelIterator in nonexisting level " << level << " requested!");

  return UGGridLevelIteratorFactory<codim,PiType,UGGrid<dim,dimworld> >::getIterator(theGrid, level);
}

template < int dim, int dimworld >
template<int codim>
typename UGGrid<dim,dimworld>::Traits::template codim<codim>::LevelIterator
UGGrid < dim, dimworld >::lend (int level) const
{
  UGGridLevelIterator<codim,All_Partition, const UGGrid<dim,dimworld> > it(level);
  return it;
}

template < int dim, int dimworld >
template<int codim, PartitionIteratorType PiType>
inline UGGridLevelIterator<codim,PiType,UGGrid<dim,dimworld> >
UGGrid < dim, dimworld >::lend (int level) const
{
  UGGridLevelIterator<codim,PiType,UGGrid<dim,dimworld> > it(level);
  return it;
}

template < int dim, int dimworld >
inline int UGGrid < dim, dimworld >::size (int level, int codim) const
{
  int numberOfElements = 0;

  if(codim == 0)
  {
    typename Traits::template codim<0>::LevelIterator it = lbegin<0>(level);
    typename Traits::template codim<0>::LevelIterator endit = lend<0>(level);
    for (; it != endit; ++it)
      numberOfElements++;

  } else
  if(codim == dim)
  {
    typename Traits::template codim<dim>::LevelIterator it    = lbegin<dim>(level);
    typename Traits::template codim<dim>::LevelIterator endit = lend<dim>(level);
    for(; it != endit; ++it)
      numberOfElements++;
  }
  else
  {
    DUNE_THROW(GridError, "UGGrid<" << dim << ", " << dimworld
                                    << ">::size(int level, int codim) is only implemented"
                                    << " for codim==0 and codim==dim!");
  }

  return numberOfElements;
}


template < int dim, int dimworld >
void UGGrid < dim, dimworld >::makeNewUGMultigrid()
{
  //configure @PROBLEM $d @DOMAIN;
  std::string configureArgs[2] = {"configure " + name() + "_Problem", "d " + name() + "_Domain"};
  const char* configureArgs_c[2]     = {configureArgs[0].c_str(), configureArgs[1].c_str()};
  /** \todo Kann man ConfigureCommand so ‰ndern daﬂ man auch ohne den const_cast auskommt? */
#ifdef _3
  UG3d::ConfigureCommand(2, const_cast<char**>(configureArgs_c));
#else
  UG2d::ConfigureCommand(2, const_cast<char**>(configureArgs_c));
#endif

  //new @PROBLEM $b @PROBLEM $f @FORMAT $h @HEAP;
  char* newArgs[4];
  for (int i=0; i<4; i++)
    newArgs[i] = (char*)::malloc(50*sizeof(char));

  sprintf(newArgs[0], "new %s", name().c_str());

  sprintf(newArgs[1], "b %s_Problem", name().c_str());
  sprintf(newArgs[2], "f DuneFormat");
  sprintf(newArgs[3], "h %dM", heapsize);

#ifdef _3
  if (UG3d::NewCommand(4, newArgs))
#else
  if (UG2d::NewCommand(4, newArgs))
#endif
    DUNE_THROW(GridError, "UGGrid::makeNewMultigrid failed!");

  for (int i=0; i<4; i++)
    free(newArgs[i]);

  // Get a direct pointer to the newly created multigrid
  multigrid_ = UG_NS<dim>::GetMultigrid(name().c_str());
  if (!multigrid_)
    DUNE_THROW(GridError, "UGGrid::makeNewMultigrid failed!");
}

template < int dim, int dimworld >
bool UGGrid < dim, dimworld >::mark(int refCount,
                                    typename Traits::template codim<0>::EntityPointer & e )
{
  typename TargetType<0,dim>::T* target = getRealEntity<0>(*e).target_;

#ifdef _3
  if (!UG3d::EstimateHere(target))
    return false;

  return UG3d::MarkForRefinement(target,
                                 UG3d::RED,    // red refinement rule
                                 0);    // no user data
#else
  if (!UG2d::EstimateHere(target))
    return false;

  return UG2d::MarkForRefinement(target,
                                 UG2d::RED,   // red refinement rule
                                 0);    // no user data
#endif
}

template < int dim, int dimworld >
bool UGGrid < dim, dimworld >::mark(typename Traits::template codim<0>::EntityPointer & e,
#ifdef _3
                                    UG3d::RefinementRule rule
#else
                                    UG2d::RefinementRule rule
#endif
                                    )
{
  typename TargetType<0,dim>::T* target = getRealEntity<0>(*e).target_;

#ifdef _3
  if (!UG3d::EstimateHere(target))
    return false;

  return UG3d::MarkForRefinement(target,
                                 rule,
                                 0);    // no user data
#else
  if (!UG2d::EstimateHere(target))
    return false;

  return UG2d::MarkForRefinement(target,
                                 rule,
                                 0);    // no user data
#endif
}

template < int dim, int dimworld >
bool UGGrid < dim, dimworld >::adapt()
{
  int rv;
  int mode;

  assert(multigrid_);

  // Set UG's currBVP variable to the BVP corresponding to this
  // grid.  This is necessary if we have more than one UGGrid in use.
  std::string BVPName = name() + "_Problem";
#ifdef _3
  void* thisBVP = UG3d::BVP_GetByName(BVPName.c_str());
#else
  void* thisBVP = UG2d::BVP_GetByName(BVPName.c_str());
#endif

  if (thisBVP == NULL)
    DUNE_THROW(GridError, "Couldn't find grid's own boundary value problem!");

#ifdef _3
  UG3d::Set_Current_BVP((void**)thisBVP);
#else
  UG2d::Set_Current_BVP((void**)thisBVP);
#endif

  mode = UG_NS<dim>::GM_REFINE_TRULY_LOCAL;

  if (refinementType_==COPY)
    mode = mode | UG_NS<dim>::GM_COPY_ALL;

  if (omitGreenClosure_)
    mode = mode | UG_NS<dim>::GM_REFINE_NOT_CLOSED;

  // I don't really know what this means
  int seq = UG_NS<dim>::GM_REFINE_PARALLEL;

  // I don't really know what this means either
  int mgtest = UG_NS<dim>::GM_REFINE_NOHEAPTEST;

  rv = AdaptMultiGrid(multigrid_,mode,seq,mgtest);

  if (rv!=0)
    DUNE_THROW(GridError, "UG::adapt() returned with error code " << rv);

  // Collapse the complete grid hierarchy into a single level if requested
  if (refinementType_==COLLAPSE)
    if (Collapse(multigrid_))
      DUNE_THROW(GridError, "UG?d::Collapse returned error code!");

  // Renumber everything
  for (int i=0; i<=maxlevel(); i++) {

    typename Traits::template codim<0>::LevelIterator eIt    = lbegin<0>(i);
    typename Traits::template codim<0>::LevelIterator eEndIt = lend<0>(i);

    int id = 0;
    for (; eIt!=eEndIt; ++eIt)
      getRealEntity<0>(*eIt).target_->ge.id = id++;

    typename Traits::template codim<dim>::LevelIterator vIt    = lbegin<dim>(i);
    typename Traits::template codim<dim>::LevelIterator vEndIt = lend<dim>(i);

    id = 0;
    for (; vIt!=vEndIt; ++vIt)
      getRealEntity<dim>(*vIt).target_->id = id++;

  }

  /** \bug Should return true only if at least one element has actually
      been refined */
  return true;
}

template <int dim, int dimworld>
void UGGrid <dim, dimworld>::adaptWithoutClosure()
{
  bool omitClosureBackup = omitGreenClosure_;
  omitGreenClosure_ = true;
  adapt();
  omitGreenClosure_ = omitClosureBackup;
}

template < int dim, int dimworld >
void UGGrid < dim, dimworld >::globalRefine(int refCount)
{
  // mark all entities for grid refinement
  typename Traits::template codim<0>::LevelIterator iIt    = lbegin<0>(maxlevel());
  typename Traits::template codim<0>::LevelIterator iEndIt = lend<0>(maxlevel());

  for (; iIt!=iEndIt; ++iIt)
    mark(1, iIt);

  this->preAdapt();
  adapt();
  this->postAdapt();

}

template < int dim, int dimworld >
void UGGrid < dim, dimworld >::loadBalance(int strategy, int minlevel, int depth, int maxlevel, int minelement)
{
  /** \todo Test for valid arguments */
  std::string argStrings[4];
  std::stringstream numberAsAscii[4];

  numberAsAscii[0] << strategy;
  argStrings[0] = "lb " + numberAsAscii[0].str();

  numberAsAscii[1] << minlevel;
  argStrings[1] = "c " + numberAsAscii[1].str();

  numberAsAscii[2] << depth;
  argStrings[2] = "d " + numberAsAscii[2].str();

  numberAsAscii[3] << minelement;
  argStrings[3] = "e " + numberAsAscii[3].str();

  const char* argv[4] = {argStrings[0].c_str(),
                         argStrings[1].c_str(),
                         argStrings[2].c_str(),
                         argStrings[3].c_str()};

#ifdef _2
  int errCode = UG2d::LBCommand(4, (char**)argv);
#else
  int errCode = UG3d::LBCommand(4, (char**)argv);
#endif

  if (errCode)
    DUNE_THROW(GridError, "UG?d::LBCommand returned error code " << errCode);
}

#ifdef ModelP
template <class T, template <class> class P>
class Foo {
public:

  static int gather(DDD_OBJ obj, void* data)
  {
    int codim=0;
    enum {dim = 2};

    P<T>* p = (P<T>*)data;

    int index = 0;
    switch (codim) {
    case 0 :
      index = ((UG2d::element*)obj)->ge.id;
      break;
    case dim :
      index = ((UG2d::node*)obj)->id;
      break;
    default :
      DUNE_THROW(GridError, "UGGrid::communicate only implemented for this codim");
    }

    p->gather(*dataArray, index);

    return 0;
  }

  static int scatter(DDD_OBJ obj, void* data)
  {
    int codim=0;
    enum {dim = 2};

    P<T>* p = (P<T>*)data;

    int index = 0;
    switch (codim) {
    case 0 :
      index = ((UG2d::element*)obj)->ge.id;
      break;
    case dim :
      index = ((UG2d::node*)obj)->id;
      break;
    default :
      DUNE_THROW(GridError, "UGGrid::communicate only implemented for codim 0 and dim");
    }

    p->scatter(*dataArray, index);

    return 0;
  }

  static T* dataArray;

};

template <class T, template<class> class P>
T* Foo<T,P>::dataArray = 0;
#endif

template < int dim, int dimworld >
template<class T, template<class> class P, int codim>
void UGGrid < dim, dimworld >::communicate (T& t, InterfaceType iftype, CommunicationDirection dir, int level)
{
  std::cout << "UGGrid communicator not implemented yet!\n";
#ifdef ModelP
  Foo<T,P> foo;

  Foo<T,P>::dataArray = &t;

  /** \todo Should be in a namespace */
  DDD_IFAExchange(0,
                  level,   //GRID_ATTR(g),
                  sizeof(P<T>),
                  &Foo<T,P>::gather,
                  &Foo<T,P>::scatter);
#endif
}


template < int dim, int dimworld >
void UGGrid < dim, dimworld >::createbegin()
{}


template < int dim, int dimworld >
void UGGrid < dim, dimworld >::createend()
{
  // set the subdomainIDs
  typename TargetType<0,dim>::T* theElement;
  for (theElement=multigrid_->grids[0]->elements[0]; theElement!=NULL; theElement=theElement->ge.succ)
    UG_NS<dim>::SetSubdomain(theElement, 1);

#ifdef _3
  UG3d::SetEdgeAndNodeSubdomainFromElements(multigrid_->grids[0]);
#else
  UG2d::SetEdgeAndNodeSubdomainFromElements(multigrid_->grids[0]);
#endif

  // Complete the UG-internal grid data structure
#ifdef _3
  if (CreateAlgebra(multigrid_) != UG3d::GM_OK)
#else
  if (CreateAlgebra(multigrid_) != UG2d::GM_OK)
#endif
    DUNE_THROW(IOError, "Call of 'UG::CreateAlgebra' failed!");

  /* here all temp memory since CreateMultiGrid is released */
#define ReleaseTmpMem(p,k) Release(p, UG::FROM_TOP,k)
  ReleaseTmpMem(multigrid_->theHeap, multigrid_->MarkKey);
#undef ReleaseTmpMem
  multigrid_->MarkKey = 0;
}
