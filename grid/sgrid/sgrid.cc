// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_SGRID_CC
#define DUNE_SGRID_CC

#include <algorithm>
#include <iostream>
#include <assert.h>

#include <dune/common/stdstreams.hh>
#include <dune/common/typetraits.hh>

namespace Dune {


  //************************************************************************
  // SGeometry

#if 0
  // members for SGeometry, general case dim!=0
  template<int mydim, int cdim, class GridImp>
  inline SGeometry<mydim,cdim,GridImp>::SGeometry (bool b)
  {
    builtinverse = false;
    if (!b) return;

    // copy arguments
    s = 0.0;
    for (int j=0; j<cdim; j++) {
      // make unit vectors
      A[j]   = FieldVector<sgrid_ctype, cdim>(0.0);
      A[j][j] = 1.0;
    }

    // make corners
    for (int i=0; i<(1<<mydim); i++)     // there are 2^d corners
    {
      // use binary representation of corner number to assign corner coordinates
      int mask = 1;
      c[i] = s;
      for (int k=0; k<cdim; k++)
      {
        if (i&mask) c[i] = c[i]+A[k];
        mask = mask<<1;
      }
    }
  }
#endif

  template<int mydim, int cdim, class GridImp>
  inline void SGeometry<mydim,cdim,GridImp>::make(FieldMatrix<sgrid_ctype,mydim+1,cdim>& __As)
  {
    // clear jacobian
    builtinverse = false;

    // copy arguments
    s = __As[mydim];
    for (int j=0; j<mydim; j++) A[j] = __As[j];

    // make corners
    for (int i=0; i<(1<<mydim); i++)     // there are 2^d corners
    {
      // use binary representation of corner number to assign corner coordinates
      int mask = 1;
      c[i] = s;
      for (int k=0; k<cdim; k++)
      {
        if (i&mask) c[i] = c[i]+A[k];
        mask = mask<<1;
      }
    }
  }

  template<int mydim, int cdim, class GridImp>
  inline GeometryType SGeometry<mydim,cdim,GridImp>::type () const
  {
    return cube;
  }

  template<int mydim, int cdim, class GridImp>
  inline int SGeometry<mydim,cdim,GridImp>::corners () const
  {
    return 1<<mydim;
  }

  template<int mydim, int cdim, class GridImp>
  inline const FieldVector<sgrid_ctype, cdim>& SGeometry<mydim,cdim,GridImp>::operator[] (int i) const
  {
    return c[i];
  }

  template<int mydim, int cdim, class GridImp>
  inline FieldVector<sgrid_ctype, cdim> SGeometry<mydim,cdim,GridImp>::global (const FieldVector<sgrid_ctype, mydim>& local) const
  {
    FieldVector<sgrid_ctype, cdim> global = s;
    // global += A^t * local
    A.umtv(local,global);

    return global;
  }

  template<int mydim, int cdim, class GridImp>
  inline FieldVector<sgrid_ctype, mydim> SGeometry<mydim,cdim,GridImp>::local (const FieldVector<sgrid_ctype, cdim>& global) const
  {
    FieldVector<sgrid_ctype, mydim> l;     // result
    FieldVector<sgrid_ctype, cdim> rhs = global-s;
    for (int k=0; k<mydim; k++)
      l[k] = (rhs*A[k]) / (A[k]*A[k]);
    return l;
  }

  template<int mydim, int cdim, class GridImp>
  inline bool SGeometry<mydim,cdim,GridImp>::checkInside (const FieldVector<sgrid_ctype, mydim>& local) const
  {
    // check wether they are in the reference element
    for(int i=0; i<mydim; i++)
    {
      if((local[i] < 0.0) || (local[i] > 1.0 ))
        return false;
    }
    return true;
  }

  template<int mydim, int cdim, class GridImp>
  inline sgrid_ctype SGeometry<mydim,cdim,GridImp>::integrationElement (const FieldVector<sgrid_ctype, mydim>& local) const
  {
    sgrid_ctype s = 1.0;
    for (int j=0; j<mydim; j++) s *= A[j].one_norm();

    return s;
  }

  template<int mydim, int cdim, class GridImp>
  inline const FieldMatrix<sgrid_ctype,mydim,mydim>& SGeometry<mydim,cdim,GridImp>::jacobianInverseTransposed (const FieldVector<sgrid_ctype, mydim>& local) const
  {
    for (int i=0; i<mydim; ++i)
      for (int j=0; j<mydim; ++j)
        Jinv[j][i] = A[j][i];
    for (int i=0; i<mydim; i++) Jinv[i][i] = 1.0/Jinv[i][i];
    if (!builtinverse)
    {
      builtinverse = true;
    }
    return Jinv;
  }

  template<int mydim, int cdim, class GridImp>
  inline void SGeometry<mydim,cdim,GridImp>::print (std::ostream& ss, int indent) const
  {
    for (int k=0; k<indent; k++) ss << " ";ss << "SGeometry<" << mydim << "," << cdim << ">" << std::endl;
    for (int k=0; k<indent; k++) ss << " ";ss << "{" << std::endl;
    for (int k=0; k<indent+2; k++) ss << " ";ss << "Position: " << s << std::endl;
    for (int j=0; j<mydim; j++)
    {
      for (int k=0; k<indent+2; k++) ss << " ";
      ss << "direction " << j << "  " << A(j) << std::endl;
    }
    for (int j=0; j<1<<mydim; j++)
    {
      for (int k=0; k<indent+2; k++) ss << " ";
      ss << "corner " << j << "  " << c[j] << std::endl;
    }
    if (builtinverse)
    {
      for (int k=0; k<indent+2; k++) ss << " ";ss << "Jinv ";
      Jinv.print(ss,indent+2);
    }
    for (int k=0; k<indent+2; k++) ss << " ";ss << "builtinverse " << builtinverse << std::endl;
    for (int k=0; k<indent; k++) ss << " ";ss << "}";
  }

#if 0
  // special case dim=0
  template<int cdim, class GridImp>
  inline SGeometry<0,cdim,GridImp>::SGeometry (bool b)
  {
    if (!b) return;
    s = 0.0;
  }
#endif

  template<int cdim, class GridImp>
  inline void SGeometry<0,cdim,GridImp>::make (FieldMatrix<sgrid_ctype,1,cdim>& __As)
  {
    s = __As[0];
  }

  template<int cdim, class GridImp>
  inline GeometryType SGeometry<0,cdim,GridImp>::type () const
  {
    return cube;
  }

  template<int cdim, class GridImp>
  inline int SGeometry<0,cdim,GridImp>::corners () const
  {
    return 1;
  }

  template<int cdim, class GridImp>
  inline const FieldVector<sgrid_ctype, cdim>& SGeometry<0,cdim,GridImp>::operator[] (int i) const
  {
    return s;
  }

  template<int cdim, class GridImp>
  inline void SGeometry<0,cdim,GridImp>::print (std::ostream& ss, int indent) const
  {
    for (int i=0; i<indent; i++) ss << " ";
    ss << "SGeometry<0," << cdim << "> at position " << s;
  }

  //************************************************************************
  // inline methods for SEntityBase

  template<int n>
  static inline FixedArray<int,n>& coarsen (FixedArray<int,n>& in)
  {
    for (int i=0; i<n; i++) in[i] = in[i]/2;
    return in;
  }

  template<int codim, int dim, class GridImp>
  inline SEntityBase<codim,dim,GridImp>::SEntityBase (GridImp* _grid, int _l, int _id)
  {
    grid = _grid;
    l = _l;
    id = _id;
    z = grid->z(_l,_id,codim);
    builtgeometry = false;
  }

  template<int codim, int dim, class GridImp>
  inline SEntityBase<codim,dim,GridImp>::SEntityBase ()
  {
    builtgeometry = false;
  }

  template<int codim, int dim, class GridImp>
  inline void SEntityBase<codim,dim,GridImp>::make (GridImp* _grid, int _l, int _id)
  {
    grid = _grid;
    l = _l;
    id = _id;
    z = grid->z(_l,_id,codim);
    builtgeometry = false;
  }

  template<int codim, int dim, class GridImp>
  inline void SEntityBase<codim,dim,GridImp>::make (int _l, int _id)
  {
    l = _l;
    id = _id;
    z = grid->z(_l,_id,codim);
    builtgeometry = false;
  }

  template<int codim, int dim, class GridImp>
  inline int SEntityBase<codim,dim,GridImp>::globalIndex () const
  {
    int ind = 0;
    for(int i=0; i<this->l; i++)
      ind += this->grid->size(i,codim);
    return ind+this->index();
  }

  template<int codim, int dim, class GridImp>
  inline const typename GridImp::template Codim<codim>::Geometry& SEntityBase<codim,dim,GridImp>::geometry () const
  {
    if (builtgeometry) return geo;

    // find dim-codim direction vectors and reference point
    FieldMatrix<sgrid_ctype,dim-codim+1,dimworld> __As;

    // count number of direction vectors found
    int dir=0;
    FieldVector<sgrid_ctype, dim> p1,p2;
    FixedArray<int,dim> t=z;

    // check all directions
    for (int i=0; i<dim; i++)
      if (t[i]%2==1)
      {
        // coordinate i is odd => gives one direction vector
        t[i] += 1;                 // direction i => even
        p2 = grid->pos(l,t);
        t[i] -= 2;                 // direction i => even
        p1 = grid->pos(l,t);
        t[i] += 1;                 // revert t to original state
        __As[dir] = p2-p1;
        dir++;
      }

    // find reference point, subtract 1 from all odd directions
    for (int i=0; i<dim; i++)
      if (t[i]%2==1)
        t[i] -= 1;
    __As[dir] =grid->pos(l,t);     // all components of t are even

    // make element
    geo.make(__As);
    builtgeometry = true;
    // return result
    return geo;
  }

  //************************************************************************
  // inline methods for SEntity

  // singleton holding mapper of unit cube
  template<int dim>
  struct SUnitCubeMapper {
    static CubeMapper<dim> mapper;      // one cube per direction
  };

  // initialize static variable with bool constructor (which makes reference elements)
  template<int dim>
  CubeMapper<dim> SUnitCubeMapper<dim>::mapper(FixedArray<int,dim>(1));


  // codim 0
  template<int dim, class GridImp> template<int cc>
  inline int SEntity<0,dim,GridImp>::count () const
  {
    return SUnitCubeMapper<dim>::mapper.elements(cc);
  }

  // subentity construction
  template<int dim, class GridImp> template<int cc>
  inline typename SEntity<0,dim,GridImp>::template Codim<cc>::EntityPointer SEntity<0,dim,GridImp>::entity (int i) const
  {
    // find expanded coordinates of entity in reference cube
    // has components in {0,1,2}
    FixedArray<int,dim> zref = SUnitCubeMapper<dim>::mapper.z(i,cc);

    // compute expanded coordinates of entity in global coordinates
    FixedArray<int,dim> zentity;
    for (int i=0; i<dim; i++) zentity[i] = this->z[i] + zref[i] - 1;

    // make Iterator
    return SLevelIterator<cc,All_Partition,const GridImp>(this->grid,this->l,(this->grid)->n(this->l,zentity));
  }

  // default implementation uses entity method
  template<int dim, class GridImp> template<int cc>
  inline int SEntity<0,dim,GridImp>::subIndex (int i) const
  {
    if(cc == dim) // the vertex case
    {
      // find expanded coordinates of entity in reference cube
      // has components in {0,1,2}
      // the grid hold the memory because its faster
      FixedArray<int,dim> &zref = this->grid->zrefStatic;
      FixedArray<int,dim> &zentity = this->grid->zentityStatic;

      zref = SUnitCubeMapper<dim>::mapper.z(i,dim);
      for (int i=0; i<dim; i++) zentity[i] = this->z[i] + zref[i] - 1;
      return this->grid->n(this->l,zentity);
    }
    else
    {
      return entity<cc>(i)->index();
    }
  }

  template<int dim, class GridImp>
  inline typename SEntity<0,dim,GridImp>::IntersectionIterator SEntity<0,dim,GridImp>::ibegin () const
  {
    return IntersectionIterator(SIntersectionIterator<GridImp>(this->grid,this,0));
  }

  template<int dim, class GridImp>
  inline typename SEntity<0,dim,GridImp>::IntersectionIterator SEntity<0,dim,GridImp>::iend () const
  {
    return IntersectionIterator(SIntersectionIterator<GridImp>(this->grid,this,count<1>()));
  }

  template<int dim, class GridImp>
  inline void SEntity<0,dim,GridImp>::make_father () const
  {
    // check level
    if (this->l<=0)
    {
      father_id = 0;
      built_father = true;
      return;
    }

    // reduced coordinates from expanded coordinates
    FixedArray<int,dim> zz = this->grid->compress(this->l,this->z);

    // look for odd coordinates
    FieldVector<sgrid_ctype, dim> delta;
    for (int i=0; i<dim; i++)
      if (zz[i]%2)
      {
        // component i is odd
        zz[i] -= 1;
        zz[i] /= 2;
        delta[i] = 1.0;
      }
      else
      {
        // component i is even
        zz[i] /= 2;
        delta[i] = 0.0;
      }

    // zz is now the reduced coordinate of the father, compute id
    int partition = this->grid->partition(this->l,this->z);
    father_id = this->grid->n((this->l)-1,this->grid->expand((this->l)-1,zz,partition));

    // now make a subcube of size 1/2 in each direction
    FieldMatrix<sgrid_ctype,dim+1,dimworld> __As;
    FieldVector<sgrid_ctype, dim> v;
    for (int i=0; i<dim; i++)
    {
      v = 0.0; v[i] = 0.5;
      __As[i] = v;
    }
    for (int i=0; i<dim; i++) v[i] = 0.5*delta[i];
    __As[dim] =v;
    in_father_local.make(__As);     // build geometry

    built_father = true;
  }

  template<int dim, class GridImp>
  inline typename SEntity<0,dim,GridImp>::EntityPointer SEntity<0,dim,GridImp>::father () const
  {
    if (!built_father) make_father();
    if (this->l>0)
      return SLevelIterator<0,All_Partition,const GridImp>((this->grid),(this->l)-1,father_id);
    else
      return SLevelIterator<0,All_Partition,const GridImp>((this->grid),this->l,this->id);
  }

  template<int dim, class GridImp>
  inline
  const typename GridImp::template Codim<0>::Geometry&
  SEntity<0,dim,GridImp>::geometryInFather () const
  {
    if (!built_father) make_father();
    return in_father_local;
  }

  template<int dim, class GridImp>
  inline typename SEntity<0,dim,GridImp>::HierarchicIterator SEntity<0,dim,GridImp>::hbegin (int maxLevel) const
  {
    return HierarchicIterator(SHierarchicIterator<GridImp>(this->grid,*this,maxLevel,false));
  }

  template<int dim, class GridImp>
  inline typename SEntity<0,dim,GridImp>::HierarchicIterator SEntity<0,dim,GridImp>::hend (int maxLevel) const
  {
    return HierarchicIterator(SHierarchicIterator<GridImp>(this->grid,*this,maxLevel,true));
  }


  // codim dim
  template<int dim, class GridImp>
  inline void SEntity<dim,dim,GridImp>::make_father () const
  {
    // check level
    if (this->l<=0)
    {
      father_id = 0;
      built_father = true;
      return;
    }

    // reduced coordinates from expanded coordinates
    // reduced coordinates of a fine grid vertex can be interpreted as
    // expanded coordinates on the next coarser level !
    FixedArray<int,dim> zz = this->grid->compress(this->l,this->z);

    // to find father, make all coordinates odd
    FieldVector<sgrid_ctype, dim> delta;
    for (int i=0; i<dim; i++)
      if (zz[i]%2)
      {
        // component i is odd
        delta[i] = 0.0;
      }
      else
      {
        // component i is even
        if (zz[i]>0)
        {
          zz[i] -= 1;                       // now it is odd and >= 1
          delta[i] = 0.5;
        }
        else
        {
          zz[i] += 1;                       // now it is odd and >= 1
          delta[i] = -0.5;
        }
      }

    // zz is now an expanded coordinate on the coarse grid
    father_id = this->grid->n((this->l)-1,zz);

    // compute the local coordinates in father
    in_father_local = 0.5;
    for (int i=0; i<dim; i++) in_father_local[i] += delta[i];

    built_father = true;
  }

  template<int dim, class GridImp>
  inline typename SEntity<dim,dim,GridImp>::EntityPointer SEntity<dim,dim,GridImp>::ownersFather () const
  {
    if (!built_father) make_father();
    if (this->l>0)
      return EntityPointer(SLevelIterator<0,All_Partition,GridImp>((this->grid),(this->l)-1,father_id));
    else
      return EntityPointer(SLevelIterator<0,All_Partition,GridImp>((this->grid),this->l,this->id));
  }

  template<int dim, class GridImp>
  inline const FieldVector<sgrid_ctype, dim>& SEntity<dim,dim,GridImp>::positionInOwnersFather () const
  {
    if (!built_father) make_father();
    return in_father_local;
  }

  //************************************************************************
  // inline methods for HierarchicIterator

  template<class GridImp>
  inline void SHierarchicIterator<GridImp>::push_sons (int level, int fatherid)
  {
    // check level
    if (level+1>maxLevel) return;     // nothing to do

    // compute reduced coordinates of element
    FixedArray<int,dim> z =
      this->grid->z(level,fatherid,0);      // expanded coordinates from id
    FixedArray<int,dim> zred =
      this->grid->compress(level,z);     // reduced coordinates from expaned coordinates

    // refine to first son
    for (int i=0; i<dim; i++) zred[i] = 2*zred[i];

    // generate all \f$2^{dim}\f$ sons
    int partition = this->grid->partition(level,z);
    for (int b=0; b<(1<<dim); b++)
    {
      FixedArray<int,dim> zz = zred;
      for (int i=0; i<dim; i++)
        if (b&(1<<i)) zz[i] += 1;
      // zz is reduced coordinate of a son on level level+1
      int sonid = this->grid->n(level+1,this->grid->expand(level+1,zz,partition));

      // push son on stack
      SHierarchicStackElem son(level+1,sonid);
      //stack.push(StackElem(level+1,sonid));
      stack.push(son);
    }
  }

  template<class GridImp>
  inline SHierarchicIterator<GridImp>::SHierarchicIterator (GridImp* _grid,
                                                            const SEntity<0,GridImp::dimension,GridImp>& _e,
                                                            int _maxLevel, bool makeend) :
    Dune::SEntityPointer<0,GridImp>(_grid,_e.level(),_e.index())
  {
    // without sons, we are done
    // (the end iterator is equal to the calling iterator)
    if (makeend) return;

    // remember element where begin has been called
    orig_l = this->e.level();
    orig_id = _grid->template getRealEntity<0>(this->e).index();

    // push original element on stack
    SHierarchicStackElem originalElement(orig_l, orig_id);
    stack.push(originalElement);

    // compute maxLevel
    maxLevel = std::min(_maxLevel,this->grid->maxLevel());

    // ok, push all the sons as well
    push_sons(orig_l,orig_id);

    // and pop the first son
    increment();
  }

  template<class GridImp>
  inline void SHierarchicIterator<GridImp>::increment ()
  {
    // check empty stack
    if (stack.empty()) return;

    // OK, lets pop
    SHierarchicStackElem newe = stack.pop();
    this->l = newe.l;
    this->id = newe.id;
    this->e.make(this->l,this->id);     // here is our new element

    // push all sons of this element if it is not the original element
    if (newe.l!=orig_l || newe.id!=orig_id)
      push_sons(newe.l,newe.id);
  }

  //************************************************************************
  // inline methods for IntersectionIterator

  template<class GridImp>
  inline void SIntersectionIterator<GridImp>::make (int _count) const
  {
    // reset cache flags
    built_intersections = false;
    valid_nb = false;
    valid_count = false;

    // start with given neighbor
    count = _count;

    // check if count is valid
    if (count<0 || count>=self.e.template count<1>())
    {
      ne.id = -1;
      return;     // done, this is end iterator
    }
    valid_count = true;

    // and compute compressed coordinates of neighbor
    FixedArray<int,dim> zrednb = zred;
    if (count%2)
      zrednb[count/2] += 1;           // odd
    else
      zrednb[count/2] -= 1;           // even

    // now check if neighbor exists
    is_on_boundary = !ne.grid->exists(self.l,zrednb);
    if (is_on_boundary)
    {
      ne.id = -1;
      return;     // ok, done it
    }

    // now neighbor is in the grid and must be initialized.
    // First compute its id
    ne.id = ne.grid->n(self.l,
                       ne.grid->expand(self.l,zrednb,partition));

    //        ne.id = id;
    //        ne.l = l;
    ne.e.make(ne.l,ne.id);
  }

  template<class GridImp>
  inline SIntersectionIterator<GridImp>::SIntersectionIterator
    (GridImp* _grid, const SEntity<0,dim,GridImp>* _self, int _count) :
    self(*_self), ne(self),
    //    grid(_grid), l(_self->l), id(_self->id),
    partition(_grid->partition(ne.l,_self->z)),
    zred(_grid->compress(ne.l,_self->z))
  {
    // make neighbor
    make(_count);
  }

  template<class GridImp>
  inline bool SIntersectionIterator<GridImp>::equals (const SIntersectionIterator<GridImp>& i) const
  {
    return (self.id==i.self.id)&&(self.l==i.self.l)&&(count==i.count);
  }

  template<class GridImp>
  inline int SIntersectionIterator<GridImp>::level () const
  {
    return ne.l;
  }

  template<class GridImp>
  inline typename SIntersectionIterator<GridImp>::EntityPointer SIntersectionIterator<GridImp>::inside () const
  {
    return self;
  }

  template<class GridImp>
  inline typename SIntersectionIterator<GridImp>::EntityPointer SIntersectionIterator<GridImp>::outside () const
  {
    return ne;
  }

  template<class GridImp>
  inline void SIntersectionIterator<GridImp>::increment ()
  {
    count++;
    make(count);
  }

  template<class GridImp>
  inline bool SIntersectionIterator<GridImp>::boundary () const
  {
    return is_on_boundary;
  }

  template<class GridImp>
  inline bool SIntersectionIterator<GridImp>::neighbor () const
  {
    return (!is_on_boundary);
  }

  template<class GridImp>
  inline void SIntersectionIterator<GridImp>::makeintersections () const
  {
    if (built_intersections) return;     // already done
    if (!valid_count) return;     // nothing to do

    // compute direction and value in direction
    int dir = count/2;
    int c = count%2;

    // compute expanded coordinates of entity
    FixedArray<int,dim> z1 = self.grid->template getRealEntity<0>(self.e).z;
    if (c==1)
      z1[dir] += 1;           // odd
    else
      z1[dir] -= 1;           // even

    // z1 is even in direction dir, all others must be odd because it is codim 1
    FieldMatrix<sgrid_ctype,dim,dim> __As;
    FieldVector<sgrid_ctype, dim> p1,p2;
    int t;

    // local coordinates in self
    p1 = 0.0;
    p1[dir] = c;        // all points have p[dir]=c in entity
    __As[dim-1] = p1;     // position vector
    t = 0;
    for (int i=0; i<dim; ++i)     // this loop makes dim-1 direction vectors
      if (i!=dir)
      {
        // each i!=dir gives one direction vector
        p2 = p1;
        p2[i] = 1.0;
        __As[t] = p2-p1;                 // a direction vector
        ++t;
      }
    is_self_local.make(__As);     // build geometry

    // local coordinates in neighbor
    p1 = 0.0;
    p1[dir] = 1-c;        // all points have p[dir]=1-c in entity
    __As[dim-1] = p1;       // position vector
    t = 0;
    for (int i=0; i<dim; ++i)     // this loop makes dim-1 direction vectors
      if (i!=dir)
      {
        // each i!=dir gives one direction vector
        p2 = p1;
        p2[i] = 1.0;
        __As[t] = p2-p1;                 // a direction vector
        ++t;
      }
    is_nb_local.make(__As);     // build geometry

    // global coordinates
    t = 0;
    for (int i=0; i<dim; i++)
      if (i!=dir)
      {
        // each i!=dir gives one direction vector
        z1[i] += 1;                 // direction i => even
        p2 = ne.grid->pos(self.level(),z1);
        z1[i] -= 2;                 // direction i => even
        p1 = ne.grid->pos(self.level(),z1);
        z1[i] += 1;                 // revert t to original state
        __As[t] = p2-p1;
        ++t;
      }
    for (int i=0; i<dim; i++)
      if (i!=dir)
        z1[i] -= 1;
    __As[t] = ne.grid->pos(self.level(),z1);
    is_global.make(__As);     // build geometry

    built_intersections = true;
  }

  template<class GridImp>
  inline typename SIntersectionIterator<GridImp>::LocalGeometry& SIntersectionIterator<GridImp>::intersectionSelfLocal () const
  {
    makeintersections();
    return is_self_local;
  }

  template<class GridImp>
  inline typename SIntersectionIterator<GridImp>::LocalGeometry& SIntersectionIterator<GridImp>::intersectionNeighborLocal () const
  {
    makeintersections();
    return is_nb_local;
  }

  template<class GridImp>
  inline typename SIntersectionIterator<GridImp>::Geometry& SIntersectionIterator<GridImp>::intersectionGlobal () const
  {
    makeintersections();
    return is_global;
  }

  template<class GridImp>
  inline int SIntersectionIterator<GridImp>::numberInSelf () const
  {
    return count;
  }

  template<class GridImp>
  inline int SIntersectionIterator<GridImp>::numberInNeighbor () const
  {
    return (count/2)*2 + (1-count%2);
  }

  template<class GridImp>
  inline FieldVector<typename GridImp::ctype, GridImp::dimensionworld>
  SIntersectionIterator<GridImp>::unitOuterNormal (const FieldVector<typename GridImp::ctype, GridImp::dimension-1>& local) const
  {
    // while we are at it, compute normal direction
    FieldVector<sgrid_ctype, dimworld> normal(0.0);
    if (count%2)
      normal[count/2] =  1.0; // odd
    else
      normal[count/2] = -1.0; // even

    return normal;
  }
  //************************************************************************
  // inline methods for SLevelIterator

  template<int codim, PartitionIteratorType pitype, class GridImp>
  inline void SLevelIterator<codim,pitype,GridImp>::increment ()
  {
    this->id++;
    this->e.make(this->l,this->id);
  }

  //************************************************************************
  // inline methods for SEntityPointer

  template<int codim, class GridImp>
  inline bool SEntityPointer<codim,GridImp>::equals (const SEntityPointer<codim,GridImp>& i) const
  {
    return (id==i.id)&&(l==i.l)&&(grid==i.grid);
  }

  template<int codim, class GridImp>
  inline typename SEntityPointer<codim,GridImp>::Entity& SEntityPointer<codim,GridImp>::dereference () const
  {
    return e;
  }

  template<int codim, class GridImp>
  inline int SEntityPointer<codim,GridImp>::level () const
  {
    return l;
  }


  //************************************************************************
  // inline methods for SGrid
  template<int dim, int dimworld>
  inline void SGrid<dim,dimworld>::makeSGrid (const int* N_,
                                              const sgrid_ctype* L_, const sgrid_ctype* H_)
  {
    IsTrue< dimworld <= std::numeric_limits<int>::digits >::yes();

    L = 1;
    for (int i=0; i<dim; i++) low[i] = L_[i];
    for (int i=0; i<dim; i++) H[i] = H_[i];
    for (int i=0; i<dim; i++) N[0][i] = N_[i];

    // define coarse mesh
    mapper[0].make(N[0]);
    for (int i=0; i<dim; i++)
      h[0][i] = (H[i]-low[i])/((sgrid_ctype)N[0][i]);

    dinfo << "level=" << L-1 << " size=(" << N[L-1][0];
    for (int i=1; i<dim; i++) dinfo << "," <<  N[L-1][i];
    dinfo << ")" << std::endl;
  }

  template<int dim, int dimworld>
  inline SGrid<dim,dimworld>::SGrid (const int* N_, const sgrid_ctype* H_)
    : theglobalidset(*this), theleafindexset(*this)
  {
    IsTrue< dimworld <= std::numeric_limits<int>::digits >::yes();

    sgrid_ctype L_[dim];
    for (int i=0; i<dim; i++)
      L_[i] = 0;

    makeSGrid(N_,L_, H_);
    indexsets.push_back( new SGridLevelIndexSet<SGrid<dim,dimworld> >(*this,0) );
  }

  template<int dim, int dimworld>
  inline SGrid<dim,dimworld>::SGrid (const int* N_, const sgrid_ctype* L_, const sgrid_ctype* H_)
    : theglobalidset(*this), theleafindexset(*this)
  {
    IsTrue< dimworld <= std::numeric_limits<int>::digits >::yes();

    makeSGrid(N_, L_, H_);
    indexsets.push_back( new SGridLevelIndexSet<SGrid<dim,dimworld> >(*this,0) );
  }

  template<int dim, int dimworld>
  inline SGrid<dim,dimworld>::SGrid ()
    : theglobalidset(*this), theleafindexset(*this)
  {
    int N_[dim];
    sgrid_ctype L_[dim];
    sgrid_ctype H_[dim];

    for(int i = 0; i < dim; ++i) {
      N_[i] = 1;
      L_[i] = 0.0;
      H_[i] = 1.0;
    }

    makeSGrid(N_, L_, H_);
    indexsets.push_back( new SGridLevelIndexSet<SGrid<dim,dimworld> >(*this,0) );
  }

  template<int dim, int dimworld>
  inline void SGrid<dim,dimworld>::globalRefine (int refCount)
  {
    for(int ref=0; ref<refCount; ref++)
    {

      // refine the mesh
      for (int i=0; i<dim; i++) N[L][i] = 2*N[L-1][i];
      mapper[L].make(N[L]);

      // compute mesh size
      for (int i=0; i<dim; i++)
        h[L][i] = (H[i]-low[i])/((sgrid_ctype)N[L][i]);
      L++;

      //     std::cout << "level=" << L-1 << " size=(" << N[L-1][0];
      //     for (int i=1; i<dim; i++) std::cout << "," <<  N[L-1][i];
      //     std::cout << ")" << std::endl;

      indexsets.push_back( new SGridLevelIndexSet<SGrid<dim,dimworld> >(*this,maxLevel()) );
    }
  }

  template<int dim, int dimworld>
  inline int SGrid<dim,dimworld>::maxLevel () const
  {
    return L-1;
  }

  template <int dim, int dimworld> template <int cd, PartitionIteratorType pitype>
  inline typename SGrid<dim,dimworld>::Traits::template Codim<cd>::template Partition<pitype>::LevelIterator
  SGrid<dim,dimworld>::lbegin (int level) const
  {
    return SLevelIterator<cd,pitype,const SGrid<dim,dimworld> > (this,level,0);
  }

  template <int dim, int dimworld> template <int cd, PartitionIteratorType pitype>
  inline typename SGrid<dim,dimworld>::Traits::template Codim<cd>::template Partition<pitype>::LevelIterator
  SGrid<dim,dimworld>::lend (int level) const
  {
    return SLevelIterator<cd,pitype,const SGrid<dim,dimworld> > (this,level,size(level,cd));
  }

  template <int dim, int dimworld> template <int cd, PartitionIteratorType pitype>
  inline typename SGrid<dim,dimworld>::Traits::template Codim<cd>::template Partition<pitype>::LeafIterator
  SGrid<dim,dimworld>::leafbegin () const
  {
    return SLevelIterator<cd,pitype,const SGrid<dim,dimworld> > (this,maxLevel(),0);
  }

  template <int dim, int dimworld> template <int cd, PartitionIteratorType pitype>
  inline typename SGrid<dim,dimworld>::Traits::template Codim<cd>::template Partition<pitype>::LeafIterator
  SGrid<dim,dimworld>::leafend () const
  {
    return SLevelIterator<cd,pitype,const SGrid<dim,dimworld> > (this,maxLevel(),size(maxLevel(),cd));
  }

  template<int dim, int dimworld>
  inline int SGrid<dim,dimworld>::size (int level, int codim) const
  {
    return mapper[level].elements(codim);
  }

  template<int dim, int dimworld>
  inline int SGrid<dim,dimworld>::global_size (int codim) const
  {
    int gSize = 0;
    for(int i=0; i <= this->maxLevel(); i++)
      gSize += this->size(i,codim);
    return gSize;
  }

  template<int dim, int dimworld>
  inline FieldVector<sgrid_ctype, dim> SGrid<dim,dimworld>::pos (int level, FixedArray<int,dim>& z) const
  {
    FieldVector<sgrid_ctype, dim> x;
    for (int k=0; k<dim; k++)
      x[k] = (z[k]*h[level][k])*0.5 + low[k];
    return x;
  }

  template<int dim, int dimworld>
  inline int SGrid<dim,dimworld>::calc_codim (int level, const FixedArray<int,dim>& z) const
  {
    return mapper[level].codim(z);
  }

  template<int dim, int dimworld>
  inline int SGrid<dim,dimworld>::n (int level, const FixedArray<int,dim> z) const
  {
    return mapper[level].n(z);
  }

  template<int dim, int dimworld>
  inline FixedArray<int,dim> SGrid<dim,dimworld>::z (int level, int i, int codim) const
  {
    return mapper[level].z(i,codim);
  }

  template<int dim, int dimworld>
  inline FixedArray<int,dim> SGrid<dim,dimworld>::compress (int level, const FixedArray<int,dim>& z) const
  {
    return mapper[level].compress(z);
  }

  template<int dim, int dimworld>
  inline FixedArray<int,dim> SGrid<dim,dimworld>::expand (int level, const FixedArray<int,dim>& r, int b) const
  {
    return mapper[level].expand(r,b);
  }

  template<int dim, int dimworld>
  inline int SGrid<dim,dimworld>::partition (int level, const FixedArray<int,dim>& z) const
  {
    return mapper[level].partition(z);
  }

  template<int dim, int dimworld>
  inline bool SGrid<dim,dimworld>::exists (int level, const FixedArray<int,dim>& zred) const
  {
    for (int i=0; i<dim; i++)
    {
      if (zred[i]<0) return false;
      if (zred[i]>=N[level][i]) return false;
    }
    return true;
  }


} // end namespace

#endif
