// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRAPEDATAIO_HH
#define DUNE_GRAPEDATAIO_HH

//- system includes

//- Dune includes
#include <dune/common/misc.hh>
#include <dune/common/capabilities.hh>
#include <dune/grid/common/grid.hh>

#include <dune/io/file/asciiparser.hh>
#include <dune/fem/dofmanager.hh>

//- Local includes


namespace Dune {

  /*!
     Specify the format to store grid and vector data
   */
  enum GrapeIOFileFormatType
  { ascii ,     //!< store data in a human readable form
    xdr ,       //!< store data in SUN's library routines
                //!< for external data representation (xdr)
    pgm };      //!< store data in portable graymap file format



  typedef std::string GrapeIOStringType;

  /** \brief convert type to string
   */
  template <typename T>
  inline GrapeIOStringType typeIdentifier ()
  {
    GrapeIOStringType tmp = "unknown";
    return tmp;
  }

  template <>
  inline GrapeIOStringType typeIdentifier<float> ()
  {
    GrapeIOStringType tmp = "float";
    return tmp;
  }

  template <>
  inline GrapeIOStringType typeIdentifier<int> ()
  {
    GrapeIOStringType tmp = "int";
    return tmp;
  }

  template <>
  inline GrapeIOStringType typeIdentifier<double> ()
  {
    GrapeIOStringType tmp = "double";
    return tmp;
  }

  template <int dim, int dimworld, class GridImp, bool hasBackupRestore>
  class GrapeDataIOImp
  {
    typedef GridImp GridType;
  public:
    /** Write Grid with GridType file filename and time
     *
     * This method uses the Grid Interface Method writeGrid
     * to actually write the grid, within this method the real file name is
     * generated out of filename and timestep
     */
    inline static bool writeGrid (const GridType & grid,
                                  const GrapeIOFileFormatType ftype, const GrapeIOStringType & fnprefix
                                  , double time=0.0, int timestep=0, int precision = 6);

    //! get Grid from file with time and timestep , return true if ok
    inline static bool readGrid (GridType & grid,
                                 const GrapeIOStringType & fnprefix , double & time , int timestep);

    //! get Grid from file with time and timestep , return true if ok
    inline static GridType * restoreGrid (
      const GrapeIOStringType & fnprefix , double & time , int timestep)
    {
      GridType * grid = new GridType ();
      assert( grid );
      readGrid(*grid,fnprefix,time,timestep);
      return grid;
    }
  };

  template <int dim, int dimworld, class GridImp>
  class GrapeDataIOImp<dim,dimworld,GridImp,false>
  {
    typedef GridImp GridType;
  public:
    /** Write Grid with GridType file filename and time
     *
     * This method uses the Grid Interface Method writeGrid
     * to actually write the grid, within this method the real file name is
     * generated out of filename and timestep
     */
    inline static bool writeGrid (const GridType & grid,
                                  const GrapeIOFileFormatType ftype, const GrapeIOStringType & fnprefix
                                  , double time=0.0, int timestep=0, int precision = 6)
    {
      return false;
    }

    //! get Grid from file with time and timestep , return true if ok
    inline static bool readGrid (GridType & grid,
                                 const GrapeIOStringType & fnprefix , double & time , int timestep)
    {
      return false;
    }

    //! get Grid from file with time and timestep , return true if ok
    inline static GridType * restoreGrid (
      const GrapeIOStringType & fnprefix , double & time , int timestep)
    {
      return 0;
    }
  };


  template <class GridType>
  class GrapeDataIO
  {
  public:
    GrapeDataIO () {};

    /** Write Grid with GridType file filename and time
     *
     * This method uses the Grid Interface Method writeGrid
     * to actually write the grid, within this method the real file name is
     * generated out of filename and timestep
     */
    inline bool writeGrid (const GridType & grid,
                           const GrapeIOFileFormatType ftype, const GrapeIOStringType fnprefix
                           , double time=0.0, int timestep=0, int precision = 6) const
    {
      const bool hasBackupRestore = Capabilities::hasBackupRestoreFacilities<GridType>::v;
      return GrapeDataIOImp<GridType::dimension,GridType::dimensionworld,GridType,hasBackupRestore>::
             writeGrid(grid,ftype,fnprefix,time,timestep,precision);
    }

    //! get Grid from file with time and timestep , return true if ok
    inline bool readGrid (GridType & grid,
                          const GrapeIOStringType fnprefix , double & time , int timestep)
    {
      const bool hasBackupRestore = Capabilities::hasBackupRestoreFacilities<GridType>::v;
      return GrapeDataIOImp<GridType::dimension,GridType::dimensionworld,GridType,hasBackupRestore>::
             readGrid(grid,fnprefix,time,timestep);
    }

    //! get Grid from file with time and timestep , return true if ok
    inline GridType * restoreGrid(const GrapeIOStringType fnprefix , double & time , int timestep)
    {
      const bool hasBackupRestore = Capabilities::hasBackupRestoreFacilities<GridType>::v;
      return GrapeDataIOImp<GridType::dimension,GridType::dimensionworld,GridType,hasBackupRestore>::
             restoreGrid(fnprefix,time,timestep);
    }

    /**
       Write DiscreteFunctions
     */
    //! write disc func information file and write dofs to file+timestep
    //! this method use the write method of the implementation of the
    //! discrete function
    template <class DiscreteFunctionType>
    inline bool writeData(DiscreteFunctionType & df,
                          const GrapeIOFileFormatType ftype, const GrapeIOStringType filename,
                          int timestep, int precision = 6);

    //! same as write only read
    template <class DiscreteFunctionType>
    inline bool readData(DiscreteFunctionType & df,
                         const GrapeIOStringType filename, int timestep);
  };

  template <int dim, int dimworld, class GridImp, bool hasBackupRestore>
  inline bool GrapeDataIOImp<dim,dimworld,GridImp,hasBackupRestore> :: writeGrid
    (const GridImp & grid,
    const GrapeIOFileFormatType ftype, const GrapeIOStringType & fnprefix ,
    double time, int timestep, int precision )
  {
    // write dof manager, that corresponds to grid
    bool hasDm = false;
    {
      typedef DofManager<GridImp> DofManagerType;
      typedef DofManagerFactory<DofManagerType> DMFactoryType;

      std::string dmname(fnprefix);
      dmname += "_dm";
      hasDm = DMFactoryType::writeDofManager(grid,dmname,timestep);
    }

    // write Grid itself
    {
      const char *path = "";
      std::fstream file (fnprefix.c_str(),std::ios::out);
      file << "Grid: "   << transformToGridName(grid.type()) << std::endl;
      file << "Format: " << ftype <<  std::endl;
      file << "Precision: " << precision << std::endl;
      int writeDm = (hasDm) ? 1 : 0;
      file << "DofManager: " << writeDm << std::endl;

      GrapeIOStringType fnstr = genFilename(path,fnprefix,timestep,precision);

      file.close();
      switch (ftype)
      {
      case xdr  :   return grid.template writeGrid<xdr>  (fnstr,time);
      case ascii :   return grid.template writeGrid<ascii>(fnstr,time);
      //case xdr  :   return grid.writeGrid(xdr,fnstr,time);
      default :
      {
        std::cerr << ftype << " GrapeIOFileFormatType not supported at the moment! " << __FILE__ << __LINE__ << "\n";
        assert(false);
        abort();
        return false;
      }
      }
      return false;
    }
    return false;

  }

  template <int dim, int dimworld, class GridImp, bool hasBackupRestore>
  inline bool GrapeDataIOImp<dim,dimworld,GridImp,hasBackupRestore> :: readGrid
    (GridImp & grid, const GrapeIOStringType & fnprefix , double & time , int timestep)
  {
    int helpType;

    char gridname [1024];
    readParameter(fnprefix,"Grid",gridname);
    std::string gname (gridname);

    if(transformToGridName(grid.type()) != gname)
    {
      std::cerr << "\nERROR: " << transformToGridName(grid.type()) << " tries to read " << gname << " file. \n";
      abort();
    }

    readParameter(fnprefix,"Format",helpType);
    GrapeIOFileFormatType ftype = (GrapeIOFileFormatType) helpType;

    int precision = 6;
    readParameter(fnprefix,"Precision",precision);

    int hasDm = 0;
    readParameter(fnprefix,"DofManager",hasDm);

    const char *path = "";
    GrapeIOStringType fn = genFilename(path,fnprefix,timestep,precision);
    std::cout << "Read file: fnprefix = `" << fn << "' \n";

    bool succeded = false;
    switch (ftype)
    {
    case xdr  :   succeded = grid.template readGrid<xdr>  (fn,time); break;
    case ascii :   succeded = grid.template readGrid<ascii>(fn,time); break;
    default :
    {
      std::cerr << ftype << " GrapeIOFileFormatType not supported at the moment! \n";
      assert(false);
      abort();
      return false;
    }
    }

    // write dof manager, that corresponds to grid
    if(hasDm)
    {
      typedef DofManager<GridImp> DofManagerType;
      typedef DofManagerFactory<DofManagerType> DMFactoryType;

      std::string dmname(fn);
      dmname += "_dm";
      //std::cout << "Read DofManager from file " << dmname << "\n";
      // this call creates DofManager if not already existing
      DMFactoryType::getDofManager(grid);
      succeded = DMFactoryType::writeDofManager(grid,dmname,timestep);
    }
    return succeded;
  }

  template <class GridType>
  template <class DiscreteFunctionType>
  inline bool GrapeDataIO<GridType> :: writeData(DiscreteFunctionType & df,
                                                 const GrapeIOFileFormatType ftype, const GrapeIOStringType filename, int timestep, int precision )
  {
    {
      typedef typename DiscreteFunctionType::FunctionSpaceType DiscreteFunctionSpaceType;
      typedef typename DiscreteFunctionSpaceType::DomainFieldType DomainFieldType;
      typedef typename DiscreteFunctionSpaceType::RangeFieldType RangeFieldType;

      enum { n = DiscreteFunctionSpaceType::DimDomain };
      enum { m = DiscreteFunctionSpaceType::DimRange };

      std::fstream file( filename.c_str() , std::ios::out );
      GrapeIOStringType d = typeIdentifier<DomainFieldType>();
      GrapeIOStringType r = typeIdentifier<RangeFieldType>();

      file << "DomainField: " << d << std::endl;
      file << "RangeField: " << r << std::endl;
      file << "Dim_Domain: " << n << std::endl;
      file << "Dim_Range: " << m << std::endl;
      file << "Space: " << df.getFunctionSpace().type() << std::endl;
      file << "Format: " << ftype << std::endl;
      file << "Precision: " << precision << std::endl;
      file << "Polynom_order: " << df.getFunctionSpace().polynomOrder() << std::endl;
      file.close();
    }

    const char * path = "";
    GrapeIOStringType fn = genFilename(path,filename,timestep,precision);

    if(ftype == xdr)
      return df.write_xdr(fn);
    if(ftype == ascii)
      return df.write_ascii(fn);
    if(ftype == pgm)
      return df.write_pgm(fn);

    return false;
  }

  template <class GridType>
  template <class DiscreteFunctionType>
  inline bool GrapeDataIO<GridType> ::
  readData(DiscreteFunctionType & df, const GrapeIOStringType filename, int timestep)
  {
    typedef typename DiscreteFunctionType::FunctionSpaceType DiscreteFunctionSpaceType;
    typedef typename DiscreteFunctionSpaceType::DomainFieldType DomainFieldType;
    typedef typename DiscreteFunctionSpaceType::RangeFieldType RangeFieldType;

    enum { tn = DiscreteFunctionSpaceType::DimDomain };
    enum { tm = DiscreteFunctionSpaceType::DimRange };

    int n,m;
    GrapeIOStringType r,d;
    GrapeIOStringType tr (typeIdentifier<RangeFieldType>());
    GrapeIOStringType td (typeIdentifier<DomainFieldType>());

    readParameter(filename,"DomainField",d,false);
    readParameter(filename,"RangeField",r,false);
    readParameter(filename,"Dim_Domain",n,false);
    readParameter(filename,"Dim_Range",m,false);
    int space;
    readParameter(filename,"Space",space,false);
    int filetype;
    readParameter(filename,"Format",filetype,false);
    GrapeIOFileFormatType ftype = static_cast<GrapeIOFileFormatType> (filetype);
    int precision;
    readParameter(filename,"Precision",precision,false);

    if((d != td) || (r != tr) || (n != tn) || (m != tm) )
    {
      std::cerr << d << " | " << td << " DomainField in read!\n";
      std::cerr << r << " | " << tr << " RangeField  in read!\n";
      std::cerr << n << " | " << tn << " in read!\n";
      std::cerr << m << " | " << tm << " in read!\n";
      std::cerr << "Can not initialize DiscreteFunction with wrong FunctionSpace! \n";
      abort();
    }

    const char * path = "";
    GrapeIOStringType fn = genFilename(path,filename,timestep,precision);

    if(ftype == xdr)
      return df.read_xdr(fn);
    if(ftype == ascii)
      return df.read_ascii(fn);
    if(ftype == pgm)
      return df.read_pgm(fn);

    std::cerr << ftype << " GrapeIOFileFormatType not supported at the moment! in file " << __FILE__ << " line " << __LINE__ << "\n";
    abort();

    return false;
  }


} // end namespace

#endif
