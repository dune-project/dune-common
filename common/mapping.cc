// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

template<typename DFieldType,typename RFieldType, class DType, class RType>
Mapping<DFieldType,RFieldType,DType,RType> Mapping<DFieldType,RFieldType,DType,RType>::operator+(const Vector<RFieldType> &mapping) const {
  const Mapping<DFieldType,RFieldType,DType,RType> &m = dynamic_cast<const Mapping<DFieldType,RFieldType,DType,RType>& >( mapping );

  Mapping<DFieldType,RFieldType,DType,RType> newMapping = *this;

  for ( typename std::vector<term>::const_iterator it = m.lincomb_.begin(); it != m.lincomb_.end(); it++ ) {
    newMapping.lincomb_.push_back( *it );
  }

  return newMapping;
}

template<typename DFieldType,typename RFieldType, class DType, class RType>
Mapping<DFieldType,RFieldType,DType,RType> Mapping<DFieldType,RFieldType,DType,RType>::operator-(const Vector<RFieldType> &mapping) const {
  const Mapping<DFieldType,RFieldType,DType,RType> &m = dynamic_cast<const Mapping<DFieldType,RFieldType,DType,RType>& >( mapping );

  Mapping<DFieldType,RFieldType,DType,RType> newMapping = *this;

  for ( typename std::vector<term>::const_iterator it = m.lincomb_.begin(); it != m.lincomb_.end(); it++ ) {
    newMapping.lincomb_.push_back( term( *it->v_, -it->scalar_ ) );
  }

  return newMapping;
}

template<typename DFieldType,typename RFieldType, class DType, class RType>
Vector<RFieldType>& Mapping<DFieldType,RFieldType,DType,RType>::operator+=(const Vector<RFieldType> &mapping)  {
  const Mapping<DFieldType,RFieldType,DType,RType> &m = dynamic_cast<const Mapping<DFieldType,RFieldType,DType,RType>& >( mapping );

  for ( typename std::vector<term>::const_iterator it = m.lincomb_.begin(); it != m.lincomb_.end(); it++ ) {
    lincomb_.push_back( *it );
  }

  return *this;
}

template<typename DFieldType,typename RFieldType, class DType, class RType>
Vector<RFieldType>& Mapping<DFieldType,RFieldType,DType,RType>::operator-=(const Vector<RFieldType> &mapping)  {
  const Mapping<DFieldType,RFieldType,DType,RType> &m = dynamic_cast<const Mapping<DFieldType,RFieldType,DType,RType>& >( mapping );

  for ( typename std::vector<term>::const_iterator it = m.lincomb_.begin(); it != m.lincomb_.end(); it++ ) {
    lincomb_.push_back( term( *it->v_, -it->scalar_ ) );
  }

  return *this;
}

template<typename DFieldType,typename RFieldType, class DType, class RType>
Vector<RFieldType>& Mapping<DFieldType,RFieldType,DType,RType>::operator*=(const Field &factor)  {
  for ( typename std::vector<term>::iterator it = lincomb_.begin(); it != lincomb_.end(); it++ ) {
    it->scalar_ *= factor;
  }
  return *this;
}



template<typename DFieldType,typename RFieldType, class DType, class RType>
Vector<RFieldType>& Mapping<DFieldType,RFieldType,DType,RType>::operator/=(const Field &divisor)  {
  for ( typename std::vector<term>::iterator it = lincomb_.begin(); it != lincomb_.end(); it++ ) {
    it->scalar_ /= divisor;
  }
  return *this;
}

template<typename DFieldType,typename RFieldType, class DType, class RType>
Vector<RFieldType>& Mapping<DFieldType,RFieldType,DType,RType>::operator=(const Vector<RFieldType> &mapping)  {
  const Mapping<DFieldType,RFieldType,DType,RType> &m = dynamic_cast<const Mapping<DFieldType,RFieldType,DType,RType>& >( mapping );

  lincomb_.erase( lincomb_.begin(), lincomb_.end() );

  for ( typename std::vector<term>::const_iterator it = m.lincomb_.begin(); it != m.lincomb_.end(); it++ ) {
    lincomb_.push_back( term( *it->v_, -it->scalar_ ) );
  }

  return *this;
}

template<typename DFieldType,typename RFieldType, class DType, class RType>
Mapping<DFieldType,RFieldType,DType,RType> Mapping<DFieldType,RFieldType,DType,RType>::operator*(const Field &factor) const {
  Mapping<DFieldType,RFieldType,DType,RType> newMapping = *this;

  for ( typename std::vector<term>::iterator it = newMapping.lincomb_.begin(); it != newMapping.lincomb_.end(); it++ ) {
    it->scalar_ *= factor;
  }

  return newMapping;
}

template<typename DFieldType,typename RFieldType, class DType, class RType>
Mapping<DFieldType,RFieldType,DType,RType> Mapping<DFieldType,RFieldType,DType,RType>::operator/(const Field &divisor) const {
  Mapping<DFieldType,RFieldType,DType,RType> newMapping = *this;
  for ( typename std::vector<term>::iterator it = newMapping.lincomb_.begin(); it != newMapping.lincomb_.end(); it++ ) {
    it->scalar_ /= divisor;
  }
  return newMapping;
}
