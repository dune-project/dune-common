// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "io/amirameshwriter.hh"
#include "../grid/simplegrid.hh"
#include <string>
#include <stdio.h>

void Dune::AmiraMeshWriter::write(void* g,
                                  const std::string& filename) const
{
  printf("This is the AmiraMesh writer!\n");
  SimpleGrid<3,3>* grid = (SimpleGrid<3,3>*)g;

  int maxlevel = grid->maxlevel();
  levelinfo<3>* li = grid->get_levelinfo(maxlevel);

  // write the amiramesh header
  FILE* fp = fopen(filename.c_str(), "w");

  fprintf(fp, "# AmiraMesh 3D ASCII 2.0\n");
  fprintf(fp, "# CreationDate: Mon Nov 17 18:03:33 2003\n\n\n");

  fprintf(fp, "define Lattice %d %d %d\n\n", li->ne[0], li->ne[1], li->ne[2]);

  fprintf(fp, "Parameters {\n");
  fprintf(fp, "    BoundingBox 5.88235 194.118 5.88235 194.118 8.21401 128.786,\n");
  fprintf(fp, "    CoordType \"uniform\",\n");
  fprintf(fp, "    Content \"%dx%dx%d double, uniform coordinates\"\n",
          li->ne[0], li->ne[1], li->ne[2]);
  fprintf(fp, "}\n\n");

  fprintf(fp, "Lattice { double Data } @1\n\n");

  fprintf(fp, "# Data section follows\n");
  fprintf(fp, "@1\n");

  SimpleGrid<3,3>::Traits<0>::LevelIterator iter = grid->lbegin<0>(maxlevel);

  for (; iter!= grid->lend<0>(maxlevel); ++iter) {
    fprintf(fp, "%f\n", iter->index(), *iter);
  }

  fclose(fp);

  return;
}
