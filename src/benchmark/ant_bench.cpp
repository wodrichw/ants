
#include <benchmark/benchmark.h>

#include "app/facade.hpp"

static void run_save(benchmark::State& state, std::string const& filename) {
    ProjectArguments config("", "../assets/saves/" + filename, false, false, true);
    AntGameFacade game(config);
    for(auto _ : state) {
        for(ulong i = 0; i < 60; ++i) {
            game.engine_update();
        }
    }
}

static void run_small(benchmark::State& state) {
    run_save(state, "small_sq");
}
static void run_mid(benchmark::State& state) {
    run_save(state, "mid_sq");
}
BENCHMARK(run_small);
BENCHMARK(run_mid);

BENCHMARK_MAIN();
