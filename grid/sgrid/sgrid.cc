// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __SGRID_CC__
#define __SGRID_CC__

namespace Dune {

#include <assert.h>

  //************************************************************************
  // INLINE METHOD IMPLEMENTATIONS FOR SGRID
  //************************************************************************

  //************************************************************************

  // singleton holding reference elements
  template<int dim>
  struct SReferenceElement {
    static SElement<dim,dim> refelem;
  };

  // initialize static variable with default constructor (which makes reference elements)
  template<int dim>
  SElement<dim,dim> SReferenceElement<dim>::refelem;


  // members for SElementBase
  template<int dim, int dimworld>
  inline SElementBase<dim,dimworld>::SElementBase ()
  {
    builtinverse = false;
  }

  template<int dim, int dimworld>
  inline SElement<dim,dim>& SElementBase<dim,dimworld>::refelem ()
  {
    return SReferenceElement<dim,dim>::refelem
  }

  template<int dim, int dimworld>
  inline Vec<dimworld,sgrid_ctype> SElementBase<dim,dimworld>::global (const Vec<dim,sgrid_ctype>& local)
  {
    return s+(A*local);
  }

  template<int dim, int dimworld>
  inline Vec<dim,sgrid_ctype> SElementBase<dim,dimworld>::local (const Vec<dimworld,sgrid_ctype>& global)
  {
    Vec<dim,sgrid_ctype> l;     // result
    Vec<dimworld,sgrid_ctype> rhs = global-s;
    for (int k=0; k<dim; k++)
      l(k) = (rhs*A(k)) / (A(k)*A(k));
    return l;
  }

  template<int dim, int dimworld>
  inline sgrid_ctype SElementBase<dim,dimworld>::integration_element (const Vec<dim,sgrid_ctype>& local)
  {
    sgrid_ctype s = 1.0;
    for (int j=0; j<dim; j++) s *= A(j).norm1();
    return s;
  }

  template<int dim, int dimworld>
  inline Mat<dim,dim>& SElementBase<dim,dimworld>::Jacobian_inverse (const Vec<dim,sgrid_ctype>& local)
  {
    assert(dim==dimworld);

    Jinv = A;
    if (!builtinverse)
    {
      for (int i=0; i<dim; i++) Jinv(i,i) = 1.0/Jinv(i,i);
      builtinverse = true;
    }
    return Jinv;
  }

  template<int dim, int dimworld>
  inline void SElementBase<dim,dimworld>::print (std::ostream& ss, int indent)
  {
    for (int k=0; k<indent; k++) ss << " ";ss << "SElementBase<" << dim << "," << dimworld << ">" << endl;
    for (int k=0; k<indent; k++) ss << " ";ss << "{" << endl;
    for (int k=0; k<indent+2; k++) ss << " ";ss << "Position: " << s << endl;
    for (int j=0; j<dim; j++)
    {
      for (int k=0; k<indent+2; k++) ss << " ";
      ss << "direction " << j << "  " << A(j) << endl;
    }
    for (int j=0; j<1<<dim; j++)
    {
      for (int k=0; k<indent+2; k++) ss << " ";
      ss << "corner " << j << "  " << c[j] << endl;
    }
    for (int k=0; k<indent+2; k++) ss << " ";ss << "Jinv ";
    Jinv.print(ss,indent+2);
    for (int k=0; k<indent+2; k++) ss << " ";ss << "builtinverse " << builtinverse << endl;
    for (int k=0; k<indent; k++) ss << " ";ss << "}";
  }

  // members for SElement, must use specialization

  // reference element constructor, dim>3
  template<int dim, int dimworld>
  inline SElement<dim,dimworld>::SElement ()
  {
    // copy arguments
    s = 0.0;
    for (int j=0; j<dim; j++) A(j) = Vec<dimworld,sgrid_ctype>(j);     // make unit vectors

    // make corners
    for (int i=0; i<(1<<dim); i++)     // there are 2^d corners
    {
      // use binary representation of corner number to assign corner coordinates
      int mask=1;
      c[i] = s;
      for (int k=0; k<dim; k++)
      {
        if (i&mask) c[i] = c[i]+A(k);
        mask = mask<<1;
      }
    }
  }

  template<int dimworld>
  inline SElement<1,dimworld>::SElement ()
  {
    // copy arguments
    s = 0.0;
    A(0) = Vec<dimworld,sgrid_ctype>(0);     // make unit vectors

    // make corners
    c[0] = s;
    c[1] = s+A(0);
  }

  template<int dimworld>
  inline SElement<2,dimworld>::SElement ()
  {
    // copy arguments
    s = 0.0;
    A(0) = Vec<dimworld,sgrid_ctype>(0);     // make unit vectors
    A(1) = Vec<dimworld,sgrid_ctype>(1);     // make unit vectors

    // make corners
    c[0] = s;
    c[1] = s+A(0);
    c[2] = s+A(0)+A(1);
    c[3] = s+A(1);
  }

  template<int dimworld>
  inline SElement<3,dimworld>::SElement ()
  {
    // copy arguments
    s = 0.0;
    A(0) = Vec<dimworld,sgrid_ctype>(0);     // make unit vectors
    A(1) = Vec<dimworld,sgrid_ctype>(1);     // make unit vectors
    A(2) = Vec<dimworld,sgrid_ctype>(2);     // make unit vectors

    // make corners
    c[0] = s;
    c[1] = s+A(0);
    c[2] = s+A(0)+A(1);
    c[3] = s+A(1);
    c[4] = s+A(2);
    c[5] = s+A(0)+A(2);
    c[6] = s+A(0)+A(1)+A(2);
    c[7] = s+A(1)+A(2);
  }


  template<int dimworld>
  inline SElement<1,dimworld>::SElement (const Vec<dimworld,sgrid_ctype>& s_, const Vec<dimworld,sgrid_ctype>& r0)
  {
    // copy arguments
    s = s_;
    A(0) = r0;

    // make corners
    c[0] = s;
    c[1] = s+A(0);
  }

  template<int dimworld>
  inline SElement<2,dimworld>::SElement (const Vec<dimworld,sgrid_ctype>& s_, const Vec<dimworld,sgrid_ctype>& r0,
                                         const Vec<dimworld,sgrid_ctype>& r1)
  {
    // copy arguments
    s = s_;
    A(0) = r0;
    A(1) = r1;

    // make corners
    c[0] = s;
    c[1] = s+A(0);
    c[2] = s+A(0)+A(1);
    c[3] = s+A(1);
  }

  template<int dimworld>
  inline SElement<3,dimworld>::SElement (const Vec<dimworld,sgrid_ctype>& s_, const Vec<dimworld,sgrid_ctype>& r0,
                                         const Vec<dimworld,sgrid_ctype>& r1, const Vec<dimworld,sgrid_ctype>& r2)
  {
    // copy arguments
    s = s_;
    A(0) = r0;
    A(1) = r1;
    A(2) = r2;

    // make corners
    c[0] = s;
    c[1] = s+A(0);
    c[2] = s+A(0)+A(1);
    c[3] = s+A(1);
    c[4] = s+A(2);
    c[5] = s+A(0)+A(2);
    c[6] = s+A(0)+A(1)+A(2);
    c[7] = s+A(1)+A(2);
  }

  template<int dim, int dimworld>
  inline SElement<dim,dimworld>::SElement (const Vec<dimworld,sgrid_ctype>& s_, Vec<dimworld,sgrid_ctype> r_[dim])
  {
    // copy arguments
    s = 0.0;
    for (int j=0; j<dim; j++) A(j) = r_[j];

    // make corners
    for (int i=0; i<(1<<dim); i++)     // there are 2^d corners
    {
      // use binary representation of corner number to assign corner coordinates
      int mask=1;
      c[i] = 0.0;
      for (k=0; k<dim; k++)
      {
        if (i&mask) c[i] = c[i]+A(k);
        mask = mask<<1;
      }
    }
  }

  template<int dimworld>
  inline SElement<1,dimworld>::SElement (const Vec<dimworld,sgrid_ctype>& s_, Vec<dimworld,sgrid_ctype> r_[1])
  {
    // copy arguments
    s = s_;
    A(0) = r_[0];

    // make corners
    c[0] = s;
    c[1] = s+A(0);
  }

  template<int dimworld>
  inline SElement<2,dimworld>::SElement (const Vec<dimworld,sgrid_ctype>& s_, Vec<dimworld,sgrid_ctype> r_[2])
  {
    // copy arguments
    s = s_;
    A(0) = r_[0];
    A(1) = r_[1];

    // make corners
    c[0] = s;
    c[1] = s+A(0);
    c[2] = s+A(0)+A(1);
    c[3] = s+A(1);
  }

  template<int dimworld>
  inline SElement<3,dimworld>::SElement (const Vec<dimworld,sgrid_ctype>& s_, Vec<dimworld,sgrid_ctype> r_[3])
  {
    // copy arguments
    s = s_;
    A(0) = r_[0];
    A(1) = r_[1];
    A(2) = r_[2];

    // make corners
    c[0] = s;
    c[1] = s+A(0);
    c[2] = s+A(0)+A(1);
    c[3] = s+A(1);
    c[4] = s+A(2);
    c[5] = s+A(0)+A(2);
    c[6] = s+A(0)+A(1)+A(2);
    c[7] = s+A(1)+A(2);
  }

  // type
  template<int dim, int dimworld>
  inline ElementType SElement<dim,dimworld>::type ()
  {
    return unknown;
  }

  template<int dimworld>
  inline ElementType SElement<1,dimworld>::type ()
  {
    return line;
  }

  template<int dimworld>
  inline ElementType SElement<2,dimworld>::type ()
  {
    return quadrilateral;
  }

  template<int dimworld>
  inline ElementType SElement<3,dimworld>::type ()
  {
    return hexahedron;
  }

  //************************************************************************
  // inline methods for SEntity

  // general
  // unfortunately, you have to define all three versions seperately, at least with the Intel compiler
  // in debug mode ...
  template<int codim, int dim, int dimworld>
  inline int SEntity<codim,dim,dimworld>::level ()
  {
    return 0;
  }

  template<int dim, int dimworld>
  inline int SEntity<0,dim,dimworld>::level ()
  {
    return 0;
  }

  template<int dim, int dimworld>
  inline int SEntity<dim,dim,dimworld>::level ()
  {
    return 0;
  }

  template<int codim, int dim, int dimworld>
  inline int SEntity<codim,dim,dimworld>::index ()
  {
    return 0;
  }

  template<int dim, int dimworld>
  inline int SEntity<0,dim,dimworld>::index ()
  {
    return 0;
  }

  template<int dim, int dimworld>
  inline int SEntity<dim,dim,dimworld>::index ()
  {
    return 0;
  }

  template<int codim, int dim, int dimworld>
  inline SElement<dim-codim,dimworld> SEntity<codim,dim,dimworld>::geometry ()
  {
    return 0;
  }

  // codim 0
  template<int dim, int dimworld> template<int cc>
  inline int SEntity<0,dim,dimworld>::count ()
  {
    return 0;
  }

  template<int dim, int dimworld> template<int cc>
  inline SLevelIterator<cc,dim,dimworld> SEntity<0,dim,dimworld>::entity (int i)
  {
    return SLevelIterator<cc,dim,dimworld>();
  }

  template<int dim, int dimworld>
  inline SEntity<0,dim,dimworld>::NeighborIterator SEntity<0,dim,dimworld>::nbegin ()
  {
    return SEntity<0,dim,dimworld>::NeighborIterator();
  }

  template<int dim, int dimworld>
  inline SEntity<0,dim,dimworld>::NeighborIterator SEntity<0,dim,dimworld>::nend ()
  {
    return SEntity<0,dim,dimworld>::NeighborIterator();
  }

  template<int dim, int dimworld>
  inline SLevelIterator<0,dim,dimworld> SEntity<0,dim,dimworld>::father ()
  {
    return SLevelIterator<0,dim,dimworld>();
  }

  template<int dim, int dimworld>
  inline SElement<dim,dim> SEntity<0,dim,dimworld>::father_relative_local ()
  {
    return SElement<dim,dim>();
  }

  template<int dim, int dimworld>
  inline SEntity<0,dim,dimworld>::HierarchicIterator SEntity<0,dim,dimworld>::hbegin (int maxlevel)
  {
    return SEntity<0,dim,dimworld>::HierarchicIterator();
  }

  template<int dim, int dimworld>
  inline SEntity<0,dim,dimworld>::HierarchicIterator SEntity<0,dim,dimworld>::hend (int maxlevel)
  {
    return SEntity<0,dim,dimworld>::HierarchicIterator();
  }


  // codim dim
  template<int dim, int dimworld>
  inline SLevelIterator<0,dim,dimworld> SEntity<dim,dim,dimworld>::father ()
  {
    return SLevelIterator<0,dim,dimworld>();
  }

  template<int dim, int dimworld>
  inline Vec<dim,sgrid_ctype> SEntity<dim,dim,dimworld>::local ()
  {
    return Vec<dim,sgrid_ctype>();
  }

  //************************************************************************
  // inline methods for HierarchicIterator

  template<int dim, int dimworld>
  inline SEntity<0,dim,dimworld>::HierarchicIterator SEntity<0,dim,dimworld>::HierarchicIterator::operator++ ()
  {
    return SEntity<0,dim,dimworld>::HierarchicIterator();
  }

  template<int dim, int dimworld>
  inline SEntity<0,dim,dimworld>::HierarchicIterator SEntity<0,dim,dimworld>::HierarchicIterator::operator++ (int i)
  {
    return SEntity<0,dim,dimworld>::HierarchicIterator();
  }

  template<int dim, int dimworld>
  inline bool SEntity<0,dim,dimworld>::HierarchicIterator::operator== (const SEntity<0,dim,dimworld>::HierarchicIterator& i) const
  {
    return true;
  }

  template<int dim, int dimworld>
  inline bool SEntity<0,dim,dimworld>::HierarchicIterator::operator!= (const SEntity<0,dim,dimworld>::HierarchicIterator& i) const
  {
    return false;
  }

  template<int dim, int dimworld>
  inline SEntity<0,dim,dimworld>& SEntity<0,dim,dimworld>::HierarchicIterator::operator* ()
  {
    return virtual_element;
  }

  template<int dim, int dimworld>
  inline SEntity<0,dim,dimworld>* SEntity<0,dim,dimworld>::HierarchicIterator::operator-> ()
  {
    return &virtual_element;
  }


  //************************************************************************
  // inline methods for NeighborIterator

  template<int dim, int dimworld>
  inline SEntity<0,dim,dimworld>::NeighborIterator SEntity<0,dim,dimworld>::NeighborIterator::operator++ ()
  {
    return SEntity<0,dim,dimworld>::NeighborIterator();
  }

  template<int dim, int dimworld>
  inline SEntity<0,dim,dimworld>::NeighborIterator SEntity<0,dim,dimworld>::NeighborIterator::operator++ (int i)
  {
    return SEntity<0,dim,dimworld>::NeighborIterator();
  }

  template<int dim, int dimworld>
  inline bool SEntity<0,dim,dimworld>::NeighborIterator::operator== (const SEntity<0,dim,dimworld>::NeighborIterator& i) const
  {
    return true;
  }

  template<int dim, int dimworld>
  inline bool SEntity<0,dim,dimworld>::NeighborIterator::operator!= (const SEntity<0,dim,dimworld>::NeighborIterator& i) const
  {
    return false;
  }

  template<int dim, int dimworld>
  inline SEntity<0,dim,dimworld>& SEntity<0,dim,dimworld>::NeighborIterator::operator* ()
  {
    return virtual_element;
  }

  template<int dim, int dimworld>
  inline SEntity<0,dim,dimworld>* SEntity<0,dim,dimworld>::NeighborIterator::operator-> ()
  {
    return &virtual_element;
  }

  template<int dim, int dimworld>
  inline SElement<dim-1,dim> SEntity<0,dim,dimworld>::NeighborIterator::intersection_self_local ()
  {
    return SElement<dim-1,dim>();
  }

  template<int dim, int dimworld>
  inline SElement<dim-1,dimworld> SEntity<0,dim,dimworld>::NeighborIterator::intersection_self_global ()
  {
    return SElement<dim-1,dimworld>();
  }

  template<int dim, int dimworld>
  inline int SEntity<0,dim,dimworld>::NeighborIterator::number_in_self ()
  {
    return 0;
  }

  template<int dim, int dimworld>
  inline SElement<dim-1,dim> SEntity<0,dim,dimworld>::NeighborIterator::intersection_neighbor_local ()
  {
    return SElement<dim-1,dim>();
  }

  template<int dim, int dimworld>
  inline SElement<dim-1,dimworld> SEntity<0,dim,dimworld>::NeighborIterator::intersection_neighbor_global ()
  {
    return SElement<dim-1,dimworld>();
  }

  template<int dim, int dimworld>
  inline int SEntity<0,dim,dimworld>::NeighborIterator::number_in_neighbor ()
  {
    return 0;
  }

  //************************************************************************
  // inline methods for SLevelIterator

  template<int codim, int dim, int dimworld>
  inline SLevelIterator<codim,dim,dimworld> SLevelIterator<codim,dim,dimworld>::operator++ ()
  {
    return SLevelIterator<codim,dim,dimworld>();
  }

  template<int codim, int dim, int dimworld>
  inline SLevelIterator<codim,dim,dimworld> SLevelIterator<codim,dim,dimworld>::operator++ (int i)
  {
    return SLevelIterator<codim,dim,dimworld>();
  }

  template<int codim, int dim, int dimworld>
  inline bool SLevelIterator<codim,dim,dimworld>::operator== (const SLevelIterator<codim,dim,dimworld>& i) const
  {
    return true;
  }

  template<int codim, int dim, int dimworld>
  inline bool SLevelIterator<codim,dim,dimworld>::operator!= (const SLevelIterator<codim,dim,dimworld>& i) const
  {
    return false;
  }

  template<int codim, int dim, int dimworld>
  inline SEntity<codim,dim,dimworld>& SLevelIterator<codim,dim,dimworld>::operator* ()
  {
    return virtual_element;
  }

  template<int codim, int dim, int dimworld>
  inline SEntity<codim,dim,dimworld>* SLevelIterator<codim,dim,dimworld>::operator-> ()
  {
    return &virtual_element;
  }


  //************************************************************************
  // inline methods for SGrid

  template<int dim, int dimworld>
  inline SGrid<dim,dimworld>::SGrid (double H_, int N0_, int L_)
  {
    H = H_; N0 = N0_; L = L_;
  }

  template<int dim, int dimworld>
  inline int SGrid<dim,dimworld>::maxlevel ()
  {
    return L-1;
  }

  template <int dim, int dimworld> template <int codim>
  inline SLevelIterator<codim,dim,dimworld> SGrid<dim,dimworld>::lbegin (int level)
  {
    return SLevelIterator<codim,dim,dimworld>();
  }

  template <int dim, int dimworld> template <int codim>
  inline SLevelIterator<codim,dim,dimworld> SGrid<dim,dimworld>::lend (int level)
  {
    return SLevelIterator<codim,dim,dimworld>();
  }

  template<int dim, int dimworld>
  inline int SGrid<dim,dimworld>::size (int level, int codim)
  {
    return 1;
  }



} // end namespace

#endif
