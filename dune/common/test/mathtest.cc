// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#include <iomanip>
#include <iostream>
#include <sstream>

#include <dune/common/classname.hh>
#include <dune/common/gmpfield.hh>
#include <dune/common/hybridutilities.hh>
#include <dune/common/indices.hh>
#include <dune/common/math.hh>
#include <dune/common/quadmath.hh>
#include <dune/common/test/testsuite.hh>


using namespace Dune::Hybrid;
using namespace Dune::Indices;
using Dune::TestSuite;

template<class T, T n>
constexpr inline static auto next(std::integral_constant<T, n>)
  -> std::integral_constant<T, n+1>
{
  return {};
}

template<class T, T k>
auto testStaticFactorial (std::integral_constant<T, k> _k = {}) -> TestSuite
{
  TestSuite t;

  std::cout << "test factorial\n{";

  forEach(integralRange(_k), [&](auto _i) {

    const auto value = Dune::factorial(_i);
    const auto control = _i() == 0 ? 1 : _i() * Dune::factorial(_i() - 1);

    t.check( value() == control );

    std::cout<< ' ' << value() << ',';
  });

  std::cout << "};\n\n";

  return t;
}

template<class T, T k>
auto testStaticBinomial (std::integral_constant<T, k> _k = {}) -> TestSuite
{
  TestSuite t;

  std::cout << "test binomial\n";

  forEach(integralRange(_k), [&](auto _i) {
    std::cout << "{";
    forEach(integralRange(next(_i)), [&](auto _j) {

      const auto value = Dune::binomial(_i, _j);
      const auto control = Dune::factorial(_i) / ( Dune::factorial(_j) * Dune::factorial(_i() - _j()) );

      t.check( value() == control );

      std::cout<< ' ' << value() << ',';
    });

    std::cout << "};\n";
  });

  std::cout << "\n";

  return t;
}

template <class T>
struct Digits10
{
  static int value () {
    return std::numeric_limits<T>::digits10;
  }
};

#if HAVE_GMP
template< unsigned int precision >
struct Digits10<Dune::GMPField<precision>>
{
  static int value () {
    return int(precision * std::log10(2));
  }
};
#endif

// check the correct definition of mathematical constants by comparing the
// digits against an explicit value provided as a string constant
template<class T>
auto testMathematicalConstants () -> TestSuite
{
  TestSuite t;

  const auto digits10 = Digits10<T>::value();

  static const std::string e = "2.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274274663919320030599218174135966290435729003342952605956307381323286279434907632338298807531952510190115738341879307021540891499348841675092447614606680822648001684774118537423454424371075390777449920695517027618386062613313845830007520449338265602976067371132007093287091274437470472306969772093101416928368190255151086574637721112523897844250569536967707854499699679468644549059879316368892300987931277361782154249992295763514822082698951936680331825288693984964651058209392398294887933203625094431173012381970684161403970198376793206832823764648042953118023287825098194558153017567173613320698112509961818815930416903515988885193458072738667385894228792284998920868058257492796104841984443634632449684875602336248270419786232090021609902353043699418491463140934317381436405462531520961836908887070167683964243781405927145635490613031072085103837505101157477041718986106873969655212671546889570350354";

  {
    std::stringstream e_sstr;
    e_sstr << std::setprecision(digits10+2) << Dune::MathematicalConstants<T>::e();
    std::string e_T = e_sstr.str();

    auto [it1,it2] = std::mismatch(e_T.begin(), e_T.end(), e.begin());
    auto pos = std::distance(e_T.begin(), it1);
    t.check(pos >= digits10);
  }

  static const std::string pi = "3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066470938446095505822317253594081284811174502841027019385211055596446229489549303819644288109756659334461284756482337867831652712019091456485669234603486104543266482133936072602491412737245870066063155881748815209209628292540917153643678925903600113305305488204665213841469519415116094330572703657595919530921861173819326117931051185480744623799627495673518857527248912279381830119491298336733624406566430860213949463952247371907021798609437027705392171762931767523846748184676694051320005681271452635608277857713427577896091736371787214684409012249534301465495853710507922796892589235420199561121290219608640344181598136297747713099605187072113499999983729780499510597317328160963185950244594553469083026425223082533446850352619311881710100031378387528865875332083814206171776691473035982534904287554687311595628638823537875937519577818577805321712268066130019278766111959092164201989";

  {
    std::stringstream pi_sstr;
    pi_sstr << std::setprecision(digits10+2) << Dune::MathematicalConstants<T>::pi();
    std::string pi_T = pi_sstr.str();

    auto [it1,it2] = std::mismatch(pi_T.begin(), pi_T.end(), pi.begin());
    auto pos = std::distance(pi_T.begin(), it1);
    t.check(pos >= digits10);
  }
  return t;
}

int main(int argc, char** argv)
{
  TestSuite t;

  t.subTest(testStaticFactorial(_5));
  t.subTest(testStaticBinomial(_5));

  t.subTest(testMathematicalConstants<float>());
  t.subTest(testMathematicalConstants<double>());
  t.subTest(testMathematicalConstants<long double>());

#if HAVE_QUADMATH
  t.subTest(testMathematicalConstants<Dune::Float128>());
#endif

#if HAVE_GMP
  t.subTest(testMathematicalConstants<Dune::GMPField<3318>>());
#endif

  return t.exit();
}
