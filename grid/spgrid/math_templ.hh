// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef MATH_TEMPL_HH
#define MATH_TEMPL_HH

template <int exp> inline const int pow(int base) {
  return base*pow<exp-1>(base);
}
template <> inline const int pow<0>(int base) {
  return 1;
}

template <int exp> inline const double pow(double base) {
  return base*pow<exp-1>(base);
}
template <> inline const double pow<0>(double base) {
  return 1;
}

#endif // MATH_TEMPL_HH
