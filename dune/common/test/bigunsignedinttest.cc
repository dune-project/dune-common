// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstdint>
#include <limits>
#include <iostream>

#include <dune/common/typetraits.hh>
#include <dune/common/bigunsignedint.hh>
#include <dune/common/hash.hh>

#define CHECK(x) \
  do { \
    if (!(x)) { \
      pass = false; \
      std::cerr << "FAILED: " << #x << std::endl; \
    } \
  } while(false)

int main()
{
  bool pass = true;

  typedef Dune::bigunsignedint<16> ShortInteger;
  typedef Dune::bigunsignedint<128> BigInteger;

  static_assert(Dune::IsNumber<ShortInteger>::value);
  static_assert(Dune::IsNumber<BigInteger>::value);

  /* Test std::numeric_limits for ShortInteger (should be same as for uint16_t) */
  CHECK(std::numeric_limits<ShortInteger>::min() == std::numeric_limits<std::uint16_t>::min());
  CHECK(std::numeric_limits<ShortInteger>::max() == std::numeric_limits<std::uint16_t>::max());
  CHECK(std::numeric_limits<ShortInteger>::digits == std::numeric_limits<std::uint16_t>::digits);
  CHECK(std::numeric_limits<ShortInteger>::epsilon() == std::numeric_limits<std::uint16_t>::epsilon());
  CHECK(std::numeric_limits<ShortInteger>::round_error() == std::numeric_limits<std::uint16_t>::round_error());

  CHECK(std::numeric_limits<ShortInteger>::is_exact);
  CHECK(std::numeric_limits<ShortInteger>::is_integer);
  CHECK(!std::numeric_limits<ShortInteger>::is_signed);

  /* Test std::numeric_limits for BigInteger */
  CHECK(std::numeric_limits<BigInteger>::min() == 0u);
  CHECK(std::numeric_limits<BigInteger>::digits == 128);
  CHECK(std::numeric_limits<BigInteger>::epsilon() == 0u);
  CHECK(std::numeric_limits<BigInteger>::round_error() == 0u);

  CHECK(std::numeric_limits<BigInteger>::is_exact);
  CHECK(std::numeric_limits<BigInteger>::is_integer);
  CHECK(!std::numeric_limits<BigInteger>::is_signed);

  /* Test constructor */
  CHECK(BigInteger(10u) == 10u);
  CHECK(BigInteger(10) == BigInteger(10u));

  try {
    BigInteger tmp(-10);
    pass = false;
    std::cerr << "FAILED: BigInteger(-10) should throw an exception." << std::endl;
  }
  catch(const Dune::Exception&) {
    /* Ignore */
  }
  catch(...) {
    pass = false;
    std::cerr << "FAILED: BigInteger(-10) threw an unexpected exception." << std::endl;
  }

  /* Test conversion */
  CHECK(BigInteger(10u).touint() == 10u);
  CHECK(BigInteger(10u).todouble() == 10.0);

  /* Check BigInteger arithmetic */
  CHECK(BigInteger(10u) + BigInteger(3u) == BigInteger(10u + 3u));
  BigInteger tmp(10u); tmp += BigInteger(3u);
  CHECK(tmp == BigInteger(10u + 3u));
  CHECK(BigInteger(10u) - BigInteger(3u) == BigInteger(10u - 3u));
  tmp = BigInteger(10u); tmp -= BigInteger(3u);
  CHECK(tmp == BigInteger(10u - 3u));
  CHECK(BigInteger(10u) * BigInteger(3u) == BigInteger(10u * 3u));
  tmp = BigInteger(10u); tmp *= BigInteger(3u);
  CHECK(tmp == BigInteger(10u * 3u));
  CHECK(BigInteger(10u) / BigInteger(3u) == BigInteger(10u / 3u));
  tmp = BigInteger(10u); tmp /= BigInteger(3u);
  CHECK(tmp == BigInteger(10u / 3u));
  CHECK(BigInteger(10u) % BigInteger(3u) == BigInteger(10u % 3u));
  tmp = BigInteger(10u); tmp %= BigInteger(3u);
  CHECK(tmp == BigInteger(10u % 3u));

  CHECK(BigInteger(100000u) + BigInteger(30000u) == BigInteger(100000u + 30000u));
  tmp = BigInteger(100000u); tmp += BigInteger(30000u);
  CHECK(tmp == BigInteger(100000u + 30000u));
  CHECK(BigInteger(100000u) - BigInteger(30000u) == BigInteger(100000u - 30000u));
  tmp = BigInteger(100000u); tmp -= BigInteger(30000u);
  CHECK(tmp == BigInteger(100000u - 30000u));
  CHECK(BigInteger(70000u) - BigInteger(30000u) == BigInteger(70000u - 30000u));
  tmp = BigInteger(70000u); tmp -= BigInteger(30000u);
  CHECK(tmp == BigInteger(70000u - 30000u));
  CHECK(BigInteger(100000u) * BigInteger(30000u) == BigInteger(100000u * 30000u));
  tmp = BigInteger(100000u); tmp *= BigInteger(30000u);
  CHECK(tmp == BigInteger(100000u * 30000u));
  CHECK(BigInteger(100000u) / BigInteger(30000u) == BigInteger(100000u / 30000u));
  tmp = BigInteger(100000u); tmp /= BigInteger(30000u);
  CHECK(tmp == BigInteger(100000u / 30000u));
  CHECK(BigInteger(100000u) % BigInteger(30000u) == BigInteger(100000u % 30000u));
  tmp = BigInteger(100000u); tmp %= BigInteger(30000u);
  CHECK(tmp == BigInteger(100000u % 30000u));

  /* Test hashing */
  {
    Dune::hash<BigInteger> hasher;
    CHECK(hasher(BigInteger(100)) == hasher(BigInteger(100)));
  }
  const BigInteger one{1};
  const BigInteger zero{0};
  CHECK((one & one) == one);
  CHECK((one & zero) == zero);
  CHECK((one | one) == one);
  CHECK((one | zero) == one);
  CHECK((one ^ one) == zero);
  CHECK((one ^ zero) == one);

  return pass ? 0 : 1;
}
