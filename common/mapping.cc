// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

template<typename Field, class Domain, class Range>
Vector <Field>  Mapping<Field,Domain,Range>::operator + (const Vector <Field> & a) const {
  Mapping <Field,Domain,Range> res;

  return res;
}

template< typename Field, class Domain, class Range >
Vector <Field>  Mapping<Field,Domain,Range>::operator - (const Vector <Field> & a) const {
  Mapping <Field,Domain,Range> res;


  return res;
}

template<typename Field, class Domain, class Range>
Vector <Field>  Mapping<Field,Domain,Range>::operator * (const Field & a) const {
  Mapping <Field,Domain,Range> res;


  return res;
}

template<typename Field, class Domain, class Range>
Vector <Field>  Mapping<Field,Domain,Range>::operator / (const Field & a) const {
  Mapping <Field,Domain,Range> res;


  return res;
}

template<typename Field, class Domain, class Range>
Vector <Field>&  Mapping<Field,Domain,Range>::operator = (const Vector <Field> & a)  {



  return *this;
}

template<typename Field, class Domain, class Range>
Vector <Field>&  Mapping<Field,Domain,Range>::operator += (const Vector <Field> & a)  {



  return *this;
}


template<typename Field, class Domain, class Range>
Vector <Field>&  Mapping<Field,Domain,Range>::operator -= (const Vector <Field> & a)  {



  return *this;
}

template<typename Field, class Domain, class Range>
Vector <Field>&  Mapping<Field,Domain,Range>::operator *= (const Field & a)  {



  return *this;
}

template<typename Field, class Domain, class Range>
Vector <Field>&  Mapping<Field,Domain,Range>::operator /= (const Field & a)  {



  return *this;
}
