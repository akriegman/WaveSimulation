#pragma once

#include <unsupported/Eigen/CXX11/Tensor>
#include "Globals.h"
#include "Convenience.h"
#include <cmath>

namespace Eigen
{
#if 0
static g::scalar laplaceCoeffs = {1, 1, 1,
									 1,-8, 1,
									 1, 1, 1};
static const TensorMap<Mat> laplace(laplaceCoeffs, 3, 3);
#endif

inline Mat laplacian()
{
  Mat lap(3, 3);
  lap << 1, 1, 1,
	     1,-8, 1,
	     1, 1, 1;
  return lap;
}


template<int n>
inline Tensor<g::scalar, n> gaussian(g::scalar sigma, int rad)
{
  int diam = 2 * rad + 1;
  ArrayXs lin = ArrayXs::LinSpaced(diam, -rad / sigma, rad / sigma);
  Dims<n> ones, diams;
  for (int j = 0; j < n; j++)
  {
	ones[j] = 1;
	diams[j] = diam;
  }
  Tensor<g::scalar, n> out(diams);
  out.setZero();
  for (int i = 0; i < n; i++)
  {
	ones[i] = diam; diams[i] = 1;
	out += TensorMap<Tensor<g::scalar, n>>(lin.data(), ones).broadcast(diams).square();
	ones[i] = 1; diams[i] = diam;
  }
  return (out * (g::scalar)(-0.5)).exp();
}

inline Mat sineMode(int qx, int qy, int width, int height)
{
  ArrayXs sinx = ArrayXs::LinSpaced( width, 0, EIGEN_PI * qx).sin();
  ArrayXs siny = ArrayXs::LinSpaced(height, 0, EIGEN_PI * qy).sin();
  return TensorMap<Vec>(sinx.data(), width).contract(TensorMap<Vec>(siny.data(), height), Pairs<0>{});
}

} // end namespace Eigen