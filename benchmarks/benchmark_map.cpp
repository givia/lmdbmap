#include <benchmark/benchmark.h>
#include <lmdbmap/map.hpp>
#include <lmdbmap/environment.hpp>
#include <lmdbmap/transaction.hpp>
#include <filesystem>
#include <string>
#include <vector>

class MapBenchmark : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) {
        db_path = "bench_db_" + std::to_string(state.thread_index());
        std::filesystem::remove_all(db_path);
        env = std::make_unique<lmdbmap::environment>(db_path);
        map = std::make_unique<lmdbmap::map<int, std::string>>(*env, "bench_map");
    }

    void TearDown(const ::benchmark::State& state) {
        map.reset();
        env.reset();
        std::filesystem::remove_all(db_path);
    }

    std::string db_path;
    std::unique_ptr<lmdbmap::environment> env;
    std::unique_ptr<lmdbmap::map<int, std::string>> map;
};

BENCHMARK_DEFINE_F(MapBenchmark, InsertSingleTxn)(benchmark::State& state) {
    int i = 0;
    for (auto _ : state) {
        lmdbmap::transaction txn(*env);
        map->insert(txn, i++, "value");
        txn.commit();
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(MapBenchmark, InsertSingleTxn)->Range(8, 8<<10);

BENCHMARK_DEFINE_F(MapBenchmark, InsertBatchTxn)(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        lmdbmap::transaction txn(*env);
        state.ResumeTiming();
        
        for (int i = 0; i < state.range(0); ++i) {
            map->put(txn, i, "value");
        }
        
        txn.commit();
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK_REGISTER_F(MapBenchmark, InsertBatchTxn)->Range(8, 8<<10);

BENCHMARK_DEFINE_F(MapBenchmark, GetSingleTxn)(benchmark::State& state) {
    // Pre-populate
    {
        lmdbmap::transaction txn(*env);
        for (int i = 0; i < state.range(0); ++i) {
            map->put(txn, i, "value");
        }
        txn.commit();
    }

    int i = 0;
    for (auto _ : state) {
        lmdbmap::transaction txn(*env, true);
        auto val = map->get(txn, i++ % state.range(0));
        benchmark::DoNotOptimize(val);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(MapBenchmark, GetSingleTxn)->Range(8, 8<<10);

BENCHMARK_DEFINE_F(MapBenchmark, GetBatchTxn)(benchmark::State& state) {
    // Pre-populate
    {
        lmdbmap::transaction txn(*env);
        for (int i = 0; i < state.range(0); ++i) {
            map->put(txn, i, "value");
        }
        txn.commit();
    }

    for (auto _ : state) {
        lmdbmap::transaction txn(*env, true);
        for (int i = 0; i < state.range(0); ++i) {
            auto val = map->get(txn, i);
            benchmark::DoNotOptimize(val);
        }
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK_REGISTER_F(MapBenchmark, GetBatchTxn)->Range(8, 8<<10);

BENCHMARK_MAIN();
