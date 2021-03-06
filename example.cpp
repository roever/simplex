#include <iostream>
#include <Eigen/Dense>

#include "solver.hpp"
#include "eigen.hpp"
#include "matrix.hpp"

using EigenSolver = simplex::Solver<Eigen::MatrixXd>;
using Solver = simplex::Solver<simplex::Matrix<float>>;

int main()
{
  // an maximisation problem with eigen matrices

  // create a vector for objective function and a matrix for the constraints
  Eigen::MatrixXd constraints(3, 3);
  Eigen::VectorXd objectiveFunction(2);

  // setup problem
  objectiveFunction <<	1, 2;
  constraints <<  2, 3, 34,
                  1, 5, 45,
                  1, 0, 15;

  // solve
  EigenSolver solver1(EigenSolver::MODE_MAXIMIZE, objectiveFunction, constraints);

  // output result
  switch (solver1.hasSolution())
  {
    case EigenSolver::SOL_FOUND:
      std::cout << "The minimum is: " << solver1.getOptimum() << std::endl;
      std::cout << "The solution is: " << solver1.getSolution().transpose() << std::endl;
      break;

    case EigenSolver::SOL_NONE:
      std::cout << "The linear problem has no solution.\n";
      break;

    default:
      std::cout << "Some error occured\n";
      break;
  }

  std::cout << std::endl;

  // similar with the matrix provided in matrix.hpp header

  simplex::Matrix<float> mobjectiveFunction;
  mobjectiveFunction.resize(4, 1);
  mobjectiveFunction(0) = 1;
  mobjectiveFunction(1) = 1;
  mobjectiveFunction(2) = 1;
  mobjectiveFunction(3) = 1;

  simplex::Matrix<float> mconstraints;
  mconstraints.resize(3, 5);
  mconstraints(0, 0) = 1;
  mconstraints(0, 1) = 1;
  mconstraints(0, 2) = 0;
  mconstraints(0, 3) = 0;
  mconstraints(0, 4) = 20;

  mconstraints(1, 0) = 0;
  mconstraints(1, 1) = 1;
  mconstraints(1, 2) = 1;
  mconstraints(1, 3) = 0;
  mconstraints(1, 4) = 20;

  mconstraints(2, 0) = 0;
  mconstraints(2, 1) = 0;
  mconstraints(2, 2) = 1;
  mconstraints(2, 3) = 1;
  mconstraints(2, 4) = 20;

  Solver solver3(Solver::MODE_MINIMIZE, mobjectiveFunction, mconstraints);

  switch (solver3.hasSolution())
  {
    case Solver::SOL_FOUND:
      {
        std::cout << "The minimum is: " << solver3.getOptimum() << std::endl;
        auto sol = solver3.getSolution();
        std::cout << "The solution is: " << sol(0) << " " << sol(1) << " " << sol(2) << " " << sol(3) << std::endl;
      }
      break;

    case Solver::SOL_NONE:
      std::cout << "The linear problem has no solution.\n";
      break;

    default:
      std::cout << "Some error occured\n";
      break;
  }

  return 0;
}

