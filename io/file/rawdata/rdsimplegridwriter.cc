// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "../../grid/simplegrid.hh"

// ///////////////////////////////////////////////
// Specialization for SimpleGrid<3,3>
// ///////////////////////////////////////////////
namespace Dune {

  template<class T>
  class RawDataWriter<SimpleGrid<3,3>, T> {

  public:

    static void write(const SimpleGrid<3,3>& grid,
                      const Array<T>& sol,
                      const std::string& filename);


    RawDataWriter() {}

  };

}

template<class T>
void Dune::RawDataWriter<Dune::SimpleGrid<3,3>, T>::write(const Dune::SimpleGrid<3,3>& grid,
                                                          const Array<T>& sol,
                                                          const std::string& filename)
{
  printf("This is the rawdata writer for SimpleGrid<3,3>!\n");

  int maxlevel = grid.maxlevel();
  const levelinfo<3>* li = grid.get_levelinfo(maxlevel);


  // write the amiramesh header
  FILE* fp = fopen(filename.c_str(), "wb");

  for (Array<T>::Iterator iter = sol.begin(); iter!=sol.end(); ++iter) {

    fwrite(iter.operator->(), sizeof(T), 1, fp);

  }

  fclose(fp);

}
