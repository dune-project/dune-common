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

  // the reference elements as global variables
  SElement<1,1> SGridrefelem1D(Vec<1>(0,0.0),Vec<1>(0,1.0));
  SElement<2,2> SGridrefelem2D(Vec<2>(0,0.0),Vec<2>(0,1.0),Vec<2>(1,1.0));
  SElement<3,3> SGridrefelem3D(Vec<3>(0,0.0),Vec<3>(0,1.0),Vec<3>(1,1.0),Vec<3>(2,1.0));

  template<int dimworld>
  inline SElement<1,dimworld>::SElement (const Vec<dimworld>& s_, const Vec<dimworld>& r0)
  {
    // copy arguments
    s = s_;
    A(0) = r0;

    // make corners
    c[0] = s;
    c[1] = s+A(0);
  }

  template<int dimworld>
  inline SElement<2,dimworld>::SElement (const Vec<dimworld>& s_, const Vec<dimworld>& r0, const Vec<dimworld>& r1)
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
  inline SElement<3,dimworld>::SElement (const Vec<dimworld>& s_, const Vec<dimworld>& r0,
                                         const Vec<dimworld>& r1, const Vec<dimworld>& r2)
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

  template<int dimworld>
  inline SElement<1,dimworld>::SElement (const Vec<dimworld>& s_, Vec<dimworld> r_[1])
  {
    // copy arguments
    s = s_;
    A(0) = r_[0];

    // make corners
    c[0] = s;
    c[1] = s+A(0);
  }

  template<int dimworld>
  inline SElement<2,dimworld>::SElement (const Vec<dimworld>& s_, Vec<dimworld> r_[2])
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
  inline SElement<3,dimworld>::SElement (const Vec<dimworld>& s_, Vec<dimworld> r_[3])
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

  template<int dim, int dimworld>
  inline ElementType SElement<dim,dimworld>::type ()
  {
    switch (dim)
    {
    case 1 : return line;
    case 2 : return quadrilateral;
    case 3 : return hexahedron;
    default : return unknown;
    }
  }

  template<int dim, int dimworld>
  inline int SElement<dim,dimworld>::corners ()
  {
    return 1<<dim;
  }

  template<int dim, int dimworld>
  inline Vec<dimworld>& SElement<dim,dimworld>::operator[] (int i)
  {
    return c[i];
  }

  template<int dim, int dimworld>
  inline SElement<dim,dim>& SElement<dim,dimworld>::refelem ()
  {
    return SElement<dim,dim>();     // this should not happen
  }

  template<int dimworld>
  inline SElement<1,1>& SElement<1,dimworld>::refelem ()
  {
    return SGridrefelem1D;
  }

  template<int dimworld>
  inline SElement<2,2>& SElement<2,dimworld>::refelem ()
  {
    return SGridrefelem2D;
  }

  template<int dimworld>
  inline SElement<3,3>& SElement<3,dimworld>::refelem ()
  {
    return SGridrefelem3D;
  }

  template<int dim, int dimworld>
  inline Vec<dimworld> SElement<dim,dimworld>::global (Vec<dim> local)
  {
    return A*local;
  }

  template<int dim, int dimworld>
  inline Vec<dim> SElement<dim,dimworld>::local (Vec<dimworld> global)
  {
    Vec<dim> l;     // result
    Vec<dimworld> rhs = global-s;
    for (int k=0; k<dim; k++)
      l(k) = (rhs*A(k)) / (A(k)*A(k));
    return l;
  }


  //************************************************************************
  // inline methods for SEntity

  // general
  template<int codim, int dim, int dimworld>
  inline int SEntity<codim,dim,dimworld>::level ()
  {
    return 0;
  }

  template<int codim, int dim, int dimworld>
  inline int SEntity<codim,dim,dimworld>::index ()
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
  inline Vec<dim> SEntity<dim,dim,dimworld>::local ()
  {
    return Vec<dim>();
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
