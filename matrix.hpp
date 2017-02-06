#pragma once

#include <vector>

namespace simplex {

/// a really tirvial class for 2d-matrices. If you need more bells ans whistles go somewhere
/// else. See example for usage
template <class T>
class Matrix
{
  private:
    int rows = 0;
    int columns = 0;

    std::vector<T> values;

  public:

    Matrix() = default;

    T & operator()(int row, int col) { return values[row*columns+col]; }
    const T & operator()(int row, int col) const { return values[row*columns+col]; }

    // these two allow access of the matrix like a vector. It doesn't even depend
    // on row or column vector...
    T & operator()(int pos) { return values[pos]; }
    const T & operator()(int pos) const { return values[pos]; }

    int getColumns() const { return columns; }
    int getRows() const { return rows; }

    void resize(int r, int c)
    {
      // make sure the resized vector is completely zeroed
      values.clear();
      values.resize(c*r);
      rows = r;
      columns = c;
    }
};

/// trait to use our simplistic class with the Solver
template <class T> struct matrix_traits<Matrix<T>>
{
  using index_t = int;
  using scalar_t = T;
  static index_t columns(const Matrix<T> & m) { return m.getColumns(); }
  static index_t rows(const Matrix<T> & m) { return m.getRows(); }
  static auto get(const Matrix<T> & m, index_t row, index_t col) { return m(row, col); }
  static void set(Matrix<T> & m, index_t row, index_t col, double val) { m(row, col) = val; }
  static void resize(Matrix<T> & m, index_t rows, index_t cols) { m.resize(rows, cols); }
};

}
