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

template <int codim, int dim, int dimworld, PartitionIteratorType PiType>
class UGGridLevelIteratorFactory
{
public:
  static inline
  UGGridLevelIterator<codim,dim,dimworld,PiType> getIterator(typename UGTypes<dim>::GridType* theGrid, int level) {
    std::cout << "LevelIteratorFactory default implementation" << std::endl;
  }

};


#ifdef _2
template <>
class UGGridLevelIteratorFactory<2,2,2,All_Partition>
{
public:
  static inline
  UGGridLevelIterator<2,2,2,All_Partition> getIterator(UGTypes<2>::GridType* theGrid, int level) {

    UGGridLevelIterator<2,2,2,All_Partition> it(level);
    it.setToTarget(theGrid->firstNode[0], level);
    return it;
  }

};

template <>
class UGGridLevelIteratorFactory<0,2,2,All_Partition>
{
public:
  static inline
  UGGridLevelIterator<0,2,2,All_Partition> getIterator(UGTypes<2>::GridType* theGrid, int level) {

    UGGridLevelIterator<0,2,2,All_Partition> it(level);
    it.setToTarget(theGrid->elements[0], level);
    return it;
  }

};
#endif

#ifdef _3
template <>
class UGGridLevelIteratorFactory<3,3,3,All_Partition>
{
public:
  static inline
  UGGridLevelIterator<3,3,3,All_Partition> getIterator(UGTypes<3>::GridType* theGrid, int level) {

    UGGridLevelIterator<3,3,3,All_Partition> it(level);

    UG3d::node* mytarget = theGrid->firstNode[0];
    it.setToTarget(mytarget, level);
    return it;
  }
};

template <>
class UGGridLevelIteratorFactory<0,3,3,All_Partition>
{
public:
  static inline
  UGGridLevelIterator<0,3,3,All_Partition> getIterator(UGTypes<3>::GridType* theGrid, int level) {

    UGGridLevelIterator<0,3,3,All_Partition> it(level);

    it.setToTarget(theGrid->elements[0], level);
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
inline UGGrid < dim, dimworld >::UGGrid() : multigrid_(NULL), refinementType_(LOCAL)
{
  init(500, 10);
}

template < int dim, int dimworld >
inline UGGrid < dim, dimworld >::UGGrid(unsigned int heapSize, unsigned envHeapSize)
  : multigrid_(NULL), refinementType_(LOCAL)
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
#ifdef _3
    //UG3d::DisposeMultiGrid(multigrid_);
#else
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

    //         for (int i=0; i<4; i++)
    //             free(newformatArgs[i]);

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
inline UGGridLevelIterator<codim, dim, dimworld, All_Partition>
UGGrid<dim, dimworld>::lbegin (int level) const
{
  assert(multigrid_);
  typename UGTypes<dim>::GridType* theGrid = multigrid_->grids[level];

  if (!theGrid)
    DUNE_THROW(GridError, "LevelIterator in nonexisting level " << level << " requested!");

  return UGGridLevelIteratorFactory<codim,dim,dimworld,All_Partition>::getIterator(theGrid, level);
}

template<int dim, int dimworld>
template<int codim, PartitionIteratorType PiType>
inline UGGridLevelIterator<codim, dim, dimworld,PiType>
UGGrid<dim, dimworld>::lbegin (int level) const
{
  assert(multigrid_);
  typename UGTypes<dim>::GridType* theGrid = multigrid_->grids[level];

  if (!theGrid)
    DUNE_THROW(GridError, "LevelIterator in nonexisting level " << level << " requested!");

  return UGGridLevelIteratorFactory<codim,dim,dimworld,PiType>::getIterator(theGrid, level);
}

template < int dim, int dimworld > template<int codim>
inline UGGridLevelIterator<codim,dim,dimworld, All_Partition>
UGGrid < dim, dimworld >::lend (int level) const
{
  UGGridLevelIterator<codim,dim,dimworld,All_Partition> it(level);
  return it;
}

template < int dim, int dimworld >
template<int codim, PartitionIteratorType PiType>
inline UGGridLevelIterator<codim,dim,dimworld,PiType>
UGGrid < dim, dimworld >::lend (int level) const
{
  UGGridLevelIterator<codim,dim,dimworld,PiType> it(level);
  return it;
}

template < int dim, int dimworld >
inline int UGGrid < dim, dimworld >::size (int level, int codim) const
{
  int numberOfElements = 0;

  if(codim == 0)
  {
    UGGridLevelIterator<0,dim,dimworld, All_Partition> endit = lend<0>(level);
    for(UGGridLevelIterator<0,dim,dimworld, All_Partition> it = lbegin<0>(level);
        it != endit; ++it)
      numberOfElements++;
  } else
  if(codim == dim)
  {
    UGGridLevelIterator<dim,dim,dimworld, All_Partition> endit = lend<dim>(level);
    for(UGGridLevelIterator<dim,dim,dimworld, All_Partition> it = lbegin<dim>(level);
        it != endit; ++it)
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
inline bool UGGrid < dim, dimworld >::adapt()
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

#ifdef _3
  mode = UG3d::GM_REFINE_TRULY_LOCAL;

  if (refinementType_==COPY)
    mode = mode | UG3d::GM_COPY_ALL;

  // I don't really know what this means
  int seq = UG3d::GM_REFINE_PARALLEL;

  // I don't really know what this means either
  int mgtest = UG3d::GM_REFINE_NOHEAPTEST;

  rv = UG3d::AdaptMultiGrid(multigrid_,mode,seq,mgtest);
#else
  mode = UG2d::GM_REFINE_TRULY_LOCAL;

  if (refinementType_==COPY)
    mode = mode | UG2d::GM_COPY_ALL;

  // I don't really know what this means
  int seq = UG2d::GM_REFINE_PARALLEL;

  // I don't really know what this means either
  int mgtest = UG2d::GM_REFINE_NOHEAPTEST;

  /** \todo Why don't I have to mention the namespace?? */
  rv = AdaptMultiGrid(multigrid_,mode,seq,mgtest);
#endif

  if (rv!=0)
    DUNE_THROW(GridError, "UG::adapt() returned with error code " << rv);

  // Collapse the complete grid hierarchy into a single level if requested
#ifdef _3
  if (refinementType_==COLLAPSE)
    if (UG3d::Collapse(multigrid_))
      DUNE_THROW(GridError, "UG3d::Collapse returned error code!");
#else
  if (refinementType_==COLLAPSE)
    if (UG2d::Collapse(multigrid_))
      DUNE_THROW(GridError, "UG2d::Collapse returned error code!");
#endif

  /** \bug Should return true only if at least one element has actually
      been refined */
  return true;
}


template < int dim, int dimworld >
void UGGrid < dim, dimworld >::globalRefine(int refCount)
{
  // mark all entities for grid refinement
  UGGridLevelIterator<0, dim, dimworld, All_Partition> iIt    = lbegin<0>(maxlevel());
  UGGridLevelIterator<0, dim, dimworld, All_Partition> iEndIt = lend<0>(maxlevel());

  for (; iIt!=iEndIt; ++iIt)
    iIt->mark(1);

  this->preAdapt();
  adapt();
  this->postAdapt();

}
