// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __SGRID_CC__
#define __SGRID_CC__

namespace Dune {

#include <assert.h>

  //************************************************************************
  // SElement

  // singleton holding reference elements
  template<int dim>
  struct SReferenceElement {
    static SElement<dim,dim> refelem;
  };

  // initialize static variable with bool constructor (which makes reference elements)
  template<int dim>
  SElement<dim,dim> SReferenceElement<dim>::refelem(true);


  // members for SElement, general case dim!=0
  template<int dim, int dimworld>
  inline SElement<dim,dimworld>::SElement (bool b)
  {
    builtinverse = false;

    if (!b) return;

    // copy arguments
    s = 0.0;
    for (int j=0; j<dim; j++) A(j) = Vec<dimworld,sgrid_ctype>(j);     // make unit vectors

    // make corners
    for (int i=0; i<(1<<dim); i++)     // there are 2^d corners
    {
      // use binary representation of corner number to assign corner coordinates
      c[i] = s;
      for (int k=0; k<dim; k++)
        if (i&(1<<k)) c[i] = c[i]+A(k);
    }
  }

  template<int dim, int dimworld>
  inline void SElement<dim,dimworld>::make (Mat<dimworld,dim+1,sgrid_ctype>& As)
  {
    // clear jacobian
    builtinverse = false;

    // copy arguments
    s = As(dim);
    for (int j=0; j<dim; j++) A(j) = As(j);

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
  inline SElement<dim,dim>& SElement<dim,dimworld>::refelem ()
  {
    return SReferenceElement<dim>::refelem;
  }

  template<int dim, int dimworld>
  inline int SElement<dim,dimworld>::corners ()
  {
    return 1<<dim;
  }

  template<int dim, int dimworld>
  inline Vec<dimworld,sgrid_ctype>& SElement<dim,dimworld>::operator[] (int i)
  {
    return c[i];
  }

  template<int dim, int dimworld>
  inline Vec<dimworld,sgrid_ctype> SElement<dim,dimworld>::global (const Vec<dim,sgrid_ctype>& local)
  {
    return s+(A*local);
  }

  template<int dim, int dimworld>
  inline Vec<dim,sgrid_ctype> SElement<dim,dimworld>::local (const Vec<dimworld,sgrid_ctype>& global)
  {
    Vec<dim,sgrid_ctype> l;     // result
    Vec<dimworld,sgrid_ctype> rhs = global-s;
    for (int k=0; k<dim; k++)
      l(k) = (rhs*A(k)) / (A(k)*A(k));
    return l;
  }

  template<int dim, int dimworld>
  inline sgrid_ctype SElement<dim,dimworld>::integration_element (const Vec<dim,sgrid_ctype>& local)
  {
    sgrid_ctype s = 1.0;
    for (int j=0; j<dim; j++) s *= A(j).norm1();
    return s;
  }

  template<int dim, int dimworld>
  inline Mat<dim,dim>& SElement<dim,dimworld>::Jacobian_inverse (const Vec<dim,sgrid_ctype>& local)
  {
    assert(dim==dimworld);

    for (int i=0; i<dim; ++i)
      for (int j=0; j<dim; ++j)
        Jinv(i,j) = A(i,j);
    if (!builtinverse)
    {
      for (int i=0; i<dim; i++) Jinv(i,i) = 1.0/Jinv(i,i);
      builtinverse = true;
    }
    return Jinv;
  }

  template<int dim, int dimworld>
  inline void SElement<dim,dimworld>::print (std::ostream& ss, int indent)
  {
    for (int k=0; k<indent; k++) ss << " ";ss << "SElement<" << dim << "," << dimworld << ">" << endl;
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
    if (builtinverse)
    {
      for (int k=0; k<indent+2; k++) ss << " ";ss << "Jinv ";
      Jinv.print(ss,indent+2);
    }
    for (int k=0; k<indent+2; k++) ss << " ";ss << "builtinverse " << builtinverse << endl;
    for (int k=0; k<indent; k++) ss << " ";ss << "}";
  }

  // special case dim=0
  template<int dimworld>
  inline SElement<0,dimworld>::SElement (bool b)
  {
    if (!b) return;
    s = 0.0;
  }

  template<int dimworld>
  inline void SElement<0,dimworld>::make (Mat<dimworld,1,sgrid_ctype>& As)
  {
    s = As(0);
  }

  template<int dimworld>
  inline ElementType SElement<0,dimworld>::type ()
  {
    return vertex;
  }

  template<int dimworld>
  inline int SElement<0,dimworld>::corners ()
  {
    return 1;
  }

  template<int dimworld>
  inline Vec<dimworld,sgrid_ctype>& SElement<0,dimworld>::operator[] (int i)
  {
    return s;
  }

  template<int dimworld>
  inline void SElement<0,dimworld>::print (std::ostream& ss, int indent)
  {
    for (int i=0; i<indent; i++) ss << " ";
    ss << "SElement<0," << dimworld << "> at position " << s;
  }

  //************************************************************************
  // inline methods for SEntityBase

  template<int n>
  static inline Tupel<int,n>& coarsen (Tupel<int,n>& in)
  {
    for (int i=0; i<n; i++) in[i] = in[i]/2;
    return in;
  }

  template<int codim, int dim, int dimworld>
  inline SEntityBase<codim,dim,dimworld>::SEntityBase (SGrid<dim,dimworld>& _grid, int _l, int _id) : grid(_grid),geo(true)
  {
    l = _l;
    id = _id;
    z = _grid.z(_l,_id,codim);
    builtgeometry = false;
  }

  template<int codim, int dim, int dimworld>
  inline void SEntityBase<codim,dim,dimworld>::make (int _l, int _id)
  {
    l = _l;
    id = _id;
    z = grid.z(_l,_id,codim);
    builtgeometry = false;
  }

  template<int codim, int dim, int dimworld>
  inline int SEntityBase<codim,dim,dimworld>::level ()
  {
    return l;
  }

  template<int codim, int dim, int dimworld>
  inline int SEntityBase<codim,dim,dimworld>::index ()
  {
    return id;
  }

  template<int codim, int dim, int dimworld>
  inline SElement<dim-codim,dimworld>& SEntityBase<codim,dim,dimworld>::geometry ()
  {
    if (builtgeometry) return geo;

    // find dim-codim direction vectors and reference point
    Mat<dimworld,dim-codim+1,sgrid_ctype> As;

    // count number of direction vectors found
    int dir=0;
    Vec<dim,sgrid_ctype> p1,p2;
    Tupel<int,dim> t=z;

    // check all directions
    for (int i=0; i<dim; i++)
      if (t[i]%2==1)
      {
        // coordinate i is odd => gives one direction vector
        t[i] += 1;                 // direction i => even
        p2 = grid.pos(l,t);
        t[i] -= 2;                 // direction i => even
        p1 = grid.pos(l,t);
        t[i] += 1;                 // revert t to original state
        As(dir) = p2-p1;
        dir++;
      }

    // find reference point, subtract 1 from all odd directions
    for (int i=0; i<dim; i++)
      if (t[i]%2==1)
        t[i] -= 1;
    As(dir) =grid.pos(l,t);     // all components of t are even

    // make element
    geo.make(As);
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
  CubeMapper<dim> SUnitCubeMapper<dim>::mapper(Tupel<int,dim>(1));


  // codim 0
  template<int dim, int dimworld> template<int cc>
  inline int SEntity<0,dim,dimworld>::count ()
  {
    return SUnitCubeMapper<dim>::mapper.elements(cc);
  }


  template<int dim, int dimworld> template<int cc>
  inline SLevelIterator<cc,dim,dimworld> SEntity<0,dim,dimworld>::entity (int i)
  {
    // find expanded coordinates of entity in reference cube
    // has components in {0,1,2}
    Tupel<int,dim> zref = SUnitCubeMapper<dim>::mapper.z(i,cc);

    // compute expanded coordinates of entity in global coordinates
    Tupel<int,dim> zentity;
    for (int i=0; i<dim; i++) zentity[i] = z[i] + zref[i] - 1;

    // make Iterator
    return SLevelIterator<cc,dim,dimworld>(grid,l,grid.n(l,zentity));
  }

  template<int dim, int dimworld>
  inline SNeighborIterator<dim,dimworld> SEntity<0,dim,dimworld>::nbegin ()
  {
    return SNeighborIterator<dim,dimworld>(grid,*this,0);
  }

  template<int dim, int dimworld>
  inline SNeighborIterator<dim,dimworld> SEntity<0,dim,dimworld>::nend ()
  {
    return SNeighborIterator<dim,dimworld>(grid,*this,count<1>());
  }


  template<int dim, int dimworld>
  inline void SEntity<0,dim,dimworld>::make_father ()
  {
    // check level
    if (l<=0)
    {
      father_id = 0;
      built_father = true;
      return;
    }

    // reduced coordinates from expanded coordinates
    Tupel<int,dim> zz = grid.compress(l,z);

    // look for odd coordinates
    Vec<dim,sgrid_ctype> delta;
    for (int i=0; i<dim; i++)
      if (zz[i]%2)
      {
        // component i is odd
        zz[i] -= 1;
        zz[i] /= 2;
        delta(i) = 1.0;
      }
      else
      {
        // component i is even
        zz[i] /= 2;
        delta(i) = 0.0;
      }

    // zz is now the reduced coordinate of the father, compute id
    int partition = grid.partition(l,z);
    father_id = grid.n(l-1,grid.expand(l-1,zz,partition));

    // now make a subcube of size 1/2 in each direction
    Mat<dim,dim+1,sgrid_ctype> As;
    Vec<dim,sgrid_ctype> v;
    for (int i=0; i<dim; i++)
    {
      v = 0.0; v(i) = 0.5;
      As(i) = v;
    }
    for (int i=0; i<dim; i++) v(i) = 0.5*delta(i);
    As(dim) =v;
    in_father_local.make(As);     // build geometry

    built_father = true;
  }

  template<int dim, int dimworld>
  inline SLevelIterator<0,dim,dimworld> SEntity<0,dim,dimworld>::father ()
  {
    if (!built_father) make_father();
    if (l>0)
      return SLevelIterator<0,dim,dimworld>(grid,l-1,father_id);
    else
      return SLevelIterator<0,dim,dimworld>(grid,l,id);
  }

  template<int dim, int dimworld>
  inline SElement<dim,dim>& SEntity<0,dim,dimworld>::father_relative_local ()
  {
    if (!built_father) make_father();
    return in_father_local;
  }

  template<int dim, int dimworld>
  inline SHierarchicIterator<dim,dimworld> SEntity<0,dim,dimworld>::hbegin (int maxlevel)
  {
    return SHierarchicIterator<dim,dimworld>(grid,*this,maxlevel,false);
  }

  template<int dim, int dimworld>
  inline SHierarchicIterator<dim,dimworld> SEntity<0,dim,dimworld>::hend (int maxlevel)
  {
    return SHierarchicIterator<dim,dimworld>(grid,*this,maxlevel,true);
  }


  // codim dim
  template<int dim, int dimworld>
  inline void SEntity<dim,dim,dimworld>::make_father ()
  {
    // check level
    if (l<=0)
    {
      father_id = 0;
      built_father = true;
      return;
    }

    // reduced coordinates from expanded coordinates
    // reduced coordinates of a fine grid vertex can be interpreted as
    // expanded coordinates on the next coarser level !
    Tupel<int,dim> zz = grid.compress(l,z);

    // to find father, make all coordinates odd
    Vec<dim,sgrid_ctype> delta;
    for (int i=0; i<dim; i++)
      if (zz[i]%2)
      {
        // component i is odd
        delta(i) = 0.0;
      }
      else
      {
        // component i is even
        if (zz[i]>0)
        {
          zz[i] -= 1;                       // now it is odd and >= 1
          delta(i) = 0.5;
        }
        else
        {
          zz[i] += 1;                       // now it is odd and >= 1
          delta(i) = -0.5;
        }
      }

    // zz is now an expanded coordinate on the coarse grid
    father_id = grid.n(l-1,zz);

    // compute the local coordinates in father
    in_father_local = 0.5;
    for (int i=0; i<dim; i++) in_father_local(i) += delta(i);

    built_father = true;
  }

  template<int dim, int dimworld>
  inline SLevelIterator<0,dim,dimworld> SEntity<dim,dim,dimworld>::father ()
  {
    if (!built_father) make_father();
    if (l>0)
      return SLevelIterator<0,dim,dimworld>(grid,l-1,father_id);
    else
      return SLevelIterator<0,dim,dimworld>(grid,l,id);
  }

  template<int dim, int dimworld>
  inline Vec<dim,sgrid_ctype>& SEntity<dim,dim,dimworld>::local ()
  {
    if (!built_father) make_father();
    return in_father_local;
  }

  //************************************************************************
  // inline methods for HierarchicIterator

  template<int dim, int dimworld>
  inline void SHierarchicIterator<dim,dimworld>::push_sons (int level, int fatherid)
  {
    // check level
    if (level+1>maxlevel) return;     // nothing to do

    // compute reduced coordinates of element
    Tupel<int,dim> z = grid.z(level,fatherid,0);      // expanded coordinates from id
    Tupel<int,dim> zred = grid.compress(level,z);     // reduced coordinates from expaned coordinates

    // refine to first son
    for (int i=0; i<dim; i++) zred[i] = 2*zred[i];

    // generate all \f$2^{dim}\f$ sons
    int partition = grid.partition(level,z);
    for (int b=0; b<(1<<dim); b++)
    {
      Tupel<int,dim> zz = zred;
      for (int i=0; i<dim; i++)
        if (b&(1<<i)) zz[i] += 1;
      // zz is reduced coordinate of a son on level level+1
      int sonid = grid.n(level+1,grid.expand(level+1,zz,partition));

      // push son on stack
      stack.push_front(StackElem(level+1,sonid));
    }
  }

  template<int dim, int dimworld>
  inline SHierarchicIterator<dim,dimworld>::SHierarchicIterator (SGrid<dim,dimworld>& _grid,
                                                                 SEntity<0,dim,dimworld>& _e, int _maxlevel, bool makeend) :
    grid(_grid),e(_e)
  {
    // without sons, we are done (i.e. this is te end iterator, having original element in it)
    if (makeend) return;

    // remember element where begin has been called
    orig_l = e.l;
    orig_id = e.id;

    // push original element on stack
    stack.push_front(StackElem(orig_l,orig_id));

    // compute maxlevel
    maxlevel = MIN(_maxlevel,grid.maxlevel());

    // ok, push all the sons as well
    push_sons(e.l,e.id);

    // and pop the first son
    operator++();
  }

  template<int dim, int dimworld>
  inline SHierarchicIterator<dim,dimworld>& SHierarchicIterator<dim,dimworld>::operator++ ()
  {
    // check empty stack
    if (stack.isempty()) return *this;

    // OK, lets pop
    StackElem newe = stack.pop_front();
    e.make(newe.l,newe.id);     // here is our new element

    // push all sons of this element if it is not the original element
    if (newe.l!=orig_l || newe.id!=orig_id)
      push_sons(newe.l,newe.id);

    return *this;
  }

  template<int dim, int dimworld>
  inline bool SHierarchicIterator<dim,dimworld>::operator== (const SHierarchicIterator<dim,dimworld>& i) const
  {
    return !operator!=(i);
  }

  template<int dim, int dimworld>
  inline bool SHierarchicIterator<dim,dimworld>::operator!= (const SHierarchicIterator<dim,dimworld>& i) const
  {
    return (stack.size()!=i.stack.size()) || (e.id!=i.e.id) || (e.l!=i.e.l) ;
  }

  template<int dim, int dimworld>
  inline SEntity<0,dim,dimworld>& SHierarchicIterator<dim,dimworld>::operator* ()
  {
    return e;
  }

  template<int dim, int dimworld>
  inline SEntity<0,dim,dimworld>* SHierarchicIterator<dim,dimworld>::operator-> ()
  {
    return &e;
  }


  //************************************************************************
  // inline methods for NeighborIterator

  template<int dim, int dimworld>
  inline void SNeighborIterator<dim,dimworld>::make (int _count)
  {
    // reset cache flags
    built_intersections = false;
    valid_count = false;

    // start with given neighbor
    count = _count;

    // check if count is valid
    if (count<0 || count>=self.count<1>()) return;     // done, this is end iterator
    valid_count = true;

    // and compute compressed coordinates of neighbor
    Tupel<int,dim> zrednb = zred;
    if (count%2)
      zrednb[count/2] += 1;           // odd
    else
      zrednb[count/2] -= 1;           // even

    // while we are at it, compute normal direction
    normal = 0.0;
    if (count%2)
      normal(count/2) =  1.0;           // odd
    else
      normal(count/2) = -1.0;           // even

    // now check if neighbor exists
    is_on_boundary = !grid.exists(self.level(),zrednb);
    if (is_on_boundary) return;     // ok, done it

    // now neighbor is in the grid and must be initialized.
    // First compute its id
    int nbid = grid.n(self.level(),grid.expand(self.level(),zrednb,partition));

    // and make it
    e.make(self.level(),nbid);
  }

  template<int dim, int dimworld>
  inline SNeighborIterator<dim,dimworld>::SNeighborIterator
    (SGrid<dim,dimworld>& _grid, SEntity<0,dim,dimworld>& _self, int _count)
    : grid(_grid), self(_self), e(_grid,_self.l, _self.id), is_self_local(false), is_global(false),
      is_nb_local(false)
  {
    // compute own compressed coordinates once
    zred = grid.compress(self.l,self.z);
    partition = grid.partition(self.l,self.z);

    // make neighbor
    make(_count);
  }

  template<int dim, int dimworld>
  inline SNeighborIterator<dim,dimworld>& SNeighborIterator<dim,dimworld>::operator++ ()
  {
    count++;
    make(count);
    return *this;
  }

  template<int dim, int dimworld>
  inline bool SNeighborIterator<dim,dimworld>::operator== (const SNeighborIterator<dim,dimworld>& i) const
  {
    return (count==i.count)&&(&self==&(i.self));
  }

  template<int dim, int dimworld>
  inline bool SNeighborIterator<dim,dimworld>::operator!= (const SNeighborIterator<dim,dimworld>& i) const
  {
    return (count!=i.count)||(&self!=&(i.self));
  }

  template<int dim, int dimworld>
  inline SEntity<0,dim,dimworld>& SNeighborIterator<dim,dimworld>::operator* ()
  {
    return e;
  }

  template<int dim, int dimworld>
  inline SEntity<0,dim,dimworld>* SNeighborIterator<dim,dimworld>::operator-> ()
  {
    return &e;
  }

  template<int dim, int dimworld>
  inline bool SNeighborIterator<dim,dimworld>::boundary ()
  {
    return is_on_boundary;
  }

  template<int dim, int dimworld>
  inline void SNeighborIterator<dim,dimworld>::makeintersections ()
  {
    if (built_intersections) return;     // already done
    if (!valid_count) return;     // nothing to do

    // compute direction and value in direction
    int dir = count/2;
    int c = count%2;

    // compute expanded coordinates of entity
    Tupel<int,dim> z1 = self.z;
    if (c==1)
      z1[dir] += 1;           // odd
    else
      z1[dir] -= 1;           // even

    // z1 is even in direction dir, all others must be odd because it is codim 1
    Mat<dim,dim,sgrid_ctype> As;
    Vec<dim,sgrid_ctype> p1,p2;
    int t;

    // local coordinates in self
    p1 = 0.0;
    p1(dir) = c;        // all points have p[dir]=c in entity
    As(dim-1) = p1;     // position vector
    t = 0;
    for (int i=0; i<dim; ++i)     // this loop makes dim-1 direction vectors
      if (i!=dir)
      {
        // each i!=dir gives one direction vector
        p2 = p1;
        p2(i) = 1.0;
        As(t) = p2-p1;                 // a direction vector
        ++t;
      }
    is_self_local.make(As);     // build geometry

    // local coordinates in neighbor
    p1 = 0.0;
    p1(dir) = 1-c;        // all points have p[dir]=1-c in entity
    As(dim-1) = p1;       // position vector
    t = 0;
    for (int i=0; i<dim; ++i)     // this loop makes dim-1 direction vectors
      if (i!=dir)
      {
        // each i!=dir gives one direction vector
        p2 = p1;
        p2(i) = 1.0;
        As(t) = p2-p1;                 // a direction vector
        ++t;
      }
    is_nb_local.make(As);     // build geometry

    // global coordinates
    t = 0;
    for (int i=0; i<dim; i++)
      if (i!=dir)
      {
        // each i!=dir gives one direction vector
        z1[i] += 1;                 // direction i => even
        p2 = grid.pos(self.level(),z1);
        z1[i] -= 2;                 // direction i => even
        p1 = grid.pos(self.level(),z1);
        z1[i] += 1;                 // revert t to original state
        As(t) = p2-p1;
        ++t;
      }
    for (int i=0; i<dim; i++)
      if (i!=dir)
        z1[i] -= 1;
    As(t) =grid.pos(self.level(),z1);
    is_global.make(As);     // build geometry

    built_intersections = true;
  }

  template<int dim, int dimworld>
  inline SElement<dim-1,dim>& SNeighborIterator<dim,dimworld>::intersection_self_local ()
  {
    makeintersections();
    return is_self_local;
  }

  template<int dim, int dimworld>
  inline SElement<dim-1,dimworld>& SNeighborIterator<dim,dimworld>::intersection_self_global ()
  {
    makeintersections();
    return is_global;
  }

  template<int dim, int dimworld>
  inline SElement<dim-1,dim>& SNeighborIterator<dim,dimworld>::intersection_neighbor_local ()
  {
    makeintersections();
    return is_nb_local;
  }

  template<int dim, int dimworld>
  inline SElement<dim-1,dimworld>& SNeighborIterator<dim,dimworld>::intersection_neighbor_global ()
  {
    makeintersections();
    return is_global;
  }

  template<int dim, int dimworld>
  inline int SNeighborIterator<dim,dimworld>::number_in_self ()
  {
    return count;
  }

  template<int dim, int dimworld>
  inline int SNeighborIterator<dim,dimworld>::number_in_neighbor ()
  {
    return (count/2)*2 + (1-count%2);
  }

  template<int dim, int dimworld>
  inline Vec<dimworld,sgrid_ctype>&
  SNeighborIterator<dim,dimworld>::unit_outer_normal (Vec<dim-1,sgrid_ctype>& local)
  {
    return normal;
  }

  template<int dim, int dimworld>
  inline Vec<dimworld,sgrid_ctype>&
  SNeighborIterator<dim,dimworld>::unit_outer_normal ()
  {
    return normal;
  }

  //************************************************************************
  // inline methods for SLevelIterator

  template<int codim, int dim, int dimworld>
  inline SLevelIterator<codim,dim,dimworld>::SLevelIterator (SGrid<dim,dimworld>& _grid, int _l, int _id) : grid(_grid),e(_grid,_l,_id)
  {
    l = _l;
    id = _id;
  }

  template<int codim, int dim, int dimworld>
  inline SLevelIterator<codim,dim,dimworld>& SLevelIterator<codim,dim,dimworld>::operator++ ()
  {
    id++;
    e.make(l,id);
    return *this;
  }

  template<int codim, int dim, int dimworld>
  inline bool SLevelIterator<codim,dim,dimworld>::operator== (const SLevelIterator<codim,dim,dimworld>& i) const
  {
    return (id==i.id)&&(l==i.l)&&(&grid==&i.grid);
  }

  template<int codim, int dim, int dimworld>
  inline bool SLevelIterator<codim,dim,dimworld>::operator!= (const SLevelIterator<codim,dim,dimworld>& i) const
  {
    return (id!=i.id)||(l!=i.l)||(&grid!=&i.grid);
  }

  template<int codim, int dim, int dimworld>
  inline SEntity<codim,dim,dimworld>& SLevelIterator<codim,dim,dimworld>::operator* ()
  {
    return e;
  }

  template<int codim, int dim, int dimworld>
  inline SEntity<codim,dim,dimworld>* SLevelIterator<codim,dim,dimworld>::operator-> ()
  {
    return &e;
  }

  template<int codim, int dim, int dimworld>
  inline int SLevelIterator<codim,dim,dimworld>::level ()
  {
    return l;
  }


  //************************************************************************
  // inline methods for SGrid

  template<int dim, int dimworld>
  inline SGrid<dim,dimworld>::SGrid (Tupel<int,dim> N_, Tupel<sgrid_ctype,dim> H_, int L_)
  {
    L = L_;
    H = H_;

    // define coarse mesh
    N[0] = N_;
    mapper[0].make(N[0]);

    // refine the mesh
    for (int l=1; l<L; l++)
    {
      for (int i=0; i<dim; i++) N[l][i] = 2*N[l-1][i];
      mapper[l].make(N[l]);
    }

    // compute mesh size
    for (int l=0; l<L; l++)
    {
      for (int i=0; i<dim; i++) h[l](i) = H[i]/((sgrid_ctype)N[l][i]);
    }

    cout << "Making SGrid with " << L << " level(s)." << endl;
    for (int l=0; l<L; l++)
      mapper[l].print(cout,0);
  }

  template<int dim, int dimworld>
  inline int SGrid<dim,dimworld>::maxlevel ()
  {
    return L-1;
  }

  template <int dim, int dimworld> template <int codim>
  inline SLevelIterator<codim,dim,dimworld> SGrid<dim,dimworld>::lbegin (int level)
  {
    return SLevelIterator<codim,dim,dimworld>(*this,level,0);
  }

  template <int dim, int dimworld> template <int codim>
  inline SLevelIterator<codim,dim,dimworld> SGrid<dim,dimworld>::lend (int level)
  {
    return SLevelIterator<codim,dim,dimworld>(*this,level,size(level,codim));
  }

  template<int dim, int dimworld>
  inline int SGrid<dim,dimworld>::size (int level, int codim)
  {
    return mapper[level].elements(codim);
  }

  template<int dim, int dimworld>
  inline Vec<dim,sgrid_ctype> SGrid<dim,dimworld>::pos (int level, Tupel<int,dim>& z)
  {
    Vec<dim,sgrid_ctype> x;
    for (int k=0; k<dim; k++) x(k) = (z[k]*h[level](k))*0.5;
    return x;
  }

  template<int dim, int dimworld>
  inline int SGrid<dim,dimworld>::codim (int level, Tupel<int,dim>& z)
  {
    return mapper[level].codim(z);
  }

  template<int dim, int dimworld>
  inline int SGrid<dim,dimworld>::n (int level, Tupel<int,dim> z)
  {
    return mapper[level].n(z);
  }

  template<int dim, int dimworld>
  inline Tupel<int,dim> SGrid<dim,dimworld>::z (int level, int i, int codim)
  {
    return mapper[level].z(i,codim);
  }

  template<int dim, int dimworld>
  inline Tupel<int,dim> SGrid<dim,dimworld>::compress (int level, Tupel<int,dim>& z)
  {
    return mapper[level].compress(z);
  }

  template<int dim, int dimworld>
  inline Tupel<int,dim> SGrid<dim,dimworld>::expand (int level, Tupel<int,dim>& r, int b)
  {
    return mapper[level].expand(r,b);
  }

  template<int dim, int dimworld>
  inline int SGrid<dim,dimworld>::partition (int level, Tupel<int,dim>& z)
  {
    return mapper[level].partition(z);
  }

  template<int dim, int dimworld>
  inline bool SGrid<dim,dimworld>::exists (int level, Tupel<int,dim>& zred)
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
