// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ARRAYLIST_HH__
#define __DUNE_ARRAYLIST_HH__

#include <vector>
#include <dune/common/fixedarray.hh>

namespace Dune {
  // forward declaration
  template<class T, int N>
  class ArrayListIterator;
}

namespace std {
  /**
   * @brief Iterator traits for the Dune::ArrayListIterator.
   */
  template<>
  template<class T, int N>
  struct iterator_traits<Dune::ArrayListIterator<T,N> >{
    typedef T value_type;
    typedef int difference_type;
    typedef random_access_iterator_tag iterator_category;
    typedef T*                         pointer;
    typedef T&                         reference;
  };
}

namespace Dune {

  /**
   * @brief A random access data structure.
   * Internally the data is organized in a list of arrays of fixed size. Whenever the capacity
   * of the array list is not sufficient a new Array is allocated. In contrast to
   * std::vector this approach prevents data copying. On the outside
   * we provide the same interface as the stl random access containers.
   */
  template<class T, int N=100>
  class ArrayList {
  public:
    /**
     * @brief The member type.
     */
    typedef T memberType;

    /**
     * @brief The number of elements in one chunk of the list.
     * This has to be at least one. The default is 100.
     */
    enum { chunkSize_ = (N > 0) ? N : 1};

    /**
     * @brief The iterator needs access to the private variables.
     */
    friend class ArrayListIterator<memberType,N>;

    /**
     * @brief A random access iterator.
     */
    typedef ArrayListIterator<memberType,N> iterator;

    /**
     * @brief A constant random access iterator.
     */
    typedef const ArrayListIterator<memberType,N> const_iterator;


    /**
     * @brief Get an iterator that is positioned at the first element.
     * @return The iterator.
     */
    iterator begin();

    /**
     * @brief Get a random access iterator that is positioned at the
     * first element.
     * @return The iterator.
     */
    const_iterator begin() const;

    /**
     * @brief Get a random access iterator positioned after the last
     * element
     */
    iterator end();

    /**
     * @brief Get a random access iterator positioned after the last
     * element
     */
    const_iterator end() const;

    /**
     * @brief Append an entry to the list.
     * @param entry The new entry.
     */
    void push_back(const T& entry);
    /**
     * @brief Constructor.
     * Constructs an Array list with one chunk.
     */
    ArrayList();

    /**
     * @brief Destructor.
     */
    ~ArrayList();
  protected:
    /** @brief the data chunks of our list. */
    std::vector<FixedArray<memberType,chunkSize_>* > chunks_;
    /** @brief The current data capacity. */
    int capacity_;
    /** @brief The current number of elements in our data structure. */
    int size_;
  };

  template<class T, int N>
  int operator-(const ArrayListIterator<T,N>& a, const ArrayListIterator<T,N>& b);

  /*
      template<class T, int N>
      int operator+(const ArrayListIterator<T,N>& a, const ArrayListIterator<T,N>& b);
   */
  template<class T, int N>
  ArrayListIterator<T,N> operator+(int i, const ArrayListIterator<T,N>& a);

  template<class T, int N>
  ArrayListIterator<T,N> operator-(int i, const ArrayListIterator<T,N>& a);

  template<class T, int N>
  ArrayListIterator<T,N> operator+(const ArrayListIterator<T,N>& a, int i);

  template<class T, int N>
  ArrayListIterator<T,N> operator-(const ArrayListIterator<T,N>& a, int i);

  template<class T, int N>
  bool operator<(const ArrayListIterator<T,N>& a, const ArrayListIterator<T,N>& b);

  template<class T, int N>
  bool operator<=(const ArrayListIterator<T,N>& a, const ArrayListIterator<T,N>& b);

  template<class T, int N>
  bool operator>(const ArrayListIterator<T,N>& a, const ArrayListIterator<T,N>& b);

  template<class T, int N>
  bool operator>=(const ArrayListIterator<T,N>& a, const ArrayListIterator<T,N>& b);

  template<class T, int N>
  class ArrayListIterator {

    friend class ArrayList<T,N>;
    friend int operator-<>(const ArrayListIterator<T,N>& a, const ArrayListIterator<T,N>& b);
    friend ArrayListIterator<T,N> operator+<>(const ArrayListIterator<T,N>& a, int i);
    friend ArrayListIterator<T,N> operator-<>(const ArrayListIterator<T,N>& a, int i);
    friend ArrayListIterator<T,N> operator+<>(int i, const ArrayListIterator<T,N>& a);
    friend ArrayListIterator<T,N> operator-<>(int i, const ArrayListIterator<T,N>& a);
    friend bool operator< <>(const ArrayListIterator<T,N>& a, const ArrayListIterator<T,N>& b);
    friend bool operator<=<>(const ArrayListIterator<T,N>& a, const ArrayListIterator<T,N>& b);
    friend bool operator><>(const ArrayListIterator<T,N>& a, const ArrayListIterator<T,N>& b);
    friend bool operator>=<>(const ArrayListIterator<T,N>& a, const ArrayListIterator<T,N>& b);

  public:
    /**
     * @brief The member type.
     */
    typedef T memberType;

    /**
     * @brief The number of elements in one chunk of the list.
     * This has to be at least one. The default is 100.
     */
    enum { chunkSize_ = (N > 0) ? N : 1};

    /**
     * @brief Comares to iterators.
     * @return true if the iterators are for the same list and
     * at the position.
     */
    bool operator==(const ArrayListIterator<memberType,N>& other) const;

    /**
     * @brief Checks wether to iterator are different.
     * @return True if the position or list differ.
     */
    bool operator!=(const ArrayListIterator<memberType,N>& other) const;


    /**
     * @brief Increment the iterator.
     */
    ArrayListIterator<memberType,N>& operator++();

    /**
     * @brief Increment the iterator.
     */
    ArrayListIterator<memberType,N> operator++(int i);

    /**
     * @brief decrement the iterator.
     */
    ArrayListIterator<memberType,N>& operator--();

    /**
     * @brief decrement the iterator.
     */
    ArrayListIterator<memberType,N> operator--(int i);

    /**
     * @brief Increments the iterator by an arbitrary value of positions.
     * @param count The number of positions to advance.
     * @return The iterator at that position.
     */
    const ArrayListIterator<memberType,N>& operator+=(int count);

    /**
     * @brief Decremnets the iterator by an arbitrary value of positions.
     * @param count The number of positions to advance.
     * @return The iterator at that position.
     */
    const ArrayListIterator<memberType,N>& operator-=(int count);

    /**
     * @brief Get the value of the list at an arbitrary position.
     * @return The value at that postion.
     */
    memberType& operator[](int i) const;

    /**
     * @brief Access the element at the current position.
     * @return The element at the current position.
     */
    memberType& operator*() const;

    const ArrayListIterator<memberType,N>& operator=(const ArrayListIterator<memberType,N>& other);

    ArrayListIterator(const ArrayListIterator<memberType,N>& other);
  private:
    /**
     * @brief Constructor.
     * @param list The list we are an iterator for.
     * @param position The initial position of the iterator.
     */
    ArrayListIterator(ArrayList<T,N>& arrayList, int position);

    /**
     * @brief The current postion.
     */
    int position_;
    /**
     * @brief The list we are an iterator for.
     */
    ArrayList<T,N>& list_;
  };


  template<class T, int N>
  ArrayList<T,N>::ArrayList() : capacity_(0), size_(0){
    chunks_.reserve(100);
  }

  template<class T, int N>
  ArrayList<T,N>::~ArrayList(){
    for(int chunk=capacity_/chunkSize_-1; chunk>=0; chunk--)
      delete chunks_[chunk];
  }



  template<class T, int N>
  void ArrayList<T,N>::push_back(const T& entry){
    int chunk = size_/chunkSize_;
    if(size_==capacity_) {
      chunks_[chunk] = new FixedArray<memberType,chunkSize_>();
      capacity_ += chunkSize_;
    }
    chunks_[chunk]->operator[]((size_++)%chunkSize_)=entry;
  }

  template<class T, int N>
  ArrayListIterator<T,N> ArrayList<T,N>::begin(){
    return ArrayListIterator<T,N>(*this, 0);
  }

  template<class T, int N>
  const ArrayListIterator<T,N> ArrayList<T,N>::begin() const {
    return ArrayListIterator<T,N>(*this, 0);
  }

  template<class T, int N>
  ArrayListIterator<T,N> ArrayList<T,N>::end(){
    return ArrayListIterator<T,N>(*this, size_);
  }

  template<class T, int N>
  const ArrayListIterator<T,N> ArrayList<T,N>::end() const {
    return ArrayListIterator<T,N>(*this, size_);
  }

  template<class T, int N>
  int operator-(const ArrayListIterator<T,N>& a, const ArrayListIterator<T,N>& b){
    return a.position_ - b.position_;
  }

  template<class T, int N>
  ArrayListIterator<T,N> operator+(const ArrayListIterator<T,N>& a, int i){
    ArrayListIterator<T,N> b=a;
    b.position_+=i;
    return b;
  }

  template<class T, int N>
  ArrayListIterator<T,N> operator-(const ArrayListIterator<T,N>& a, int i){
    ArrayListIterator<T,N> b=a;
    b.position_-=i;
    return b;
  }


  template<class T, int N>
  ArrayListIterator<T,N> operator+(int i, const ArrayListIterator<T,N>& a){
    ArrayListIterator<T,N> b=a;
    b.position_+=i;
    return b;
  }

  template<class T, int N>
  ArrayListIterator<T,N> operator-(int i, const ArrayListIterator<T,N>& a){
    ArrayListIterator<T,N> b=a;
    b.position_-=i;
    return b;
  }

  template<class T, int N>
  bool operator<(const ArrayListIterator<T,N>& a, const ArrayListIterator<T,N>& b){
    return a.position_ < b.position_;
  }

  template<class T, int N>
  bool operator<=(const ArrayListIterator<T,N>& a, const ArrayListIterator<T,N>& b){
    return a.position_ <= b.position_;
  }

  template<class T, int N>
  bool operator>(const ArrayListIterator<T,N>& a, const ArrayListIterator<T,N>& b){
    return a.position_ > b.position_;
  }

  template<class T, int N>
  bool operator>=(const ArrayListIterator<T,N>& a, const ArrayListIterator<T,N>& b){
    return a.position_ >= b.position_;
  }

  template<class T, int N>
  bool ArrayListIterator<T,N>::operator==(const ArrayListIterator<memberType,N>& other) const {
    // Makes only sense if we reference a common list
    assert((&list_)==(&(other.list_)));
    return position_==other.position_ ;
  }

  template<class T, int N>
  bool ArrayListIterator<T,N>::operator!=(const ArrayListIterator<memberType,N>& other) const {
    // Makes only sense if we reference a common list
    assert((&list_)==(&(other.list_)));
    return position_!=other.position_;
  }

  template<class T, int N>
  ArrayListIterator<T,N>&  ArrayListIterator<T,N>::operator++(){
    ++position_;
    return *this;
  }

  template<class T, int N>
  ArrayListIterator<T,N>&  ArrayListIterator<T,N>::operator--(){
    --position_;
    return *this;
  }

  template<class T, int N>
  ArrayListIterator<T,N>  ArrayListIterator<T,N>::operator++(int i) {
    ArrayListIterator<T,N> al=*this;
    ++position_;
    return al;
  }

  template<class T, int N>
  ArrayListIterator<T,N>  ArrayListIterator<T,N>::operator--(int i){
    ArrayListIterator<T,N> al=*this;
    --position_;
    return al;
  }

  template<class T, int N>
  const ArrayListIterator<T,N>&  ArrayListIterator<T,N>::operator+=(int count){
    position_ += count;
    return *this;
  }

  template<class T, int N>
  const ArrayListIterator<T,N>&  ArrayListIterator<T,N>::operator-=(int count){
    position_ -= count;
    return *this;
  }

  template<class T, int N>
  T& ArrayListIterator<T,N>::operator[](int i) const {
    return list_.chunks_[i/chunkSize_]->operator[](i%chunkSize_);
  }


  template<class T, int N>
  T& ArrayListIterator<T,N>::operator*() const {
    return list_.chunks_[position_/chunkSize_]->operator[](position_%chunkSize_);
  }

  template<class T, int N>
  const ArrayListIterator<T,N>& ArrayListIterator<T,N>::operator=(const ArrayListIterator<T,N>& other){
    position_=other.position_;
    list_=other.list_;
    return *this;
  }


  template<class T, int N>
  ArrayListIterator<T,N>::ArrayListIterator(ArrayList<T,N>& arrayList, int position) : position_(position), list_(arrayList){}

  template<class T, int N>
  ArrayListIterator<T,N>::ArrayListIterator(const ArrayListIterator<T,N>& other)
    : position_(other.position_), list_(other.list_){}
}
#endif
