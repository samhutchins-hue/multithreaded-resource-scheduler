#pragma once
#include "Config.hpp"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <vector>

class Storehouse {
public:
  Storehouse();

  // -- Agent Interface --
  void add_energy();

  // -- Worker Interface --
  // Returns true on success, false on timeout
  bool acquire_gatherer();
  void release_gatherer();

  // Returns Hammer ID (0-N) on success, -1 on failure
  int acquire_builder();
  void release_builder(int hammer_id);

  // Returns {Hammer ID, Saw ID} on success, {-1, -1} on failure
  std::pair<int, int> acquire_guardian();
  void release_guardian(int hammer_id, int saw_id);

  // -- Auditor Interface --
  struct Stats {
    int current_energy;
    int total_produced;
    int total_consumed;
    int tasks_completed;
  };
  Stats get_statistics();
  void record_task_completion();

private:
  // Synchronization Primitives
  // Standard mutex/cv are more stable with ThreadSanitizer than shared_mutex
  std::shared_mutex stats_mutex; // protects energy fields
  std::condition_variable_any
      energy_cv; // used to wait for/ signal energy availability

  // Internal State
  int current_energy = 100;
  int total_energy_produced = 0;
  int total_energy_consumed = 0;
  std::atomic<int> total_tasks_completed = 0;

  // Tool Pools (Using timed_mutex to support try_lock_for)
  std::vector<std::unique_ptr<std::timed_mutex>> hammers; // one per hammer
  std::vector<std::unique_ptr<std::timed_mutex>> saws;    // one per saw

  // Helper for tool acquisition logic
  int try_lock_any_tool(std::vector<std::unique_ptr<std::timed_mutex>> &tools);
};
