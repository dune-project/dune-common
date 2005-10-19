// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// ///////////////////////////////////////////////
// Specialization of the AmiraMesh writer for SimpleGrid<3,3>
// ///////////////////////////////////////////////

#include <dune/grid/simplegrid.hh>

namespace Dune {

  template<>
  class AmiraMeshWriter<SimpleGrid<3,3>, double> {

  public:

    static void write(const SimpleGrid<3,3>& grid,
                      const Array<double>& sol,
                      const std::string& filename);


    AmiraMeshWriter() {}

  };

}

//template<>
void Dune::AmiraMeshWriter<Dune::SimpleGrid<3,3>, double>::write(const Dune::SimpleGrid<3,3>& grid,
                                                                 const Array<double>& sol,
                                                                 const std::string& filename)
{
  printf("This is the AmiraMesh writer for SimpleGrid<3,3>!\n");

  int maxlevel = grid.maxLevel();
  const levelinfo<3>* li = grid.get_levelinfo(maxlevel);

  // determine current time
  time_t time_val = time(NULL);
  struct tm* localtime_val = localtime(&time_val);
  const char* asctime_val = asctime(localtime_val);

  // write the amiramesh header
  FILE* fp = fopen(filename.c_str(), "w");

  fprintf(fp, "# AmiraMesh 3D ASCII 2.0\n");
  fprintf(fp, "# CreationDate: %s\n\n\n", asctime_val);

  fprintf(fp, "define Lattice %d %d %d\n\n", li->ne[0], li->ne[1], li->ne[2]);

  fprintf(fp, "Parameters {\n");
  // SimpleGrids always have the unit bounding box
  fprintf(fp, "    BoundingBox 0 1 0 1 0 1,\n");
  fprintf(fp, "    CoordType \"uniform\",\n");
  fprintf(fp, "    Content \"%dx%dx%d double, uniform coordinates\"\n",
          li->ne[0], li->ne[1], li->ne[2]);
  fprintf(fp, "}\n\n");

  fprintf(fp, "Lattice { double Data } @1\n\n");

  fprintf(fp, "# Data section follows\n");
  fprintf(fp, "@1\n");

  //     SimpleGrid<3,3>::template Codim<0>::LevelIterator iter = grid.lbegin<0>(maxlevel);

  //     for (; iter!= grid.lend<0>(maxlevel); ++iter) {
  //         fprintf(fp, "%f\n", iter->index(), *iter);
  //     }

  Array<double>::Iterator iter = sol.begin();
  for (; iter!=sol.end(); ++iter) {
    fprintf(fp, "%f\n", *iter);
  }

  fclose(fp);

}
