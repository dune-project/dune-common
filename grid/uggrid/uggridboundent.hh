// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_UGGRID_BOUNDENT_HH__
#define __DUNE_UGGRID_BOUNDENT_HH__


namespace Dune {

#define NEIGH_DEBUG

  /** \todo Please doc me! */
  template<int dim, int dimworld>
  class UGGridBoundaryEntity
    : public BoundaryEntityDefault <dim,dimworld, UGCtype,
          UGGridElement,UGGridBoundaryEntity>
  {
    friend class UGGridIntersectionIterator<dim,dimworld>;
  public:
    UGGridBoundaryEntity () : _geom (false) , /* _elInfo ( NULL ), */
                              _neigh (-1) {};

    //! return type of boundary , i.e. Neumann, Dirichlet ...
    BoundaryType type ()
    {
#ifdef NEIGH_DEBUG
      if(_elInfo->boundary[_neigh] == NULL)
      {
        std::cerr << "No Boundary, fella! \n";
        abort();
      }
#endif
      return (( _elInfo->boundary[_neigh]->bound < 0 ) ? Neumann : Dirichlet );
    }

    //! return identifier of boundary segment, number
    int id ()
    {
#ifdef NEIGH_DEBUG
      if(_elInfo->boundary[_neigh] == NULL)
      {
        std::cerr << "No Boundary, fella! \n";
        abort();
      }
#endif
      return _elInfo->boundary[_neigh]->bound;
    }

    //! return true if geometry of ghost cells was filled
    bool hasGeometry () { return _geom.builtGeom(_elInfo,0,0,0); }

    //! return geometry of the ghost cell
    UGGridElement<dim,dimworld> geometry ()
    {
      return _geom;
    }

  private:

    int _neigh;

    // UGGrid<dim,dimworld> & _grid;
    UGGridElement<dim,dimworld> _geom;

  };

}  // namespace Dune

#endif
