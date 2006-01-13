// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COLLECTIVECOMMUNICATION_HH
#define DUNE_COLLECTIVECOMMUNICATION_HH

#include <iostream>
#include <complex>
#include <algorithm>

#include "exceptions.hh"

namespace Dune
{
  // a default class for collective communication
  template<typename C>
  class CollectiveCommunication
  {
  public:
    CollectiveCommunication ()
    {}

    int rank () const
    {
      return 0;
    }

    int size () const
    {
      return 1;
    }

    template<typename T>
    T sum (T& in) const     // MPI does not know about const :-(
    {
      return in;
    }

    template<typename T>
    int sum (T* inout, int len) const
    {
      return 0;
    }

    template<typename T>
    T prod (T& in) const     // MPI does not know about const :-(
    {
      return in;
    }

    template<typename T>
    int prod (T* inout, int len) const
    {
      return 0;
    }

    template<typename T>
    T min (T& in) const     // MPI does not know about const :-(
    {
      return in;
    }

    template<typename T>
    int min (T* inout, int len) const
    {
      return 0;
    }

    template<typename T>
    T max (T& in) const     // MPI does not know about const :-(
    {
      return in;
    }

    template<typename T>
    int max (T* inout, int len) const
    {
      return 0;
    }

    int barrier () const
    {
      return 0;
    }

    template<typename T>
    int broadcast (T* inout, int len, int root) const
    {
      return 0;
    }

    template<typename T>
    int gather (T* in, T* out, int len, int root) const     // note out must have same size as in
    {
      for (int i=0; i<len; i++)
        out[i] = in[i];
      return 0;
    }
  };
}

#endif
