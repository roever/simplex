# Simplex

A simple class for solving inequality systems using a simplex solver.

- it features a maximiser and minimizer
- the matrix used for specifying the problem, working with the tableau and container for the
  solution is adaptable via a trait
- you can use the included extremely simple matrix class
- you can use the prepared eigen support to use eigen matrices
- header-only andonly dependent on c++ standard headers

## Documentation

- doxygen comments in all headers
- look at solver.hpp and example.cpp first
- matrix.hpp provides a trivial matrix to use with the solver
- eigen.hpp provide the traits to use with eigen matrices

## Copyright

Copyright (C) 2012  Tamas Bolner For more information, visit: http://blog.bolner.hu/2012/08/22/simplex/
Adapted for modern C++ (C) 2017 Andreas Roever
