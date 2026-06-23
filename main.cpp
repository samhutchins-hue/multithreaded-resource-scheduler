#include "Auditor.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include "Random.hpp"
#include "Storehouse.hpp"
#include "Worker.hpp"
#include <atomic>
#include <csignal>
#include <cstring>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

std::atomic<bool> simulation_running{true};

void handle_sigint(int sig) { simulation_running = false; }

void handle_stop(int) { simulation_running = false; }

int main(int argc, char *argv[]) {
  std::signal(SIGINT, handle_sigint);
  std::signal(SIGTERM, handle_stop);
  // Default Settings
  int seed = 42;
  int duration = 10;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
      seed = std::atoi(argv[++i]);
    } else if (strcmp(argv[i], "--duration") == 0 && i + 1 < argc) {
      duration = std::atoi(argv[++i]);
    } else if (strcmp(argv[i], "--verbose") == 0) {
      Logger::set_verbose(true);
    }
  }

  Random::seed(seed);
  Logger::log("Simulation Starting... Seed: " + std::to_string(seed));

  Storehouse storehouse;

  // 1. Spawn Agent
  auto agent_task = [&storehouse]() {
    while (simulation_running) {
      std::this_thread::sleep_for(Config::AGENT_RATE);
      storehouse.add_energy();
    }
  };

  std::thread agent_thread(agent_task);

  // 2. Spawn Workers
  std::vector<std::unique_ptr<Worker>> workers;
  for (int i = 0; i < Config::GATHERER_COUNT; ++i)
    workers.push_back(std::make_unique<Worker>(i, WorkerType::GATHERER,
                                               storehouse, simulation_running));
  for (int i = 0; i < Config::BUILDER_COUNT; ++i)
    workers.push_back(std::make_unique<Worker>(i, WorkerType::BUILDER,
                                               storehouse, simulation_running));
  for (int i = 0; i < Config::GUARDIAN_COUNT; ++i)
    workers.push_back(std::make_unique<Worker>(i, WorkerType::GUARDIAN,
                                               storehouse, simulation_running));

  for (auto &w : workers)
    w->start();

  // 3. Spawn Auditor
  Auditor auditor(storehouse, workers, simulation_running);
  auditor.start();

  // Run Simulation
  Logger::log("All threads running. Waiting " + std::to_string(duration) +
              "s...");

  int elapsed_ms = 0;
  while (simulation_running && elapsed_ms < duration * 1000) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    elapsed_ms += 100;
  }

  // Shutdown Sequence
  Logger::log("Time is up! Stopping simulation...");
  simulation_running = false;

  // Join all threads
  agent_thread.join();
  auditor.join();
  for (auto &w : workers)
    w->join();

  Logger::log("Simulation Cleanly Exited.");

  return 0;
}
