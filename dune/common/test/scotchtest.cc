#include <config.h>

#include <iostream>
#include <fstream>
#include <vector>

#include <scotch.h>

#include <dune/common/exceptions.hh>

// write graph to file
void prepare (std::string filename)
{
  std::ofstream out(filename, std::ios_base::out);

  // out << "0\n4	4\n0	000\n1	1\n1	0\n1	3\n1	2";

  out << "0\n16	48\n0	000\n2	1	4\n3	0	2	5\n3	1	3	6\n2	2	7\n3	0	5	8\n4	1	4	6	9\n4	2	5	7	10\n3	3	6	11\n3	4	9	12\n4	5	8	10	13\n4	6	9	11	14\n3	7	10	15\n2	8	13\n3	9	12	14\n3	10	13	15\n2	11	14";
}

int main (int argc, char** argv)
{
  SCOTCH_errorProg (argv[0]);

  // Initialize source graph
  SCOTCH_Graph grafdat;
  if (SCOTCH_graphInit (&grafdat) != 0) {
    DUNE_THROW(Dune::Exception, "cannot initialize graph");
  }

  prepare("graph_file.grf");

  FILE* fileptr = nullptr;
  if ((fileptr = fopen ("graph_file.grf", "r")) == nullptr) {
    DUNE_THROW(Dune::Exception, "cannot open file");
  }

  // Read source graph
  if (SCOTCH_graphLoad (&grafdat, fileptr, -1, 0) != 0) {
    DUNE_THROW(Dune::Exception, "cannot load graph");
  }

  fclose (fileptr);

  SCOTCH_Num vertnbr = 0;
  SCOTCH_graphSize (&grafdat, &vertnbr, nullptr);

  std::vector<SCOTCH_Num> colotab(vertnbr, 0);
  std::vector<SCOTCH_Num> cnbrtab(vertnbr, 0);

  SCOTCH_Num colonbr = 0;
  if (SCOTCH_graphColor (&grafdat, colotab.data(), &colonbr, 0) != 0) {
    DUNE_THROW(Dune::Exception, "cannot color graph");
  }

  std::cout << "Number of colors: " << colonbr << std::endl;

  // Sum-up color histogram
  for (SCOTCH_Num vertnum = 0; vertnum < vertnbr; vertnum++)
    cnbrtab[colotab[vertnum]]++;

  for (SCOTCH_Num colonum = 0; colonum < colonbr; colonum++)
    std::cout << "Color " << colonum << ": " << cnbrtab[colonum] << std::endl;

  SCOTCH_graphExit (&grafdat);

  return EXIT_SUCCESS;
}
