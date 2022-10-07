<!--
SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
-->

This document is a collection of thoughts and rationals on a proper SIMD
interface for Dune.

What do we want?
================

We want an abstraction layer for SIMD-libraries, that allows the core library
to handle simd-vector-like types where (until now) it could only handle scalar
types.  It is expected that those parts of the core library that want to
support vector-like types will need some adaptation to account for corner
cases that appear only when vectorizing.  However, it should usually be
uneccessary to maintain a scalar version of the code -- the vectorized version
should be able to handle both vectorized and scalar data types.

What this abstraction layer does not provide (at least initially) is a way to
actually create vector types from scratch.  It must however provide a way to
create corresponding types to a type that already exist.  I.e. if your code
got simd-vector-type argument, the abstraction layer will provide you with the
the number of lanes, and the type of the entries.  It will also provide you
with a way to create simd types with the same number of lanes but a different
entry type.

Built-in Types
==============

We generally do not want to have to modify existing interfaces.  This implies
that the built-in types must be a valid "vectorization library" that the
abstraction layer can deal with.  Since the built-in types are not classes,
this precludes certain idioms that are widespread among vectorization
libraries.

For instance if `x` is of a vector type, in many libraries one would access
the `i`'th lane of `x` with the expression `x[i]`.  We cannot support this
expression if `x` is of a scalar built-in type, because we can overload
`operator[]` only for class types.  An alternative syntax is to use a
free-standing access function `lane(i, x)`.

Restriction on Vectorization Libraries
======================================

We generally expect vectorization libraries to provide all the usual operators
(arithmetic operations, assignment, comparisons) for their vector types.
Comparisons should yield mask types specific to that vectorization library;
these must be summarized to `bool` with functions of the abstraction layer
(like `anyTrue()`) before they can be used in `if`-conditions.

We may require them to provide conversions from scalar types to vector types
to some extend, however, an exact specification needs more experience.

Specifically for vectors (or masks) `v1` and `v2` of type `V`, with associated
scalar type `T=Scalar<V>`, we require

- for any unary arithmetic expression `@v1` (where `@` is one of `+`, `-`, or
  `~`):  
  `lane(l,@v1) == T(@lane(l,v1))` for all `l`  
  there are no side-effects

- for any binary arithmetic expression `v1@v2` (where `@` is one of `+`, `-`,
  `*`, `/`, `%`, `<<`, `>>`, `&`, `|`, `^`):  
  `lane(l,v1@v2) == T(lane(l,v1)@lane(l,v2))` for all `l`  
  there are no side-effects

- for any compound assignment expression `v1@=v2` (where `@` is one of`+`,
  `-`, `*`, `/`, `%`, `<<`, `>>`, `&`, `|`, `^`):  
  `v1@=v2` has the same side-effects as `lane(l,v1)@=lane(l,v2)` for all `l`  
  the result of `v1@=v2` is an lvalue denoting `v1`

- for any comparison expression `v1@v2` (where `@` is one of `==`, `!=`, `<`,
  `<=`, `>` or `>=`):  
  `lane(l,v1@v2) == lane(l,v1)@lane(l,v2)` for all `l`  
  The result of `v1@v2` is a prvalue of type `Mask<V>`  
  there are no side-effects

- for the unary logic expression `!v1`:  
  `lane(l,!v1) == !lane(l,v1)` for all `l`  
  The result of `!v1` is a prvalue of type `Mask<V>`  
  there are no side-effects

- for any binary logic expression `v1@v2` (where `@` is one of `&&` or `||`):  
  `lane(l,v1@v2) == lane(l,v1)@lane(l,v2)` for all `l`  
  The result of `v1@v2` is a prvalue of type `Mask<V>`  
  there are no side-effects

Note 1: Short-circuiting may or may not happen for `&&` and `||` -- it will
happen for the built-in types, but it cannot happen for proper multi-lane simd
types.

Note 2: For all expressions there is a lane-wise equality requirement with the
scalar operation.  This requirement is formulated such that promotions of
arguments are permitted, but not required.  This is necessary to allow both
the built-in types (which are promoted) and proper simd types (which typically
are not promoted to stay within the same simd register).

Note 3: The `==` in the lane-wise equality requirement may be overloaded to
account for proxies returned by `lane()`.

Note 4: Any expression that is invalid for the scalar case is not required for
the simd case either.

`#include` Structure
====================

There will be one header that ensures that the interface names are available.
This include also pulls in the part of the abstraction layer that enables use
of the built-in scalar types.  Any code that only makes use of the abstraction
layer needs to include this header, and only this header.

Any compilation unit (generally a `.cc`-file) that creates vectorized types
(other then the scalar built-in types) using some vectorization library, and
hands those types to vectorization-enabled dune code, is responsible for

1. including the necessary headers providing the abstraction for that
   vectorization library, as specified in the documentation of the
   abstraction, and

2. making sure the compilation with all the compiler/linker settings (flags,
   defines, libraries) needed by the vectorization library.

The ADL-Problem
===============

Consider the following example of a vectorization of
`Dune::FooVector::two_norm()`, which is implemented in
`dune/common/foovector.hh`:

```c++
// SIMD interface and implementation for scalar built-ins
#include <dune/common/simd/interface.hh>

namespace Dune {
  template<class T>
  class FooVector
  {
    T data_[FOO];
  public:
    T two_norm2() const {
      using Dune::Simd::lane;
      using Dune::Simd::lanes;
      T sum(0);
      for(const auto &entry : data_)
      {
        // break vectorization for demonstration purposes
        for(std::size_t l = 0; l < lanes(entry); ++l)
          lane(l, sum) += lane(l, entry) * lane(l, entry);
      }
      return sum;
    }
  };
}
```

This can then be used like this:

```c++
#include <dune/common/foovector.hh>
// provide dune-abstraction for mysimdlib
// also pulls in the necessary includes for mysimdlib
#include <dune/common/simd/mysimdlib.hh>

int main()
{
  using T = mysimdlib::Vector;
  Dune::FooVector<T> x(T(0));
  x.two_norm2();
}
```

This will not work.  At least not with a straightforward implementation of
`lane()` and `lanes()`, where `dune/common/simd/simdlib.hh` simply puts
overloads into the `Dune::Simd` namespace.  Here's why.

The compiler has several ways to find functions that are not qualified by
namespaces (or something similar).  One way is unqualified lookup: the
compiler looks for functions that are in some enclosing scope _at the time the
template containing the function call is read_ (early binding).  Another is
argument-dependend lookup (ADL): the compiler looks for functions in the
namespaces associated with the types of its arguments _at the time the
function call is instantiated_ (late binding).

In the example above, `T` a.k.a. `mysimdlib::Vector` is defined in the
namespace `mysimdlib`, which is unlikely to contain the functions `lane()` and
`lanes()`.  The abstraction layer could put overloads of those functions into
that namespace, but, well, you're not supposed to meddle in foreign namespace
unless given special permission.  As a consequence, `lane()` and `lanes()`
cannot be found by ADL.

And they cannot be found by any other lookup either.  After preprocessing the
compiler will see something like this:

```c++
// from dune/common/simd/interface.hh
namespace Dune::Simd {
  std::size_t lanes(double) { return 1; }
  double  lane(std::size_t, double  v) { return v; }
  double& lane(std::size_t, double& v) { return v; }
}

// from dune/common/foovector.hh
namespace Dune {
  template<class T>
  class FooVector
  {
    T data_[FOO];
  public:
    T two_norm2() const {
      using Dune::Simd::lane;
      using Dune::Simd::lanes;
      T sum(0);
      for(const auto &entry : data_)
      {
        // break vectorization for demonstration purposes
        for(std::size_t l = 0; l < lanes(entry); ++l)
          lane(l, sum) += lane(l, entry) * lane(l, entry);
      }
      return sum;
    }
  };
}

// from some mysimdlib-specific header
namespace mysimdlib {
  class Vector { /*...*/ };
}

// from dune/common/simd/mysimdlib.hh
namespace Dune::Simd {
  std::size_t lanes(mysimdlib::Vector v);
  double  lane(std::size_t, mysimdlib::Vector);
  double& lane(std::size_t, mysimdlib::Vector&);
}

// from myprog.cc
int main()
{
  using T = mysimdlib::Vector;
  Dune::FooVector<T> x(T(0));
  x.two_norm2();
}
```

At the time when the definition of `Dune::FooVector::two_norm2()` is read,
only the declarations for `lane()` and `lanes()` for scalar built-in types are
visible.  By the time `Dune::FooVector<mysimdlib::Vector>::two_norm2()` is
instantiated, the proper declarations for `lane()` and `lanes()` are visible.
But that is too late, because unqualified lookup does early binding.  It would
be OK for late binding, but only ADL does that, and ADL does not work as noted
above.

Note that ADL is the _only_ type of lookup that does late binding.  So we
cannot simply require the user to use another type of lookup.

Working around the ADL Problem
==============================

To get around the ADL problem, we can attempt the following:

```c++
// dune/common/simd/interface.hh

namespace Dune::Simd {
  namespace Overloads {
    struct ADLTag {};
  }

  template<class T>
  std::size_t lanes(T v)
  {
    return lanes(Overloads::ADLTag(), v);
  }

  template<class T>
  auto lane(std::size_t i, T v)
  {
    return lane(Overloads::ADLTag(), i, v);
  }

  //...

  // implementation for scalar built-ins
  namespace Overloads {
    std::size_t lanes(ADLTag, double) { return 1; }
    double lane(ADLTag, std::size_t, double v) { return v; }
    // etc...
  }
}
```

And for each vectorization library:
```c++
// dune/common/simd/mysimdlib.hh

#include <mysimdlib.h>

#include <dune/common/simd/interface.hh>

namespace Dune::Simd::Overloads {
  std::size_t lanes(ADLTag, mysimdlib::Vector v);
  double lane(ADLTag, std::size_t, mysimdlib::Vector v);
  // etc...
}
```

Core Dune code can then use the functions in `Dune::Simd` without
restrictions.  These functions themselves make sure to find the implementation
functions via ADL, so that the lookup uses late binding and thus can find
functions that are declared later.
