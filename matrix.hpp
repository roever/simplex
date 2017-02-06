#pragma once

#include <vector>

namespace simplex {

template <class T>
class Matrix
{
  public:

  int rows;
  int columns;

  std::vector<T> values;

  T & operator()(int row, int col) { return values[row*columns+col]; }
  const T & operator()(int row, int col) const { return values[row*columns+col]; }

  void resize(int r, int c)
  {
    values.clear();
    values.resize(c*r);
    rows = r;
    columns = c;
  }
};



template <class T> struct matrix_traits<Matrix<T>>
{
  using index_t = int;
  using scalar_t = double;
  static index_t columns(const Matrix<T> & m) { return m.columns; }
  static index_t rows(const Matrix<T> & m) { return m.rows; }
  static auto get(const Matrix<T> & m, index_t row, index_t col) { return m(row, col); }
  static void set(Matrix<T> & m, index_t row, index_t col, double val) { m(row, col) = val; }
  static void resize(Matrix<T> & m, index_t rows, index_t cols) { m.resize(rows, cols); }
};

}
