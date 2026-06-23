#pragma once
#include <chrono>

// NOTE: Do NOT Change names of any of the variables in this file 
namespace Config {
    // Resources
    constexpr int INITIAL_ENERGY = 100;
    constexpr int HAMMER_COUNT = 5;
    constexpr int SAW_COUNT = 3;

    // Agent
    constexpr int AGENT_ENERGY_ADD = 20;
    constexpr auto AGENT_RATE = std::chrono::milliseconds(800);

    // Auditor
    constexpr auto AUDITOR_RATE = std::chrono::milliseconds(500);
    constexpr auto STARVATION_THRESHOLD = std::chrono::seconds(10);
    constexpr int MAX_STARVED_WORKERS = 5;

    // Worker Counts
    constexpr int GATHERER_COUNT = 10;
    constexpr int BUILDER_COUNT = 6;
    constexpr int GUARDIAN_COUNT = 4;

    // Worker Timings (ms)
    constexpr int TIMEOUT_MS = 500;
}