// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifndef DUNE_PLOCALINDEX_HH
#define DUNE_PLOCALINDEX_HH

#include "localindex.hh"
#include "indexset.hh"
#include "mpitraits.hh"

#include <iostream>

namespace Dune
{


  /** @addtogroup Common_Parallel
   *
   * @{
   */
  /**
   * @file
   * @brief Provides classes for use as the local index in ParallelIndexSet for distributed computing.
   * @author Markus Blatt
   */

  template<class T> class ParallelLocalIndex;

  /**
   * @brief Print the local index to a stream.
   * @param os The output stream to print to.
   * @param index The index to print.
   */
  template<class T>
  std::ostream& operator<<(std::ostream& os, const ParallelLocalIndex<T>& index)
  {
    os<<"{local="<<index.localIndex_<<", attr="<<T(index.attribute_)<<", public="
    <<(index.public_ ? true : false)<<"}";
    return os;
  }

  /**
   * @brief An index present on the local process with an additional attribute flag.
   */
  template<typename T>
  class ParallelLocalIndex
  {
#if HAVE_MPI
    // friend declaration needed for MPITraits
    friend struct MPITraits<ParallelLocalIndex<T> >;
#endif
    friend std::ostream& operator<<<>(std::ostream& os, const ParallelLocalIndex<T>& index);

  public:
    /**
     * @brief The type of the attributes.
     * Normally this will be an enumeration like
     * <pre>
     * enum Attributes{owner, border, overlap};
     * </pre>
     */
    typedef T Attribute;
    /**
     * @brief Constructor.
     *
     * The local index will be initialized to 0.
     * @param attribute The attribute of the index.
     * @param isPublic True if the index might also be
     * known to other processes.
     */
    ParallelLocalIndex(const Attribute& attribute, bool isPublic);

    /**
     * @brief Constructor.
     *
     * @param localIndex The local index.
     * @param attribute The attribute of the index.
     * @param isPublic True if the index might also be
     * known to other processes.
     */
    ParallelLocalIndex(size_t localIndex, const Attribute& attribute, bool isPublic=true);
    /**
     * @brief Parameterless constructor.
     *
     * Needed for use in container classes.
     */
    ParallelLocalIndex();

#if 0
    /**
     * @brief Constructor.
     * @param globalIndex The global index.
     * @param attribute The attribute of the index.
     * @param local The local index.
     * @param isPublic True if the index might also be
     * known to other processes.
     *
     */
    ParallelLocalIndex(const Attribute& attribute, size_t local, bool isPublic);
#endif

    /**
     * @brief Get the attribute of the index.
     * @return The associated attribute.
     */
    inline const Attribute attribute() const;

    /**
     * @brief Set the attribute of the index.
     * @param attribute The associated attribute.
     */
    inline void setAttribute(const Attribute& attribute);

    /**
     * @brief get the local index.
     * @return The local index.
     */
    inline size_t local() const;

    /**
     * @brief Convert to the local index represented by an int.
     */
    inline operator size_t() const;

    /**
     * @brief Assign a new local index.
     *
     * @param index The new local index.
     */
    inline ParallelLocalIndex<Attribute>& operator=(size_t index);

    /**
     * @brief Check whether the index might also be known other processes.
     * @return True if the index might be known to other processors.
     */
    inline bool isPublic() const;

    /**
     * @brief Get the state.
     * @return The state.
     */
    inline LocalIndexState state() const;

    /**
     * @brief Set the state.
     * @param state The state to set.
     */
    inline void setState(const LocalIndexState& state);

  private:
    /** @brief The local index. */
    size_t localIndex_;

    /** @brief An attribute for the index. */
    char attribute_;

    /** @brief True if the index is also known to other processors. */
    char public_;

    /**
     * @brief The state of the index.
     *
     * Has to be one of LocalIndexState!
     * @see LocalIndexState.
     */
    char state_;

  };

  template<typename T>
  bool operator==(const ParallelLocalIndex<T>& p1,
                  const ParallelLocalIndex<T>& p2)
  {
    if(p1.local()!=p2.local())
      return false;
    if(p1.attribute()!=p2.attribute())
      return false;
    if(p1.isPublic()!=p2.isPublic())
      return false;
    return true;
  }
  template<typename T>
  bool operator!=(const ParallelLocalIndex<T>& p1,
                  const ParallelLocalIndex<T>& p2)
  {
    return !(p1==p2);
  }


  template<typename T>
  struct LocalIndexComparator<ParallelLocalIndex<T> >
  {
    static bool compare(const ParallelLocalIndex<T>& t1,
                        const ParallelLocalIndex<T>& t2){
      return t1.attribute()<t2.attribute();
    }
  };


#if HAVE_MPI

  //! \todo Please doc me!
  template<typename T>
  class MPITraits<ParallelLocalIndex<T> >
  {
  public:
    static MPI_Datatype getType();
  private:
    static MPI_Datatype type;

  };

#endif

  template<class T>
  ParallelLocalIndex<T>::ParallelLocalIndex(const T& attribute, bool isPublic)
    : localIndex_(0), attribute_(static_cast<char>(attribute)),
      public_(static_cast<char>(isPublic)), state_(static_cast<char>(VALID))
  {}


  template<class T>
  ParallelLocalIndex<T>::ParallelLocalIndex(size_t local, const T& attribute, bool isPublic)
    : localIndex_(local), attribute_(static_cast<char>(attribute)),
      public_(static_cast<char>(isPublic)), state_(static_cast<char>(VALID))
  {}

  template<class T>
  ParallelLocalIndex<T>::ParallelLocalIndex()
    : localIndex_(0), attribute_(), public_(static_cast<char>(false)),
      state_(static_cast<char>(VALID))
  {}

  template<class T>
  inline const T ParallelLocalIndex<T>::attribute() const
  {
    return T(attribute_);
  }

  template<class T>
  inline void
  ParallelLocalIndex<T>::setAttribute(const Attribute& attribute)
  {
    attribute_ = attribute;
  }

  template<class T>
  inline size_t ParallelLocalIndex<T>::local() const
  {
    return localIndex_;
  }

  template<class T>
  inline ParallelLocalIndex<T>::operator size_t() const
  {
    return localIndex_;
  }

  template<class T>
  inline ParallelLocalIndex<T>&
  ParallelLocalIndex<T>::operator=(size_t index)
  {
    localIndex_=index;
    return *this;
  }

  template<class T>
  inline bool ParallelLocalIndex<T>::isPublic() const
  {
    return static_cast<bool>(public_);
  }

  template<class T>
  inline LocalIndexState ParallelLocalIndex<T>::state() const
  {
    return LocalIndexState(state_);
  }

  template<class T>
  inline void ParallelLocalIndex<T>::setState(const LocalIndexState& state)
  {
    state_=static_cast<char>(state);
  }

#if HAVE_MPI

  template<typename T>
  MPI_Datatype MPITraits<ParallelLocalIndex<T> >::getType()
  {

    if(type==MPI_DATATYPE_NULL) {
      int length = 1;
      MPI_Aint base, disp;
      MPI_Datatype types[1] = {MPITraits<char>::getType()};
      ParallelLocalIndex<T> rep;
      MPI_Get_address(&rep, &base);
      MPI_Get_address(&(rep.attribute_), &disp);
      disp -= base;

      MPI_Datatype tmp;
      MPI_Type_create_struct(1, &length, &disp, types, &tmp);

      MPI_Type_create_resized(tmp, 0, sizeof(ParallelLocalIndex<T>), &type);
      MPI_Type_commit(&type);

      MPI_Type_free(&tmp);
    }
    return type;
  }

  template<typename T>
  MPI_Datatype MPITraits<ParallelLocalIndex<T> >::type = MPI_DATATYPE_NULL;

#endif


  /** @} */
} // namespace Dune

#endif
