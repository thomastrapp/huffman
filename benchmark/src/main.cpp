#include <cstdlib>
#include "benchmark/benchmark.h"

#include "encode/main.h"

int main(int argc, const char** argv)
{
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  return EXIT_SUCCESS;
}

