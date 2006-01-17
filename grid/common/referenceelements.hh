// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_REFERENCEELEMENTS_HH
#define DUNE_REFERENCEELEMENTS_HH

#include <iostream>
#include "dune/common/fvector.hh"
#include "dune/common/exceptions.hh"
#include "dune/grid/common/grid.hh"
#include "dune/common/misc.hh"

/**
 * @file
 * @brief  Definition of reference elements for all types and dims
 * @author Peter Bastian
 */

namespace Dune
{
  namespace {
    template <class RT, int codim>
    FieldVector<typename RT::CoordType, RT::d>
    mapGlobal(const RT& refElem,
              const FieldVector<typename RT::CoordType, RT::d-codim>& l,
              int i, int cdim)
    {
      typedef typename RT::CoordType CoordType;
      const int dim = RT::d;

      assert(cdim == codim);
      static FieldMatrix<CoordType, dim-codim, dim> mat;

      int zeroLocalIdx = refElem.subEntity(i, cdim, 0, dim);
      FieldVector<CoordType, dim> result =
        refElem.position(zeroLocalIdx, dim);
      for (int j = 0; j < dim-cdim; ++j) {
        int indexLocal = refElem.subEntity(i, cdim, j+1, dim);
        mat[j] = refElem.position(indexLocal, dim) - result;
      }

      mat.umtv(l, result);

      return result;
    }

  } // end unnamed namespace


  /**
     @defgroup GridReferenceElements Reference Elements
     @ingroup Grid
     @brief Classes describing the geometry of the reference elements.

     Every entity of a grid is obtained by a mapping from the some reference element. The topology
     of the reference elements needs only to be described once. The reference element describes
     how an entity is built up hierarchically from entities of lower codimension and what the types of these
     lower-dimensional entities are.

         Here is a graphical representation of the reference elements:
     -# <b>One-dimensional reference element. For d=1 the simplex and cube are identical</b>
       \image html line.png
     -# <b>Two-dimensional reference simplex (a.k.a. triangle)</b>
           \image html triangle.png
     -# <b>Three-dimensional reference simplex (a.k.a. tetrahedron)</b>
       <table class="imagelist"><tr>
           <td>\image html tetrahedron.png "Face Numbering"</td>
           <td>\image html tetrahedron_edges.png "Edge Numbering"</td>
       </tr></table>
     -# <b>Two-dimensional reference cube (a.k.a. quadrilateral)</b>
           \image html quadrilateral.png
     -# <b>Three-dimensional reference cube (a.k.a. hexahedron)</b>
       <table class="imagelist"><tr>
       <td>\image html hexahedron.png "Face Numbering"</td>
           <td>\image html hexahedron_edges.png "Edge Numbering"</td>
       </tr></table>
     -# <b>Prism reference element</b>
       <table class="imagelist"><tr>
           <td>\image html prism.png "Face Numbering"</td>
           <td>\image html prism_edges.png "Edge Numbering"</td>
       </tr></table>
     -# <b>Pyramid reference element</b>
       <table class="imagelist"><tr>
           <td>\image html pyramid.png "Face Numbering"</td>
           <td>\image html pyramid_edges.png "Edge Numbering"</td>
       </tr></table>

         Note that the cube reference elements are available for any dimension.
   */
  /**
      @addtogroup GridReferenceElements
      @{
   */

  /***********************************************************
  * Interface for reference elements
  ***********************************************************/

  //! This is the abstract base class for reference elements
  template<typename ctype, int dim>
  class ReferenceElement
  {
  public:

    // compile time sizes
    enum { d=dim };    // maps from R^d

    // export types
    typedef ctype CoordType;

    //! number of entities of codim c
    virtual int size (int c) const = 0;

    //! number of subentities of codim cc of entitity (i,c)
    virtual int size (int i, int c, int cc) const = 0;

    //! number of ii'th subentity with codim cc>c of (i,c)
    virtual int subEntity (int i, int c, int ii, int cc) const = 0;

    //! position of entity (i,c)
    virtual const FieldVector<ctype,dim>& position (int i, int c) const = 0;

    //! maps local coordinate on subentity i of codim cdim onto reference element coordinates
    template <int codim>
    FieldVector<ctype, dim> global(const FieldVector<ctype, dim-codim>& local, int i, int cdim) const
    {
      return mapGlobal<ReferenceElement<ctype, dim>, codim>(*this, local, i, cdim);
    }

    //! type of (i,c)
    virtual NewGeometryType type (int i, int c) const = 0;

    //! volume of the reference element
    virtual double volume () const = 0;

    //! virtual destructor
    virtual ~ReferenceElement ()
    {}
  };

  //! The wrapper allows Imp to be used as a reference element without making methods of Imp virtual
  template<typename Imp>
  class ReferenceElementWrapper :
    public ReferenceElement<typename Imp::CoordType,Imp::d>,
    private Imp
  {
  public:

    // compile time sizes
    enum { d=Imp::d };    // maps from R^d

    // export types
    typedef typename Imp::CoordType CoordType;

    //! number of entities of codim c
    int size (int c) const
    {
      return Imp::size(c);
    }

    //! number of subentities of codim cc of entitity (i,c)
    int size (int i, int c, int cc) const
    {
      return Imp::size(i,c,cc);
    }

    //! number of ii'th subentity with codim cc of (i,c)
    int subEntity (int i, int c, int ii, int cc) const
    {
      return Imp::subEntity(i,c,ii,cc);
    }

    //! position of entity (i,c)
    const FieldVector<CoordType,d>& position (int i, int c) const
    {
      return Imp::position(i,c);
    }

    //! maps local coordinate on subentity i of codim cdim onto reference element coordinates
    template <int codim>
    FieldVector<CoordType, d> global(const FieldVector<CoordType, d-codim>& l,
                                     int i, int cdim) const
    {
      return Imp::global(l, i, cdim);
    }

    //! type of (i,c)
    NewGeometryType type (int i, int c) const
    {
      return Imp::type(i,c);
    }

    //! volume of the reference element
    double volume () const
    {
      return Imp::volume();
    }
  };

  // output operator for wrapped reference elements
  template<typename T>
  inline std::ostream& operator<< (std::ostream& s,
                                   const ReferenceElementWrapper<T>& r)
  {
    enum {dim=T::d};

    std::cout << "REFERENCE ELEMENT " << GeometryName(r.type(0,0))
    << " dimension=" << dim
    << " volume=" << r.volume()
    << std::endl;

    for (int c=0; c<=dim; c++)
    {
      std::cout << r.size(c) << " codim " << c << " entitie(s)" << std::endl;
      for (int i=0; i<r.size(c); i++)
      {
        std::cout << "  entity=" << i
        << " codim=" << c
        << " type=" << GeometryName(r.type(i,c))
        << " position=(" << r.position(i,c) << ")"
        << std::endl;

        for (int cc=c+1; cc<=dim; cc++)
        {
          std::cout << "    " << r.size(i,c,cc)
          << " subentities of codim " << cc
          << std::endl;

          for (int ii=0; ii<r.size(i,c,cc); ii++)
          {
            std::cout << "    index=" << ii
            << " subentity=" << r.subEntity(i,c,ii,cc)
            << " position=(" << r.position(r.subEntity(i,c,ii,cc),cc) << ")"
            << std::endl;
          }
        }
      }
    }

    return s;
  }


  /***********************************************************
  * The hypercube in any dimension
  ***********************************************************/


  //! This is the reference cube without virtual functions
  template<typename ctype, int dim>
  class ReferenceCube
  {
  public:
    enum { MAXE = Power_m_p<3,dim>::power }; // maximum number of entities per codim

    // compile time sizes
    enum { d=dim };    // maps from R^d

    // export types
    typedef ctype CoordType;

    //! build up the reference cube
    ReferenceCube ()
    {
      for (int i=0; i<=dim; ++i)
        sizes[i] = 0;
      for (int i=0; i<MAXE; ++i)
        for (int j=0; j<=dim; ++j)
          for (int k=0; k<=dim; ++k)
            if (j==k)
              subsizes[i][j][k] = 1;
            else
              subsizes[i][j][k] = 0;
      for (int i=0; i<MAXE; ++i)
        for (int j=0; j<=dim; ++j)
          hierarchy[i][j][0][j] = i;
      FieldVector<int,dim> direction;
      for (int c=dim; c>=0; --c)
        generate(0,c,direction);


    }

    //! number of entities of codim c
    int size (int c) const
    {
      return sizes[c];
    }

    //! number of subentities of codim cc of entitity (i,c)
    int size (int i, int c, int cc) const
    {
      return subsizes[i][c][cc];
    }

    //! number of ii'th subentity with codim cc of (i,c)
    int subEntity (int i, int c, int ii, int cc) const
    {
      return hierarchy[i][c][ii][cc];
    }

    //! position of entity (i,c)
    const FieldVector<ctype,dim>& position (int i, int c) const
    {
      return pos[i][c];
    }

    //! maps local coordinate on subentity i of codim cdim onto reference element coordinates
    template <int codim>
    FieldVector<ctype, dim> global(const FieldVector<ctype, dim-codim>& local, int i, int cdim) const
    {
      return mapGlobal<ReferenceCube<ctype, dim>, codim>(*this, local, i, cdim);
    }

    //! type of (i,c)
    NewGeometryType type (int i, int c) const
    {
      return NewGeometryType(NewGeometryType::cube, dim);
    }

    //! volume of the reference element
    double volume () const
    {
      return 1.0;
    }

    //! position of entity (i,c)
    const FieldVector<int,dim>& iposition (int i, int c) const
    {
      return ipos[i][c];
    }

  private:

    class IdMapper {
    public:
      int& operator() (const FieldVector<int,dim>& x)
      {
        int index=x[dim-1];
        for (int i=dim-2; i>=0; --i) index = 3*index+x[i];
        return id[index];
      }
    private:
      int id[1<<(2*dim)];
    };

    void generate (int k, int c, FieldVector<int,dim>& direction)
    {
      if (k<c)
      {
        // select kth direction
        for (int i=0; i<dim; ++i)
        {
          bool done=false;
          for (int j=0; j<k; ++j)
            if (i<=direction[j]) {
              done = true;
              break;
            }
          if (done) continue;
          direction[k] = i;     // new direction selected
          generate(k+1,c,direction);
        }
      }
      else
      {
        //                std::cout << "c=" << c
        //                                      << " directions=(" << direction << ")"
        //                                      << std::endl;

        // c directions have been selected
        // for each there are 2 choices, ie 2^c possibilities in total
        for (int b=0; b<(1<<c); ++b)
        {
          // make coordinate in dim-cube
          FieldVector<int,dim> x;
          for (int i=0; i<dim; ++i) x[i] = 1;
          for (int i=0; i<c; i++)
            if (((1<<i)&b)==0)
              x[direction[i]] = 0;
            else
              x[direction[i]] = 2;

          int entity = sizes[c];
          (sizes[c])++;
          if (sizes[c]>MAXE)
            DUNE_THROW(GridError, "MAXE in ReferenceCube exceeded");

          // print info
          //                      std::cout << " x=(" << x << ")->"
          //                                            << "id=" << entity
          //                                            << std::endl;

          // store id in map
          idmap(x) = entity;

          // assign position
          for (int i=0; i<dim; i++)
            pos[entity][c][i] = x[i]*0.5;

          // assign integer position
          ipos[entity][c] = x;

          // generate subentities
          for (int cc=c+1; cc<=dim; ++cc)
            generatesub(k,cc,direction,x,c);
        }
      }
    }

    void generatesub (int k, int cc, FieldVector<int,dim>& direction,
                      FieldVector<int,dim>& e, int c)
    {
      if (k<cc)
      {
        // select kth direction
        for (int i=0; i<dim; ++i)
        {
          bool done=false;
          for (int j=0; j<c; ++j)
            if (i==direction[j]) {
              done = true;
              break;
            }
          for (int j=c; j<k; ++j)
            if (i<=direction[j]) {
              done = true;
              break;
            }
          if (done) continue;
          direction[k] = i;     // new direction selected
          generatesub(k+1,cc,direction,e,c);
        }
      }
      else
      {
        //                std::cout << "  cc=" << cc
        //                                      << " directions=(" << direction << ")"
        //                                      << std::endl;

        // cc-c additional directions have been selected
        // for each there are 2 choices, ie 2^(cc-c) possibilities in total
        for (int b=0; b<(1<<(cc-c)); ++b)
        {
          // make coordinate in dim-cube
          FieldVector<int,dim> x(e);
          for (int i=0; i<(cc-c); i++)
            if (((1<<i)&b)==0)
              x[direction[i+c]] = 0;
            else
              x[direction[i+c]] = 2;

          int entity = idmap(e);
          int subentity = idmap(x);
          int index = subsizes[entity][c][cc];
          (subsizes[entity][c][cc])++;
          if (subsizes[entity][c][cc]>MAXE)
            DUNE_THROW(GridError, "MAXE in ReferenceCube exceeded");

          // print info
          //                      std::cout << "  (" << e << ")," << entity
          //                                            << " has subentity (" << x << ")," << subentity
          //                                            << " at index=" << index
          //                                            << std::endl;

          // store id
          hierarchy[entity][c][index][cc] = subentity;
        }
      }
    }

    IdMapper idmap;
    int sizes[dim+1];
    int subsizes[MAXE][dim+1][dim+1];
    int hierarchy[MAXE][dim+1][MAXE][dim+1];
    FieldVector<ctype,dim> pos[MAXE][dim+1];
    FieldVector<int,dim> ipos[MAXE][dim+1];
  };


  //! Make the reference cube accessible as a container
  template<typename ctype, int dim>
  class ReferenceCubeContainer
  {
  public:

    //! export type elements in the container
    typedef ReferenceCube<ctype,dim> value_type;

    //! return element of the container via geometry type
    const value_type& operator() (NewGeometryType type) const
    {
      if ( type.isCube() )
        return cube_;
      DUNE_THROW(RangeError, "expected a cube!");
    }

  private:
    ReferenceCube<ctype,dim> cube_;
  };




  /***********************************************************
  * the simplex in any dimension (line,triangle,tetrahedron)
  ***********************************************************/


  /*

     see the reference elements:

     y
   | 2(0,1)
   | |\
   | | \
   | |  \
   | |   \
   | |    \
   | |_ _ _\
   |  0(0,0)  1(1,0)
   ||_ _ _ _ _ _ _ _
     x


     linear triangular
     area=1/2;
     ---------------------


     3 (0,0,1)

   ||`
   |\ `
   | \  `
   |  \   `
   |   \  .' 2 (0,1,0)
   |    .' |
   |  .' \ |
   ||.'_ _ \|

     0       1
     (0,0,0,)  (1,0,0)

     linear tetrahedron
     volume = 1/6;
     --------------------
   */


  //reference simplex without virtual functions
  //! Reference simplex
  template<typename ctype, int dim>
  class ReferenceSimplex
  {
  public:
    enum {MAXE = (((dim+1)<<1)-(dim+1))+(2*((dim-1)>>1))}; // 1D=2; 2D=3; in 3D: 6 edges for a tetrahedron
    enum {d=dim};


    typedef ctype CoordType;

    ReferenceSimplex ()
    {
      for (int i=0; i<=dim; ++i)
        sizes[i]=0;
      for (int i=0; i<MAXE; ++i)
        for (int j=0; j<=dim; ++j)
          for (int k=0; k<=dim; ++k)
            if (j==k)
              subsizes[i][j][k] = 1;
            else
              subsizes[i][j][k] = 0;
      for (int i=0; i<MAXE; ++i)
        for (int j=0; j<=dim; ++j)
          subentityindex[i][j][0][j] = i;

      for (int c=dim; c>=0; --c)
        entity_details (c);
    }



    //! number of entities of codim c
    int size (int c) const
    {
      //entity_size(c);
      return sizes[c];
    }
    //! number of subentities of codim cc of entitity (i,c)
    int size (int i, int c, int cc) const
    {
      return subsizes[i][c][cc];
    }
    //! number of ii'th subentity with codim cc of (i,c)
    int subEntity (int i, int c, int ii, int cc) const
    {
      return subentityindex[i][c][ii][cc];
    }

    //! position of entity (i,c)
    const FieldVector<ctype,dim>& position (int i, int c) const
    {
      return pos[i][c];
    }

    //! maps local coordinate on subentity i of codim cdim onto reference element coordinates
    template <int codim>
    FieldVector<ctype, dim> global(const FieldVector<ctype, dim-codim>& local, int i, int cdim) const
    {
      return mapGlobal<ReferenceSimplex<ctype, dim>, codim>(*this, local, i, cdim);
    }

    //! type of (i,c)
    NewGeometryType type (int i, int c) const
    {
      return NewGeometryType(NewGeometryType::simplex,dim);
    }


    //! volume of the reference element

    double volume () const
    {
      if (dim==2) return 0.5;
      return 1.0/6.0;
    }
    //! position of entity (i,c)
    //   const FieldVector<int,dim>& iposition (int i, int c) const
    //     {
    //       //return ipos[i][c];
    //     }
  private:

    //details of entities and subentities
    void entity_details(int c) //(int c)
    {

      sizes[dim]=dim+1; // simplex definition

      // position of vertices, there are dim+1 vertices
      FieldVector<int,dim> x;
      x=0;
      // vertex is codim=dim entity
      for (int n=0; n<dim; n++)
      {
        pos[0][dim][n]=x[n];

      }
      for(int k=1; k<=dim; ++k)
      {
        for (int j=0; j<dim; ++j)
        {
          x[j]=0;
          x[k-1]=1;
          pos[k][dim][j]= x[j];

        }
      }

      // position of centre of gravity of the element
      // codim=0 for element or cell
      sizes[0]=1; // only 1 cell !!
      int node;
      for(int k=0; k<dim; ++k)
      { node=0;
        pos[sizes[0]-1][0][k]=(pos[0][dim][k])/sizes[dim];
        subentityindex[sizes[0]-1][0][0][dim]=0;
        node+=1;
        for (int j=1; j<sizes[dim]; ++j)
        {
          pos[sizes[0]-1][0][k]+=(pos[j][dim][k])/(sizes[dim]);
          subentityindex[sizes[0]-1][0][j][dim]=j;
          node+=1;
        }}
      subsizes[sizes[0]-1][0][dim]=node;

      // the simplex itself has one simplex ;)
      subsizes[0][0][0]=1;

      // every vertex entity has one sub entity --> itself
      for (int k=0; k<MAXE; ++k) subsizes[k][dim][dim]=1;

      //++++++++++++++++
      if(dim==1) // line
      {
        // node indices on element
        for(int i=0; i<subsizes[0][0][dim]; ++i)
          subentityindex[0][0][i][dim]=i;
      }
      else if(dim==2)     // triangle
      {
        sizes[dim-1]=3;           // edge

        // hard coding the number of subentities
        // triangle has 3 vertices, 3 edges
        subsizes[0][0][dim]=3;
        subsizes[0][0][dim-1]=3;
        // triangle  has 2 vertices on each  edge
        for (int k=0; k<3; ++k) {
          subsizes[k][1][dim]=2;
          // triangle  has 1 edge on each edge ;-)
          subsizes[k][1][1]=1;
        }
        // subentity indices
        // node indices on element
        for(int i=0; i<subsizes[0][0][dim]; ++i)
          subentityindex[0][0][i][dim]=i;
        // edge indices on element
        for(int i=0; i<subsizes[0][0][dim-1]; ++i)
          subentityindex[0][0][i][1]=i;

        // node indices on edge 0
        subentityindex[0][1][0][dim]=1;
        subentityindex[0][1][1][dim]=2;
        // node indices on edge 1
        subentityindex[1][1][0][dim]=2;
        subentityindex[1][1][1][dim]=0;
        // node indices on edge 2
        // 2 = (subsizes[0][0][dim-1])-1  work around icc warning
        subentityindex[(subsizes[0][0][dim-1])-1][dim-1][0][dim]=0;
        subentityindex[(subsizes[0][0][dim-1])-1][dim-1][1][dim]=1;

        for(int j=0; j<dim; ++j)
        {

          //edge 0 (nodes 1,2)
          // 2 = (subsizes[0][0][dim])-1 work around icc warning
          pos[0][1][j]=(pos[1][dim][j]+pos[(subsizes[0][0][dim])-1][dim][j])/2.0;
          //edge 1 (nodes 0,2)
          pos[1][1][j]=(pos[0][dim][j]+pos[(subsizes[0][0][dim])-1][dim][j])/2.0;
          //edge 2 (nodes 0,1)
          pos[(subsizes[0][0][dim-1])-1][1][j]=(pos[0][dim][j]+pos[1][2][j])/2.0;

          // //edge 0 (nodes 1,2)
          //               pos[0][1][j]=(pos[1][dim][j]+pos[2][dim][j])/2.0;
          //               //edge 1 (nodes 0,2)
          //               pos[1][1][j]=(pos[0][dim][j]+pos[2][dim][j])/2.0;
          //               //edge 2 (nodes 0,1)
          //               pos[(subsizes[0][0][dim-1])-1][1][j]=(pos[0][dim][j]+pos[1][2][j])/2.0;
        }
      }
      else if(dim==3) // tetrahedron
      {
        sizes[1]=4;   // face
        sizes[dim-1]=6;   // edge

        // hard coding the number of subentities
        // tetrahedron has 4 vertices, 6 edges and 4 facese on element
        subsizes[0][0][dim]=4;
        subsizes[0][0][dim-1]=6;
        subsizes[0][0][1]=4;
        //  tetrahedron has 3 vertices on each triang. face
        for(int i=0; i<subsizes[0][0][1]; ++i)
          subsizes[i][1][dim]=3;
        //  tetrahedron has 3 edges on each triang. face
        for(int i=0; i<subsizes[0][0][1]; ++i)
          subsizes[i][1][dim-1]=3;
        //  tetrahedron has 1 face on each triang. face!
        for(int i=0; i<subsizes[0][0][1]; ++i)
          subsizes[i][1][1]=1;

        //  tetrahedron has 2 vertices on each edge
        //  we have 6 edges, an we know that
        for (int k=0; k<6; ++k)
          subsizes[k][dim-1][dim]=2;

        //  tetrahedron has 1 edge on each edge!
        //  we have 6 edges, an we know that
        for (int k=0; k<6; ++k)
          subsizes[k][dim-1][dim-1]=1;

        // subentity indices
        // node indices on element
        for(int i=0; i<subsizes[0][0][dim]; ++i)
          subentityindex[0][0][i][dim]=i;
        // edge indices on element
        for(int i=0; i<subsizes[0][0][dim-1]; ++i)
          subentityindex[0][0][i][dim-1]=i;
        // face indices on element
        for(int i=0; i<subsizes[0][0][1]; ++i)
          subentityindex[0][0][i][1]=i;

        // node numbering- normal pointing outward
        // reference triangle in dune has a counter-clockwise
        // numbering. so  nodes on each triangle face in tetrahedron numbered same way
        int faceindx;
        int edgeindx;
        // node indices on face 0
        subentityindex[0][1][0][dim]=1;
        subentityindex[0][1][1][dim]=2;
        subentityindex[0][1][(subsizes[0][1][dim]-1)][dim]=3;
        // node indices on face 1
        subentityindex[1][1][0][dim]=0;
        subentityindex[1][1][1][dim]=3;
        subentityindex[1][1][(subsizes[1][1][dim]-1)][dim]=2;
        // node indices on face 2
        faceindx= subsizes[0][0][1]-2;
        subentityindex[faceindx][1][0][dim]=0;
        subentityindex[faceindx][1][1][dim]=1;
        subentityindex[faceindx][1][(subsizes[faceindx][1][dim]-1)][dim]=3;
        // node indices on face 3
        faceindx= subsizes[0][0][1]-1;
        subentityindex[faceindx][1][0][dim]=0;
        subentityindex[faceindx][1][1][dim]=2;
        subentityindex[faceindx][1][(subsizes[faceindx][1][dim]-1)][dim]=1;

        // edge indices on face 0
        subentityindex[0][1][0][dim-1]=5;
        subentityindex[0][1][1][dim-1]=4;

        edgeindx=subsizes[0][1][dim-1]-1;
        subentityindex[0][1][edgeindx][dim-1]=1;
        // edge indices on face 1
        subentityindex[1][1][0][dim-1]=5;
        subentityindex[1][1][1][dim-1]=2;

        edgeindx=subsizes[1][1][dim-1]-1;
        subentityindex[1][1][edgeindx][dim-1]=3;
        // edge indices on face 2
        faceindx= subsizes[0][0][1]-2;
        subentityindex[faceindx][1][0][dim-1]=4;
        subentityindex[faceindx][1][1][dim-1]=3;

        edgeindx=subsizes[faceindx][1][dim-1]-1;
        subentityindex[faceindx][1][edgeindx][dim-1]=0;
        // edge indices on face 3
        faceindx= subsizes[0][0][1]-1;
        subentityindex[faceindx][1][0][dim-1]=1;
        subentityindex[faceindx][1][1][dim-1]=0;

        edgeindx=subsizes[faceindx][1][dim-1]-1;
        subentityindex[faceindx][1][edgeindx][dim-1]=2;

        // node indices on edge 0
        subentityindex[0][dim-1][0][dim]=0;
        subentityindex[0][dim-1][1][dim]=1;
        // node indices on edge 1
        subentityindex[1][dim-1][0][dim]=1;
        subentityindex[1][dim-1][1][dim]=2;
        // node indices on edge 2

        subentityindex[(subsizes[0][0][dim-1]-4)][dim-1][0][dim]=0;
        subentityindex[(subsizes[0][0][dim-1]-4)][dim-1][1][dim]=2;
        // node indices on edge 3
        subentityindex[(subsizes[0][0][dim-1]-3)][dim-1][0][dim]=0;
        subentityindex[(subsizes[0][0][dim-1]-3)][dim-1][1][dim]=3;
        // node indices on edge 4
        subentityindex[(subsizes[0][0][dim-1]-2)][dim-1][0][dim]=1;
        subentityindex[(subsizes[0][0][dim-1]-2)][dim-1][1][dim]=3;
        // node indices on edge 5
        subentityindex[(subsizes[0][0][dim-1]-1)][dim-1][0][dim]=2;
        subentityindex[(subsizes[0][0][dim-1]-1)][dim-1][1][dim]=3;

        for(int j=0; j<dim; ++j)
        {

          //face 0 (nodes 1,2,3)
          pos[0][1][j]=(pos[1][dim][j]+pos[2][dim][j]+pos[3][dim][j])/3.0;
          //face 1 (nodes 0,2,3)
          pos[1][1][j]=(pos[0][dim][j]+pos[2][dim][j]+pos[3][dim][j])/3.0;
          //face 2 (nodes 0,1,3)
          pos[2][1][j]=(pos[0][dim][j]+pos[1][dim][j]+pos[3][dim][j])/3.0;
          //face 3 (nodes 0,1,2)
          pos[3][1][j]=(pos[0][dim][j]+pos[1][dim][j]+pos[2][dim][j])/3.0;

          //edge 0 (nodes 0,1)
          pos[0][2][j]=(pos[0][dim][j]+pos[1][dim][j])/2.0;
          //edge 1 (nodes 1,2)
          pos[1][2][j]=(pos[1][dim][j]+pos[2][dim][j])/2.0;
          //edge 2 (nodes 0,2)
          pos[2][2][j]=(pos[0][dim][j]+pos[2][dim][j])/2.0;
          //edge 3 (nodes 0,3)
          pos[3][2][j]=(pos[0][dim][j]+pos[3][dim][j])/2.0;
          //edge 4 (nodes 1,3)
          pos[4][2][j]=(pos[1][dim][j]+pos[3][dim][j])/2.0;
          //edge 5 (nodes 2,3)
          pos[5][2][j]=(pos[2][dim][j]+pos[3][dim][j])/2.0;
        }
      }
      else
        DUNE_THROW(NotImplemented, "dim not implemented yet");

      //++++++++++++++++

    }

    int sizes[dim+1];
    int subsizes[MAXE][dim+1][dim+1];
    int subentityindex[MAXE][dim+1][MAXE][dim+1];
    FieldVector<ctype,dim> pos[MAXE][dim+1];

  };




  //! Make the reference simplex accessible as a container
  template<typename ctype, int dim>
  class ReferenceSimplexContainer
  {
  public:

    //! export type elements in the container
    typedef ReferenceSimplex<ctype,dim> value_type;

    //! return element of the container via geometry type
    const value_type& operator() (NewGeometryType type) const
    {
      if ( type.isSimplex())
        return simplices;
      DUNE_THROW(RangeError, "expected a simplex!");
    }

  private:
    ReferenceSimplex<ctype,dim> simplices;
  };


  //++++++++++++++++++++++++++++++++++++++++
  //Reference Prism
  //++++++++++++++++++++++++++++++++++++++++

  /*                                   5(0,1,1)
                       ¸.¸
                       y            ¸ *  . ` ,
   |           *_________ *4(1,0,1)
   |   3(0,0,1)|     .    |
   |           |     |    |
   |           |     .    |
   |           |     |    |
   |           |   ¸2¸(0,1,0)
   |           | ¸`    *¸ |
   |           |*________*|
   |    0(0,0,0)         1(1,0,0)
   ||_ _ _ _ _ _ _ _
                       x

                       volume of triangular prism =1/2

                       http://hal.iwr.uni-heidelberg.de/dune/doc/appl/refelements.html
   */


  template<typename ctype, int dim>
  class ReferencePrism;

  template<typename ctype>
  class ReferencePrism<ctype,3>
  {
  public:
    //assert dim=3;// dim must be 3
    enum {dim=3};
    enum {MAXE=9}; // 9 edges
    enum {d=dim};
    typedef ctype CoordType;


    ReferencePrism()
    {
      for (int i=0; i<=3; ++i)
        sizes[i]=0;

      for (int i=0; i<MAXE; ++i)
        for (int j=0; j<=dim; ++j)
          for (int k=0; k<=dim; ++k)
            if (j==k)
              subsizes[i][j][k] = 1;
            else
              subsizes[i][j][k] = 0;
      for (int i=0; i<MAXE; ++i)
        for (int j=0; j<=dim; ++j)
          subentityindex[i][j][0][j] = i;

      for (int c=3; c>=0; --c)
        prism_entities (c);
    }

    //! number of entities of codim c
    int size (int c) const
    {
      return sizes[c];
    }

    //! number of subentities of codim cc of entitity (i,c)
    int size (int i, int c, int cc) const
    {
      return subsizes[i][c][cc];
    }

    //! number of ii'th subentity with codim cc of (i,c)
    int subEntity (int i, int c, int ii, int cc) const
    {
      return subentityindex[i][c][ii][cc];
    }

    //! position of entity (i,c)
    const FieldVector<ctype,dim>& position (int i, int c) const
    {
      return pos[i][c];
    }

    //! maps local coordinate on subentity i of codim cdim onto reference element coordinates
    template <int codim>
    FieldVector<ctype, dim> global(const FieldVector<ctype, dim-codim>& local, int i, int cdim) const
    {
      return mapGlobal<ReferencePrism<ctype, dim>, codim>(*this, local, i, cdim);
    }

    //! type of (i,c)
    NewGeometryType type (int i, int c) const
    {
      switch (c)
      {
      case 3 : return NewGeometryType(NewGeometryType::cube,0);
      case 2 : return NewGeometryType(NewGeometryType::cube,1);
      case 0 : return NewGeometryType(NewGeometryType::prism,3);
      case 1 :
        switch (i)
        {
        case 0 : return NewGeometryType(NewGeometryType::simplex,2);
        case 1 : return NewGeometryType(NewGeometryType::cube,2);
        case 2 : return NewGeometryType(NewGeometryType::cube,2);
        case 3 : return NewGeometryType(NewGeometryType::cube,2);
        case 4 : return NewGeometryType(NewGeometryType::simplex,2);
        default :
          DUNE_THROW(RangeError, "i argument out of range");
        }
      default :
        DUNE_THROW(RangeError, "codim argument out of range");
      }
    }

    //! volume of the reference prism

    double volume () const
    {
      double vol=1.0/2.0;
      return vol;

    }
  private:

    void prism_entities(int c)
    {
      // compile time error if dim is not equal to 3
      IsTrue<dim == 3>::yes();
      // hard coding the size of entities
      sizes[0]=1; // element
      sizes[1]=5; // face
      sizes[2]=9; // edge
      sizes[3]=6; // vertex

      //-----------------------------------------------
      // hard coding the number of subentities
      // prism has 6 vertices, 9 edges and 5 facese on element
      subsizes[0][0][3]=6;
      subsizes[0][0][2]=9;
      subsizes[0][0][1]=5;

      // a prism itself  has one prism ;)
      subsizes[0][0][0]=1;

      // face indices according to that given in
      //http://hal.iwr.uni-heidelberg.de/dune/doc/appl/refelements.html

      // prism has one face on each face!
      for(int ii=0; ii<5; ++ii)
        subsizes[ii][1][1]=1;
      //  prism has 3 vertices on bott triang. face
      subsizes[0][1][3]=3;
      //  prism has 4 vertices on front,right and left  rectang. faces
      subsizes[1][1][3]=4;
      subsizes[2][1][3]=4;
      subsizes[3][1][3]=4;
      //  prism has 3 vertices on top triang. face
      subsizes[4][1][3]=3;

      // prism has 3 edges on a bott triang. face
      subsizes[0][1][2]=3;
      // prism has 4 edges on  front,right and left rec. faces
      subsizes[1][1][2]=4;
      subsizes[2][1][2]=4;
      subsizes[3][1][2]=4;
      // prism has 3 edges on a top triang. face
      subsizes[4][1][2]=3;
      // prism has 2 vertices on each  edge
      // also 1 edge on each edge
      for (int k=0; k<9; ++k) {
        subsizes[k][2][3]=2;
        subsizes[k][2][2]=1;
      }

      //------------------------------------------

      // positions of vertex with local index "i", there are 6 vertices
      // here c = codim = 3
      // -----------------------------------
      FieldVector<int,3> x;
      x=0;
      for (int n=0; n<3; n++)
      {
        pos[0][3][n]=x[n];

      }
      for(int k=1; k<=5; ++k)
        for (int j=0; j<3; ++j)
        {
          if(k>3)
          {
            x[j]=0;
            x[k-4]=1;
            x[2]=1;
            pos[k][3][j]= x[j];
          }
          else
          {
            x[j]=0;
            x[k-1]=1;
            pos[k][3][j]= x[j];
          }
        }
      //---------------------------------------

      // position of centre of gravity of the element
      for(int k=0; k<3; ++k)
      {
        pos[sizes[0]-1][0][k]=(pos[0][3][k])/sizes[3];
        for (int j=1; j<sizes[3]; ++j)
          pos[sizes[0]-1][0][k]+=(pos[j][3][k])/(sizes[3]);
      }

      // subentity indices - in agreement with UG description
      // see ug-cvs/gm/elements.c

      // node indices on element
      for(int i=0; i<subsizes[0][0][3]; ++i)
        subentityindex[0][0][i][3]=i;
      // edge indices on element
      for(int i=0; i<subsizes[0][0][2]; ++i)
        subentityindex[0][0][i][2]=i;
      // face indices on element
      for(int i=0; i<subsizes[0][0][1]; ++i)
        subentityindex[0][0][i][1]=i;

      // node indices on face 0
      subentityindex[0][1][0][3]=0;
      subentityindex[0][1][1][3]=2;
      subentityindex[0][1][2][3]=1;
      // node indices on face 1
      subentityindex[1][1][0][3]=0;
      subentityindex[1][1][1][3]=1;
      subentityindex[1][1][2][3]=4;
      subentityindex[1][1][3][3]=3;
      // node indices on face 2
      subentityindex[2][1][0][3]=1;
      subentityindex[2][1][1][3]=2;
      subentityindex[2][1][2][3]=5;
      subentityindex[2][1][3][3]=4;
      // node indices on face 3
      subentityindex[3][1][0][3]=2;
      subentityindex[3][1][1][3]=0;
      subentityindex[3][1][2][3]=3;
      subentityindex[3][1][3][3]=5;
      // node indices on face 4
      for(int i=0; i<subsizes[4][1][3]; ++i)
        subentityindex[4][1][i][3]=i+3;

      // edge indices on face 0
      subentityindex[0][1][0][2]=2;
      subentityindex[0][1][1][2]=1;
      subentityindex[0][1][2][2]=0;
      // edge indices on face 1
      subentityindex[1][1][0][2]=0;
      subentityindex[1][1][1][2]=4;
      subentityindex[1][1][2][2]=6;
      subentityindex[1][1][3][2]=3;
      // edge indices on face 2
      subentityindex[2][1][0][2]=1;
      subentityindex[2][1][1][2]=5;
      subentityindex[2][1][2][2]=7;
      subentityindex[2][1][3][2]=4;
      // edge indices on face 3
      subentityindex[3][1][0][2]=2;
      subentityindex[3][1][1][2]=3;
      subentityindex[3][1][2][2]=8;
      subentityindex[3][1][3][2]=5;
      // edge indices on face 4
      subentityindex[4][1][0][2]=6;
      subentityindex[4][1][1][2]=7;
      subentityindex[4][1][2][2]=8;

      // node indices on edge 0
      subentityindex[0][2][0][3]=0;
      subentityindex[0][2][1][3]=1;
      // node indices on edge 1
      subentityindex[1][2][0][3]=1;
      subentityindex[1][2][1][3]=2;
      // node indices on edge 2
      subentityindex[2][2][0][3]=2;
      subentityindex[2][2][1][3]=0;
      // node indices on edge 3
      subentityindex[3][2][0][3]=0;
      subentityindex[3][2][1][3]=3;
      // node indices on edge 4
      subentityindex[4][2][0][3]=1;
      subentityindex[4][2][1][3]=4;
      // node indices on edge 5
      subentityindex[5][2][0][3]=2;
      subentityindex[5][2][1][3]=5;
      // node indices on edge 6
      subentityindex[6][2][0][3]=3;
      subentityindex[6][2][1][3]=4;
      // node indices on edge 7
      subentityindex[7][2][0][3]=4;
      subentityindex[7][2][1][3]=5;
      // node indices on edge 8
      subentityindex[8][2][0][3]=5;
      subentityindex[8][2][1][3]=3;
      //

      //position of faces and edges
      for(int j=0; j<3; ++j)
      {
        //face 0 (nodes 0,1,2)
        pos[0][1][j]=(pos[0][3][j]+pos[1][3][j]+pos[2][3][j])/3.0;
        //face 1 (nodes 0,1,3,4)
        pos[1][1][j]=(pos[0][3][j]+pos[1][3][j]+pos[3][3][j]+pos[4][3][j])/4.0;
        //face 2 (nodes 1,2,4,5)
        pos[2][1][j]=(pos[1][3][j]+pos[2][3][j]+pos[4][3][j]+pos[5][3][j])/4.0;
        //face 3 (nodes 0,2,3,5)
        pos[3][1][j]=(pos[0][3][j]+pos[2][3][j]+pos[3][3][j]+pos[5][3][j])/4.0;
        //face 4 (nodes 3,4,5)
        pos[4][1][j]=(pos[3][3][j]+pos[4][3][j]+pos[5][3][j])/3.0;

        //edge 0 (nodes 0,1)
        pos[0][2][j]=(pos[0][3][j]+pos[1][3][j])/2.0;
        //edge 1 (nodes 1,2)
        pos[1][2][j]=(pos[1][3][j]+pos[2][3][j])/2.0;
        //edge 2 (nodes 2,0)
        pos[2][2][j]=(pos[2][3][j]+pos[0][3][j])/2.0;
        //edge 3 (nodes 0,3)
        pos[3][2][j]=(pos[0][3][j]+pos[3][3][j])/2.0;
        //edge 4 (nodes 1,4)
        pos[4][2][j]=(pos[1][3][j]+pos[4][3][j])/2.0;
        //edge 5 (nodes 2,5)
        pos[5][2][j]=(pos[2][3][j]+pos[5][3][j])/2.0;
        //edge 6 (nodes 3,4)
        pos[6][2][j]=(pos[3][3][j]+pos[4][3][j])/2.0;
        //edge 7 (nodes 4,5)
        pos[7][2][j]=(pos[4][3][j]+pos[5][3][j])/2.0;
        //edge 8 (nodes 5,3)
        pos[8][2][j]=(pos[5][3][j]+pos[3][3][j])/2.0;

      }


    }


    int sizes[dim+1];
    int subsizes[MAXE][dim+1][dim+1];
    int subentityindex[MAXE][dim+1][MAXE][dim+1];
    FieldVector<ctype,dim> pos[MAXE][dim+1];


  };

  //! Make the reference Prism accessible as a container
  template<typename ctype, int dim>
  class ReferencePrismContainer
  {
  public:

    //! export type elements in the container
    typedef ReferencePrism<ctype,dim> value_type;

    //! return element of the container via geometry type
    const value_type& operator() (NewGeometryType type) const
    {
      if (type.isPrism())
        return pris;
      DUNE_THROW(RangeError, "expected a prism!");
    }

  private:
    ReferencePrism<ctype,dim> pris;
  };


  //++++++++++++++++++++++++++++++++++++
  // Reference Pyramid
  //++++++++++++++++++++++++++++++++++++

  /*

     http://hal.iwr.uni-heidelberg.de/dune/doc/appl/refelements.html
     volume of pyramid = 1/3;
   */


  template<typename ctype, int dim>
  class ReferencePyramid;

  template<typename ctype>
  class ReferencePyramid<ctype, 3>
  {

  public:
    enum {dim=3};
    enum {MAXE=8}; // 8 edges
    enum {d=dim};
    typedef ctype CoordType;


    ReferencePyramid()
    {
      for (int i=0; i<=3; ++i)
        sizes[i]=0;

      for (int i=0; i<MAXE; ++i)
        for (int j=0; j<=dim; ++j)
          for (int k=0; k<=dim; ++k)
            if (j==k)
              subsizes[i][j][k] = 1;
            else
              subsizes[i][j][k] = 0;
      for (int i=0; i<MAXE; ++i)
        for (int j=0; j<=dim; ++j)
          subentityindex[i][j][0][j] = i;

      for (int c=3; c>=0; --c)
        pyramid_entities (c);
    }

    //! number of entities of codim c
    int size (int c) const
    {
      return sizes[c];
    }

    //! number of subentities of codim cc of entitity (i,c)
    int size (int i, int c, int cc) const
    {
      return subsizes[i][c][cc];
    }

    //! number of ii'th subentity with codim cc of (i,c)
    int subEntity (int i, int c, int ii, int cc) const
    {
      return subentityindex[i][c][ii][cc];
    }

    //! position of entity (i,c)
    const FieldVector<ctype,dim>& position (int i, int c) const
    {
      return pos[i][c];
    }

    //! maps local coordinate on subentity i of codim cdim onto reference element coordinates
    template <int codim>
    FieldVector<ctype, dim> global(const FieldVector<ctype, dim-codim>& local, int i, int cdim) const
    {
      return mapGlobal<ReferencePyramid<ctype, dim>, codim>(*this, local, i, cdim);
    }

    //! type of (i,c)
    NewGeometryType type (int i, int c) const
    {
      switch (c)
      {
      case 3 : return NewGeometryType(NewGeometryType::cube,0);
      case 2 : return NewGeometryType(NewGeometryType::cube,1);
      case 0 : return NewGeometryType(NewGeometryType::pyramid,3);
      case 1 :
        switch (i)
        {
        case 0 : return NewGeometryType(NewGeometryType::cube,2);
        case 1 :
        case 2 :
        case 3 :
        case 4 : return NewGeometryType(NewGeometryType::simplex,2);
        default :
          DUNE_THROW(RangeError, "i argument out of range");
        }
      default :
        DUNE_THROW(RangeError, "codim argument out of range");
      }
    }

    //! volume of the reference pyramid

    double volume () const
    {
      double vol=1.0/3.0;
      return vol;

    }
  private:

    void pyramid_entities(int c)

    {
      // compile time error if dim is not 3
      IsTrue<dim == 3>::yes();
      // hard coding the size of entities
      sizes[0]=1; // element
      sizes[1]=5; // face
      sizes[2]=8; // edge
      sizes[3]=5; // vertex

      //-----------------------------------------------
      // hard coding the number of subentities
      // pyramid has 5 vertices, 8 edges and 5 facese on element
      subsizes[0][0][3]=5;
      subsizes[0][0][2]=8;
      subsizes[0][0][1]=5;

      // a pyramid itself has one pyramid ;)
      subsizes[0][0][0]=1;
      // pyramid has one face on each face!
      for(int ii=0; ii<5; ++ii)
        subsizes[ii][1][1]=1;
      // face indices according to that given in
      //http://hal.iwr.uni-heidelberg.de/dune/doc/appl/refelements.html

      //  pyramid has 4 vertices on bott rect. face
      subsizes[0][1][3]=4;
      //  pyramid has 3 vertices on front,right,back and left triang. faces
      subsizes[1][1][3]=3;
      subsizes[2][1][3]=3;
      subsizes[3][1][3]=3;
      subsizes[4][1][3]=3;

      // pyramid has 4 edges on a bott rect. face
      subsizes[0][1][2]=4;
      // pyramid has 3 edges on front,right,back and left triang. faces
      for(int i=1; i<5; ++i)
        subsizes[i][1][2]=3;

      // pyramid has 2 vertices on each  edge
      for (int k=0; k<8; ++k) {
        subsizes[k][2][3]=2;
        //also one edge on each edge!!
        subsizes[k][2][2]=1;
      }
      //------------------------------------------

      // positions of vertex with local index "i", there are 5 vertices
      // here c = codim = 3
      // -----------------------------------


      FieldVector<int,3> x;
      x=0;
      for (int n=0; n<3; n++)
      {
        pos[0][3][n]=x[n];

      }

      for(int k=1; k<=4; ++k)
        for (int j=0; j<3; ++j)
        {
          if(k==1)
          {
            x[j]=0;
            x[k-1]=1;
            pos[k][3][j]= x[j];
          }
          else if(k==2)
          {
            x[j]=0;
            x[k-2]=1;
            x[1]=1;
            pos[k][3][j]= x[j];
          }
          else
          {
            x[j]=0;
            x[k-2]=1;
            pos[k][3][j]= x[j];
          }
        }

      //---------------------------------------

      // position of centre of gravity of the element
      for(int k=0; k<3; ++k)
      {
        pos[sizes[0]-1][0][k]=(pos[0][3][k])/sizes[3];
        for (int j=1; j<sizes[3]; ++j)
          pos[sizes[0]-1][0][k]+=(pos[j][3][k])/(sizes[3]);
      }

      // subentity indices - in agreement with UG description
      // see ug-cvs/gm/elements.c

      // node indices on element
      for(int i=0; i<subsizes[0][0][3]; ++i)
        subentityindex[0][0][i][3]=i;
      // edge indices on element
      for(int i=0; i<subsizes[0][0][2]; ++i)
        subentityindex[0][0][i][2]=i;
      // face indices on element
      for(int i=0; i<subsizes[0][0][1]; ++i)
        subentityindex[0][0][i][1]=i;

      // node indices on face 0
      subentityindex[0][1][0][3]=0;
      subentityindex[0][1][1][3]=3;
      subentityindex[0][1][2][3]=2;
      subentityindex[0][1][3][3]=1;
      // node indices on face 1
      subentityindex[1][1][0][3]=0;
      subentityindex[1][1][1][3]=1;
      subentityindex[1][1][2][3]=4;
      // node indices on face 2
      subentityindex[2][1][0][3]=1;
      subentityindex[2][1][1][3]=2;
      subentityindex[2][1][2][3]=4;
      // node indices on face 3
      subentityindex[3][1][0][3]=2;
      subentityindex[3][1][1][3]=3;
      subentityindex[3][1][2][3]=4;
      // node indices on face 4
      subentityindex[4][1][0][3]=3;
      subentityindex[4][1][1][3]=0;
      subentityindex[4][1][2][3]=4;

      // edge indices on face 0
      subentityindex[0][1][0][2]=3;
      subentityindex[0][1][1][2]=2;
      subentityindex[0][1][2][2]=1;
      subentityindex[0][1][3][2]=0;
      // edge indices on face 1
      subentityindex[1][1][0][2]=0;
      subentityindex[1][1][1][2]=5;
      subentityindex[1][1][2][2]=4;
      // edge indices on face 2
      subentityindex[2][1][0][2]=1;
      subentityindex[2][1][1][2]=6;
      subentityindex[2][1][2][2]=5;
      // edge indices on face 3
      subentityindex[3][1][0][2]=2;
      subentityindex[3][1][1][2]=7;
      subentityindex[3][1][2][2]=6;
      // edge indices on face 4
      subentityindex[4][1][0][2]=3;
      subentityindex[4][1][1][2]=4;
      subentityindex[4][1][2][2]=7;

      // node indices on edge 0
      subentityindex[0][2][0][3]=0;
      subentityindex[0][2][1][3]=1;
      // node indices on edge 1
      subentityindex[1][2][0][3]=1;
      subentityindex[1][2][1][3]=2;
      // node indices on edge 2
      subentityindex[2][2][0][3]=2;
      subentityindex[2][2][1][3]=3;
      // node indices on edge 3
      subentityindex[3][2][0][3]=3;
      subentityindex[3][2][1][3]=0;
      // node indices on edge 4
      subentityindex[4][2][0][3]=0;
      subentityindex[4][2][1][3]=4;
      // node indices on edge 5
      subentityindex[5][2][0][3]=1;
      subentityindex[5][2][1][3]=4;
      // node indices on edge 6
      subentityindex[6][2][0][3]=2;
      subentityindex[6][2][1][3]=4;
      // node indices on edge 7
      subentityindex[7][2][0][3]=3;
      subentityindex[7][2][1][3]=4;


      //position of faces and edges
      for(int j=0; j<3; ++j)
      {
        //face 0 (nodes 0,1,2,3)
        pos[0][1][j]=(pos[0][3][j]+pos[1][3][j]+pos[2][3][j]+pos[3][3][j])/4.0;
        //face 1 (nodes 0,1,4)
        pos[1][1][j]=(pos[0][3][j]+pos[1][3][j]+pos[4][3][j])/3.0;
        //face 2 (nodes 1,2,4)
        pos[2][1][j]=(pos[1][3][j]+pos[2][3][j]+pos[4][3][j])/3.0;
        //face 3 (nodes 2,3,4)
        pos[3][1][j]=(pos[2][3][j]+pos[3][3][j]+pos[4][3][j])/3.0;
        //face 4 (nodes 3,0,4)
        pos[4][1][j]=(pos[3][3][j]+pos[0][3][j]+pos[4][3][j])/3.0;

        //edge 0 (nodes 0,1)
        pos[0][2][j]=(pos[0][3][j]+pos[1][3][j])/2.0;
        //edge 1 (nodes 1,2)
        pos[1][2][j]=(pos[1][3][j]+pos[2][3][j])/2.0;
        //edge 2 (nodes 2,3)
        pos[2][2][j]=(pos[2][3][j]+pos[3][3][j])/2.0;
        //edge 3 (nodes 3,0)
        pos[3][2][j]=(pos[3][3][j]+pos[0][3][j])/2.0;
        //edge 4 (nodes 0,4)
        pos[4][2][j]=(pos[0][3][j]+pos[4][3][j])/2.0;
        //edge 5 (nodes 1,4)
        pos[5][2][j]=(pos[1][3][j]+pos[4][3][j])/2.0;
        //edge 6 (nodes 2,4)
        pos[6][2][j]=(pos[2][3][j]+pos[4][3][j])/2.0;
        //edge 7 (nodes 3,4)
        pos[7][2][j]=(pos[3][3][j]+pos[4][3][j])/2.0;


      }


    }


    int sizes[dim+1];
    int subsizes[MAXE][dim+1][dim+1];
    int subentityindex[MAXE][dim+1][MAXE][dim+1];
    FieldVector<ctype,dim> pos[MAXE][dim+1];


  };

  //! Make the reference Pyramid accessible as a container
  template<typename ctype, int dim>
  class ReferencePyramidContainer
  {
  public:

    //! export type elements in the container
    typedef ReferencePrism<ctype,dim> value_type;

    //! return element of the container via geometry type
    const value_type& operator() (NewGeometryType type) const
    {
      if (type.isPyramid())
        return pyram;
      DUNE_THROW(RangeError, "expected a pyramid!");
    }

  private:
    ReferencePyramid<ctype,dim> pyram;
  };



  /***********************************************************
  * The general container and the singletons
  ***********************************************************/


  //! Combines all reference elements
  template<typename ctype, int dim>
  class ReferenceElementContainer
  {
  public:

    //! export type elements in the container
    typedef ReferenceElement<ctype,dim> value_type;

    //! return element of the container via geometry type
    const ReferenceElement<ctype,dim>& operator() (NewGeometryType type) const
    {
      if ( type.isCube())
        return hcube;
      else if ( type.isSimplex() )
        return simplices;
      else
        DUNE_THROW(NotImplemented, "type not implemented yet");
    }

  private:
    ReferenceElementWrapper<ReferenceCube<ctype,dim> > hcube;
    ReferenceElementWrapper<ReferenceSimplex<ctype,dim> > simplices;
  };

  //! Combines all reference elements for dim==3
  template<typename ctype>
  class ReferenceElementContainer<ctype, 3>
  {
    enum { dim=3 };
  public:

    //! export type elements in the container
    typedef ReferenceElement<ctype,dim> value_type;

    //! return element of the container via geometry type
    const ReferenceElement<ctype,dim>& operator() (NewGeometryType type) const
    {
      if ( type.isCube() )
        return hcube;
      else if( type.isSimplex() )
        return simplices;
      else if ( type.isPrism() )
        return pris;
      else if( type.isPyramid() )
        return pyram;
      else
        DUNE_THROW(NotImplemented, "type not implemented yet");
    }

  private:
    ReferenceElementWrapper<ReferenceCube<ctype,dim> > hcube;
    ReferenceElementWrapper<ReferenceSimplex<ctype,dim> > simplices;
    ReferenceElementWrapper<ReferencePrism<ctype,dim> > pris;
    ReferenceElementWrapper<ReferencePyramid<ctype,dim> > pyram;
  };

  // singleton holding several reference element containers
  template<typename ctype, int dim>
  struct ReferenceElements {
    static ReferenceCubeContainer<ctype,dim> cube;
    static ReferenceSimplexContainer<ctype,dim> simplices;
    static ReferenceElementContainer<ctype,dim> general;
  };

  // singleton holding several reference element containers dim==3
  template<typename ctype>
  struct ReferenceElements<ctype,3> {
    static ReferenceCubeContainer<ctype,3> cube;
    static ReferenceSimplexContainer<ctype,3> simplices;
    static ReferencePrismContainer<ctype,3> prism;
    static ReferencePyramidContainer<ctype,3> pyramid;
    static ReferenceElementContainer<ctype,3> general;
  };

  /** @} */
}
#endif
