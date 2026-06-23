#include "Storehouse.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include "Random.hpp"
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

Storehouse::Storehouse() {
  // TODO: Initialize hammers and saws based on Config counts
  for (int i{}; i < Config::HAMMER_COUNT; ++i) {
    hammers.push_back(std::make_unique<std::timed_mutex>());
  }

  for (int i{}; i < Config::SAW_COUNT; ++i) {
    saws.push_back(std::make_unique<std::timed_mutex>());
  }
}

void Storehouse::add_energy() {
  // TODO: lock stats_mutex, add energy, update total produced, unlock, and
  // notify_all
  std::unique_lock<std::shared_mutex> stats_lock(stats_mutex);
  current_energy += Config::AGENT_ENERGY_ADD;
  total_energy_produced += Config::AGENT_ENERGY_ADD;
  stats_lock.unlock();
  energy_cv.notify_all();
}

// ---------------------------------------------------------
// GATHERER Implementation
// ---------------------------------------------------------
bool Storehouse::acquire_gatherer() {
  // TODO: lock stats_mutex,
  // TODO: wait for energy and consume energy if available, unlock
  // TODO: return success status

  std::unique_lock<std::shared_mutex> stats_lock(stats_mutex);
  bool success = energy_cv.wait_for(stats_lock, std::chrono::milliseconds(100),
                                    [this] { return current_energy >= 1; });
  if (!success)
    return false;

  current_energy--;
  total_energy_consumed++;
  return true;
}

void Storehouse::release_gatherer() {
  // Gatherers only consume energy, no tools to release
}

// ---------------------------------------------------------
// BUILDER Implementation
// ---------------------------------------------------------
int Storehouse::acquire_builder() {
  // TODO: Step 1: Acquire Energy (2 units)
  //       lock stats_mutex, wait for energy, consume energy if available,
  //       unlock
  std::unique_lock<std::shared_mutex> stats_lock(stats_mutex);
  bool success = energy_cv.wait_for(stats_lock, std::chrono::milliseconds(100),
                                    [this] { return current_energy >= 2; });
  if (!success)
    return -1;

  const int consumed_energy{2};
  current_energy -= consumed_energy;
  total_energy_consumed += consumed_energy;

  stats_lock.unlock();

  // TODO: Step 2: Acquire Hammer
  // 	     Try to acquire any hammer using try_lock_for_any_tool, return
  // hammer ID on success,
  //       Upon failure, rollback energy and return -1

  const int acquired_id = try_lock_any_tool(hammers);

  if (acquired_id != -1) {
    return acquired_id;
  } else {
    stats_lock.lock();

    current_energy += consumed_energy;
    total_energy_consumed -= consumed_energy;

    energy_cv.notify_all();
    return -1;
  }
}

void Storehouse::release_builder(int hammer_id) {
  if (hammer_id >= 0 && hammer_id < (int)hammers.size()) {
    hammers[hammer_id]->unlock();
  }
}

// ---------------------------------------------------------
// GUARDIAN Implementation
// ---------------------------------------------------------
std::pair<int, int> Storehouse::acquire_guardian() {
  // TODO: Step 1: Acquire Energy (3 units)
  //       lock stats_mutex, wait for energy, consume energy if available,
  //       unlock
  std::unique_lock<std::shared_mutex> stats_lock(stats_mutex);
  bool success = energy_cv.wait_for(stats_lock, std::chrono::milliseconds(100),
                                    [this] { return current_energy >= 3; });
  if (!success)
    return {-1, -1};

  const int consumed_energy{3};
  current_energy -= consumed_energy;
  total_energy_consumed += consumed_energy;

  stats_lock.unlock();

  // TODO: Step 2: Try to acquire both a hammer and a saw within timeout
  using namespace std::chrono;
  auto const expire{steady_clock::now() + milliseconds(Config::TIMEOUT_MS)};

  while (steady_clock::now() < expire) {
    // TODO: Check for timeout condition and break if exceeded

    // TODO: Loop through hammers and try_lock_for,
    // TODO: if hammer acquired, loop through saws and try_lock_for
    for (int h = 0; h < (int)hammers.size(); ++h) {
      if (hammers[h]->try_lock_for(milliseconds(Config::TIMEOUT_MS))) {
        for (int s{0}; s < (int)saws.size(); ++s) {
          if (saws[s]->try_lock_for(milliseconds(Config::TIMEOUT_MS))) {
            return {h, s};
          }
        }
        hammers[h]->unlock();
      }
    }
    // Small backoff to allow other threads to progress
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }

  // TODO: Step 3: Rollback Energy on failure and notify all
  stats_lock.lock();
  current_energy += consumed_energy;
  total_energy_consumed -= consumed_energy;

  energy_cv.notify_all();

  return {-1, -1};
}

void Storehouse::release_guardian(int hammer_id, int saw_id) {
  if (hammer_id != -1)
    hammers[hammer_id]->unlock();
  if (saw_id != -1)
    saws[saw_id]->unlock();
}

// ---------------------------------------------------------
// Helper & Auditor Methods
// ---------------------------------------------------------
Storehouse::Stats Storehouse::get_statistics() {
  std::lock_guard<std::shared_mutex> lock(stats_mutex);
  return {current_energy, total_energy_produced, total_energy_consumed,
          total_tasks_completed};
}

void Storehouse::record_task_completion() { total_tasks_completed++; }

int Storehouse::try_lock_any_tool(
    std::vector<std::unique_ptr<std::timed_mutex>> &tools) {
  // TODO: Loop through tools and try_lock_for, return index on success, -1 on
  // failure
  for (int i{}; i < (int)tools.size(); ++i) {
    if (tools[i]->try_lock_for(std::chrono::milliseconds(Config::TIMEOUT_MS))) {
      return i;
    }
  }

  return -1;
}
