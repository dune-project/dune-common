// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_VTKWRITER_HH
#define DUNE_VTKWRITER_HH

#include <iostream>
#include <vector>
#include <list>
#include <dune/common/exceptions.hh>
#include "dune/grid/common/mcmgmapper.hh"
#include "dune/grid/common/referenceelements.hh"

/** @file
        @author Peter Bastian
        @brief Provides file i/o for the visualization toolkit
 */

/**
        put vtk io intro here ...
 */


namespace Dune
{
  //! options for VTK output
  struct VTKOptions
  {
    enum Type {
      ascii, binary, conforming, nonconforming
    };
  };


  // map type to name in data array
  template<class T>
  struct VTKTypeNameTraits {
    std::string operator () (){
      return "";
    }
  };

  template<>
  struct VTKTypeNameTraits<char> {
    std::string operator () () {
      return "Int8";
    }
    typedef int PrintType;
  };

  template<>
  struct VTKTypeNameTraits<unsigned char> {
    std::string operator () () {
      return "UInt8";
    }
    typedef int PrintType;
  };

  template<>
  struct VTKTypeNameTraits<short> {
    std::string operator () () {
      return "Int16";
    }
    typedef short PrintType;
  };

  template<>
  struct VTKTypeNameTraits<unsigned short> {
    std::string operator () () {
      return "UInt16";
    }
    typedef unsigned short PrintType;
  };

  template<>
  struct VTKTypeNameTraits<int> {
    std::string operator () () {
      return "Int32";
    }
    typedef int PrintType;
  };

  template<>
  struct VTKTypeNameTraits<unsigned int> {
    std::string operator () () {
      return "UInt32";
    }
    typedef unsigned int PrintType;
  };

  template<>
  struct VTKTypeNameTraits<float> {
    std::string operator () () {
      return "Float32";
    }
    typedef float PrintType;
  };

  template<>
  struct VTKTypeNameTraits<double> {
    std::string operator () () {
      return "Float64";
    }
    typedef double PrintType;
  };


  /** @brief A class providing vtk file i/o
   */
  template<class GridImp>
  class VTKWriter {
    class VTKFunction;

    // extract types
    enum {n=GridImp::dimension};
    enum {w=GridImp::dimensionworld};
    typedef typename GridImp::ctype DT;
    typedef typename GridImp::Traits::template Codim<0>::Entity Entity;
    typedef typename GridImp::Traits::template Codim<0>::Entity Cell;
    typedef typename GridImp::Traits::template Codim<0>::LeafIterator CellIterator;
    typedef typename GridImp::Traits::template Codim<n>::Entity Vertex;
    typedef typename GridImp::Traits::template Codim<n>::LeafIterator VertexIterator;
    typedef typename std::list<VTKFunction*>::iterator functioniterator;

    template<int dim>
    struct P1Layout
    {
      bool contains (int codim, Dune::GeometryType gt)
      {
        if (codim==dim) return true;
        return false;
      }
    };
    typedef typename GridImp::template Codim<0>::LeafIndexSet IS;
    typedef MultipleCodimMultipleGeomTypeMapper<GridImp,IS,P1Layout> VM;

  public:
    //! constructor from a grid
    VTKWriter (const GridImp& g) : grid(g)
    {
      indentCount = 0;
      numPerLine = 4*3;     //should be a multiple of 3 !
    }

    //! add a grid function for output
    template<class RT, int m>
    void addCellData (const GridFunction<GridImp,RT,m>& f, std::string name)
    {
      VTKFunction* p = new GridFunctionWrapper<RT,m>(f,name);
      celldata.push_back(p);
    }

    //! add a grid function for output
    template<class RT, int m>
    void addVertexData (const GridFunction<GridImp,RT,m>& f, std::string name)
    {
      VTKFunction* p = new GridFunctionWrapper<RT,m>(f,name);
      vertexdata.push_back(p);
    }

    //! clear list of registered functions
    void clear ()
    {
      typedef typename std::list<VTKFunction*>::iterator iterator;
      for (iterator it=celldata.begin(); it!=celldata.end(); ++it)
        delete *it;
      celldata.clear();
      for (iterator it=vertexdata.begin(); it!=vertexdata.end(); ++it)
        delete *it;
      vertexdata.clear();
    }

    //! destructor
    ~VTKWriter ()
    {
      this->clear();
    }

    //! write output; interface might change later
    void write (const char* name, VTKOptions::Type dm = VTKOptions::ascii)
    {
      // make data mode visible to private functions
      datamode=dm;

      if (grid.comm().size()==1)
      {
        std::ofstream file;
        char fullname[128];
        sprintf(fullname,"%s.vtu",name);
        file.open(fullname);
        writeDataFile(file);
        file.close();
      }
      else
      {
        std::ofstream file;
        char fullname[128];
        sprintf(fullname,"%s-%04d-%04d.vtu",name,grid.comm().size(),grid.comm().rank());
        file.open(fullname);
        writeDataFile(file);
        file.close();
        grid.comm().barrier();
        if (grid.comm().rank()==0)
        {
          sprintf(fullname,"%s-%04d.pvtu",name,grid.comm().size());
          file.open(fullname);
          writeParallelHeader(file,name);
          file.close();
        }
        grid.comm().barrier();
      }
    }

  private:

    //! write header file in parallel case to stream
    void writeParallelHeader (std::ostream& s, const char* name)
    {
      // xml header
      s << "<?xml version=\"1.0\"?>" << std::endl;

      // VTKFile
      s << "<VTKFile type=\"PUnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\">" << std::endl;
      indentUp();

      // PUnstructuredGrid
      indent(s); s << "<PUnstructuredGrid GhostLevel=\"0\">" << std::endl;
      indentUp();

      // PPointData
      indent(s); s << "<PPointData ";
      for (functioniterator it=vertexdata.begin(); it!=vertexdata.end(); ++it)
        if ((*it)->ncomps()==1)
        {
          s << "Scalars=\"" << (*it)->name() << "\"" ;
          break;
        }
      for (functioniterator it=vertexdata.begin(); it!=vertexdata.end(); ++it)
        if ((*it)->ncomps()>1)
        {
          s << "Vectors=\"" << (*it)->name() << "\"" ;
          break;
        }
      s << ">" << std::endl;
      indentUp();
      for (functioniterator it=vertexdata.begin(); it!=vertexdata.end(); ++it)
      {
        indent(s); s << "<PDataArray type=\"Float32\" Name=\"" << (*it)->name() << "\" ";
        if ((*it)->ncomps()>1)
          s << "NumberOfComponents=\"" << (*it)->ncomps() << "\" ";
        if (datamode==VTKOptions::ascii)
          s << "format=\"ascii\"/>" << std::endl;
        else
          s << "format=\"binary\"/>" << std::endl;
      }
      indentDown();
      indent(s); s << "</PPointData>" << std::endl;

      // PCellData
      indent(s); s << "<PCellData ";
      for (functioniterator it=celldata.begin(); it!=celldata.end(); ++it)
        if ((*it)->ncomps()==1)
        {
          s << "Scalars=\"" << (*it)->name() << "\"" ;
          break;
        }
      for (functioniterator it=celldata.begin(); it!=celldata.end(); ++it)
        if ((*it)->ncomps()>1)
        {
          s << "Vectors=\"" << (*it)->name() << "\"" ;
          break;
        }
      s << ">" << std::endl;
      indentUp();
      for (functioniterator it=celldata.begin(); it!=celldata.end(); ++it)
      {
        indent(s); s << "<PDataArray type=\"Float32\" Name=\"" << (*it)->name() << "\" ";
        if ((*it)->ncomps()>1)
          s << "NumberOfComponents=\"" << (*it)->ncomps() << "\" ";
        if (datamode==VTKOptions::ascii)
          s << "format=\"ascii\"/>" << std::endl;
        else
          s << "format=\"binary\"/>" << std::endl;
      }
      indentDown();
      indent(s); s << "</PCellData>" << std::endl;

      // PPoints
      indent(s); s << "<PPoints>" << std::endl;
      indentUp();
      indent(s); s << "<DataArray type=\"Float32\" Name=\"Coordinates\" NumberOfComponents=\"" << "3" << "\" ";
      if (datamode==VTKOptions::ascii)
        s << "format=\"ascii\"/>" << std::endl;
      else
        s << "format=\"binary\"/>" << std::endl;
      indentDown();
      indent(s); s << "</PPoints>" << std::endl;

      // Pieces
      for (int i=0; i<grid.comm().size(); i++)
      {
        char fullname[128];
        sprintf(fullname,"%s-%04d-%04d.vtu",name,grid.comm().size(),i);
        indent(s); s << "<Piece Source=\"" << fullname << "\"/>" << std::endl;
      }

      // /PUnstructuredGrid
      indentDown();
      indent(s); s << "</PUnstructuredGrid>" << std::endl;

      // /VTKFile
      indentDown();
      s << "</VTKFile>" << std::endl;

    }


    //! write data file to stream
    void writeDataFile (std::ostream& s)
    {
      // xml header
      s << "<?xml version=\"1.0\"?>" << std::endl;

      // VTKFile
      s << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\">" << std::endl;
      indentUp();

      // UnstructuredGrid
      indent(s); s << "<UnstructuredGrid>" << std::endl;
      indentUp();

      // Piece
      vertexmapper = new VM(grid,grid.leafIndexSet());
      number.resize(vertexmapper->size());
      for (int i=0; i<number.size(); i++) number[i] = -1;
      nvertices = 0;

      ncells = 0;
      for (CellIterator it=grid.template leafbegin<0>(); it!=grid.template leafend<0>(); ++it)
        if (it->partitionType()==InteriorEntity)
        {
          ncells++;
          for (int i=0; i<it->template count<n>(); ++i)
          {
            int alpha = vertexmapper->template map<n>(*it,i);
            if (number[alpha]<0)
              number[alpha] = nvertices++;
          }
        }
      indent(s); s << "<Piece NumberOfPoints=\"" << nvertices << "\" NumberOfCells=\"" << ncells << "\">" << std::endl;
      indentUp();

      // PointData
      writeVertexDataConforming(s);

      // CellData
      writeCellData(s);

      // Points
      writePointsConforming(s);

      // Cells
      writeCellsConforming(s);

      // /Piece
      indentDown();
      indent(s); s << "</Piece>" << std::endl;
      delete vertexmapper; number.clear();

      // /UnstructuredGrid
      indentDown();
      indent(s); s << "</UnstructuredGrid>" << std::endl;

      // /VTKFile
      indentDown();
      s << "</VTKFile>" << std::endl;
    }

    void writeCellData (std::ostream& s)
    {
      indent(s); s << "<CellData ";
      for (functioniterator it=celldata.begin(); it!=celldata.end(); ++it)
        if ((*it)->ncomps()==1)
        {
          s << "Scalars=\"" << (*it)->name() << "\"" ;
          break;
        }
      for (functioniterator it=celldata.begin(); it!=celldata.end(); ++it)
        if ((*it)->ncomps()>1)
        {
          s << "Vectors=\"" << (*it)->name() << "\"" ;
          break;
        }
      s << ">" << std::endl;
      indentUp();
      for (functioniterator it=celldata.begin(); it!=celldata.end(); ++it)
      {
        VTKAsciiDataArrayWriter<float> *p = new VTKAsciiDataArrayWriter<float>(s,(*it)->name(),(*it)->ncomps());
        for (CellIterator i=grid.template leafbegin<0>(); i!=grid.template leafend<0>(); ++i)
          if (i->partitionType()==InteriorEntity)
            for (int j=0; j<(*it)->ncomps(); j++)
              p->write((*it)->evaluate(j,*i,ReferenceElements<DT,n>::general(i->geometry().type()).position(0,0)));
        delete p;
      }
      indentDown();
      indent(s); s << "</CellData>" << std::endl;
    }

    void writeVertexDataConforming (std::ostream& s)
    {
      indent(s); s << "<PointData ";
      for (functioniterator it=vertexdata.begin(); it!=vertexdata.end(); ++it)
        if ((*it)->ncomps()==1)
        {
          s << "Scalars=\"" << (*it)->name() << "\"" ;
          break;
        }
      for (functioniterator it=vertexdata.begin(); it!=vertexdata.end(); ++it)
        if ((*it)->ncomps()>1)
        {
          s << "Vectors=\"" << (*it)->name() << "\"" ;
          break;
        }
      s << ">" << std::endl;
      indentUp();
      for (functioniterator it=vertexdata.begin(); it!=vertexdata.end(); ++it)
      {
        VTKAsciiDataArrayWriter<float> *p = new VTKAsciiDataArrayWriter<float>(s,(*it)->name(),(*it)->ncomps());
        std::vector<bool> visited(vertexmapper->size());
        for (int i=0; i<visited.size(); i++) visited[i] = false;
        for (CellIterator eit=grid.template leafbegin<0>(); eit!=grid.template leafend<0>(); ++eit)
          if (eit->partitionType()==InteriorEntity)
            for (int i=0; i<eit->template count<n>(); ++i)
            {
              int alpha = vertexmapper->template map<n>(*eit,i);
              if (!visited[alpha])
              {
                for (int j=0; j<(*it)->ncomps(); j++)
                  p->write((*it)->evaluate(j,*eit,ReferenceElements<DT,n>::general(eit->geometry().type()).position(i,n)));
                visited[alpha] = true;
              }
            }
        delete p;
      }
      indentDown();
      indent(s); s << "</PointData>" << std::endl;
    }

    void writePointsConforming (std::ostream& s)
    {
      indent(s); s << "<Points>" << std::endl;
      indentUp();

      VTKAsciiDataArrayWriter<float> *p = new VTKAsciiDataArrayWriter<float>(s,"Coordinates",3);
      std::vector<bool> visited(vertexmapper->size());
      for (int i=0; i<visited.size(); i++) visited[i] = false;
      for (CellIterator it=grid.template leafbegin<0>(); it!=grid.template leafend<0>(); ++it)
        if (it->partitionType()==InteriorEntity)
          for (int i=0; i<it->template count<n>(); ++i)
          {
            int alpha = vertexmapper->template map<n>(*it,i);
            if (!visited[alpha])
            {
              int dimw=w;
              for (int j=0; j<std::min(dimw,3); j++)
                p->write(it->geometry()[i][j]);
              for (int j=std::min(dimw,3); j<3; j++)
                p->write(0.0);
              visited[alpha] = true;
            }
          }
      delete p;

      indentDown();
      indent(s); s << "</Points>" << std::endl;
    }

    void writeCellsConforming (std::ostream& s)
    {
      indent(s); s << "<Cells>" << std::endl;
      indentUp();

      // connectivity
      VTKAsciiDataArrayWriter<int> *p1 = new VTKAsciiDataArrayWriter<int>(s,"connectivity",1);
      for (CellIterator it=grid.template leafbegin<0>(); it!=grid.template leafend<0>(); ++it)
        if (it->partitionType()==InteriorEntity)
          for (int i=0; i<it->template count<n>(); ++i)
            p1->write(number[vertexmapper->template map<n>(*it,renumber(*it,i))]);
      delete p1;

      // offsets
      VTKAsciiDataArrayWriter<int> *p2 = new VTKAsciiDataArrayWriter<int>(s,"offsets",1);
      int offset = 0;
      for (CellIterator it=grid.template leafbegin<0>(); it!=grid.template leafend<0>(); ++it)
        if (it->partitionType()==InteriorEntity)
        {
          offset += it->template count<n>();
          p2->write(offset);
        }
      delete p2;

      // types
      VTKAsciiDataArrayWriter<unsigned char> *p3 = new VTKAsciiDataArrayWriter<unsigned char>(s,"types",1);
      for (CellIterator it=grid.template leafbegin<0>(); it!=grid.template leafend<0>(); ++it)
        if (it->partitionType()==InteriorEntity)
        {
          unsigned char vtktype=3;
          if (n==1)
            vtktype=3;
          if (n==2)
          {
            if (it->geometry().type()==simplex)
              vtktype=5;
            if (it->geometry().type()==cube)
              vtktype=9;
          }
          if (n==3)
          {
            if (it->geometry().type()==simplex)
              vtktype=10;
            if (it->geometry().type()==pyramid)
              vtktype=14;
            if (it->geometry().type()==prism)
              vtktype=13;
            if (it->geometry().type()==cube)
              vtktype=12;
          }
          p3->write(vtktype);
        }
      delete p3;

      indentDown();
      indent(s); s << "</Cells>" << std::endl;
    }

    // a streaming writer for data array tags
    template<class T>
    class VTKAsciiDataArrayWriter {
    public:
      //! make a new data array writer
      VTKAsciiDataArrayWriter (std::ostream& theStream, std::string name, int ncomps)
        : s(theStream), counter(0), numPerLine(12)
      {
        VTKTypeNameTraits<T> tn;
        s << "<DataArray type=\"" << tn() << "\" Name=\"" << name << "\" ";
        if (ncomps>1)
          s << "NumberOfComponents=\"" << ncomps << "\" ";
        s << "format=\"ascii\">" << std::endl;
      }

      //! write one data element to output stream
      void write (T data)
      {
        typedef typename VTKTypeNameTraits<T>::PrintType PT;
        s << (PT) data << " ";
        counter++;
        if (counter%numPerLine==0) s << std::endl;
      }

      //! finish output; writes end tag
      ~VTKAsciiDataArrayWriter ()
      {
        if (counter%numPerLine!=0) s << std::endl;
        s << "</DataArray>" << std::endl;
      }

    private:
      std::ostream& s;
      int counter;
      int numPerLine;
    };

    // a streaming writer for data array tags
    template<class T>
    class VTKBinaryDataArrayWriter {
    public:
      //! make a new data array writer
      VTKBinaryDataArrayWriter (std::ostream& theStream, std::string name, int ncomps)
        : s(theStream)
      {
        VTKTypeNameTraits<T> tn;
        s << "<DataArray type=\"" << tn() << "\" Name=\"" << name << "\" ";
        if (ncomps>1)
          s << "NumberOfComponents=\"" << ncomps << "\" ";
        s << "format=\"ascii\">" << std::endl;
      }

      //! write one data element to output stream
      void write (T data)
      {
        typedef typename VTKTypeNameTraits<T>::PrintType PT;
        s << (PT) data << " ";
      }

      //! finish output; writes end tag
      ~VTKBinaryDataArrayWriter ()
      {
        s << "</DataArray>" << std::endl;
      }

    private:
      std::ostream& s;
    };

    // A base class for grid functions with any return type and dimension
    // Trick : use double as return type
    class VTKFunction
    {
    public:
      //! return number of components
      virtual int ncomps () const = 0;

      //! evaluate single component comp in the entity e at local coordinates xi
      /*! Evaluate the function in an entity at local coordinates.
            @param[in]  comp   number of component to be evaluated
            @param[in]  e      reference to grid entity of codimension 0
            @param[in]  xi     point in local coordinates of the reference element of e
            \return            value of the component
       */
      virtual double evaluate (int comp, const Entity& e, const Dune::FieldVector<DT,n>& xi) const = 0;

      // get name
      virtual std::string name () const = 0;

      // non virtual destructor
      ~VTKFunction () {}
    };

    //!
    template<class RT, int m>
    class GridFunctionWrapper : public VTKFunction
    {
    public:
      //! return number of components
      virtual int ncomps () const
      {
        return m;
      }

      //! evaluate single component comp in the entity e at local coordinates xi
      /*! Evaluate the function in an entity at local coordinates.
            @param[in]  comp   number of component to be evaluated
            @param[in]  e      reference to grid entity of codimension 0
            @param[in]  xi     point in local coordinates of the reference element of e
            \return            value of the component
       */
      virtual double evaluate (int comp, const Entity& e, const Dune::FieldVector<DT,n>& xi) const
      {
        return func.evallocal(comp,e,xi);
      }

      // get name
      virtual std::string name () const
      {
        return myname;
      }

      // constructor remembers reference to a grid function
      GridFunctionWrapper (const GridFunction<GridImp,RT,m>& f, std::string s) : func(f), myname(s)
      {}

    private:
      const GridFunction<GridImp,RT,m>& func;
      std::string myname;
    };

    void indentUp ()
    {
      indentCount++;
    }

    void indentDown ()
    {
      indentCount--;
    }

    void indent (std::ostream& s)
    {
      for (int i=0; i<indentCount; i++)
        s << "  ";
    }

    // renumber VTK -> Dune
    int renumber (const Entity& e, int i)
    {
      if (n==2)
      {
        if (e.geometry().type()==cube)
        {
          const int renumbering[4] = {0,1,3,2};
          return renumbering[i];
        }
        return i;
      }
      if (n==3)
      {
        if (e.geometry().type()==prism)
        {
          const int renumbering[6] = {0,2,1,3,5,4};
          return renumbering[i];
        }
        if (e.geometry().type()==cube)
        {
          const int renumbering[8] = {0,1,3,2,4,5,7,6};
          return renumbering[i];
        }
      }
      return i;
    }

    // the list of registered functions
    std::list<VTKFunction*> celldata;
    std::list<VTKFunction*> vertexdata;

    // the grid
    const GridImp& grid;

    // intend counter
    int indentCount;
    int numPerLine;

    // temporary grid information
    int ncells;
    int nvertices;
    VM* vertexmapper;
    std::vector<int> number;
    VTKOptions::Type datamode;
  };


}
#endif
