// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
template <class OperatorType, class DiscFuncType>
void Dune::IterativeSolver<OperatorType, DiscFuncType>::check() const
{
  if (!errorNorm_)
    DUNE_THROW(SolverError, "You need to supply a norm-computing routine to an iterative solver!");

  if (!iterationStep)
    DUNE_THROW(SolverError, "You need to supply an iteration step to an iterative solver!");

  iterationStep->check();
}

template <class OperatorType, class DiscFuncType>
inline
void Dune::IterativeSolver<OperatorType, DiscFuncType>::solve()
{

  int i;

  // Check whether the solver is set up properly
  check();

  if (verbosity_ != QUIET)
    std::cout << "--- LinearSolver ---\n";

  double error = std::numeric_limits<double>::max();

  double normOfOldCorrection = 0;

  // Loop until desired tolerance or maximum number of iterations is reached
  for (i=0; i<numIt && (error>tolerance_ || isnan(error)); i++) {

    // Backup of the current solution for the error computation later on
    DiscFuncType oldSolution = iterationStep->getSol();

    // Perform one iteration step
    iterationStep->iterate();

    // Compute error
    double oldNorm = errorNorm_->compute(oldSolution);
    oldSolution -= iterationStep->getSol();

    double normOfCorrection = errorNorm_->compute(oldSolution);

    error = normOfCorrection / oldNorm;

    double convRate = normOfCorrection / normOfOldCorrection;

    normOfOldCorrection = normOfCorrection;

    // Output
    if (verbosity_ != QUIET) {
      std::cout << i << "  --  ||u^{n+1} - u^n||: " << error << ",      "
                << "convrate " << convRate << "\n";
    }

  }

  if (verbosity_ != QUIET) {
    std::cout << i << " iterations performed\n";
    std::cout << "--------------------\n";
  }

}
