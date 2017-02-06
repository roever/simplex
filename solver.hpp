#pragma once

#include <cstdint>
#include <limits>

namespace simplex {

/// Trait class for the matrix operations needed by the solver class, for an example
/// implementation look at the header "eigen.hpp" or "matrix.hpp"
///
/// You must provide the following members for your matrix type M:
/// - index_t type to use for indexing the matrix... probably usually int, except if you really
///           have huge matricies... but then this solver is probably not what you need
/// - scalar_t type used for the numerical values. Most likely float or double. These values are
///            supposedly stored in the matrix and are used by the Solver for all numerical calculations
///            there must be a numeric_limits specialisation for scalar_t available
/// - static index_t columns(const M & m) return the number of columns of matrix m
/// - static index_t rows(const M & m) return number of rows of matrix m
/// - static scalar_t get(const M & m, index_t row, index_t col) return value in the given row and column of matrix m
/// - static void set(M & m, index_t row, index_t col, scalar_t val) set value in m to val
/// - static void resize(M & m, index_t rows, index_t cols) resize m to the given size and !!IMPORTANT!! zero all values
template <class M> struct matrix_traits;

/// Solve unequality systems using the simplex method
/// \tparam M a matrix class used to store the problem and the solution, you have to provide
///           a specialisation of struct matrix_traits for the matrix type you want to use
template <class M>
class Solver
{
  public:
    enum SolutionType {
      SOL_FOUND,             ///< at least one solution found
      SOL_NONE,              ///< No solution has been found
      SOL_ERR_OBJ_COLUMN,    ///< objective function must contain exactly one column
      SOL_ERR_OBJ_ROWS,      ///< The coefficient vector of the objective function must contain at least one row.
      SOL_ERR_OBJ_COEFF,     ///< One of the coefficients of the objective function is zero.
      SOL_ERR_CONSTR_ROWS,   ///< The constraint matrix must contain at least one row.
      SOL_ERR_CONSTR_COLUMN, ///< The constraint matrix must contain one column more than there are number of variables
      SOL_ERR_CONSTR_RHS,    ///< All righthand-side coefficients of the constraint matrix must be non-negative.
    };

    enum Mode {
      MODE_MINIMIZE,   ///< minimize objective function, constraints are assumed to be of form a11*x1+a12*x2+... >= b1
      MODE_MAXIMIZE    ///< maximize objective function, constraints are assumed to be of form a11*x1+a12*x2+... <= b1
    };

  private:

    // convenience types for indexing the matrix and the elements in the matrix
    using index_t = typename matrix_traits<M>::index_t;
    using scalar_t = typename matrix_traits<M>::scalar_t;

    static constexpr bool closeTo1(scalar_t v)
    {
      return (scalar_t)(1)-10*std::numeric_limits<scalar_t>::epsilon() <= v && v <= (scalar_t)(1)+10*std::numeric_limits<scalar_t>::epsilon();
    }
    constexpr bool closeTo0(scalar_t v, scalar_t epsilon)
    {
      return -epsilon <= v && v <= epsilon;
    }

    enum SolutionType foundSolution;
    scalar_t optimum;
    M solution;  // a vector with one column with the solution

    // Searches for the pivot row in the given column, by calculating the ratios.
    // Tries to find smallest non-negative ratio.
    // Returns -1 if all possible pivots are 0 or if the ratios are negative.
    // Deals with cases like this:  0/negative < 0/positive
    //
    // tableau, the simplex tableau to work on
    // column the column selected and to find the pivot row in
    // pivot returns the number of the pivot row, only valid when the function returns true
    // the function returns true, if a row has been found
    bool findPivot_min(const M & tableau, index_t column, index_t & pivot, scalar_t epsilon)
    {
      auto constantColumn = matrix_traits<M>::columns(tableau) - 1;
      index_t minIndex = 0;
      scalar_t minRatio = std::numeric_limits<scalar_t>::max();
      bool found = false;

      for (index_t i = 1; i < matrix_traits<M>::rows(tableau); i++)
      {
        if (!closeTo0(matrix_traits<M>::get(tableau, i, column), epsilon))
        {
          scalar_t ratio = matrix_traits<M>::get(tableau, i, constantColumn) / matrix_traits<M>::get(tableau, i, column);
          if (ratio >= 0)
          {
            found = true;
            if (ratio < minRatio)
            {
              minIndex = i;
              minRatio = ratio;
            }
            else if ((closeTo0(ratio, epsilon)) && (closeTo0(minRatio, epsilon)))
            {
              // 0/negative < 0/positive
              if (matrix_traits<M>::get(tableau, i, constantColumn) < matrix_traits<M>::get(tableau, minIndex, constantColumn))
              {
                minIndex = i;
                minRatio = ratio;
              }
            }
          }
        }
      }

      pivot = minIndex;
      return found;
    }

    // the real simplex method implementation
    //
    // tableau is the simplex tableau to work on
    // variableNum The number of variables (dimensions)
    // returns the solution type for the found solution
    SolutionType simplexAlgorithm(M & tableau, index_t variableNum, scalar_t epsilon)
    {
      while (true)
      {
        index_t pivotColumn = 0;

        // Find pivot column, check for halt condition
        for (index_t j = 1; j < variableNum; j++)
          if (matrix_traits<M>::get(tableau, 0, j) < matrix_traits<M>::get(tableau, 0, pivotColumn))
            pivotColumn = j;

        if (matrix_traits<M>::get(tableau, 0, pivotColumn) >= 0)
        {
          //Found no negative coefficient
          break;
        }

        // Find pivot row
        index_t pivotRow;
        if (!findPivot_min(tableau, pivotColumn, pivotRow, epsilon))
        {
          //no solution
          return SOL_NONE;
        }

        // Do pivot operation
        scalar_t div = matrix_traits<M>::get(tableau, pivotRow, pivotColumn);
        for (index_t j = 0; j < matrix_traits<M>::columns(tableau); j++)
        {
          matrix_traits<M>::set(tableau, pivotRow, j,
              matrix_traits<M>::get(tableau, pivotRow, j) / div);
        }
        for (index_t i = 0; i < matrix_traits<M>::rows(tableau); i++)
        {
          if (i != pivotRow)
          {
            scalar_t val = matrix_traits<M>::get(tableau, i, pivotColumn);
            for (index_t j = 0; j < matrix_traits<M>::columns(tableau); j++)
            {
              matrix_traits<M>::set(tableau, i,  j,
                  matrix_traits<M>::get(tableau, i, j) - matrix_traits<M>::get(tableau, pivotRow, j) * val);
            }
          }
        }
      }

      return SOL_FOUND;
    }

    // If the given column has only one coefficient with value 1 (except in topmost row), and all other
    // coefficients are zero, then returns the row of the non-zero value.
    // Otherwise return -1.
    // This method is used in the final step of maximization, when we read
    // the solution from the tableau.
    //
    // tableau to work on
    // column to work on
    // returns the row with the one 1 for the column in row
    // returns true, if such row has been found, false otherwise
    bool getPivotRow(const M & tableau, index_t column, index_t & row, scalar_t epsilon)
    {
      index_t one_row = -1;
      bool found = false;

      for (index_t i = 1; i < matrix_traits<M>::rows(tableau); i++)
      {
        if (closeTo1(matrix_traits<M>::get(tableau, i, column)))
        {
          if (found)
          {
            return false;
          }
          else
          {
            one_row = i;
            found = true;
          }
        }
        else if (!closeTo0(matrix_traits<M>::get(tableau, i, column), epsilon))
        {
          return false;
        }
      }

      row = one_row;
      return true;
    }

  public:

    /// Try to solve the given problem
    ///
    /// \param mode choose to either maximize or minimize objective function
    /// \param objectiveFunction The coefficients of the objective function that is either minimizes or maximized, it must
    ///        be a column matrix with as many rows as there are variables
    /// \param constraints full matrix for the constraints. Contains also the righthand-side values. The first
    ///        few columns are the coefficients of the constraints, the last column the right hand side
    ///        depending on mode, the inequalities are either all > rhs (MODE_MINIMIZE) or < rhs
    /// \param epsilon the limit for values to be assumed equal to zero. This value depends on the range for your
    ///        constraints and your objective function. It should be some orders of magnitude below the smallest
    ///        significant digits. There is a default value that should be good for anything in the range or 1000 down to
    ///        1/1000... so "normal values"
    ///
    /// \note There are all kinds of constraints that must be fulfilled for simplex to work, look them up
    ///       in the literature, e.g. number of rows in objective functions must be identical to the number
    ///       of columns in the constraints plus one, the one additional column contains the boundaries
    ///       of the constraints, if you want to find out what is wrong
    Solver(Mode mode, const M & objectiveFunction, const M & constraints, scalar_t
        epsilon = std::numeric_limits<scalar_t>::epsilon() * 1000) : optimum(0)
    {
      index_t numberOfVariables = matrix_traits<M>::rows(objectiveFunction);
      index_t numberOfConstraints = matrix_traits<M>::rows(constraints);

      if (matrix_traits<M>::columns(objectiveFunction) != 1)
      {
        foundSolution = SOL_ERR_OBJ_COLUMN;
        return;
      }

      if (numberOfVariables < 1)
      {
        foundSolution = SOL_ERR_OBJ_ROWS;
        return;
      }

      if (numberOfConstraints < 1)
      {
        foundSolution = SOL_ERR_CONSTR_ROWS;
        return;
      }

      if (matrix_traits<M>::columns(constraints) != numberOfVariables + 1)
      {
        foundSolution = SOL_ERR_CONSTR_COLUMN;
        return;
      }

      for (index_t i = 0; i < numberOfVariables; i++)
        if (closeTo0(objectiveFunction(i, 0), epsilon))
        {
          foundSolution = SOL_ERR_OBJ_COEFF;
          return;
        }


      for (index_t i = 0; i < numberOfConstraints; i++)
        if (matrix_traits<M>::get(constraints, i, numberOfVariables) < 0)
        {
          foundSolution = SOL_ERR_CONSTR_RHS;
          return;
        }

      M tableau;

      if (mode == MODE_MAXIMIZE)
      {
        // Maximize
        matrix_traits<M>::resize(tableau, numberOfConstraints + 1, numberOfVariables + numberOfConstraints + 1);

        // resize is assumed to fill the whole matrix with zeros

        for (index_t i = 0; i < numberOfVariables; i++)
          matrix_traits<M>::set(tableau, 0, i, -matrix_traits<M>::get(objectiveFunction, i, 0));

        for (index_t i = 0; i < numberOfConstraints; i++)
          for (index_t j = 0; j < numberOfVariables; j++)
            matrix_traits<M>::set(tableau, i+1, j, matrix_traits<M>::get(constraints, i, j));

        for (index_t i = 0; i < numberOfConstraints; i++)
        {
          matrix_traits<M>::set(tableau, i+1, i+numberOfVariables, 1);
          matrix_traits<M>::set(tableau, i+1, numberOfVariables+numberOfConstraints,
              matrix_traits<M>::get(constraints, i, numberOfVariables));
        }

        // Maximize original problem
        foundSolution = simplexAlgorithm(tableau, numberOfVariables, epsilon);

        if (foundSolution != SOL_NONE)
        {
          index_t constantColumn = matrix_traits<M>::columns(tableau) - 1;
          matrix_traits<M>::resize(solution, numberOfVariables, 1);

          // Maximize
          for (index_t i = 0; i < numberOfVariables; i++)
          {
            index_t temp;

            if (getPivotRow(tableau, i, temp, epsilon))
            {
              // Basic variable
              matrix_traits<M>::set(solution, i, 0, matrix_traits<M>::get(tableau, temp, constantColumn));
            }
            else
            {
              // Non-basic variable
              matrix_traits<M>::set(solution, i, 0, 0);
            }
          }
          optimum = matrix_traits<M>::get(tableau, 0, constantColumn);
        }
      }
      else
      {
        // Minimize: construct the Dual problem
        matrix_traits<M>::resize(tableau, numberOfVariables + 1, numberOfVariables + numberOfConstraints + 1);

        for (index_t i = 0; i < numberOfConstraints; i++)
          matrix_traits<M>::set(tableau, 0, i, -matrix_traits<M>::get(constraints, i, numberOfVariables));

        for (index_t i = 0; i < numberOfConstraints; i++)
          for (index_t j = 0; j < numberOfVariables; j++)
            matrix_traits<M>::set(tableau, j+1, i, matrix_traits<M>::get(constraints, i, j));

        for (index_t i = 0; i < numberOfVariables; i++)
        {
          matrix_traits<M>::set(tableau, i+1, i+matrix_traits<M>::rows(constraints), 1);
          matrix_traits<M>::set(tableau, i+1, numberOfVariables+numberOfConstraints,
              matrix_traits<M>::get(objectiveFunction, i, 0));
        }

        // Maximize the dual of the minimization problem
        foundSolution = simplexAlgorithm(tableau, numberOfConstraints, epsilon);

        if (foundSolution != SOL_NONE)
        {
          matrix_traits<M>::resize(solution, numberOfVariables, 1);

          // Minimize
          for (index_t i = 0; i < numberOfVariables; i++)
            matrix_traits<M>::set(solution, i, 0, matrix_traits<M>::get(tableau, 0, numberOfConstraints + i));

          optimum = matrix_traits<M>::get(tableau, 0, matrix_traits<M>::columns(tableau) - 1);
        }
      }
    }

    /// Returns true if a solution has been found.
    /// Return false otherwise.
    ///
    /// @returns boolean
    SolutionType hasSolution() const { return foundSolution; }

    /// Returns the maximum/minimum value of the objective function.
    ///
    /// \returns the value
    scalar_t getOptimum() const { return optimum; }

    /// Get the solution
    ///
    /// \return reference to the solution vector, which is a column vector with as many
    ///         columns as there were variables in the problem
    const M & getSolution() const { return solution; }
};

}

