/*
//@HEADER
// ************************************************************************
//
//                        Kokkos v. 2.0
//              Copyright (2019) Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Christian R. Trott (crtrott@sandia.gov)
//
// ************************************************************************
//@HEADER
*/

#include <experimental/mdspan>

#include <benchmark/benchmark.h>

#include "fill.hpp"

namespace stdex = std::experimental;
_MDSPAN_INLINE_VARIABLE constexpr auto dyn = stdex::dynamic_extent;

template <class MDSpan, class... DynSizes>
void BM_MDSpan_Copy_2D_right(benchmark::State& state, MDSpan, DynSizes... dyn) {
  using value_type = typename MDSpan::value_type;
  auto buffer = std::make_unique<value_type[]>(
    MDSpan{nullptr, dyn...}.mapping().required_span_size()
  );
  auto buffer2 = std::make_unique<value_type[]>(
    MDSpan{nullptr, dyn...}.mapping().required_span_size()
  );
  auto s = MDSpan{buffer.get(), dyn...};
  mdspan_benchmark::fill_random(s);
  auto dest = MDSpan{buffer2.get(), dyn...};
  for (auto _ : state) {
    for(ptrdiff_t i = 0; i < s.extent(0); ++i) {
      for (ptrdiff_t j = 0; j < s.extent(1); ++j) {
          dest(i, j) = s(i, j);
      }
    }
    benchmark::DoNotOptimize(s.data());
    benchmark::DoNotOptimize(dest.data());
  }
  state.SetBytesProcessed(s.size() * sizeof(value_type) * state.iterations());
}

BENCHMARK_CAPTURE(
  BM_MDSpan_Copy_2D_right, size_100_100, stdex::mdspan<int, 100, 100>()
);
BENCHMARK_CAPTURE(
  BM_MDSpan_Copy_2D_right, size_100_dyn, stdex::mdspan<int, 100, dyn>(), 100
);
BENCHMARK_CAPTURE(
  BM_MDSpan_Copy_2D_right, size_dyn_dyn, stdex::mdspan<int, dyn, dyn>(), 100, 100
);

//================================================================================

template <class MDSpan, class LayoutMapping>
void BM_MDSpan_Copy_2D_stride(benchmark::State& state, MDSpan, LayoutMapping map) {
  benchmark::DoNotOptimize(map);
  using value_type = typename MDSpan::value_type;
  auto buffer = std::make_unique<value_type[]>(
    map.required_span_size()
  );
  auto buffer2 = std::make_unique<value_type[]>(
    map.required_span_size()
  );
  auto s = MDSpan{buffer.get(), map};
  mdspan_benchmark::fill_random(s);
  auto dest = MDSpan{buffer2.get(), map};
  for (auto _ : state) {
    for(ptrdiff_t i = 0; i < s.extent(0); ++i) {
      for (ptrdiff_t j = 0; j < s.extent(1); ++j) {
        dest(i, j) = s(i, j);
      }
    }
    benchmark::DoNotOptimize(s.data());
    benchmark::DoNotOptimize(dest.data());
  }
  state.SetBytesProcessed(s.size() * sizeof(value_type) * state.iterations());
}

BENCHMARK_CAPTURE(
  BM_MDSpan_Copy_2D_stride, size_100_100,
  stdex::basic_mdspan<int, stdex::extents<100, 100>, stdex::layout_stride<dyn, dyn>>(),
  stdex::layout_stride<dyn, dyn>::template mapping<stdex::extents<100, 100>>(
    stdex::extents<100, 100>{},
    // layout right
    std::array<ptrdiff_t, 2>{100, 1}
  )
);
BENCHMARK_CAPTURE(
  BM_MDSpan_Copy_2D_stride, size_100_100d,
  stdex::basic_mdspan<int, stdex::extents<100, dyn>, stdex::layout_stride<dyn, dyn>>(),
  stdex::layout_stride<dyn, dyn>::template mapping<stdex::extents<100, dyn>>(
    stdex::extents<100, dyn>{100},
    // layout right
    std::array<ptrdiff_t, 2>{100, 1}
  )
);
BENCHMARK_CAPTURE(
  BM_MDSpan_Copy_2D_stride, size_100d_100,
  stdex::basic_mdspan<int, stdex::extents<dyn, 100>, stdex::layout_stride<dyn, dyn>>(),
  stdex::layout_stride<dyn, dyn>::template mapping<stdex::extents<dyn, 100>>(
    stdex::extents<dyn, 100>{100},
    // layout right
    std::array<ptrdiff_t, 2>{100, 1}
  )
);
BENCHMARK_CAPTURE(
  BM_MDSpan_Copy_2D_stride, size_100d_100d,
  stdex::basic_mdspan<int, stdex::extents<dyn, dyn>, stdex::layout_stride<dyn, dyn>>(),
  stdex::layout_stride<dyn, dyn>::template mapping<stdex::extents<dyn, dyn>>(
    stdex::extents<dyn, dyn>{100, 100},
    // layout right
    std::array<ptrdiff_t, 2>{100, 1}
  )
);

BENCHMARK_CAPTURE(
  BM_MDSpan_Copy_2D_stride, size_fixed_dyn_100_100,
  stdex::basic_mdspan<int, stdex::extents<100, 100>, stdex::layout_stride<100, dyn>>(),
  stdex::layout_stride<100, dyn>::template mapping<stdex::extents<100, 100>>(
    stdex::extents<100, 100>{},
    // layout right
    std::array<ptrdiff_t, 1>{1}
  )
);
BENCHMARK_CAPTURE(
  BM_MDSpan_Copy_2D_stride, size_fixed_dyn_100_100d,
  stdex::basic_mdspan<int, stdex::extents<100, dyn>, stdex::layout_stride<100, dyn>>(),
  stdex::layout_stride<100, dyn>::template mapping<stdex::extents<100, dyn>>(
    stdex::extents<100, dyn>{100},
    // layout right
    std::array<ptrdiff_t, 1>{1}
  )
);
BENCHMARK_CAPTURE(
  BM_MDSpan_Copy_2D_stride, size_fixed_dyn_100d_100,
  stdex::basic_mdspan<int, stdex::extents<dyn, 100>, stdex::layout_stride<100, dyn>>(),
  stdex::layout_stride<100, dyn>::template mapping<stdex::extents<dyn, 100>>(
    stdex::extents<dyn, 100>{100},
    // layout right
    std::array<ptrdiff_t, 1>{1}
  )
);
BENCHMARK_CAPTURE(
  BM_MDSpan_Copy_2D_stride, size_dyn_fixed_100d_100d,
  stdex::basic_mdspan<int, stdex::extents<dyn, dyn>, stdex::layout_stride<100, dyn>>(),
  stdex::layout_stride<100, dyn>::template mapping<stdex::extents<dyn, dyn>>(
    stdex::extents<dyn, dyn>{100, 100},
    // layout right
    std::array<ptrdiff_t, 1>{1}
  )
);

BENCHMARK_CAPTURE(
  BM_MDSpan_Copy_2D_stride, size_dyn_fixed_100_100,
  stdex::basic_mdspan<int, stdex::extents<100, 100>, stdex::layout_stride<dyn, 1>>(),
  stdex::layout_stride<dyn, 1>::template mapping<stdex::extents<100, 100>>(
    stdex::extents<100, 100>{},
    // layout right
    std::array<ptrdiff_t, 1>{100}
  )
);
BENCHMARK_CAPTURE(
  BM_MDSpan_Copy_2D_stride, size_dyn_fixed_100_100d,
  stdex::basic_mdspan<int, stdex::extents<100, dyn>, stdex::layout_stride<dyn, 1>>(),
  stdex::layout_stride<dyn, 1>::template mapping<stdex::extents<100, dyn>>(
    stdex::extents<100, dyn>{100},
    // layout right
    std::array<ptrdiff_t, 1>{100}
  )
);
BENCHMARK_CAPTURE(
  BM_MDSpan_Copy_2D_stride, size_dyn_fixed_100d_100,
  stdex::basic_mdspan<int, stdex::extents<dyn, 100>, stdex::layout_stride<dyn, 1>>(),
  stdex::layout_stride<dyn, 1>::template mapping<stdex::extents<dyn, 100>>(
    stdex::extents<dyn, 100>{100},
    // layout right
    std::array<ptrdiff_t, 1>{100}
  )
);
BENCHMARK_CAPTURE(
  BM_MDSpan_Copy_2D_stride, size_dyn_fixed_100d_100d,
  stdex::basic_mdspan<int, stdex::extents<dyn, dyn>, stdex::layout_stride<dyn, 1>>(),
  stdex::layout_stride<dyn, 1>::template mapping<stdex::extents<dyn, dyn>>(
    stdex::extents<dyn, dyn>{100, 100},
    // layout right
    std::array<ptrdiff_t, 1>{100}
  )
);

//================================================================================

template <class T, class Extents, class MapSrc, class MapDst>
void BM_MDSpan_Copy_2D_stride_diff_map(benchmark::State& state,
  T, Extents exts, MapSrc map_src, MapDst map_dest
) {
  using value_type = T;
  auto buff_src = std::make_unique<value_type[]>(
    map_src.required_span_size()
  );
  auto buff_dest = std::make_unique<value_type[]>(
    map_dest.required_span_size()
  );
  using map_stride_dyn = stdex::layout_stride<dyn, dyn>;
  using mdspan_type = stdex::basic_mdspan<T, Extents, map_stride_dyn>;
  auto src = mdspan_type{buff_src.get(), map_src};
  mdspan_benchmark::fill_random(src);
  auto dest = mdspan_type{buff_dest.get(), map_dest};
  for (auto _ : state) {
    for(ptrdiff_t i = 0; i < src.extent(0); ++i) {
      for (ptrdiff_t j = 0; j < src.extent(1); ++j) {
        dest(i, j) = src(i, j);
      }
    }
    benchmark::DoNotOptimize(src.data());
    benchmark::DoNotOptimize(dest.data());
  }
  state.SetBytesProcessed(src.extent(0) * src.extent(1) * sizeof(value_type) * state.iterations());
}

BENCHMARK_CAPTURE(
  BM_MDSpan_Copy_2D_stride_diff_map, size_100d_100d_bcast_0, int(),
  stdex::extents<dyn, dyn>{100, 100},
  stdex::layout_stride<dyn, dyn>::template mapping<stdex::extents<dyn, dyn>>(
    stdex::extents<dyn, dyn>{100, 100},
    // layout right
    std::array<ptrdiff_t, 2>{0, 1}
  ),
  stdex::layout_stride<dyn, dyn>::template mapping<stdex::extents<dyn, dyn>>(
    stdex::extents<dyn, dyn>{100, 100},
    // layout right
    std::array<ptrdiff_t, 2>{100, 1}
  )
);

BENCHMARK_CAPTURE(
  BM_MDSpan_Copy_2D_stride_diff_map, size_100d_100d_bcast_1, int(),
  stdex::extents<dyn, dyn>{100, 100},
  stdex::layout_stride<dyn, dyn>::template mapping<stdex::extents<dyn, dyn>>(
    stdex::extents<dyn, dyn>{100, 100},
    // layout right
    std::array<ptrdiff_t, 2>{1, 0}
  ),
  stdex::layout_stride<dyn, dyn>::template mapping<stdex::extents<dyn, dyn>>(
    stdex::extents<dyn, dyn>{100, 100},
    // layout right
    std::array<ptrdiff_t, 2>{100, 1}
  )
);

BENCHMARK_CAPTURE(
  BM_MDSpan_Copy_2D_stride_diff_map, size_100d_100d_bcast_both, int(),
  stdex::extents<dyn, dyn>{100, 100},
  stdex::layout_stride<dyn, dyn>::template mapping<stdex::extents<dyn, dyn>>(
    stdex::extents<dyn, dyn>{100, 100},
    // layout right
    std::array<ptrdiff_t, 2>{0, 0}
  ),
  stdex::layout_stride<dyn, dyn>::template mapping<stdex::extents<dyn, dyn>>(
    stdex::extents<dyn, dyn>{100, 100},
    // layout right
    std::array<ptrdiff_t, 2>{100, 1}
  )
);


//================================================================================

template <class T>
void BM_Raw_Copy_1D(benchmark::State& state, T, ptrdiff_t size) {
  benchmark::DoNotOptimize(size);
  using value_type = T;
  auto buffer = std::make_unique<value_type[]>(size);
  {
    // just for setup...
    auto wrapped = stdex::mdspan<T, stdex::dynamic_extent>{buffer.get(), size};
    mdspan_benchmark::fill_random(wrapped);
  }
  value_type* src = buffer.get();
  auto buffer2 = std::make_unique<value_type[]>(size);
  value_type* dest = buffer2.get();
  for (auto _ : state) {
    for(ptrdiff_t i = 0; i < size; ++i) {
      dest[i] = src[i];
    }
    benchmark::DoNotOptimize(src);
    benchmark::DoNotOptimize(dest);
  }
  state.SetBytesProcessed(size * sizeof(value_type) * state.iterations());
}

BENCHMARK_CAPTURE(
  BM_Raw_Copy_1D, size_10000, int(), 10000
);

//================================================================================

template <class T>
void BM_Raw_Copy_2D(benchmark::State& state, T, ptrdiff_t x, ptrdiff_t y) {
  using value_type = T;
  auto buffer = std::make_unique<value_type[]>(x * y);
  {
    // just for setup...
    auto wrapped = stdex::mdspan<T, stdex::dynamic_extent>{buffer.get(), x * y};
    mdspan_benchmark::fill_random(wrapped);
  }
  value_type* src = buffer.get();
  auto buffer2 = std::make_unique<value_type[]>(x * y);
  value_type* dest = buffer2.get();
  for (auto _ : state) {
    for(ptrdiff_t i = 0; i < x; ++i) {
      for(ptrdiff_t j = 0; j < y; ++j) {
        dest[i*y + j] = src[i*y + j];
      }
    }
    benchmark::DoNotOptimize(src);
    benchmark::DoNotOptimize(dest);
  }
  state.SetBytesProcessed(x * y * sizeof(value_type) * state.iterations());
}

BENCHMARK_CAPTURE(
  BM_Raw_Copy_2D, size_100_100, int(), 100, 100
);

//================================================================================

BENCHMARK_MAIN();
