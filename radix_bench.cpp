/*
	Sorting benchmark. Requires the google benchmark library.
	https://github.com/google/benchmark
*/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include <benchmark/benchmark.h>
#include "radix_sort.hpp"

static void *org_data;
static size_t org_size;

static void* read_file(const char *filename, size_t *limit) {
	void *keys = NULL;
	size_t bytes = 0;

	FILE *f = fopen(filename, "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		bytes = ftell(f);
		fseek(f, 0, SEEK_SET);

		if (*limit > 0 && *limit < bytes)
			bytes = *limit;

		printf("Allocating and reading %zu bytes.\n", bytes);
		keys = malloc(bytes);

		long rnum = fread(keys, bytes, 1, f);
		fclose(f);
		if (rnum != 1) {
			free(keys);
			return NULL;
		}
	}

	*limit = bytes;
	return keys;
}

template <typename T>
class FileSort : public ::benchmark::Fixture {
public:
	typedef T value_type;

	void SetUp(const ::benchmark::State& state) {
		if (!org_data) {
			org_data = (T*)read_file("40M_32bit_keys.dat", &org_size);
			assert(org_data);
		}
		this->max_n = org_size / sizeof(T);
		this->n = state.range(0);
		if (n <= max_n) {
			this->src = new T[n];
			this->aux = new T[n];
			memcpy(this->src, org_data, sizeof(T) * n);
		}
	}

	void TearDown(const ::benchmark::State& state) {
		delete[](this->src);
		delete[](this->aux);
		this->src = this->aux = NULL;
	}

	T *src;
	T *aux;
	size_t n;
	size_t max_n;
};


using FSu32 = FileSort<uint32_t>;

BENCHMARK_DEFINE_F(FSu32, radix_sort)(benchmark::State &state) {
	if (n > max_n)
		state.SkipWithError("Not enough source data to benchmark!");
	for (auto _ : state) {
		auto *sorted = radix_sort(src, aux, n, true);
	}
	state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * n);
}

BENCHMARK_DEFINE_F(FSu32, StdSort)(benchmark::State &state) {
	if (n > max_n)
		state.SkipWithError("Not enough source data to benchmark!");
	for (auto _ : state) {
		std::sort(src, src + n);
	}
	state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * n);
}

BENCHMARK_REGISTER_F(FSu32, radix_sort)->RangeMultiplier(8)->Range(8, 8 << 21);
BENCHMARK_REGISTER_F(FSu32, StdSort)->RangeMultiplier(8)->Range(8, 8 << 21);

BENCHMARK_MAIN();