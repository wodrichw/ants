
#include <benchmark/benchmark.h>

#include "app/facade.hpp"

static void run_save(benchmark::State& state, std::string const& filename) {
    ProjectArguments config("", "../assets/saves/" + filename, false, false, true);
    AntGameFacade game(config);
    for(auto _ : state) {
        for(ulong i = 0; i < 600; ++i) {
            game.engine_update();
        }
    }
}

static void run_small(benchmark::State& state) {
    run_save(state, "small_sq");
}
static void run_mid(benchmark::State& state) {
    run_save(state, "100_ants");
}
static void run_counter(benchmark::State& state) {
    run_save(state, "count");
}
// BENCHMARK(run_small);
BENCHMARK(run_mid);
// BENCHMARK(run_counter);



BENCHMARK_MAIN();
