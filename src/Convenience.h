#pragma once

#include <unsupported/Eigen/CXX11/Tensor>
#include <iostream>
#include <vector>
#include "Globals.h"

#define PIBAR 6.2831853071795864769252867665590057683943387987502l


namespace Eigen
{

typedef Tensor<g::scalar, 2>               Mat;
typedef Tensor<g::scalar, 1>               Vec;
typedef Tensor<g::scalar, 0>               Scal;
typedef Array<g::scalar, Dynamic, 1>       ArrayXs;
typedef Tensor<g::complex, 2>              Matplex;
typedef Mat::DimensionPair                 Pair;
template <size_t N> using                  Pairs               = std::array<Pair, N>;
template <size_t N> using                  Dims                = typename Tensor<g::scalar, N>::Dimensions;
template <size_t N> using                  Scals               = std::array<g::scalar, N>;
template <size_t N> using                  GaussianGenerator   = internal::GaussianGenerator<g::scalar, DenseIndex, N>;

template<typename Scalar, int n, typename Generator>
inline auto makeGenerator(const Generator& generator, const Dims<n>& dimensions)
{
  return TensorMap<Tensor<Scalar, n>>(0, dimensions).generate(generator);
}

template<int n, typename Params>
inline auto makeGaussian(const Params& means, const Params& sigmas, const Params& dims)
{
  return makeGenerator<g::scalar, n, GaussianGenerator<g::scalar, n>>(GaussianGenerator<g::scalar, n>(means, sigmas));
}

} // end namespace Eigen

static const g::complex RPLEX = std::polar<g::scalar>(1,                0);
static const g::complex GPLEX = std::polar<g::scalar>(1, EIGEN_PI * 2 / 3);
static const g::complex BPLEX = std::polar<g::scalar>(1, EIGEN_PI * 4 / 3);

// Convenience define to extract tensor data to std::vector
#define TENSOR_VECTOR(Scalar, t) std::vector<Scalar>(t.data(), t.data() + t.size())


// Copied from the internet
template < class T >
inline std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
  os << "[";
  for (auto ii = v.begin(); ii != v.end(); ++ii)
  {
    os << " " << *ii;
  }
  os << " ]";
  return os;
}

template<typename T>
const T& clamp(const T& v, const T& low, const T& high)
{
  return v < low ? low : (v > high ? high : v);
}