#ifndef DUNE_COMMON_ARCHIVE_HH
#define DUNE_COMMON_ARCHIVE_HH

namespace Dune {

  // Simple archive system for binary data dumps. Inspired by
  // boost::serialize, but much simpler.
  //
  // Non-trivial classes that you want to dump need to implement the method
  // template<typename Archive>
  // void archive(Archive& ar);
  //
  // That method will be called for both dumping and loading the data. You can
  // distinguish between the two cases by inspecting the nested Traits struct.
  //
  // The stream to be passed must conform to the standard ifstream / ofstream interface.

  template<typename Stream>
  class BinaryOutStreamArchive
  {

  public:

    typedef std::size_t size_type;

    struct Traits
    {

      static const bool is_writing = true;
      static const bool is_reading = false;

    };

    template<typename T>
    typename std::enable_if<
      std::is_trivial<T>::value
      >::type
    operator&(const T& data)
    {
      _os.write(reinterpret_cast<const char*>(&data),sizeof(T));
    }

    template<typename T>
    typename std::enable_if<
      !std::is_trivial<T>::value
      >::type
    operator&(T& data)
    {
      data.archive(*this);
    }

    template<typename T>
    void bulk(const T* data, size_type n)
    {
      _os.write(reinterpret_cast<const char*>(data),sizeof(T) * n);
    }

    BinaryOutStreamArchive(Stream& os)
      : _os(os)
    {}

    BinaryOutStreamArchive(const BinaryOutStreamArchive&) = delete;
    BinaryOutStreamArchive(BinaryOutStreamArchive&&) = delete;

    BinaryOutStreamArchive& operator=(const BinaryOutStreamArchive&) = delete;
    BinaryOutStreamArchive& operator=(BinaryOutStreamArchive&&) = delete;

  private:

    Stream& _os;

  };


  template<typename Stream>
  class BinaryInStreamArchive
  {

  public:

    typedef std::size_t size_type;

    struct Traits
    {

      static const bool is_writing = false;
      static const bool is_reading = true;

    };

    template<typename T>
    typename std::enable_if<
      std::is_trivial<T>::value
      >::type
    operator&(T& data)
    {
      _is.read(reinterpret_cast<char*>(&data),sizeof(T));
    }

    template<typename T>
    typename std::enable_if<
      !std::is_trivial<T>::value
      >::type
    operator&(T& data)
    {
      data.archive(*this);
    }

    template<typename T>
    void bulk(T* data, size_type n)
    {
      _is.read(reinterpret_cast<char*>(data),sizeof(T) * n);
    }

    BinaryInStreamArchive(Stream& is)
      : _is(is)
    {}

    BinaryInStreamArchive(const BinaryInStreamArchive&) = delete;
    BinaryInStreamArchive(BinaryInStreamArchive&&) = delete;

    BinaryInStreamArchive& operator=(const BinaryInStreamArchive&) = delete;
    BinaryInStreamArchive& operator=(BinaryInStreamArchive&&) = delete;

  private:

    Stream& _is;

  };


}


#endif // DUNE_COMMON_ARCHIVE_HH
