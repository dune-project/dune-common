// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ONE_D_ELEMENT_HH
#define DUNE_ONE_D_ELEMENT_HH

/** \file
 * \brief The OneDGridElement class and its specializations
 */

#include <dune/common/fixedarray.hh>
#include <dune/common/dlist.hh>

namespace Dune {

  template <int codim, int dim, int dimworld>
  class OneDGridEntity;

  /****************************************************************/
  /*       Specialization for faces in a 1d grid (i.e. vertices)  */
  /****************************************************************/


  template<>
  class OneDGridElement <0, 1> :
    public ElementDefault <0, 1, OneDCType,OneDGridElement>
  {

    template <int codim_, int dim_, int dimworld_>
    friend class OneDGridEntity;
    template <int dim_, int dimworld_>
    friend class OneDGridElement;

    template <int cc_, int dim_, int dimworld_>
    friend class OneDGridSubEntityFactory;


  public:

    /** \todo Constructor with a given coordinate */
    OneDGridElement(const double& x) {
      pos_[0] = x;
    }

    //! return the element type identifier (vertex)
    ElementType type () const {return vertex;}

    //! return the number of corners of this element (==1)
    int corners () const {return 1;}

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<OneDCType, 1>& operator[] (int i) const {
      return pos_;
    }

#if 0
    /*! return reference element corresponding to this element. If this is
       a reference element then self is returned.
     */
    OneDGridElement<0,0>& refelem ();


    //! maps a local coordinate within reference element to
    //! global coordinate in element
    FieldVector<OneDCType, 1> global (const FieldVector<OneDCType, 1>& local);

    //! Maps a global coordinate within the element to a
    //! local coordinate in its reference element
    FieldVector<OneDCType, 1> local (const FieldVector<OneDCType, 2>& global);

    //! Returns true if the point is in the current element
    bool checkInside(const FieldVector<OneDCType, 2> &global);

    // A(l)
    OneDCType integration_element (const FieldVector<OneDCType, 1>& local);

    //! can only be called for dim=dimworld!
    const Mat<1,1>& Jacobian_inverse (const FieldVector<OneDCType, 1>& local);
#endif
  private:

    //! the vertex coordinates
    FieldVector<OneDCType, 1> pos_;

  };

  //**********************************************************************
  //
  // --OneDGridElement
  /** \brief Defines the geometry part of a mesh entity.
   * \ingroup OneDGrid
   */
  template<int dim, int dimworld>
  class OneDGridElement :
    public ElementDefault <dim,dimworld, OneDCType,OneDGridElement>
  {
    template <int codim_, int dim_, int dimworld_>
    friend class OneDGridEntity;

    friend class OneDGrid<dim,dimworld>;

    template <int cc_, int dim_, int dimworld_>
    friend class OneDGridSubEntityFactory;

  public:

    /** \brief Return the element type identifier
     *
     * OneDGrid obviously supports only lines
     */
    ElementType type () const {return line;}

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () {return 2;}

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<OneDCType, dimworld>& operator[](int i) const {
      assert(i==0 || i==1);
      return vertex[i]->geometry().pos_;
    }

    /** \brief Return reference element corresponding to this element.
     *
     * \todo Implement this!
     **If this is a reference element then self is returned.
     */
    OneDGridElement<dim,dim>& refelem () {
      DUNE_THROW(NotImplemented, " OneDGridElement<dim,dim>& refelem ()");
    }

    /** \brief Maps a local coordinate within reference element to
     * global coordinate in element  */
    FieldVector<OneDCType, dimworld> global (const FieldVector<OneDCType, dim>& local) const {
      FieldVector<OneDCType, dimworld> g;
      g[0] = vertex[0]->geometry().pos_[0] * (1-local[0]) + vertex[1]->geometry().pos_[0] * local[0];
      return g;
    }

    /** \brief Maps a global coordinate within the element to a
     * local coordinate in its reference element */
    FieldVector<OneDCType, dim> local (const FieldVector<OneDCType, dimworld>& global) const {
      FieldVector<OneDCType, dim> l;
      const double& v0 = vertex[0]->geometry().pos_[0];
      const double& v1 = vertex[1]->geometry().pos_[0];
      l[0] = (global[0] - v0) / (v1 - v0);
      return l;
    }

    //! Returns true if the point is in the current element
    bool checkInside(const FieldVector<OneDCType, dimworld> &global) {
      return vertex[0]->geometry().pos_ <= global[0] && global[0] <= vertex[1]->geometry().pos_;
    }

    /** ???
     */
    OneDCType integration_element (const FieldVector<OneDCType, dim>& local) const {
      return vertex[1]->geometry().pos_ - vertex[0]->geometry().pos_;
    }

    //! The Jacobian matrix of the mapping from the reference element to this element
    const Mat<dim,dim>& Jacobian_inverse (const FieldVector<OneDCType, dim>& local) const {
      jacInverse_[0][0] = vertex[1]->geometry().pos_ - vertex[0]->geometry().pos_;
      return jacInverse_;
    }


  private:

    FixedArray<OneDGridEntity<1,1,1>*, 2> vertex;

    //! The jacobian inverse
    mutable Mat<dimworld,dimworld> jacInverse_;

    //! storage for local coords
    //FieldVector<OneDCType, dim> localCoord_;

  };



  // Include method definitions
  //#include "uggridelement.cc"

}  // namespace Dune

#endif
