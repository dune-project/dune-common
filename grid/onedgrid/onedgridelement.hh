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

  public:

    /** \brief Return the element type identifier
     *
     * OneDGrid obviously supports only lines
     */
    ElementType type () {return line;}

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () {return 2;}

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<OneDCType, dimworld>& operator[] (int i) {
      assert(i==0 || i==1);
      OneDGridEntity<1,1,1> v = *vertex[i];
      OneDGridElement<0,1> g = v.geometry();
      return v.geometry().pos_;
    }

    /** \brief Return reference element corresponding to this element.
     *
     **If this is a reference element then self is returned.
     */
    OneDGridElement<dim,dim>& refelem ();

    /** \brief Maps a local coordinate within reference element to
     * global coordinate in element  */
    FieldVector<OneDCType, dimworld> global (const FieldVector<OneDCType, dim>& local);

    /** \brief Maps a global coordinate within the element to a
     * local coordinate in its reference element */
    FieldVector<OneDCType, dim> local (const FieldVector<OneDCType, dimworld>& global);

    //! Returns true if the point is in the current element
    bool checkInside(const FieldVector<OneDCType, dimworld> &global);

    /** ???
     */
    OneDCType integration_element (const FieldVector<OneDCType, dim>& local);

    //! The Jacobian matrix of the mapping from the reference element to this element
    const Mat<dim,dim>& Jacobian_inverse (const FieldVector<OneDCType, dim>& local);


  private:

    FixedArray<DoubleLinkedList<OneDGridEntity<1,1,1> >::Iterator, 2> vertex;

    //! The jacobian inverse
    Mat<dimworld,dimworld> jac_inverse_;

    //! storage for local coords
    //FieldVector<OneDCType, dim> localCoord_;

  };



  // Include method definitions
  //#include "uggridelement.cc"

}  // namespace Dune

#endif
