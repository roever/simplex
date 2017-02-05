#pragma once

#include <vector>

template <class T>
class MyMatrix
{
  public:

  int rows;
  int columns;

  std::vector<T> values;

  T & operator()(int row, int col) { return values[row*columns+col]; }
  const T & operator()(int row, int col) const { return values[row*columns+col]; }

  void resize(int r, int c)
  {
    values.resize(c*r);
    rows = r;
    columns = c;
  }
};



template <class T> struct matrix_traits<MyMatrix<T>>
{
  using index_t = int;
  using scalar_t = double;
  static index_t columns(const MyMatrix<T> & m) { return m.columns; }
  static index_t rows(const MyMatrix<T> & m) { return m.rows; }
  static auto get(const MyMatrix<T> & m, index_t row, index_t col) { return m(row, col); }
  static void set(MyMatrix<T> & m, index_t row, index_t col, double val) { m(row, col) = val; }
};

