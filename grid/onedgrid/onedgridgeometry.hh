// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ONE_D_GEOMETRY_HH
#define DUNE_ONE_D_GEOMETRY_HH

/** \file
 * \brief The OneDGridElement class and its specializations
 */

#include <dune/common/fixedarray.hh>

namespace Dune {

  template<int mydim, int coorddim, class GridImp>
  class OneDMakeableGeometry : public Geometry<mydim, coorddim, GridImp, OneDGridGeometry>
  {
  public:

    OneDMakeableGeometry() :
      Geometry<mydim, coorddim, GridImp, OneDGridGeometry>(OneDGridGeometry<mydim, coorddim, GridImp>())
    {};

    void setToTarget(OneDEntityImp<mydim>* target) {
      this->realGeometry.target_ = target;
    }

  };


  template <int codim, int dim, class GridImp>
  class OneDGridEntity;

  /****************************************************************/
  /*       Specialization for faces in a 1d grid (i.e. vertices)  */
  /****************************************************************/

  template<class GridImp>
  class OneDGridGeometry <0, 1, GridImp> :
    public GeometryDefault <0, 1, GridImp,OneDGridGeometry>
  {

    template <int codim_, int dim_, class GridImp_>
    friend class OneDGridEntity;
    template <int mydim_, int coorddim_, class GridImp_>
    friend class OneDGridGeometry;

    template <int cc_, int dim_, class GridImp_>
    friend class OneDGridSubEntityFactory;


  public:

    //! return the element type identifier (vertex)
    GeometryType type () const {return vertex;}

    //! return the number of corners of this element (==1)
    int corners () const {return 1;}

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<OneDCType, 1>& operator[] (int i) const {
      return target_->pos_;
    }

    //private:
    OneDEntityImp<0>* target_;

  };

  //**********************************************************************
  //
  // --OneDGridGeometry
  /** \brief Defines the geometry part of a mesh entity.
   * \ingroup OneDGrid
   */
  template<int mydim, int coorddim, class GridImp>
  class OneDGridGeometry :
    public GeometryDefault <mydim, coorddim, GridImp, OneDGridGeometry>
  {
    template <int codim_, int dim_, class GridImp_>
    friend class OneDGridEntity;

    friend class OneDGrid<GridImp::dimension, GridImp::dimensionworld>;

    template <int cc_, int dim_, class GridImp_>
    friend class OneDGridSubEntityFactory;

    friend class OneDGridIntersectionIterator<GridImp>;

  public:


    /** \brief Return the element type identifier
     *
     * OneDGrid obviously supports only lines
     */
    GeometryType type () const {return cube;}

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () const {return 2;}

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<OneDCType, coorddim>& operator[](int i) const {
      assert(i==0 || i==1);
      return target_->vertex_[i]->pos_;
    }

    /** \brief Maps a local coordinate within reference element to
     * global coordinate in element  */
    FieldVector<OneDCType, coorddim> global (const FieldVector<OneDCType, mydim>& local) const {
      FieldVector<OneDCType, coorddim> g;
      g[0] = target_->vertex_[0]->pos_[0] * (1-local[0]) + target_->vertex_[1]->pos_[0] * local[0];
      return g;
    }

    /** \brief Maps a global coordinate within the element to a
     * local coordinate in its reference element */
    FieldVector<OneDCType, mydim> local (const FieldVector<OneDCType, coorddim>& global) const {
      FieldVector<OneDCType, mydim> l;
      const double& v0 = target_->vertex_[0]->pos_[0];
      const double& v1 = target_->vertex_[1]->pos_[0];
      l[0] = (global[0] - v0) / (v1 - v0);
      return l;
    }

    //! Returns true if the point is in the current element
    bool checkInside(const FieldVector<OneDCType, coorddim> &global) const {
      return target_->vertex_[0]->pos_[0] <= global[0] && global[0] <= target_->vertex_[1]->pos_[0];
    }

    /** ???
     */
    OneDCType integrationElement (const FieldVector<OneDCType, mydim>& local) const {
      return target_->vertex_[1]->pos_[0] - target_->vertex_[0]->pos_[0];
    }

    //! The Jacobian matrix of the mapping from the reference element to this element
    const FieldMatrix<OneDCType,mydim,mydim>& jacobianInverseTransposed (const FieldVector<OneDCType, mydim>& local) const {
      jacInverse_[0][0] = 1 / (target_->vertex_[1]->pos_[0] - target_->vertex_[0]->pos_[0]);
      return jacInverse_;
    }


    //private:
    OneDEntityImp<1>* target_;

    //! The jacobian inverse
    mutable FieldMatrix<OneDCType,coorddim,coorddim> jacInverse_;

  };

}  // namespace Dune

#endif
