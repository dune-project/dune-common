#ifndef DUNE_COMMON_SIMD_BASE_HH
#define DUNE_COMMON_SIMD_BASE_HH

/** @file
 *  @brief Basic definitions for SIMD Implementations
 *
 * This file provides basic definitions and template declarations that are
 * used to write SIMD abtraction layers.
 *
 * This file should never be included by users of the SIMD
 * abstraction. Include <dune/common/simd/simd.hh> instead.
 */

namespace Dune {
  //! @defgroup SIMD
  //! @{

  //! @brief Namespace for Simd Abstraction
  namespace Simd {
    /** @defgroup SIMDImpl Implementors Interface
     *
     *  This group describes the interface that you must implement if you want
     *  to write an abstraction layer for some vectorization library.
     *
     *  @{
     */

    //! @brief Namespace for the overloads and specializations that make up a
    //!        SIMD implementation
    /**
     * This namespace contains three sets of things: the struct ADLTag, which
     * is used to look up functions in this namespace using argument-dependet
     * lookup, traits classes that must be specialized by implementations, and
     * functions that must be overloaded by implementations.
     *
     * An implementation must specialize all the template classes in this
     * namespace.  To make it possible for certain specializations not to
     * participate in overload resolution, each template class provides a
     * dummy template parameter \c SFINAETag that defaults to \c void.
     *
     * An implementation must overload all functions within this namespace
     * that are defined deleted.  It may overload other functions.  It must
     * not introduce new functions.  @see SIMDDefaults for a list of
     * functions.
     */
    namespace Overloads {

      //! Tag used to force late-binding lookup in Dune::Simd::Overloads
      /**
       * This tag is used by functions in \c Dune::Simd to make
       * argument-dependent lookups (ADL) for functions in \c
       * Dune::Simd::Overloads.  The property of ADL that is used here is that
       * it binds the names of functions late, i.e. at the time of
       * instantiation, while all other lookups bind early, i.e. at the time
       * when the function call is parsed.  Using late binding enables a
       * function \c foo() to find a functions \c Overloads::foo() that has
       * been declared only after \c foo() itself has been defined:
       *
       * \code
       *   template<class V>
       *   void foo(V v)
       *   {
       *     foo(Overloads::ADLTag<6>{}, v);
       *   }
       *
       *   struct MyType {};
       *   namespace Overloads {
       *     void foo(ADLTag<4>, MyType v);
       *   }
       * \endcode
       *
       * \note It is generally an error to declare a function with an ADLTag
       *       argument outside of namespace Simd::Overloads.  An exception
       *       would be an abstraction implementation that declares all its
       *       implementation functions in its own implementation namespace,
       *       and then pulls them into the namespace Overloads by way of \c
       *       using.
       *
       * \c ADLTag<i> derived from ADLTag<i-1>.  Thus it is possible to
       * prioritize overloads by choosing an appropriate \c i.  The following
       * values for \c i are predefined:
       * - \c i==0: this is used by the defaults.
       * - \c i==2: this is used by the implementation for standard types.
       * - \c i==5: this should be the default priority for implementations
       *
       * The folloing priorities should not normally be used.  However, they
       * may sometimes be necessary:
       * - \c i==1: let the standard implementation take precedence
       * - \c i==3: override standard implementation, but strongly favour
       *   other implementations
       * - \c i==4: override standard implementation, but weakly prefer other
       *   implementations
       * - \c i==6: try to override other implementations
       *
       * \c i==6 is the highest supported priority.
       *
       * The second (bool) template argument is to make writing abstraction
       * implementations that use SFINAE to remove (some of) their functions
       * from the overload set more concise.  \c ADLTag<i,false> is not
       * defined, so instead of
       * \code
       *   std::enable_if_t<cond, ADLTag<4> >
       * \endcode
       * you may write the equivalent
       * \code
       *   ADLTag<4, cond>
       * \endcode
       */
      template<unsigned i, bool = true>
      struct ADLTag;

      template<unsigned i>
      struct ADLTag<i> : ADLTag<i-1> {};

      template<>
      struct ADLTag<0> {};

      //! should have a member type \c type
      /**
       * Implements Simd::Scalar
       */
      template<class V, class SFINAETag = void>
      struct ScalarType;

      //! should have a member type \c type
      /**
       * Implements Simd::Index
       */
      template<class V, class SFINAETag = void>
      struct IndexType;

      //! should have a member type \c type
      /**
       * Implements Simd::Mask
       */
      template<class V, class SFINAETag = void>
      struct MaskType;

      //! should be derived from an Dune::index_constant
      /**
       * Implements Simd::lanes()
       */
      template<class V, class SFINAETag = void>
      struct LaneCount;

    } // namespace Overloads

    //! @} Group SIMDImpl
  } // namespace Simd
  //! @} Group SIMD

} // namespace Dune

#endif // DUNE_COMMON_SIMD_BASE_HH
