// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_GRID_CC__
#define __DUNE_GRID_CC__

#include "../../common/misc.hh"
#include <iostream>
#include <fstream>

namespace Dune {

  //************************************************************************
  // R E F E R E N C E T O P O L O G Y
  //************************************************************************

  // the general version, throws only exception ...
  template<int dim, class ct>
  inline ReferenceTopology<dim,ct>::ReferenceTopology ()
  {
    DUNE_THROW(GridError, "dimension too large");
  }

  template<int dim, class ct>
  inline Vec<dim,ct>& ReferenceTopology<dim,ct>::center_codim0_local (int elemtype)
  {
    DUNE_THROW(GridError, "dimension too large");
  }

  template<int dim, class ct>
  inline Vec<dim-1,ct>& ReferenceTopology<dim,ct>::center_codim1_local (int elemtype, int i)
  {
    DUNE_THROW(GridError, "dimension too large");
  }

  // Specialization for dim=1, ElementType=1
  template<class ct>
  inline ReferenceTopology<1,ct>::ReferenceTopology ()
  {
    center0_local[0] = Vec<1,ct>(0.5);
  }

  template<class ct>
  inline Vec<1,ct>& ReferenceTopology<1,ct>::center_codim0_local (int elemtype)
  {
    return center0_local[0];
  }

  template<class ct>
  inline Vec<0,ct>& ReferenceTopology<1,ct>::center_codim1_local (int elemtype, int i)
  {
    return center1_local[0];
  }

  // Specialization for dim=2, ElementType=2,3
  template<class ct>
  inline ReferenceTopology<2,ct>::ReferenceTopology ()
  {
    center0_local[0] = Vec<2,ct>(1.0/3.0);
    center0_local[1] = Vec<2,ct>(0.5);
    center1_local[0] = Vec<1,ct>(0.5);
  }

  template<class ct>
  inline Vec<2,ct>& ReferenceTopology<2,ct>::center_codim0_local (int elemtype)
  {
    return center0_local[elemtype-2];
  }

  template<class ct>
  inline Vec<1,ct>& ReferenceTopology<2,ct>::center_codim1_local (int elemtype, int i)
  {
    return center1_local[0];
  }

  // Specialization for dim=3, ElementType=4..7
  template<class ct>
  inline ReferenceTopology<3,ct>::ReferenceTopology ()
  {
    center0_local[0] = Vec<3,ct>(0.25);
    center0_local[1] = Vec<3,ct>(0.0); // pyramid is missing !
    center0_local[2] = Vec<3,ct>(0.0); // prism is missing
    center0_local[3] = Vec<3,ct>(0.5);
    for (int i=0; i<6; i++) center1_local[0][i] = Vec<2,ct>(1.0/3.0);
    for (int i=0; i<6; i++) center1_local[1][i] = Vec<2,ct>(0.0);
    for (int i=0; i<6; i++) center1_local[2][i] = Vec<2,ct>(0.0);
    for (int i=0; i<6; i++) center1_local[3][i] = Vec<2,ct>(0.5);
  }

  template<class ct>
  inline Vec<3,ct>& ReferenceTopology<3,ct>::center_codim0_local (int elemtype)
  {
    return center0_local[elemtype-4];
  }

  template<class ct>
  inline Vec<2,ct>& ReferenceTopology<3,ct>::center_codim1_local (int elemtype, int i)
  {
    return center1_local[elemtype-4][i];
  }



  //************************************************************************
  // E L E M E N T
  //************************************************************************

  // general case dim!=0
  template<int dim, int dimworld, class ct,template<int,int> class ElementImp>
  inline ElementType Element<dim,dimworld,ct,ElementImp>::type ()
  {
    return asImp().type();
  }

  template<int dim, int dimworld, class ct,template<int,int> class ElementImp>
  inline int Element<dim,dimworld,ct,ElementImp>::corners ()
  {
    return asImp().corners();
  }

  template<int dim, int dimworld, class ct,template<int,int> class ElementImp>
  inline Vec<dimworld,ct>& Element<dim,dimworld,ct,ElementImp>::operator[] (int i)
  {
    return asImp().operator[](i);
  }

  template<int dim, int dimworld, class ct,template<int,int> class ElementImp>
  inline ElementImp<dim,dim>& Element<dim,dimworld,ct,ElementImp>::refelem ()
  {
    return asImp().refelem();
  }

  template<int dim, int dimworld, class ct,template<int,int> class ElementImp>
  inline Vec<dimworld,ct> Element<dim,dimworld,ct,ElementImp>::global (const Vec<dim,ct>& local)
  {
    return asImp().global(local);
  }

  template<int dim, int dimworld, class ct,template<int,int> class ElementImp>
  inline Vec<dim,ct> Element<dim,dimworld,ct,ElementImp>::local (const Vec<dimworld,ct>& global)
  {
    return asImp().local(global);
  }

  template<int dim, int dimworld, class ct,template<int,int> class ElementImp>
  inline bool Element<dim,dimworld,ct,ElementImp>::checkInside (const Vec<dim,ct>& local)
  {
    return asImp().checkInside(local);
  }

  template<int dim, int dimworld, class ct,template<int,int> class ElementImp>
  inline ct Element<dim,dimworld,ct,ElementImp>::integration_element (const Vec<dim,ct>& local)
  {
    return asImp().integration_element(local);
  }

  template<int dim, int dimworld, class ct,template<int,int> class ElementImp>
  inline Mat<dim,dim>& Element<dim,dimworld,ct,ElementImp>::Jacobian_inverse (const Vec<dim,ct>& local)
  {
    return asImp().Jacobian_inverse(local);
  }

  template<int dim, int dimworld, class ct,template<int,int> class ElementImp>
  inline void Element<dim,dimworld,ct,ElementImp>::checkIF ()
  {
    // simply call all members, this forces compiler to compile them ...
    type();

    Vec<dim,ct> l;
    Vec<dimworld,ct> g;

    // check the methods local,global, refelem and checkInside
    for(int i=0; i<corners(); i++)
    {
      Vec<dimworld,ct> & coord = operator[] (i);
      l = local  ( coord );
      if( !checkInside (l) )
      {
        std::cerr << coord;
        std::cerr << " mapping with local() went wrong! \n";
      }

      for(int j=0; j<dim; j++)
      {
        if( l(j) != refelem()[i](j) )
        {
          std::cerr << l; std::cerr << refelem()[i];
          std::cerr << "\nMapping to local coord went wrong! \n";
        }
      }

      g = global ( l );
      for(int j=0; j<dimworld; j++)
      {
        if( g(j) != coord(j) )
        {
          std::cerr << "\nglobal--local of coord " << i << " failed! \n";
          std::cerr << "started with "; std::cerr << coord;
          std::cerr << "and got: ";     std::cerr << g;
          std::cerr << "\n";
        }
      }
    }

    integration_element(l);
    if (dim==dimworld) Jacobian_inverse(l);
  }

  // special case dim=0
  template<int dimworld, class ct,template<int,int> class ElementImp>
  inline ElementType Element<0,dimworld,ct,ElementImp>::type ()
  {
    return asImp().type();
  }

  template<int dimworld, class ct,template<int,int> class ElementImp>
  inline int Element<0,dimworld,ct,ElementImp>::corners ()
  {
    return asImp().corners();
  }

  template<int dimworld, class ct,template<int,int> class ElementImp>
  inline Vec<dimworld,ct>& Element<0,dimworld,ct,ElementImp>::operator[] (int i)
  {
    return asImp().operator[](i);
  }

  template<int dimworld, class ct,template<int,int> class ElementImp>
  inline void Element<0,dimworld,ct,ElementImp>::checkIF ()
  {
    // simply call all members, this forces compiler to compile them ...
    type();
    corners();
    operator[](0);
  }

  //***********************************************************************
  //  E L E M E N T  Default
  //***********************************************************************


  //************************************************************************
  // N E I G H B O R I T E R A T O R
  //************************************************************************

  template<int dim, int dimworld, class ct,
      template<int,int> class IntersectionIteratorImp,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp
      , template<int,int> class BoundaryEntityImp
      >
  inline IntersectionIteratorImp<dim,dimworld>& IntersectionIterator<dim,dimworld,ct,IntersectionIteratorImp,EntityImp,ElementImp,BoundaryEntityImp>::operator++ ()
  {
    return asImp().operator++();
  }

  template<int dim, int dimworld, class ct,
      template<int,int> class IntersectionIteratorImp,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp
      , template<int,int> class BoundaryEntityImp
      >
  inline bool IntersectionIterator<dim,dimworld,ct,IntersectionIteratorImp,EntityImp,ElementImp,BoundaryEntityImp>::operator== (const IntersectionIteratorImp<dim,dimworld>& i) const
  {
    return asImp().operator==(i);
  }

  template<int dim, int dimworld, class ct,
      template<int,int> class IntersectionIteratorImp,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp
      , template<int,int> class BoundaryEntityImp
      >
  inline bool IntersectionIterator<dim,dimworld,ct,IntersectionIteratorImp,EntityImp,ElementImp,BoundaryEntityImp>::operator!= (const IntersectionIteratorImp<dim,dimworld>& i) const
  {
    return asImp().operator!=(i);
  }

  template<int dim, int dimworld, class ct,
      template<int,int> class IntersectionIteratorImp,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp
      , template<int,int> class BoundaryEntityImp
      >
  inline bool IntersectionIterator<dim,dimworld,ct,IntersectionIteratorImp,EntityImp,ElementImp,BoundaryEntityImp>::boundary ()
  {
    return asImp().boundary();
  }

  template<int dim, int dimworld, class ct,
      template<int,int> class IntersectionIteratorImp,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp
      , template<int,int> class BoundaryEntityImp
      >
  inline EntityImp<0,dim,dimworld>& IntersectionIterator<dim,dimworld,ct,IntersectionIteratorImp,EntityImp,ElementImp,BoundaryEntityImp>::operator* ()
  {
    return asImp().operator*();
  }

  template<int dim, int dimworld, class ct,
      template<int,int> class IntersectionIteratorImp,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp
      , template<int,int> class BoundaryEntityImp
      >
  inline EntityImp<0,dim,dimworld>* IntersectionIterator<dim,dimworld,ct,IntersectionIteratorImp,EntityImp,ElementImp,BoundaryEntityImp>::operator-> ()
  {
    return asImp().operator->();
  }

  template<int dim, int dimworld, class ct,
      template<int,int> class IntersectionIteratorImp,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp
      , template<int,int> class BoundaryEntityImp
      >
  inline Vec<dimworld,ct>& IntersectionIterator<dim,dimworld,ct,IntersectionIteratorImp,EntityImp,ElementImp,BoundaryEntityImp>::unit_outer_normal (Vec<dim-1,ct>& local)
  {
    return asImp().unit_outer_normal(local);
  }

  template<int dim, int dimworld, class ct,
      template<int,int> class IntersectionIteratorImp,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp
      , template<int,int> class BoundaryEntityImp
      >
  inline Vec<dimworld,ct>& IntersectionIterator<dim,dimworld,ct,IntersectionIteratorImp,EntityImp,ElementImp,BoundaryEntityImp>::unit_outer_normal ()
  {
    return asImp().unit_outer_normal();
  }

  template<int dim, int dimworld, class ct,
      template<int,int> class IntersectionIteratorImp,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp
      , template<int,int> class BoundaryEntityImp
      >
  inline ElementImp<dim-1,dim>& IntersectionIterator<dim,dimworld,ct,IntersectionIteratorImp,EntityImp,ElementImp,BoundaryEntityImp>::intersection_self_local ()
  {
    return asImp().intersection_self_local();
  }

  template<int dim, int dimworld, class ct,
      template<int,int> class IntersectionIteratorImp,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp
      , template<int,int> class BoundaryEntityImp
      >
  inline ElementImp<dim-1,dimworld>& IntersectionIterator<dim,dimworld,ct,IntersectionIteratorImp,EntityImp,ElementImp,BoundaryEntityImp>::intersection_self_global ()
  {
    return asImp().intersection_self_global();
  }

  template<int dim, int dimworld, class ct,
      template<int,int> class IntersectionIteratorImp,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp
      , template<int,int> class BoundaryEntityImp
      >
  inline int IntersectionIterator<dim,dimworld,ct,IntersectionIteratorImp,EntityImp,ElementImp,BoundaryEntityImp>::number_in_self ()
  {
    return asImp().number_in_self();
  }

  template<int dim, int dimworld, class ct,
      template<int,int> class IntersectionIteratorImp,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp
      , template<int,int> class BoundaryEntityImp
      >
  inline ElementImp<dim-1,dim>& IntersectionIterator<dim,dimworld,ct,IntersectionIteratorImp,EntityImp,ElementImp,BoundaryEntityImp>::intersection_neighbor_local ()
  {
    return asImp().intersection_neighbor_local();
  }

  template<int dim, int dimworld, class ct,
      template<int,int> class IntersectionIteratorImp,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp
      , template<int,int> class BoundaryEntityImp
      >
  inline ElementImp<dim-1,dimworld>& IntersectionIterator<dim,dimworld,ct,IntersectionIteratorImp,EntityImp,ElementImp,BoundaryEntityImp>::intersection_neighbor_global ()
  {
    return asImp().intersection_neighbor_global();
  }

  template<int dim, int dimworld, class ct,
      template<int,int> class IntersectionIteratorImp,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp
      , template<int,int> class BoundaryEntityImp
      >
  inline int IntersectionIterator<dim,dimworld,ct,IntersectionIteratorImp,EntityImp,ElementImp,BoundaryEntityImp>::number_in_neighbor ()
  {
    return asImp().number_in_neighbor();
  }

  template<int dim, int dimworld, class ct,
      template<int,int> class IntersectionIteratorImp,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp
      , template<int,int> class BoundaryEntityImp
      >
  inline void IntersectionIterator<dim,dimworld,ct,IntersectionIteratorImp,EntityImp,ElementImp,BoundaryEntityImp>::checkIF ()
  {
    operator++();
    operator==(asImp());
    operator!=(asImp());
    operator*();
    operator->();
    boundary();
    Vec<dim-1,ct> l;
    unit_outer_normal(l);
    unit_outer_normal();
    intersection_self_local();
    intersection_self_global();
    number_in_self();
    intersection_neighbor_local();
    intersection_neighbor_global();
    number_in_neighbor();
  }

  //************************************************************************
  // N E I G H B O R I T E R A T O R Default
  //************************************************************************
  template<int dim, int dimworld, class ct,
      template<int,int> class IntersectionIteratorImp,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp
      , template<int,int> class BoundaryEntityImp
      >
  inline Vec<dimworld,ct>& IntersectionIteratorDefault<dim,dimworld,ct,IntersectionIteratorImp,EntityImp,ElementImp,BoundaryEntityImp>::outer_normal ()
  {
    // make a copy, is nessasary
    outerNormal_ = asImp().unit_outer_normal();
    outerNormal_ *= asImp().intersection_self_global().integration_element(tmp_);
    return outerNormal_;
  }

  template<int dim, int dimworld, class ct,
      template<int,int> class IntersectionIteratorImp,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp
      , template<int,int> class BoundaryEntityImp
      >
  inline Vec<dimworld,ct>& IntersectionIteratorDefault<dim,dimworld,ct,IntersectionIteratorImp,EntityImp,ElementImp,BoundaryEntityImp>::
  outer_normal (Vec<dim-1,ct>& local)
  {
    // make a copy, is nessasary
    outerNormal_ = asImp().unit_outer_normal(local);
    outerNormal_ *= asImp().intersection_self_global().integration_element(local);
    return outerNormal_;
  }



  //************************************************************************
  // H I E R A R C H I C I T E R A T O R
  //************************************************************************

  template<int dim, int dimworld, class ct,
      template<int,int> class HierarchicIteratorImp,
      template<int,int,int> class EntityImp
      >
  inline HierarchicIteratorImp<dim,dimworld>& HierarchicIterator<dim,dimworld,ct,HierarchicIteratorImp,EntityImp>::operator++ ()
  {
    return asImp().operator++();
  }

  template<int dim, int dimworld, class ct,
      template<int,int> class HierarchicIteratorImp,
      template<int,int,int> class EntityImp
      >
  inline bool HierarchicIterator<dim,dimworld,ct,HierarchicIteratorImp,EntityImp>::operator== (const HierarchicIteratorImp<dim,dimworld>& i) const
  {
    return asImp().operator==(i);
  }

  template<int dim, int dimworld, class ct,
      template<int,int> class HierarchicIteratorImp,
      template<int,int,int> class EntityImp
      >
  inline bool HierarchicIterator<dim,dimworld,ct,HierarchicIteratorImp,EntityImp>::operator!= (const HierarchicIteratorImp<dim,dimworld>& i) const
  {
    return asImp().operator!=(i);
  }

  template<int dim, int dimworld, class ct,
      template<int,int> class HierarchicIteratorImp,
      template<int,int,int> class EntityImp
      >
  inline EntityImp<0,dim,dimworld>& HierarchicIterator<dim,dimworld,ct,HierarchicIteratorImp,EntityImp>::operator* ()
  {
    return asImp().operator*();
  }

  template<int dim, int dimworld, class ct,
      template<int,int> class HierarchicIteratorImp,
      template<int,int,int> class EntityImp
      >
  inline EntityImp<0,dim,dimworld>* HierarchicIterator<dim,dimworld,ct,HierarchicIteratorImp,EntityImp>::operator-> ()
  {
    return asImp().operator->();
  }

  template<int dim, int dimworld, class ct,
      template<int,int> class HierarchicIteratorImp,
      template<int,int,int> class EntityImp
      >
  inline void HierarchicIterator<dim,dimworld,ct,HierarchicIteratorImp,EntityImp>::checkIF ()
  {
    operator++();
    operator==(asImp());
    operator!=(asImp());
    operator*();
    operator->();
  }
  //************************************************************************
  // H I E R A R C H I C I T E R A T O R Default
  //************************************************************************

  //************************************************************************
  // E N T I T Y
  //************************************************************************

  // codim != 0, codim != dim
  template<int codim, int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline int Entity<codim,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::level ()
  {
    return asImp().level();
  }

  template<int codim, int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline int Entity<codim,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::index ()
  {
    return asImp().index();
  }

  template<int codim, int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline PartitionType Entity<codim,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::partition_type ()
  {
    return asImp().partition_type();
  }

  template<int codim, int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline ElementImp<dim-codim,dimworld>& Entity<codim,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::geometry ()
  {
    return asImp().geometry();
  }

  template<int codim, int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline void Entity<codim,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::checkIF ()
  {
    level();
    index();
    geometry();
  }

  // codim 0
  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline int Entity<0,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::level ()
  {
    return asImp().level();
  }

  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline int Entity<0,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::index ()
  {
    return asImp().index();
  }

  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline PartitionType Entity<0,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::partition_type ()
  {
    return asImp().partition_type();
  }

  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline ElementImp<dim,dimworld>& Entity<0,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::geometry ()
  {
    return asImp().geometry();
  }

  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  template<int cc>
  inline int Entity<0,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::count ()
  {
    return asImp().count<cc>();
  }

  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  template<int cc>
  inline LevelIteratorImp<cc,dim,dimworld,All_Partition> Entity<0,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::entity (int i)
  {
    return asImp().entity<cc>(i);
  }

  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline IntersectionIteratorImp<dim,dimworld> Entity<0,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::ibegin ()
  {
    return asImp().ibegin();
  }


  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline IntersectionIteratorImp<dim,dimworld> Entity<0,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::iend ()
  {
    return asImp().iend();
  }


  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline LevelIteratorImp<0,dim,dimworld,All_Partition> Entity<0,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::father ()
  {
    return asImp().father();
  }

  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline ElementImp<dim,dim>& Entity<0,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::father_relative_local ()
  {
    return asImp().father_relative_local();
  }

  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline HierarchicIteratorImp<dim,dimworld> Entity<0,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::hbegin (int maxlevel)
  {
    return asImp().hbegin(maxlevel);
  }

  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline HierarchicIteratorImp<dim,dimworld> Entity<0,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::hend (int maxlevel)
  {
    return asImp().hend(maxlevel);
  }

  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline void Entity<0,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::checkIF ()
  {
    level();
    index();
    geometry();
    count<0>();
    count<dim>();
    entity<0>(0);
    entity<dim>(0);
    ibegin();
    iend();
    father();
    father_relative_local();
    hbegin(0);
    hend(0);
  }

  // codim = dim
  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline int Entity<dim,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::level ()
  {
    return asImp().level();
  }

  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline int Entity<dim,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::index ()
  {
    return asImp().index();
  }

  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline PartitionType Entity<dim,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::partition_type ()
  {
    return asImp().partition_type();
  }

  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline ElementImp<0,dimworld>& Entity<dim,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::geometry ()
  {
    return asImp().geometry();
  }

  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline LevelIteratorImp<0,dim,dimworld,All_Partition> Entity<dim,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::father ()
  {
    return asImp().father();
  }

  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline Vec<dim,ct>& Entity<dim,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::local ()
  {
    return asImp().local();
  }

  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  inline void Entity<dim,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::checkIF ()
  {
    level();
    index();
    geometry();
    father();
    local();
  }
  //************************************************************************
  // E N T I T Y Default codim = 0
  //************************************************************************

  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      > template <int cc>
  inline int EntityDefault <0,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>::subIndex (int i)
  {
    // return index of sub Entity number i
    return (asImp().template entity<cc>(i))->index();
  }


  //************************************************************************
  // L E V E L I T E R A T O R
  //************************************************************************

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype, class ct,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int,int> class EntityImp
      >
  inline LevelIteratorImp<codim,dim,dimworld,pitype>& LevelIterator<codim,dim,dimworld,pitype,ct,LevelIteratorImp,EntityImp>::operator++ ()
  {
    return asImp().operator++();
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype, class ct,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int,int> class EntityImp
      >
  inline bool LevelIterator<codim,dim,dimworld,pitype,ct,LevelIteratorImp,EntityImp>::operator== (const LevelIteratorImp<codim,dim,dimworld,pitype>& i) const
  {
    return asImp().operator==(i);
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype, class ct,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int,int> class EntityImp
      >
  inline bool LevelIterator<codim,dim,dimworld,pitype,ct,LevelIteratorImp,EntityImp>::operator!= (const LevelIteratorImp<codim,dim,dimworld,pitype>& i) const
  {
    return asImp().operator!=(i);
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype, class ct,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int,int> class EntityImp
      >
  inline EntityImp<codim,dim,dimworld>& LevelIterator<codim,dim,dimworld,pitype,ct,LevelIteratorImp,EntityImp>::operator* ()
  {
    return asImp().operator*();
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype, class ct,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int,int> class EntityImp
      >
  inline EntityImp<codim,dim,dimworld>* LevelIterator<codim,dim,dimworld,pitype,ct,LevelIteratorImp,EntityImp>::operator-> ()
  {
    return asImp().operator->();
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype, class ct,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int,int> class EntityImp
      >
  inline int LevelIterator<codim,dim,dimworld,pitype,ct,LevelIteratorImp,EntityImp>::level ()
  {
    return asImp().level();
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype, class ct,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int,int> class EntityImp
      >
  inline void LevelIterator<codim,dim,dimworld,pitype,ct,LevelIteratorImp,EntityImp>::checkIF ()
  {
    operator++();
    operator==(asImp());
    operator!=(asImp());
    operator*();
    operator->();
    level();
  }
  //************************************************************************
  // L E V E L I T E R A T O R Default
  //************************************************************************

  //************************************************************************
  // G R I D
  //************************************************************************

  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  inline int Grid<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::maxlevel () const
  {
    return asImp().maxlevel();
  }

  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  inline int Grid<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::size (int level, int codim) const
  {
    return asImp().size(level,codim);
  }

  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  inline int Grid<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::overlap_size (int level, int codim)
  {
    return asImp().overlap_size(level,codim);
  }

  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  inline int Grid<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::ghost_size (int level, int codim)
  {
    return asImp().ghost_size(level,codim);
  }

  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  template<int codim, PartitionIteratorType pitype>
  inline LevelIteratorImp<codim,dim,dimworld,pitype> Grid<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::lbegin (int level)
  {
    return asImp().template lbegin<codim,pitype>(level);
  }

  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  template<int codim, PartitionIteratorType pitype>
  inline LevelIteratorImp<codim,dim,dimworld,pitype> Grid<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::lend (int level)
  {
    return asImp().template lend<codim,pitype>(level);
  }

  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  template<int codim>
  inline LevelIteratorImp<codim,dim,dimworld,All_Partition> Grid<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::lbegin (int level)
  {
    return asImp().template lbegin<codim>(level);
  }

  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  template<int codim>
  inline LevelIteratorImp<codim,dim,dimworld,All_Partition> Grid<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::lend (int level)
  {
    return asImp().template lend<codim>(level);
  }

  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  template<class T, template<class> class P, int codim>
  inline void Grid<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::communicate (T& t, InterfaceType iftype, CommunicationDirection dir, int level)
  {
    asImp().template communicate<T,P,codim>(t,iftype,dir,level);
  }

  // tester code
  template<int cc>
  struct meta_grid_checkIF {

    template< int dim, int dimworld, class ct, template<int,int> class GridImp,
        template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
    static void f (Grid<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>& g)
    {
      // iterate over codimension cc
      std::cout << "checking LevelIterator with codim=" << cc
                << ", dim=" << dim << ", dimworld=" << dimworld;
      LevelIteratorImp<cc,dim,dimworld,All_Partition> i = g.template lbegin<cc,All_Partition>(0);
      i.checkIF();
      std::cout << " OK."  << std::endl;

      // check entities
      std::cout << "checking Entity with codim=" << cc
                << ", dim=" << dim << ", dimworld=" << dimworld;
      i->checkIF();
      std::cout << " OK."  << std::endl;

      // check elements
      std::cout << "checking Element with dim=" << dim-cc
                << ", dimworld=" << dimworld;
      i->geometry().checkIF();
      std::cout << " OK."  << std::endl;

      // recursive call of template metaprogram
      meta_grid_checkIF<cc-1>::f(g);
    }
  };

  // recursion end via specialization
  template<>
  struct meta_grid_checkIF<0> {
    enum { cc=0 };

    template< int dim, int dimworld, class ct, template<int,int> class GridImp,
        template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
    static void f (Grid<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>& g)
    {
      // iterate over codimension cc
      std::cout << "checking LevelIterator with codim=" << cc
                << ", dim=" << dim << ", dimworld=" << dimworld;

      LevelIteratorImp<cc,dim,dimworld,All_Partition> i = g.template lbegin<cc,All_Partition>(0);
      if( g.size(0,cc) <= 1 )
      {
        std::cerr << "\nUse grid with more entities, at least 2 entities! \n";
        abort();
      }

      // make ++ on i , so we need a least 2 entities
      i.checkIF();
      std::cout << " OK."  << std::endl;

      // check entities
      std::cout << "checking Entity with codim=" << cc
                << ", dim=" << dim << ", dimworld=" << dimworld;
      i->checkIF();
      std::cout << " OK."  << std::endl;

      // check elements
      std::cout << "checking Element with dim=" << dim-cc
                << ", dimworld=" << dimworld;
      i->geometry().checkIF();
      std::cout << " OK."  << std::endl;

      // neighbor iterator
      std::cout << "checking IntersectionIterator with dim=" << dim
                << ", dimworld=" << dimworld;
      i->ibegin().checkIF();
      std::cout << " OK."  << std::endl;

      // hierarchic iterator
      std::cout << "checking HierarchicIterator with dim=" << dim
                << ", dimworld=" << dimworld;
      i->hbegin(0).checkIF();
      std::cout << " OK."  << std::endl;
    }
  };

  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  inline void Grid<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::checkIF ()
  {
    std::cout << "checking Grid with dim=" << dim << ", dimworld=" << dimworld;
    maxlevel();
    for (int i=0; i<=dim; i++) size(i,0);
    std::cout << " OK."  << std::endl;

    // check neighbor iterator, call template metaprogram which iterates over codimension
    meta_grid_checkIF<dim>::f(*this);
  }

  //************************************************************************
  // G R I D Default
  //************************************************************************

  /*
     // return LeafIterator pointing to first leaf entity of maxlevel
     template< int dim, int dimworld, class ct, template<int,int> class GridImp,
     template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
     inline typename GridDefault<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::LeafIterator
     GridDefault<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::leafbegin (int maxLevel )
     {
     // return LeafIterator pointing to first leaf entity of maxlevel
     LeafIterator tmp (asImp(),maxLevel,false);
     return tmp;
     };

     template< int dim, int dimworld, class ct, template<int,int> class GridImp,
     template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
     inline typename GridDefault<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::LeafIterator
     GridDefault<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::leafend (int maxLevel )
     {
     // return LeafIterator pointing behind the last leaf entity of maxlevel
     LeafIterator tmp (asImp(),maxLevel,true);
     return tmp;
     };
   */


  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  inline bool GridDefault<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::
  write ( const FileFormatType ftype, const char * filename , ct time, int timestep,
          bool adaptive, int processor)
  {
    const char *fn;
    const char *path = 0;
    std::fstream file (filename,std::ios::out);
    file << asImp().type() << " " << ftype;

    fn = genFilename(path,filename,timestep);
    file.close();
    switch (ftype)
    {
    case xdr  :   return asImp().template writeGrid<xdr>(fn,time);
    case ascii :   return asImp().template writeGrid<ascii>(fn,time);
    case pgm  :   return asImp().template writeGrid<pgm>(fn,time);
    default :
    {
      std::cerr << ftype << " FileFormatType not supported at the moment! \n";
      abort();
      return false;
    }
    }
    return false;
  } // end grid2File

  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  inline bool GridDefault<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::
  read ( const char * filename , ct & time, int timestep ,
         bool adaptive, int processor )
  {
    const char * fn;
    std::fstream file (filename,std::ios::in);
    GridIdentifier type;
    int helpType;

    file >> helpType;
    type = ( GridIdentifier ) helpType;
    if(type != asImp().type())
    {
      std::cerr << "Cannot read different GridIdentifier!\n";
      assert(type == asImp().type());
      abort();
    }

    file >> helpType;
    FileFormatType ftype = (FileFormatType) helpType;

    const char *path = 0;
    fn = genFilename(path,filename,timestep);
    printf("Read file: filename = `%s' \n",fn);
    file.close();

    switch (ftype)
    {
    case xdr  :   return asImp().template readGrid<xdr>  (fn,time);
    case ascii :   return asImp().template readGrid<ascii>(fn,time);
    case pgm  :   return asImp().template readGrid<pgm>  (fn,time);
    default :
    {
      std::cerr << ftype << " FileFormatType not supported at the moment! \n";
      abort();
      return false;
    }
    }
    return false;
  } // end file2Grid




  //************************************************************************
  //  G R I D Default :: LeafIterator
  //************************************************************************
  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  inline GridDefault<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::
  LeafIterator::LeafIterator (GridType &grid, int maxlevel, bool end) :
    it_ (0) , endit_ (0) , hierit_(0) , endhierit_(0)
    , en_ (0) , goNextMacroEntity_(false) , built_(false)
    , useHierarchic_ (false) , end_ ( end ) , maxLev_ ( maxlevel )
  {
    if(!end_)
    {
      it_    = new LevelIterator ( grid.template lbegin<0>( 0 ) );
      endit_ = new LevelIterator ( grid.template lend<0>( 0 ) );

      goNextMacroEntity_ = false;
      built_ = false;

      en_ = & (*it_[0]);

      if(maxLev_ == 0)
      {
        useHierarchic_ = false;
        goNextMacroEntity_ = true;
      }
      else
      {
        useHierarchic_ = true;
      }

      if(useHierarchic_)
      {
        if(en_->hasChildren())
        {
          hierit_ = new HierIterator ( en_->hbegin( maxLev_ ) );
          endhierit_ = new HierIterator ( en_->hend( maxLev_ ) );
          built_ = true;
          en_ = & (*hierit_[0]);
        }

        if( ((en_->level() < maxLev_) && ( en_->hasChildren() )))
          en_ = goNextEntity();
      }
    }
  } // end Constructor LeafIterator

  // Desctructor
  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  inline GridDefault<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::
  LeafIterator::~LeafIterator ()
  {
    if(it_) delete it_;
    if(endit_) delete endit_;
    if(hierit_) delete hierit_;
    if(endhierit_) delete endhierit_;
  }

  // operator ++, i.e. goNextEntity
  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  inline typename GridDefault<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::LeafIterator &
  GridDefault<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::LeafIterator::operator ++ ()
  {
    en_ = goNextEntity();
    return (*this);
  }

  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  inline typename GridDefault<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::LeafIterator &
  GridDefault<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::LeafIterator::operator ++ (int i)
  {
    for(int j=0; j<i; j++)
      en_ = goNextEntity();

    return (*this);
  }

  // operator ==
  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  inline bool GridDefault<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::
  LeafIterator::operator == (const LeafIterator& i) const
  {
    return end_ == i.end_;
  }
  // operator !=
  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  inline bool GridDefault<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::
  LeafIterator::operator != (const LeafIterator& i) const
  {
    return end_ != i.end_;
  }

#if 0
  // operator * ()
  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  inline EntityImp<0,dim,dimworld> &
  GridDefault<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::LeafIterator::operator*()
  {
    return (*en_);
  }

  // operator -> ()
  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  inline EntityImp<0,dim,dimworld> *
  GridDefault<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::LeafIterator::operator * ()
  {
    return en_;
  }

#endif

  // level
  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  inline int GridDefault<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::
  LeafIterator::level ()
  {
    return en_->level();
  }

  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  inline EntityImp<0,dim,dimworld> *
  GridDefault<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::LeafIterator::goNextEntity  ()
  {
    if(goNextMacroEntity_)
    {
      if( it_[0] == endit_[0] )
      {
        end_ = true;
        return 0;
      }

      ++it_[0];

      if( it_[0] == endit_[0] )
      {
        end_ = true;
        return 0;
      }

      en_ = & (*it_[0]);
      goNextMacroEntity_ = false;
    }
    if(useHierarchic_)
    {
      if( !built_ )
      {
        hierit_ = new HierIterator ( en_->hbegin( maxLev_ ) );
        endhierit_ = new HierIterator ( en_->hend( maxLev_ ) );
        built_ = true;
        en_ = & ( *hierit_[0]);
      }
      else
      {
        ++hierit_[0];
        if(hierit_[0] == endhierit_[0] )
        {
          built_ = false;
          goNextMacroEntity_ = true;
          delete hierit_;    hierit_ = 0;
          delete endhierit_; endhierit_ = 0;

          en_ = 0;

          return goNextEntity();
        }
        else
        {
          en_ = & (*hierit_[0]);
        }
      }

      if( ((en_->level() < maxLev_) && ( en_->hasChildren() )))
        return goNextEntity();
    }
    else
    {
      goNextMacroEntity_ = true;
    }

    return en_;
  }
  // end class LeafIterator


} // end namespace Dune

#endif
