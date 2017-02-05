#include <iostream>
#include <Eigen/Dense>
#include "SimplexSolver.h"
#include <vector>

using namespace std;
using namespace Eigen;



class MyMatrix
{
  public:

  int rows;
  int columns;

  std::vector<double> values;

  double & operator()(int row, int col) { return values[row*columns+col]; }
  const double & operator()(int row, int col) const { return values[row*columns+col]; }

  void resize(int r, int c)
  {
    values.resize(c*r);
    rows = r;
    columns = c;
  }
};



template <> struct matrix_traits<MatrixXd>
{
  using index_t = typename MatrixXd::Index;
  using scalar_t = typename MatrixXd::Scalar;
  static index_t columns(const MatrixXd & m) { return m.cols(); }
  static index_t rows(const MatrixXd & m) { return m.rows(); }
  static scalar_t get(const MatrixXd & m, index_t row, index_t col) { return m(row, col); }
  static void set(MatrixXd & m, index_t row, index_t col, scalar_t val) { m(row, col) = val; }
};

template <> struct matrix_traits<MyMatrix>
{
  using index_t = int;
  using scalar_t = double;
  static index_t columns(const MyMatrix & m) { return m.columns; }
  static index_t rows(const MyMatrix & m) { return m.rows; }
  static auto get(const MyMatrix & m, index_t row, index_t col) { return m(row, col); }
  static void set(MyMatrix & m, index_t row, index_t col, double val) { m(row, col) = val; }
};

int main()
{
  MatrixXd constraints(3, 3);
  VectorXd objectiveFunction(2);

  /*
    Maximization problem
   */
  objectiveFunction <<	1, 2;

  constraints <<  2, 3, 34,
                  1, 5, 45,
                  1, 0, 15;

  SimplexSolver<MatrixXd> solver1(SIMPLEX_MAXIMIZE, objectiveFunction, constraints);

  switch (solver1.hasSolution())
  {
    case SimplexSolver<MatrixXd>::SOL_FOUND:
      cout << "The minimum is: " << solver1.getOptimum() << endl;
      cout << "The solution is: " << solver1.getSolution().transpose() << endl;
      break;

    case SimplexSolver<MatrixXd>::SOL_NONE:
      cout << "The linear problem has no solution.\n";
      break;

    default:
      cout << "Some error occured\n";
      break;
  }

  cout << endl;

  /*
     Minimization problem
   */
  objectiveFunction <<	3, 4;

  constraints << 2, 1, 8,
                 1, 2, 13,
                 1, 5, 16;

  SimplexSolver<MatrixXd> solver2(SIMPLEX_MINIMIZE, objectiveFunction, constraints);

  switch (solver2.hasSolution())
  {
    case SimplexSolver<MatrixXd>::SOL_FOUND:
      cout << "The minimum is: " << solver2.getOptimum() << endl;
      cout << "The solution is: " << solver2.getSolution().transpose() << endl;
      break;

    case SimplexSolver<MatrixXd>::SOL_NONE:
      cout << "The linear problem has no solution.\n";
      break;

    default:
      cout << "Some error occured\n";
      break;
  }

  // again with my own matrix class

  MyMatrix mobjectiveFunction;
  mobjectiveFunction.resize(4, 1);
  mobjectiveFunction(0, 0) = 1;
  mobjectiveFunction(1, 0) = 1;
  mobjectiveFunction(2, 0) = 1;
  mobjectiveFunction(3, 0) = 1;

  MyMatrix mconstraints;
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

  SimplexSolver<MyMatrix> solver3(SIMPLEX_MINIMIZE, mobjectiveFunction, mconstraints);

  switch (solver3.hasSolution())
  {
    case SimplexSolver<MyMatrix>::SOL_FOUND:
      {
        cout << "The minimum is: " << solver3.getOptimum() << endl;
        auto sol = solver3.getSolution();
        cout << "The solution is: " << sol(0, 0) << " " << sol(1, 0) << " " << sol(2, 0) << " " << sol(3, 0) << endl;
      }
      break;

    case SimplexSolver<MyMatrix>::SOL_NONE:
      cout << "The linear problem has no solution.\n";
      break;

    default:
      cout << "Some error occured\n";
      break;
  }

  return 0;
}

