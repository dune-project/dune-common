// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __SGRID_CC__
#define __SGRID_CC__

#include <algorithm>
#include <iostream>
#include <assert.h>

#include <dune/common/stdstreams.hh>

namespace Dune {


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
    for (int j=0; j<dim; j++) {
      // make unit vectors
      A(j)   = FieldVector<sgrid_ctype, dimworld>(0.0);
      A(j,j) = 1.0;
    }

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
  inline void SElement<dim,dimworld>::make(Mat<dimworld,dim+1,sgrid_ctype>& __As)
  {
    // clear jacobian
    builtinverse = false;

    // copy arguments
    s = __As(dim);
    for (int j=0; j<dim; j++) A(j) = __As(j);

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
  inline FieldVector<sgrid_ctype, dimworld>& SElement<dim,dimworld>::operator[] (int i)
  {
    return c[i];
  }

  template<int dim, int dimworld>
  inline FieldVector<sgrid_ctype, dimworld> SElement<dim,dimworld>::global (const FieldVector<sgrid_ctype, dim>& local)
  {
    return s+(A*local);
  }

  template<int dim, int dimworld>
  inline FieldVector<sgrid_ctype, dim> SElement<dim,dimworld>::local (const FieldVector<sgrid_ctype, dimworld>& global)
  {
    FieldVector<sgrid_ctype, dim> l;     // result
    FieldVector<sgrid_ctype, dimworld> rhs = global-s;
    for (int k=0; k<dim; k++)
      l[k] = (rhs*A(k)) / (A(k)*A(k));
    return l;
  }

  template<int dim, int dimworld>
  inline bool SElement<dim,dimworld>::checkInside (const FieldVector<sgrid_ctype, dim>& local)
  {
    // check wether they are in the reference element
    for(int i=0; i<dim; i++)
    {
      if((local[i] < 0.0) || (local[i] > 1.0 ))
        return false;
    }
    return true;
  }

  template<int dim, int dimworld>
  inline sgrid_ctype SElement<dim,dimworld>::integration_element (const FieldVector<sgrid_ctype, dim>& local)
  {
    sgrid_ctype s = 1.0;
    for (int j=0; j<dim; j++) s *= A(j).one_norm();

    return s;
  }

  template<int dim, int dimworld>
  inline Mat<dim,dim,sgrid_ctype>& SElement<dim,dimworld>::Jacobian_inverse (const FieldVector<sgrid_ctype, dim>& local)
  {
    assert(dim==dimworld);

    for (int i=0; i<dim; ++i)
      for (int j=0; j<dim; ++j)
        Jinv(i,j) = A(i,j);
    for (int i=0; i<dim; i++) Jinv(i,i) = 1.0/Jinv(i,i);
    if (!builtinverse)
    {
      builtinverse = true;
    }
    return Jinv;
  }

  template<int dim, int dimworld>
  inline void SElement<dim,dimworld>::print (std::ostream& ss, int indent)
  {
    for (int k=0; k<indent; k++) ss << " ";ss << "SElement<" << dim << "," << dimworld << ">" << std::endl;
    for (int k=0; k<indent; k++) ss << " ";ss << "{" << std::endl;
    for (int k=0; k<indent+2; k++) ss << " ";ss << "Position: " << s << std::endl;
    for (int j=0; j<dim; j++)
    {
      for (int k=0; k<indent+2; k++) ss << " ";
      ss << "direction " << j << "  " << A(j) << std::endl;
    }
    for (int j=0; j<1<<dim; j++)
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

  // special case dim=0
  template<int dimworld>
  inline SElement<0,dimworld>::SElement (bool b)
  {
    if (!b) return;
    s = 0.0;
  }

  template<int dimworld>
  inline void SElement<0,dimworld>::make (Mat<dimworld,1,sgrid_ctype>& __As)
  {
    s = __As(0);
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
  inline FieldVector<sgrid_ctype, dimworld>& SElement<0,dimworld>::operator[] (int i)
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
  static inline FixedArray<int,n>& coarsen (FixedArray<int,n>& in)
  {
    for (int i=0; i<n; i++) in[i] = in[i]/2;
    return in;
  }

  template<int codim, int dim, int dimworld>
  inline SEntityBase<codim,dim,dimworld>::SEntityBase (SGrid<dim,dimworld>* _grid, int _l, int _id) : geo(false)
  {
    grid = _grid;
    l = _l;
    id = _id;
    z = grid->z(_l,_id,codim);
    builtgeometry = false;
  }

  template<int codim, int dim, int dimworld>
  inline SEntityBase<codim,dim,dimworld>::SEntityBase () : geo(false)
  {
    builtgeometry = false;
  }

  template<int codim, int dim, int dimworld>
  inline void SEntityBase<codim,dim,dimworld>::make (SGrid<dim,dimworld>* _grid, int _l, int _id)
  {
    grid = _grid;
    l = _l;
    id = _id;
    z = grid->z(_l,_id,codim);
    builtgeometry = false;
  }

  template<int codim, int dim, int dimworld>
  inline void SEntityBase<codim,dim,dimworld>::make (int _l, int _id)
  {
    l = _l;
    id = _id;
    z = grid->z(_l,_id,codim);
    builtgeometry = false;
  }

  template<int codim, int dim, int dimworld>
  inline int SEntityBase<codim,dim,dimworld>::level () const
  {
    return l;
  }
  //                std::cout << i->index() << " " ;
  //                for (int z=0; z<N; ++z) std::cout << "["<<j[z]<<","<<A[z]<<"] ";
  //                std::cout << std::endl;

  template<int codim, int dim, int dimworld>
  inline int SEntityBase<codim,dim,dimworld>::index () const
  {
    return id;
  }

  template<int codim, int dim, int dimworld>
  inline int SEntityBase<codim,dim,dimworld>::global_index () const
  {
    int ind = 0;
    for(int i=0; i<this->l; i++)
      ind += this->grid->size(i,codim);
    return ind+this->index();
  }

  template<int codim, int dim, int dimworld>
  inline SElement<dim-codim,dimworld>& SEntityBase<codim,dim,dimworld>::geometry ()
  {
    if (builtgeometry) return geo;

    // find dim-codim direction vectors and reference point
    Mat<dimworld,dim-codim+1,sgrid_ctype> __As;

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
        __As(dir) = p2-p1;
        dir++;
      }

    // find reference point, subtract 1 from all odd directions
    for (int i=0; i<dim; i++)
      if (t[i]%2==1)
        t[i] -= 1;
    __As(dir) =grid->pos(l,t);     // all components of t are even

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
  template<int dim, int dimworld> template<int cc>
  inline int SEntity<0,dim,dimworld>::count ()
  {
    return SUnitCubeMapper<dim>::mapper.elements(cc);
  }


  template<int dim, int dimworld> template<int cc>
  inline SLevelIterator<cc,dim,dimworld,All_Partition> SEntity<0,dim,dimworld>::entity (int i)
  {
    // find expanded coordinates of entity in reference cube
    // has components in {0,1,2}
    FixedArray<int,dim> zref = SUnitCubeMapper<dim>::mapper.z(i,cc);

    // compute expanded coordinates of entity in global coordinates
    FixedArray<int,dim> zentity;
    for (int i=0; i<dim; i++) zentity[i] = this->z[i] + zref[i] - 1;

    // make Iterator
    return SLevelIterator<cc,dim,dimworld,All_Partition>(this->grid,this->l,(this->grid)->n(this->l,zentity));
  }

  // default implementation uses entity method
  template<int dim, int dimworld> template<int cc>
  inline int SEntity<0,dim,dimworld>::subIndex (int i)
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

  template<int dim, int dimworld>
  inline SIntersectionIterator<dim,dimworld> SEntity<0,dim,dimworld>::ibegin ()
  {
    return SIntersectionIterator<dim,dimworld>(this->grid,*this,0);
  }

  template<int dim, int dimworld>
  inline void SEntity<0,dim,dimworld>::ibegin (SIntersectionIterator<dim,dimworld>& i)
  {
    return i.make(this->grid,*this,0);
  }

  template<int dim, int dimworld>
  inline SIntersectionIterator<dim,dimworld> SEntity<0,dim,dimworld>::iend ()
  {
    return SIntersectionIterator<dim,dimworld>(this->grid,*this,count<1>());
  }

  template<int dim, int dimworld>
  inline void SEntity<0,dim,dimworld>::iend (SIntersectionIterator<dim,dimworld>& i)
  {
    return i.make(this->grid,*this,count<1>());
  }


  template<int dim, int dimworld>
  inline void SEntity<0,dim,dimworld>::make_father ()
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
    Mat<dim,dim+1,sgrid_ctype> __As;
    FieldVector<sgrid_ctype, dim> v;
    for (int i=0; i<dim; i++)
    {
      v = 0.0; v[i] = 0.5;
      __As(i) = v;
    }
    for (int i=0; i<dim; i++) v[i] = 0.5*delta[i];
    __As(dim) =v;
    in_father_local.make(__As);     // build geometry

    built_father = true;
  }

  template<int dim, int dimworld>
  inline SLevelIterator<0,dim,dimworld,All_Partition> SEntity<0,dim,dimworld>::father ()
  {
    if (!built_father) make_father();
    if (this->l>0)
      return SLevelIterator<0,dim,dimworld,All_Partition>((this->grid),(this->l)-1,father_id);
    else
      return SLevelIterator<0,dim,dimworld,All_Partition>((this->grid),this->l,this->id);
  }

  template<int dim, int dimworld>
  inline void SEntity<0,dim,dimworld>::father (SEntity<0,dim,dimworld> &pa)
  {
    if (!built_father) make_father();

    if (this->l>0)
      pa.make(*(this->grid),(this->l)-1,father_id);
    else
      pa.make(*(this->grid),(this->l),this->id);
  }

  template<int dim, int dimworld>
  inline bool SEntity<0,dim,dimworld>::hasChildren ()
  {
    return ( this->grid->maxlevel() > level() );
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
    return SHierarchicIterator<dim,dimworld>(this->grid,*this,maxlevel,false);
  }

  template<int dim, int dimworld>
  inline SHierarchicIterator<dim,dimworld> SEntity<0,dim,dimworld>::hend (int maxlevel)
  {
    return SHierarchicIterator<dim,dimworld>(this->grid,*this,maxlevel,true);
  }


  // codim dim
  template<int dim, int dimworld>
  inline void SEntity<dim,dim,dimworld>::make_father ()
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

  template<int dim, int dimworld>
  inline SLevelIterator<0,dim,dimworld,All_Partition> SEntity<dim,dim,dimworld>::father ()
  {
    if (!built_father) make_father();
    if (this->l>0)
      return SLevelIterator<0,dim,dimworld,All_Partition>((this->grid),(this->l)-1,father_id);
    else
      return SLevelIterator<0,dim,dimworld,All_Partition>((this->grid),this->l,this->id);
  }

  template<int dim, int dimworld>
  inline FieldVector<sgrid_ctype, dim>& SEntity<dim,dim,dimworld>::local ()
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
    FixedArray<int,dim> z = grid->z(level,fatherid,0);      // expanded coordinates from id
    FixedArray<int,dim> zred = grid->compress(level,z);     // reduced coordinates from expaned coordinates

    // refine to first son
    for (int i=0; i<dim; i++) zred[i] = 2*zred[i];

    // generate all \f$2^{dim}\f$ sons
    int partition = grid->partition(level,z);
    for (int b=0; b<(1<<dim); b++)
    {
      FixedArray<int,dim> zz = zred;
      for (int i=0; i<dim; i++)
        if (b&(1<<i)) zz[i] += 1;
      // zz is reduced coordinate of a son on level level+1
      int sonid = grid->n(level+1,grid->expand(level+1,zz,partition));

      // push son on stack
      StackElem son(level+1,sonid);
      //stack.push(StackElem(level+1,sonid));
      stack.push(son);
    }
  }

  template<int dim, int dimworld>
  inline SHierarchicIterator<dim,dimworld>::SHierarchicIterator (SGrid<dim,dimworld>* _grid,
                                                                 SEntity<0,dim,dimworld>& _e, int _maxlevel, bool makeend) :
    grid(_grid),e(_e)
  {
    // without sons, we are done (i.e. this is te end iterator, having original element in it)
    if (makeend) return;

    // remember element where begin has been called
    orig_l = e.l;
    orig_id = e.id;

    // push original element on stack
    StackElem originalElement(orig_l, orig_id);
    stack.push(originalElement);

    // compute maxlevel
    maxlevel = std::min(_maxlevel,grid->maxlevel());

    // ok, push all the sons as well
    push_sons(e.l,e.id);

    // and pop the first son
    operator++();
  }

  template<int dim, int dimworld>
  inline SHierarchicIterator<dim,dimworld>& SHierarchicIterator<dim,dimworld>::operator++ ()
  {
    // check empty stack
    if (stack.empty()) return *this;

    // OK, lets pop
    StackElem newe = stack.pop();
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
  // inline methods for IntersectionIterator

  template<int dim, int dimworld>
  inline void SIntersectionIterator<dim,dimworld>::make (int _count)
  {
    // reset cache flags
    built_intersections = false;
    valid_count = false;

    // start with given neighbor
    count = _count;

    // check if count is valid
    if (count<0 || count>=self->template count<1>()) return;     // done, this is end iterator
    valid_count = true;

    // and compute compressed coordinates of neighbor
    FixedArray<int,dim> zrednb = zred;
    if (count%2)
      zrednb[count/2] += 1;           // odd
    else
      zrednb[count/2] -= 1;           // even

    // while we are at it, compute normal direction
    normal = 0.0;
    if (count%2)
      normal[count/2] =  1.0;           // odd
    else
      normal[count/2] = -1.0;           // even

    // now check if neighbor exists
    is_on_boundary = !grid->exists(self->level(),zrednb);
    if (is_on_boundary) return;     // ok, done it

    // now neighbor is in the grid and must be initialized.
    // First compute its id
    int nbid = grid->n(self->level(),grid->expand(self->level(),zrednb,partition));

    // and make it
    e.make(self->level(),nbid);
  }

  template<int dim, int dimworld>
  inline SIntersectionIterator<dim,dimworld>::SIntersectionIterator
    (SGrid<dim,dimworld>* _grid, SEntity<0,dim,dimworld>& _self, int _count)
    : e(_grid,_self.l, _self.id), is_self_local(false), is_global(false),
      is_nb_local(false)
  {
    grid = _grid;
    self = &_self;

    // compute own compressed coordinates once
    zred = grid->compress(self->l,self->z);
    partition = grid->partition(self->l,self->z);

    // make neighbor
    make(_count);
  }

  template<int dim, int dimworld>
  inline SIntersectionIterator<dim,dimworld>::SIntersectionIterator ()
    : is_self_local(false), is_global(false), is_nb_local(false)
  { }

  template<int dim, int dimworld>
  inline void SIntersectionIterator<dim,dimworld>::make (SGrid<dim,dimworld>* _grid, SEntity<0,dim,dimworld>& _self, int _count)
  {
    grid = _grid;
    self = &_self;

    e.make(_grid,_self.l, _self.id);

    // compute own compressed coordinates once
    zred = grid->compress(self->l,self->z);
    partition = grid->partition(self->l,self->z);

    // make neighbor
    make(_count);
  }

  template<int dim, int dimworld>
  inline SIntersectionIterator<dim,dimworld>& SIntersectionIterator<dim,dimworld>::operator++ ()
  {
    count++;
    make(count);
    return *this;
  }

  template<int dim, int dimworld>
  inline bool SIntersectionIterator<dim,dimworld>::operator== (const SIntersectionIterator<dim,dimworld>& i) const
  {
    return (count==i.count)&&(self==i.self);
  }

  template<int dim, int dimworld>
  inline bool SIntersectionIterator<dim,dimworld>::operator!= (const SIntersectionIterator<dim,dimworld>& i) const
  {
    return (count!=i.count)||(self!=i.self);
  }

  template<int dim, int dimworld>
  inline SEntity<0,dim,dimworld>& SIntersectionIterator<dim,dimworld>::operator* ()
  {
    return e;
  }

  template<int dim, int dimworld>
  inline SEntity<0,dim,dimworld>* SIntersectionIterator<dim,dimworld>::operator-> ()
  {
    return &e;
  }

  template<int dim, int dimworld>
  inline bool SIntersectionIterator<dim,dimworld>::boundary ()
  {
    return is_on_boundary;
  }

  template<int dim, int dimworld>
  inline bool SIntersectionIterator<dim,dimworld>::neighbor ()
  {
    return (!is_on_boundary);
  }

  template<int dim, int dimworld>
  inline void SIntersectionIterator<dim,dimworld>::makeintersections ()
  {
    if (built_intersections) return;     // already done
    if (!valid_count) return;     // nothing to do

    // compute direction and value in direction
    int dir = count/2;
    int c = count%2;

    // compute expanded coordinates of entity
    FixedArray<int,dim> z1 = self->z;
    if (c==1)
      z1[dir] += 1;           // odd
    else
      z1[dir] -= 1;           // even

    // z1 is even in direction dir, all others must be odd because it is codim 1
    Mat<dim,dim,sgrid_ctype> __As;
    FieldVector<sgrid_ctype, dim> p1,p2;
    int t;

    // local coordinates in self
    p1 = 0.0;
    p1[dir] = c;        // all points have p[dir]=c in entity
    __As(dim-1) = p1;     // position vector
    t = 0;
    for (int i=0; i<dim; ++i)     // this loop makes dim-1 direction vectors
      if (i!=dir)
      {
        // each i!=dir gives one direction vector
        p2 = p1;
        p2[i] = 1.0;
        __As(t) = p2-p1;                 // a direction vector
        ++t;
      }
    is_self_local.make(__As);     // build geometry

    // local coordinates in neighbor
    p1 = 0.0;
    p1[dir] = 1-c;        // all points have p[dir]=1-c in entity
    __As(dim-1) = p1;       // position vector
    t = 0;
    for (int i=0; i<dim; ++i)     // this loop makes dim-1 direction vectors
      if (i!=dir)
      {
        // each i!=dir gives one direction vector
        p2 = p1;
        p2[i] = 1.0;
        __As(t) = p2-p1;                 // a direction vector
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
        p2 = grid->pos(self->level(),z1);
        z1[i] -= 2;                 // direction i => even
        p1 = grid->pos(self->level(),z1);
        z1[i] += 1;                 // revert t to original state
        __As(t) = p2-p1;
        ++t;
      }
    for (int i=0; i<dim; i++)
      if (i!=dir)
        z1[i] -= 1;
    __As(t) =grid->pos(self->level(),z1);
    is_global.make(__As);     // build geometry

    built_intersections = true;
  }

  template<int dim, int dimworld>
  inline SElement<dim-1,dim>& SIntersectionIterator<dim,dimworld>::intersection_self_local ()
  {
    makeintersections();
    return is_self_local;
  }

  template<int dim, int dimworld>
  inline SElement<dim-1,dimworld>& SIntersectionIterator<dim,dimworld>::intersection_self_global ()
  {
    makeintersections();
    return is_global;
  }

  template<int dim, int dimworld>
  inline SElement<dim-1,dim>& SIntersectionIterator<dim,dimworld>::intersection_neighbor_local ()
  {
    makeintersections();
    return is_nb_local;
  }

  template<int dim, int dimworld>
  inline SElement<dim-1,dimworld>& SIntersectionIterator<dim,dimworld>::intersection_neighbor_global ()
  {
    makeintersections();
    return is_global;
  }

  template<int dim, int dimworld>
  inline int SIntersectionIterator<dim,dimworld>::number_in_self ()
  {
    return count;
  }

  template<int dim, int dimworld>
  inline int SIntersectionIterator<dim,dimworld>::number_in_neighbor ()
  {
    return (count/2)*2 + (1-count%2);
  }

  template<int dim, int dimworld>
  inline FieldVector<sgrid_ctype, dimworld>&
  SIntersectionIterator<dim,dimworld>::unit_outer_normal (FieldVector<sgrid_ctype, dim-1>& local)
  {
    return normal;
  }

  template<int dim, int dimworld>
  inline FieldVector<sgrid_ctype, dimworld>&
  SIntersectionIterator<dim,dimworld>::unit_outer_normal ()
  {
    return normal;
  }

  //************************************************************************
  // inline methods for SLevelIterator

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  inline SLevelIterator<codim,dim,dimworld,pitype>::SLevelIterator (SGrid<dim,dimworld>* _grid, int _l, int _id) : grid(_grid),e(_grid,_l,_id)
  {
    l = _l;
    id = _id;
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  inline SLevelIterator<codim,dim,dimworld,pitype>& SLevelIterator<codim,dim,dimworld,pitype>::operator++ ()
  {
    id++;
    e.make(l,id);
    return *this;
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  inline bool SLevelIterator<codim,dim,dimworld,pitype>::operator== (const SLevelIterator<codim,dim,dimworld,pitype>& i) const
  {
    return (id==i.id)&&(l==i.l)&&(grid==i.grid);
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  inline bool SLevelIterator<codim,dim,dimworld,pitype>::operator!= (const SLevelIterator<codim,dim,dimworld,pitype>& i) const
  {
    return (id!=i.id)||(l!=i.l)||(grid!=i.grid);
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  inline SEntity<codim,dim,dimworld>& SLevelIterator<codim,dim,dimworld,pitype>::operator* ()
  {
    return e;
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  inline SEntity<codim,dim,dimworld>* SLevelIterator<codim,dim,dimworld,pitype>::operator-> ()
  {
    return &e;
  }

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  inline int SLevelIterator<codim,dim,dimworld,pitype>::level ()
  {
    return l;
  }


  //************************************************************************
  // inline methods for SGrid
  template<int dim, int dimworld>
  inline void SGrid<dim,dimworld>::makeSGrid (const int* N_,
                                              const sgrid_ctype* L_, const sgrid_ctype* H_)
  {
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
  {
    sgrid_ctype L_[dim];
    for (int i=0; i<dim; i++)
      L_[i] = 0;

    makeSGrid(N_,L_, H_);
  }

  template<int dim, int dimworld>
  inline SGrid<dim,dimworld>::SGrid (const int* N_, const sgrid_ctype* L_, const sgrid_ctype* H_)
  {
    makeSGrid(N_, L_, H_);
  }

  template<int dim, int dimworld>
  inline SGrid<dim,dimworld>::SGrid ()
  {
#if 0
    int n[dim];
    sgrid_ctype h[dim];

    for(int i=0; i<dim; i++) n[i] = 1;
    for(int i=0; i<dim; i++) h[i] = 1.0;
    makeSGrid((int *) &n, (sgrid_ctype *) &h);
#endif
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

      std::cout << "level=" << L-1 << " size=(" << N[L-1][0];
      for (int i=1; i<dim; i++) std::cout << "," <<  N[L-1][i];
      std::cout << ")" << std::endl;
    }
  }

  template<int dim, int dimworld>
  inline int SGrid<dim,dimworld>::maxlevel () const
  {
    return L-1;
  }

  template <int dim, int dimworld> template <int cd, PartitionIteratorType pitype>
  inline SLevelIterator<cd,dim,dimworld,pitype> SGrid<dim,dimworld>::lbegin (int level)
  {
    return SLevelIterator<cd,dim,dimworld,pitype> (this,level,0);
  }

  template <int dim, int dimworld> template <int cd, PartitionIteratorType pitype>
  inline SLevelIterator<cd,dim,dimworld,pitype> SGrid<dim,dimworld>::lend (int level)
  {
    return SLevelIterator<cd,dim,dimworld,pitype> (this,level,size(level,cd));
  }

  template <int dim, int dimworld> template <int cd>
  inline SLevelIterator<cd,dim,dimworld,All_Partition> SGrid<dim,dimworld>::lbegin (int level)
  {
    return SLevelIterator<cd,dim,dimworld,All_Partition> (this,level,0);
  }

  template <int dim, int dimworld> template <int cd>
  inline SLevelIterator<cd,dim,dimworld,All_Partition> SGrid<dim,dimworld>::lend (int level)
  {
    return SLevelIterator<cd,dim,dimworld,All_Partition> (this,level,size(level,cd));
  }

  template <int dim, int dimworld>
  inline typename SGrid<dim,dimworld>::LeafIterator SGrid<dim,dimworld>::leafbegin (int level)
  {
    return this->template lbegin<0> (level);
  }

  template <int dim, int dimworld>
  inline typename SGrid<dim,dimworld>::LeafIterator SGrid<dim,dimworld>::leafend (int level)
  {
    return this->template lend<0> (level);
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
    for(int i=0; i <= this->maxlevel(); i++)
      gSize += this->size(i,codim);
    return gSize;
  }

  template<int dim, int dimworld>
  inline GridIdentifier SGrid<dim,dimworld>::type () const
  {
    return SGrid_Id;
  }

  // write Grid in ascii
  template<int dim, int dimworld> template <FileFormatType ftype>
  inline bool SGrid<dim,dimworld>::
  writeGrid (const char * filename, sgrid_ctype time )
  {
    std::fstream file (filename,std::ios::out);
    file << dim << " " << dimworld << " " << time << "\n";
    file << L << " ";
    for(int i=0; i<dim; i++) file << N[0][i] << " ";
    for(int i=0; i<dim; i++) file << low[i] << " ";
    for(int i=0; i<dim; i++) file << H[i] << " ";
    file.close();
    return true;
  }

  // read Grid in ascii
  template<int dim, int dimworld> template <FileFormatType ftype>
  inline bool SGrid<dim,dimworld>::
  readGrid (const char * filename , sgrid_ctype &time)
  {
    int n[dim];
    sgrid_ctype h[dim];
    sgrid_ctype L_[dim];
    int d,dw;
    int level;

    std::fstream file (filename,std::ios::in);
    file >> d;

    if(d != dim)
    {
      std::cerr << "Wrong dimension in grid file! \n";
      assert(d == dim);
      return false;
    }

    file >> dw;
    if(dw != dimworld)
    {
      std::cerr << "Wrong dimensionworld in grid file! \n";
      assert(dw == dimworld);
      return false;
    }

    file >> time;
    file >> level;

    for(int i=0; i<dim; i++) file >> n[i];
    for(int i=0; i<dim; i++) file >> L_[i];
    for(int i=0; i<dim; i++) file >> h[i];
    file.close();
    makeSGrid( (int *)&n,(double *) &L_, (double *)&h);
    for(int i=1; i<level; i++) globalRefine(1);
    return true;
  }

  template<int dim, int dimworld>
  inline FieldVector<sgrid_ctype, dim> SGrid<dim,dimworld>::pos (int level, FixedArray<int,dim>& z)
  {
    FieldVector<sgrid_ctype, dim> x;
    for (int k=0; k<dim; k++)
      x[k] = (z[k]*h[level][k])*0.5 + low[k];
    return x;
  }

  template<int dim, int dimworld>
  inline int SGrid<dim,dimworld>::codim (int level, FixedArray<int,dim>& z)
  {
    return mapper[level].codim(z);
  }

  template<int dim, int dimworld>
  inline int SGrid<dim,dimworld>::n (int level, FixedArray<int,dim> z)
  {
    return mapper[level].n(z);
  }

  template<int dim, int dimworld>
  inline FixedArray<int,dim> SGrid<dim,dimworld>::z (int level, int i, int codim)
  {
    return mapper[level].z(i,codim);
  }

  template<int dim, int dimworld>
  inline FixedArray<int,dim> SGrid<dim,dimworld>::compress (int level, FixedArray<int,dim>& z)
  {
    return mapper[level].compress(z);
  }

  template<int dim, int dimworld>
  inline FixedArray<int,dim> SGrid<dim,dimworld>::expand (int level, FixedArray<int,dim>& r, int b)
  {
    return mapper[level].expand(r,b);
  }

  template<int dim, int dimworld>
  inline int SGrid<dim,dimworld>::partition (int level, FixedArray<int,dim>& z)
  {
    return mapper[level].partition(z);
  }

  template<int dim, int dimworld>
  inline bool SGrid<dim,dimworld>::exists (int level, FixedArray<int,dim>& zred)
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
