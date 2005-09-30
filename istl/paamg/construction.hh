// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_AMGCONSTRUCTION_HH
#define DUNE_AMGCONSTRUCTION_HH

namespace Dune
{
  namespace Amg
  {

    /**
     * @addtogroup ISTL_PAAMG
     *
     * @{
     */

    /** @file
     * @author Markus Blatt
     * @brief Helper classes for the construction of classes without
     * empty constructor.
     */
    /**
     * @brief Traits class for generically constructing non default
     * constructable types.
     *
     * Needed because BCRSMatrix and Vector do a deep copy which is
     * too expensive.
     */
    template<typename T>
    class ConstructionTraits
    {
    public:
      /**
       * @brief A type holding all the arguments needed to call the
       * constructor.
       */
      typedef const void* Arguments;

      /**
       * @brief Construct an object with the specified arguments.
       *
       * In the default implementation the copy constructor is called.
       * @param object Pointer to the space allocated for the object.
       * @param arguments The arguments for the construction.
       */
      static inline T* construct(Arguments&  args)
      {
        return new T();
      }
    };

    template<class T>
    class ConstructionTraits<BlockVector<T> >
    {
    public:
      typedef const int Arguments;
      static inline BlockVector<T>* construct(Arguments& n)
      {
        return new BlockVector<T>(n);
      }
    };

    /** @} */
  } // namespace Amg
} // namespace Dune
#endif
