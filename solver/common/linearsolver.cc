// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
//#include "norms.hh"

template <class OP_TYPE, class VEC_TYPE>
inline
void Dune::LinearSolver<OP_TYPE, VEC_TYPE>::solve()
{

  int i;

  if (verbosity_ != QUIET)
    std::cout << "--- LinearSolver ---\n";

  double error = std::numeric_limits<double>::max();

  // Loop until desired tolerance or maximum number of iterations is reached
  for (i=0; i<numIt && error>tolerance_; i++) {

    // Backup of the current solution for the error computation later on
    VEC_TYPE oldSolution = iterationStep->getSol();

    // Perform one iteration step
    iterationStep->iterate();

    // Compute error
    //VEC_TYPE diff = oldSolution - iterationStep->getSol();
    //error = errorNorm_->compute(oldSolution - iterationStep->getSol());

    // Output
    if (verbosity_ != QUIET)
      std::cout << "||u^{n+1} - u^n||_L2: " << error << "\n";

  }

  if (verbosity_ != QUIET) {
    std::cout << i << " iterations performed\n";
    std::cout << "--------------------\n";
  }

}
