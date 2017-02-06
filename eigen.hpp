#pragma once

#include <Eigen/Dense>

namespace simplex {

/// matrix_traits specialisation for dense eigen matrices. All completely dynamic matricies
/// with float, double or long double can be just used with this... see example
template <class T> struct matrix_traits<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>>
{
  using index_t = typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>::Index;
  using scalar_t = T;
  static index_t columns(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> & m) { return m.cols(); }
  static index_t rows(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> & m) { return m.rows(); }
  static scalar_t get(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> & m, index_t row, index_t col) { return m(row, col); }
  static void set(Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> & m, index_t row, index_t col, scalar_t val) { m(row, col) = val; }
  static void resize(Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> & m, index_t rows, index_t cols) { m.setZero(rows, cols); }
};

}

