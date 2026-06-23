#pragma once
#include <random>
#include <mutex>

class Random {
public:
    static void seed(int s) {
        std::lock_guard<std::mutex> lock(rng_mutex);
        engine.seed(s);
    }

    static int get(int min, int max) {
        std::lock_guard<std::mutex> lock(rng_mutex);
        std::uniform_int_distribution<int> dist(min, max);
        return dist(engine);
    }

private:
    static inline std::mt19937 engine;
    static inline std::mutex rng_mutex;
};