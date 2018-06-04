#ifndef DUNE_COMMON_SIMD_ISSTANDARD_HH
#define DUNE_COMMON_SIMD_ISSTANDARD_HH

/** @file
 *  @ingroup SIMDStandard
 *  @brief Traits to determine which types the standard abstraction applies to
 *
 * This file contains a single traits template class `Dune::Simd::IsStandard`
 * that can be specialized to make the standard abstraction implementation
 * apply to extra types.  It is seperate from `<dune/common/simd/standard.hh>`
 * so that any header that includes this file does not need to pull in the
 * complete SIMD abstraction layer.
 */

#include <type_traits>

/** @defgroup SIMDStandard SIMD Abstraction Implementation for standard types
 *  @ingroup SIMDApp
 *
 * This implements the vectorization interface for scalar types, in particular
 * the standard arithmetic types as per `std::is_arithmetic`, as well as
 * `std::complex<T>` for any supported type `T`.
 *
 * If you have a type that provides an interface sufficiently close to the
 * standard types, you can enable support for that type in this abstraction
 * implementation by specializing `Dune::Simd::IsStandard`.  Candidates for
 * this include extended precision types and automatic differentiation types.
 * You need to make sure that this happens only once.  So if there is a Dune
 * header that should be included anyway to enable support for your type, that
 * header is a good place to add the specialization.
 */

namespace Dune {
  namespace Simd {

    /** @ingroup SIMDStandard
     *  @{
     */

    //! Which types are subject to the standard SIMD abstraction implementation
    /**
     * \note This is obsolete, do not use, see
     *       https://gitlab.dune-project.org/core/dune-common/issues/121#note_44588
     *
     * This is derive from \c std::true_type if \c T should be handled by the
     * standard SIMD abstraction implementation, otherwise from \c
     * std::false_type.  By default it declares all arithmetic types (as per
     * \c std::is_arithmetic) as handled.  In addition it declares the
     * following types as handled if \c T is handled: \c const T, \c volatile
     * T, \c T&, \c T&&, \c std::complex<T>.
     *
     * This may be specialized by dune headers that provide types that are
     * sufficiently close to the standard types.  Good candidates include
     * extended precision types and automatic differentiation types.
     *
     * You need to make sure that this happens only once.  So if there is a
     * Dune header that should be included anyway to enable support for your
     * type, that header is a good place to add the specialization.
     *
     * \note If you plan to use `IsStandard` for your own devices (you
     *       probably should not), be aware that the specialization for
     *       `std::complex` is in `<dune/common/simd/standard.hh>` so that
     *       `<dune/common/simd/isstandard.hh>` does not need to include
     *       `<complex>`.  This is because many headers will need to include
     *       `<dune/common/simd/isstandard.hh>`, and thus we try to pull in as
     *       little as we can get away with.
     */
    template<class T, class SFINAE = void>
    struct IsStandard : std::true_type {};

    //! group SIMDStandard
  } // namespace Simd
} // namespace Dune

#endif // DUNE_COMMON_SIMD_ISSTANDARD_HH
