// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

template<typename Field, class Domain, class Range>
Mapping<Field,Domain,Range> Mapping<Field,Domain,Range>::operator+(const Vector<Field> &mapping) const {
  const Mapping<Field,Domain,Range> &m = dynamic_cast<const Mapping<Field,Domain,Range>& >( mapping );

  Mapping<Field,Domain,Range> newMapping = *this;

  for ( typename std::vector<term>::const_iterator it = m.lincomb_.begin(); it != m.lincomb_.end(); it++ ) {
    newMapping.lincomb_.push_back( *it );
  }

  return newMapping;
}

template<typename Field, class Domain, class Range>
Mapping<Field,Domain,Range> Mapping<Field,Domain,Range>::operator-(const Vector<Field> &mapping) const {
  const Mapping<Field,Domain,Range> &m = dynamic_cast<const Mapping<Field,Domain,Range>& >( mapping );

  Mapping<Field,Domain,Range> newMapping = *this;

  for ( typename std::vector<term>::const_iterator it = m.lincomb_.begin(); it != m.lincomb_.end(); it++ ) {
    newMapping.lincomb_.push_back( term( *it->v_, -it->scalar_ ) );
  }

  return newMapping;
}

template<typename Field, class Domain, class Range>
Vector<Field>& Mapping<Field,Domain,Range>::operator+=(const Vector<Field> &mapping)  {
  const Mapping<Field,Domain,Range> &m = dynamic_cast<const Mapping<Field,Domain,Range>& >( mapping );

  for ( typename std::vector<term>::const_iterator it = m.lincomb_.begin(); it != m.lincomb_.end(); it++ ) {
    lincomb_.push_back( *it );
  }

  return *this;
}

template<typename Field, class Domain, class Range>
Vector<Field>& Mapping<Field,Domain,Range>::operator-=(const Vector<Field> &mapping)  {
  const Mapping<Field,Domain,Range> &m = dynamic_cast<const Mapping<Field,Domain,Range>& >( mapping );

  for ( typename std::vector<term>::const_iterator it = m.lincomb_.begin(); it != m.lincomb_.end(); it++ ) {
    lincomb_.push_back( term( *it->v_, -it->scalar_ ) );
  }

  return *this;
}

template<typename Field, class Domain, class Range>
Vector<Field>& Mapping<Field,Domain,Range>::operator*=(const Field &factor)  {
  for ( typename std::vector<term>::iterator it = lincomb_.begin(); it != lincomb_.end(); it++ ) {
    it->scalar_ *= factor;
  }
  return *this;
}



template<typename Field, class Domain, class Range>
Vector<Field>& Mapping<Field,Domain,Range>::operator/=(const Field &divisor)  {
  for ( typename std::vector<term>::iterator it = lincomb_.begin(); it != lincomb_.end(); it++ ) {
    it->scalar_ /= divisor;
  }
  return *this;
}

template<typename Field, class Domain, class Range>
Vector<Field>& Mapping<Field,Domain,Range>::operator=(const Vector<Field> &mapping)  {
  const Mapping<Field,Domain,Range> &m = dynamic_cast<const Mapping<Field,Domain,Range>& >( mapping );

  lincomb_.erase( lincomb_.begin(), lincomb_.end() );

  for ( typename std::vector<term>::const_iterator it = m.lincomb_.begin(); it != m.lincomb_.end(); it++ ) {
    lincomb_.push_back( term( *it->v_, -it->scalar_ ) );
  }

  return *this;
}



template<typename Field, class Domain, class Range>
Mapping<Field,Domain,Range> Mapping<Field,Domain,Range>::operator*(const Field &factor) const {
  Mapping<Field,Domain,Range> newMapping = *this;

  for ( typename std::vector<term>::iterator it = newMapping.lincomb_.begin(); it != newMapping.lincomb_.end(); it++ ) {
    it->scalar_ *= factor;
  }

  return newMapping;
}

template<typename Field, class Domain, class Range>
Mapping<Field,Domain,Range> Mapping<Field,Domain,Range>::operator/(const Field &divisor) const {
  Mapping<Field,Domain,Range> newMapping = *this;
  for ( typename std::vector<term>::iterator it = newMapping.lincomb_.begin(); it != newMapping.lincomb_.end(); it++ ) {
    it->scalar_ /= divisor;
  }
  return newMapping;
}
