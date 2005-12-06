// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_BOUNDARY_EXTRACTOR_HH
#define DUNE_BOUNDARY_EXTRACTOR_HH

/** \file
    \brief Contains a helper class for the creation of UGGrid objects
    \author Oliver Sander
 */

#include <vector>
#include <dune/common/fvector.hh>


namespace Dune {

  class BoundaryExtractor {

  public:

    static void detectBoundarySegments(std::vector<unsigned char> elementTypes,
                                       std::vector<unsigned int> elementVertices,
                                       std::vector<FieldVector<int, 2> >& boundarySegments);

    static void detectBoundarySegments(std::vector<unsigned char> elementTypes,
                                       std::vector<unsigned int> elementVertices,
                                       std::vector<FieldVector<int, 4> >& faceList);

    template <int NumVertices>
    static int detectBoundaryNodes(const std::vector<FieldVector<int, NumVertices> >& faceList,
                                   int noOfNodes,
                                   std::vector<int>& isBoundaryNode);

  };

  /** \brief Function object comparing two boundary segments

     This general implementation is empty.  Only specializations for dim==2 and dim==3 exist.
   */
  template <int dim>
  class CompareBoundarySegments {};

  /** \brief Function object comparing two 2d boundary segments

     This class implements a less-than operation on 2d boundary segments.  This way we can
     insert them into a std::set.
   */
  template <>
  struct CompareBoundarySegments<2> {
    bool operator()(const FieldVector<int,2>& s1, const FieldVector<int,2>& s2) const
    {
      FieldVector<int,2> sorted1, sorted2;

      // ////////////////////////////////////////////////////////////////////////////
      // Sort the two arrays to get rid of cyclic permutations in mirror symmetry
      // ////////////////////////////////////////////////////////////////////////////
      if (s1[0] < s1[1])
        sorted1 = s1;
      else {
        sorted1[0] = s1[1];
        sorted1[1] = s1[0];
      }

      if (s2[0] < s2[1])
        sorted2 = s2;
      else {
        sorted2[0] = s2[1];
        sorted2[1] = s2[0];
      }

      // ////////////////////////////////////////////////////////////////////////////
      //   Compare the two sorted arrays
      // ////////////////////////////////////////////////////////////////////////////
      for (int i=0; i<2; i++) {
        if (sorted1[i]<sorted2[i])
          return true;
        else if (sorted1[i]>sorted2[i])
          return false;
      }

      // The sorted arrays are identical
      return false;
    }
  };

  /** \brief Function object comparing two 3d boundary segments

     This class implements a less-than operation on 3d boundary segments.  This way we can
     insert them into a std::set.
   */
  template <>
  struct CompareBoundarySegments<3> {
    bool operator()(const FieldVector<int,4>& s1, const FieldVector<int,4>& s2) const
    {
      FieldVector<int,4> sorted1 = s1;
      FieldVector<int,4> sorted2 = s2;

      // ////////////////////////////////////////////////////////////////////////////
      //  The boundary extraction algorithm marks triangular segments by letting the
      //  last two entries be equal.  But this comparison algorithm relies on all
      //  four entries being different.  So for here we mark triangles by letting
      //  the last entry be -1.
      // ////////////////////////////////////////////////////////////////////////////

      if (sorted1[2]==sorted1[3])
        sorted1[3] = -1;
      if (sorted2[2]==sorted2[3])
        sorted2[3] = -1;

      // ////////////////////////////////////////////////////////////////////////////
      // Sort the two arrays to get rid of cyclic permutations in mirror symmetry
      // ////////////////////////////////////////////////////////////////////////////

      // bubble sort
      for (int i=3; i>=1; i--) {

        for (int j=0; j<i; j++) {

          if (sorted1[j] > sorted1[j+1]) {
            int tmp = sorted1[j];
            sorted1[j] = sorted1[j+1];
            sorted1[j+1] = tmp;
          }

          if (sorted2[j] > sorted2[j+1]) {
            int tmp = sorted2[j];
            sorted2[j] = sorted2[j+1];
            sorted2[j+1] = tmp;
          }

        }

      }

      // ////////////////////////////////////////////////////////////////////////////
      //   Compare the two sorted arrays
      // ////////////////////////////////////////////////////////////////////////////
      for (int i=0; i<4; i++) {
        if (sorted1[i]<sorted2[i])
          return true;
        else if (sorted1[i]>sorted2[i])
          return false;
      }

      // The sorted arrays are identical
      return false;
    }

  };


}

#endif
