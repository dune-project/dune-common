// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ONE_D_GEOMETRY_HH
#define DUNE_ONE_D_GEOMETRY_HH

/** \file
 * \brief The OneDGridElement class and its specializations
 */

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

    void setPosition(double p) {
      this->realGeometry.storeCoordsLocally_ = true;
      this->realGeometry.pos_[0] = p;
    }
  };

  template<class GridImp>
  class OneDMakeableGeometry<1,1,GridImp> : public Geometry<1, 1, GridImp, OneDGridGeometry>
  {
  public:

    OneDMakeableGeometry() :
      Geometry<1, 1, GridImp, OneDGridGeometry>(OneDGridGeometry<1, 1, GridImp>())
    {};

    void setToTarget(OneDEntityImp<1>* target) {
      this->realGeometry.target_ = target;
    }

    void setPositions(double p1, double p2) {
      this->realGeometry.storeCoordsLocally_ = true;
      this->realGeometry.pos_[0][0] = p1;
      this->realGeometry.pos_[1][0] = p2;
    }
  };

  // forward declaration
  template <int codim, int dim, class GridImp>
  class OneDGridEntity;


  template<class GridImp>
  class OneDGridGeometry <0, 1, GridImp> :
    public GeometryDefaultImplementation <0, 1, GridImp,OneDGridGeometry>
  {

    template <int codim_, int dim_, class GridImp_>
    friend class OneDGridEntity;
    template <int mydim_, int coorddim_, class GridImp_>
    friend class OneDGridGeometry;

  public:

    OneDGridGeometry() : storeCoordsLocally_(false) {}

    //! return the element type identifier (vertex)
    GeometryType type () const {return GeometryType(0);}

    //! return the number of corners of this element (==1)
    int corners () const {return 1;}

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<typename GridImp::ctype, 1>& operator[] (int i) const {
      return (storeCoordsLocally_) ? pos_ : target_->pos_;
    }

    /** \brief Maps a local coordinate within reference element to
     * global coordinate in element  */
    FieldVector<typename GridImp::ctype, 1> global (const FieldVector<typename GridImp::ctype, 0>& local) const {
      return (storeCoordsLocally_) ? pos_ : target_->pos_;
    }

    /** \brief Maps a global coordinate within the element to a
     * local coordinate in its reference element */
    FieldVector<typename GridImp::ctype, 0> local (const FieldVector<typename GridImp::ctype, 1>& global) const {
      FieldVector<typename GridImp::ctype, 0> l;
      return l;
    }

    /** \brief Returns true if the point is in the current element

       This method really doesn't make much sense for a zero-dimensional
       object.  It always returns 'true'.
     */
    bool checkInside(const FieldVector<typename GridImp::ctype, 0> &local) const {
      return true;
    }


    /** \brief !!!

       This method really doesn't make much sense for a zero-dimensional
       object.  It always returns '1'.
     */
    typename GridImp::ctype integrationElement (const FieldVector<typename GridImp::ctype, 0>& local) const {
      return 1;
    }

    //! The Jacobian matrix of the mapping from the reference element to this element
    const FieldMatrix<typename GridImp::ctype,0,0>& jacobianInverseTransposed (const FieldVector<typename GridImp::ctype, 0>& local) const {
      return jacInverse_;
    }

    //private:
    bool storeCoordsLocally_;

    // Stores the element corner positions if it is returned as geometryInFather
    FieldVector<typename GridImp::ctype,1> pos_;

    OneDEntityImp<0>* target_;

    FieldMatrix<typename GridImp::ctype,0,0> jacInverse_;
  };

  //**********************************************************************
  //
  // --OneDGridGeometry
  /** \brief Defines the geometry part of a mesh entity.
   * \ingroup OneDGrid
   */
  template<int mydim, int coorddim, class GridImp>
  class OneDGridGeometry :
    public GeometryDefaultImplementation <mydim, coorddim, GridImp, OneDGridGeometry>
  {
    template <int codim_, int dim_, class GridImp_>
    friend class OneDGridEntity;

    friend class OneDGrid<GridImp::dimension, GridImp::dimensionworld>;

    template <int cc_, int dim_, class GridImp_>
    friend class OneDGridSubEntityFactory;

    friend class OneDGridIntersectionIterator<GridImp>;

  public:

    OneDGridGeometry() : storeCoordsLocally_(false) {}

    /** \brief Return the element type identifier
     *
     * OneDGrid obviously supports only lines
     */
    GeometryType type () const {return GeometryType(1);}

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () const {return 2;}

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<typename GridImp::ctype, coorddim>& operator[](int i) const {
      assert(i==0 || i==1);
      return (storeCoordsLocally_) ? pos_[i] : target_->vertex_[i]->pos_;
    }

    /** \brief Maps a local coordinate within reference element to
     * global coordinate in element  */
    FieldVector<typename GridImp::ctype, coorddim> global (const FieldVector<typename GridImp::ctype, mydim>& local) const {
      FieldVector<typename GridImp::ctype, coorddim> g;
      g[0] = (storeCoordsLocally_)
             ? pos_[0][0] * (1-local[0]) + pos_[1][0] * local[0]
             : target_->vertex_[0]->pos_[0] * (1-local[0]) + target_->vertex_[1]->pos_[0] * local[0];
      return g;
    }

    /** \brief Maps a global coordinate within the element to a
     * local coordinate in its reference element */
    FieldVector<typename GridImp::ctype, mydim> local (const FieldVector<typename GridImp::ctype, coorddim>& global) const {
      FieldVector<typename GridImp::ctype, mydim> l;
      if (storeCoordsLocally_) {
        l[0] = (global[0] - pos_[0][0]) / (pos_[1][0] - pos_[0][0]);
      } else {
        const double& v0 = target_->vertex_[0]->pos_[0];
        const double& v1 = target_->vertex_[1]->pos_[0];
        l[0] = (global[0] - v0) / (v1 - v0);
      }
      return l;
    }

    //! Returns true if the point is in the current element
    bool checkInside(const FieldVector<typename GridImp::ctype, coorddim> &global) const {
      return (storeCoordsLocally_)
             ? pos_[0][0] <= global[0] && global[0] <= pos_[1][0]
             : target_->vertex_[0]->pos_[0] <= global[0] && global[0] <= target_->vertex_[1]->pos_[0];
    }

    /** ???
     */
    typename GridImp::ctype integrationElement (const FieldVector<typename GridImp::ctype, mydim>& local) const {
      return (storeCoordsLocally_)
             ? pos_[1][0] - pos_[0][0]
             : target_->vertex_[1]->pos_[0] - target_->vertex_[0]->pos_[0];
    }

    //! The Jacobian matrix of the mapping from the reference element to this element
    const FieldMatrix<typename GridImp::ctype,mydim,mydim>& jacobianInverseTransposed (const FieldVector<typename GridImp::ctype, mydim>& local) const {
      if (storeCoordsLocally_)
        jacInverse_[0][0] = 1 / (pos_[1][0] - pos_[0][0]);
      else
        jacInverse_[0][0] = 1 / (target_->vertex_[1]->pos_[0] - target_->vertex_[0]->pos_[0]);

      return jacInverse_;
    }


    //private:
    OneDEntityImp<1>* target_;

    bool storeCoordsLocally_;

    // Stores the element corner positions if it is returned as geometryInFather
    FieldVector<typename GridImp::ctype,coorddim> pos_[2];

    //! The jacobian inverse
    mutable FieldMatrix<typename GridImp::ctype,coorddim,coorddim> jacInverse_;

  };

}  // namespace Dune

#endif
