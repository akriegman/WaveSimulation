#pragma once

#include <complex>

// Inadvisably, I've used defines and static consts to set
// the options for my program. This means I have to recompile
// whenever I change an option. In other projects I've worked
// on since this I load the options from a JSON file or take
// them as command line arguments.

// Which differential equation to simulate
#define WAVE
//#define DIFFUSION
//#define EFFUSION

// Whether to use damping
#define DAMP

// What boundary condition to use
// TODO: implement periodic boundary condition
#define FIXED
//#define FREE
//#define INFINITE // Not implemented. Not possible?

// Whether to show the FFT of the current state
//#define FOURIER

namespace g {

using scalar = float;
using complex = std::complex<scalar>;

static const char* title = "Differential Equation Solver";
static const float maxFPS = 50;
static const int stepsPerFrame = 2;
static const int numThreads = 6;
// Threads | ~FPS | %CPU
//       1 |    6 |
//       2 |   11 | 23.5
//       4 |   19 |
//       6 |   24 | 60.0
//       8 |   27 |


static const int width = 960;
static const int height = 540;
static const float scale = 1;
static const scalar cc = 700; // God knows what units
static const scalar c = sqrt(cc);
static const scalar dt = 1 / maxFPS / stepsPerFrame;
static const scalar damp = 0.9;
static const scalar dpf = pow(damp, dt);

} // namespace g