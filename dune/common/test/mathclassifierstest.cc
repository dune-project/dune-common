// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <complex>
#include <iostream>
#include <limits>

#include <dune/common/math.hh>

int main() {
  //Initialize some variables
  int a = 42;
  const int b = 42;

  double nan = std::nan("");
  double inf = std::numeric_limits<double>::infinity();

  std::complex<double> complex_nonan(42., 42.);
  std::complex<double> complex_nan1(42.,nan);
  std::complex<double> complex_nan2(nan, 42.);
  std::complex<double> complex_nan3(nan, nan);

  std::complex<double> complex_noinf(42., 42.);
  std::complex<double> complex_inf1(42.,inf);
  std::complex<double> complex_inf2(inf, 42.);
  std::complex<double> complex_inf3(inf, inf);

  std::cout << std::boolalpha
            //check isNaN()
            << "isNaN(int): " << Dune::isNaN(a) << "\n"
            << "isNaN(const int): " << Dune::isNaN(b) << "\n"
            << "isNaN(42): " << Dune::isNaN(42) << "\n"
            << "isNaN(nan): " << Dune::isNaN(nan) << "\n"
            << "isNaN(inf): " << Dune::isNaN(inf) << "\n"

            << "isNaN(std::complex<double> without NaN): "
            << Dune::isNaN(complex_nonan) << "\n"
            << "isNaN(std::complex<double> with NaN): "
            << Dune::isNaN(complex_nan1) << " "
            << Dune::isNaN(complex_nan2) << " "
            << Dune::isNaN(complex_nan3) << "\n"

            //check isInf()
            << "isInf(int): " << Dune::isInf(a) << "\n"
            << "isInf(const int): " << Dune::isInf(b) << "\n"
            << "isInf(42): " << Dune::isInf(42) << "\n"
            << "isInf(inf): " << Dune::isInf(inf) << "\n"

            << "isInf(std::complex<double> without inf): "
            << Dune::isInf(complex_noinf) << "\n"
            << "isInf(std::complex<double> with inf): "
            << Dune::isInf(complex_inf1) << " "
            << Dune::isInf(complex_inf2) << " "
            << Dune::isInf(complex_inf3) << "\n"

            //check isFinite()
            << "isFinite(int): " << Dune::isFinite(a) << "\n"
            << "isFinite(const int): " << Dune::isFinite(b) << "\n"
            << "isFinite(42): " << Dune::isFinite(42) << "\n"
            << "isFinite(inf): " << Dune::isFinite(inf) << "\n"

            << "isFinite(std::complex<double> without inf): "
            << Dune::isFinite(complex_noinf) << "\n"
            << "isFinite(std::complex<double> with inf): "
            << Dune::isFinite(complex_inf1) << " "
            << Dune::isFinite(complex_inf2) << " "
            << Dune::isFinite(complex_inf3) << "\n"

  << std::endl;
}
