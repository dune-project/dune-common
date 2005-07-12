// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef __DUNE_REFERENCEELEMENTS_HH__
#define __DUNE_REFERENCEELEMENTS_HH__

#include <iostream>
#include "dune/common/fvector.hh"
#include "dune/common/exceptions.hh"
#include "dune/grid/common/grid.hh"
#include "power.hh"

/**
 * @file
 * @brief  Definition of reference elements for all types and dims
 * @author Peter Bastian
 */
namespace Dune
{
  /** @addtogroup Common
   *
   * @{
   */
  /**
   * @brief Definition of reference elements for all types and dims
   *
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
    enum { d=dim };        // maps from R^d

    // export types
    typedef ctype CoordType;

    //! number of entities of codim c
    virtual int size (int c) const = 0;

    //! number of subentities of codim cc of entitity (i,c)
    virtual int size (int i, int c, int cc) const = 0;

    //! number of ii'th subentity with codim cc>c of (i,c)
    virtual int subentity (int i, int c, int ii, int cc) const = 0;

    //! position of entity (i,c)
    virtual const FieldVector<ctype,dim>& position (int i, int c) const = 0;

    //! type of (i,c)
    virtual GeometryType type (int i, int c) const = 0;

    //! volume of the reference element
    virtual double volume () const = 0;

    //! virtual destructor
    virtual ~ReferenceElement ()
    {}
  };

  //! The wrapper allows Imp to be used as a reference element without making methods of Imp virtual
  template<typename Imp>
  class ReferenceElementWrapper : public ReferenceElement<typename Imp::CoordType,Imp::d>,
                                  private Imp
  {
  public:

    // compile time sizes
    enum { d=Imp::d };        // maps from R^d

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
    int subentity (int i, int c, int ii, int cc) const
    {
      return Imp::subentity(i,c,ii,cc);
    }

    //! position of entity (i,c)
    const FieldVector<CoordType,d>& position (int i, int c) const
    {
      return Imp::position(i,c);
    }

    //! type of (i,c)
    GeometryType type (int i, int c) const
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
            << " subentity=" << r.subentity(i,c,ii,cc)
            << " position=(" << r.position(r.subentity(i,c,ii,cc),cc) << ")"
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
    enum { MAXE = POWER_M_P<3,dim>::power };     // maximum number of entities per codim

    // compile time sizes
    enum { d=dim };        // maps from R^d

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
            subsizes[i][j][k] = 0;
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
    int subentity (int i, int c, int ii, int cc) const
    {
      return hierarchy[i][c][ii][cc];
    }

    //! position of entity (i,c)
    const FieldVector<ctype,dim>& position (int i, int c) const
    {
      return pos[i][c];
    }

    //! type of (i,c)
    GeometryType type (int i, int c) const
    {
      return cube;
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
          direction[k] = i;                 // new direction selected
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
          direction[k] = i;                 // new direction selected
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
    const value_type& operator() (GeometryType type) const
    {
      if ( (type==cube) || (type==line) || (type==quadrilateral) ||
           (type==hexahedron) )
        return cube;
      DUNE_THROW(RangeError, "expected a cube!");
    }

  private:
    ReferenceCube<ctype,dim> cube;
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

  template<typename ctype, int dim>
  class ReferenceSimplex
  {
  public:
    enum {MAXE = ((dim+1)<<1)-(dim+1)+2*((dim-1)>>1)}; // 1D=2; 2D=3; in 3D: 6 edges for a tetrahedron
    enum {d=dim};


    typedef ctype CoordType;

    ReferenceSimplex ()
    {
      for (int i=0; i<=dim; ++i)
        sizes[i]=0;
      for (int i=0; i<MAXE; ++i)
        for (int j=0; j<=dim; ++j)
          for (int k=0; k<=dim; ++k)
            subsizes[i][j][k] = 0;

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
    int subentity (int i, int c, int ii, int cc) const
    {
      return subentityindex[i][c][ii][cc];
    }

    //! position of entity (i,c)
    const FieldVector<ctype,dim>& position (int i, int c) const
    {
      return pos[i][c];
    }

    //! type of (i,c)
    GeometryType type (int i, int c) const
    {
      return simplex;
    }


    //! volume of the reference element

    double volume () const
    {
      double vol=1.0;
      for(int i=1; i<=dim; ++i)
        vol*=(1.0/i);
      return vol;

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
      sizes[0]=1;   // only 1 cell !!
      for(int k=0; k<dim; ++k)
      {
        pos[sizes[0]-1][0][k]=(pos[0][dim][k])/sizes[dim];
        subentityindex[sizes[0]-1][0][0][dim]=0;
        for (int j=1; j<sizes[dim]; ++j)
        {
          pos[sizes[0]-1][0][k]+=(pos[j][dim][k])/(sizes[dim]);
          subentityindex[sizes[0]-1][0][j][dim]=j;
        }
      }

      int face=0;
      int edge=0;
      for (int dir=0; dir<dim; ++dir)
      {

        for(int side=1; side<=dim; ++side)
        {

          if(side>dir)
          {
            for(int connect=side+1; connect<=dim; ++connect)
            {
              face+=1;
              subentityindex[sizes[0]-1][0][face-1][dim-2]=face-1;          // face on cell
              sizes[dim-2]=face;
              subsizes[sizes[0]-1][0][dim-2]=face;
              subsizes[face-1][dim-2][dim]=3;          // no of nodes on face
              fnindex[face-1][0]=dir;           // fnindex is face node index
              fnindex[face-1][1]=side;
              fnindex[face-1][2]=connect;
              //std::cout<<"dir::"<<dir<<side<<connect<<std::endl;
              for (int j=0; j<dim; ++j)
                subentityindex[face-1][dim-2][j][dim]=fnindex[face-1][j];

              for (int j=0; j<dim; ++j)
              {
                double sum=0;
                for(int jj=0; jj<subsizes[face-1][dim-2][dim]; ++jj)
                  sum+=pos[fnindex[face-1][jj]][dim][j];
                pos[face-1][dim-2][j]=sum/3.0;
              }

              if(face==1)
              {
                int count=0;
                for(int i=0; i<2; ++i)
                  for(int j=i+1; j<3; ++j)
                  {
                    //std::cout<<"i:"<<i<<"j:"<<j<<std::endl;
                    count+=1;
                    edge+=1;
                    sizes[dim-1]=edge;
                    subsizes[sizes[0]-1][0][dim-1]=edge;
                    subentityindex[sizes[0]-1][0][edge-1][dim-1]=edge-1;              // edge on cell
                    subsizes[edge-1][dim-1][dim]=2;               // nod on edge
                    subsizes[face-1][dim-2][dim-1]=count;               // no of edges on face
                    subentityindex[face-1][dim-2][count][dim-1]=edge;               // edge on cell
                    enindex[count-1][0]=fnindex[face-1][i];               // edge node index
                    enindex[count-1][1]=fnindex[face-1][j];
                    ceindex[edge-1][0]=fnindex[face-1][i];               // cell edge index
                    ceindex[edge-1][1]=fnindex[face-1][j];
                    subentityindex[edge-1][dim-1][0][dim]=fnindex[face-1][i];              // nod on edge
                    subentityindex[edge-1][dim-1][1][dim]=fnindex[face-1][j];              // nod on edge
                    edgeindex[fnindex[face-1][i]][fnindex[face-1][j]]=edge-1;
                    for(int k=0; k<dim; ++k)
                    {
                      double sum=0;
                      for(int kk=0; kk<subsizes[edge-1][dim-1][dim]; ++kk)
                        sum+=pos[subentityindex[edge-1][dim-1][kk][dim]][dim][k];
                      pos[edge-1][dim-1][k]=sum/2.0;
                    }
                  }
              }
              else
              {
                int count=0;

                int common=0;
                for(int i=0; i<2; ++i)
                  for(int j=i+1; j<3; ++j)
                  {

                    for(int k=0; k<edge; ++k)
                    {


                      if(ceindex[k][0]==fnindex[face-1][i] && ceindex[k][1]==fnindex[face-1][j])
                      {
                        common++;
                        count+=1;
                        subsizes[edge-1][dim-1][dim]=2;                   // nod on edge
                        subsizes[face-1][dim-2][dim-1]=count;                   // no of edges on face
                        subentityindex[face-1][dim-2][count-1][dim-1]=k;
                        enindex[count-1][0]=fnindex[face-1][i];
                        enindex[count-1][1]=fnindex[face-1][j];
                      }
                    }
                  }

                for(int i=common-1; i<2; ++i)

                {
                  int j=2;
                  count+=1;
                  edge+=1;
                  sizes[dim-1]=edge;
                  //std::cout<<"edge:"<<edge-1<<std::endl;
                  subsizes[sizes[0]-1][0][dim-1]=edge;
                  subentityindex[sizes[0]-1][0][edge-1][dim-1]=edge-1;              // edge on cell
                  subsizes[edge-1][dim-1][dim]=2;               // nod on edge
                  subsizes[face-1][dim-2][dim-1]=count;               // no of edges on face
                  subentityindex[face-1][dim-2][count-1][dim-1]=edgeindex[fnindex[face-1][i]][fnindex[face-1][j]];              //edge on face
                  enindex[count-1][0]=fnindex[face-1][i];
                  enindex[count-1][1]=fnindex[face-1][j];
                  ceindex[edge-1][0]=fnindex[face-1][i];
                  ceindex[edge-1][1]=fnindex[face-1][j];
                  subentityindex[edge-1][dim-1][0][dim]=fnindex[face-1][i];              // nod on edge
                  subentityindex[edge-1][dim-1][1][dim]=fnindex[face-1][j];              // nod on edge
                  edgeindex[fnindex[face-1][i]][fnindex[face-1][j]]=edge-1;
                  for(int k=0; k<dim; ++k)
                  {
                    double sum=0;
                    for(int kk=0; kk<subsizes[edge-1][dim-1][dim]; ++kk)
                      sum+=pos[subentityindex[edge-1][dim-1][kk][dim]][dim][k];
                    pos[edge-1][dim-1][k]=sum/2.0;
                  }
                }
              }
            }
          }
        }
      }

    }

    int sizes[dim+1];
    int subsizes[MAXE][dim+1][dim+1];
    int subentityindex[MAXE][dim+1][MAXE][dim+1];
    int edgeindex[dim+1][dim+1];
    int faceindex[dim+1][dim+1][dim+1];
    int fnindex[dim+1][dim];
    int enindex[dim+1][dim];
    int ceindex[MAXE][dim];
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
    const value_type& operator() (GeometryType type) const
    {
      if ( (type==simplex) || (type==triangle) || (type==tetrahedron) )
        return simplices;
      DUNE_THROW(RangeError, "expected a simplex!");
    }

  private:
    ReferenceSimplex<ctype,dim> simplices;
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
    const ReferenceElement<ctype,dim>& operator() (GeometryType type) const
    {
      if ( (type==cube) || (type==line) || (type==quadrilateral) ||
           (type==hexahedron) )
        return hcube;
      else if( (type==simplex ) || (type==triangle ) || (type==tetrahedron))
        return simplices;
      else
        DUNE_THROW(NotImplemented, "type not implemented yet");
    }

  private:
    ReferenceElementWrapper<ReferenceCube<ctype,dim> > hcube;
    ReferenceElementWrapper<ReferenceSimplex<ctype,dim> > simplices;
  };


  // singleton holding several reference element containers
  template<typename ctype, int dim>
  struct ReferenceElements {
    static ReferenceCubeContainer<ctype,dim> cube;
    static ReferenceSimplexContainer<ctype,dim> simplices;
    static ReferenceElementContainer<ctype,dim> general;
  };

  template<typename ctype, int dim>
  ReferenceCubeContainer<ctype,dim> ReferenceElements<ctype,dim>::cube;

  template<typename ctype, int dim>
  ReferenceSimplexContainer<ctype,dim> ReferenceElements<ctype,dim>::simplices;


  template<typename ctype, int dim>
  ReferenceElementContainer<ctype,dim> ReferenceElements<ctype,dim>::general;

  /** @} */
}
#endif
