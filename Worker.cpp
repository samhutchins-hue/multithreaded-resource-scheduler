#include "Worker.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include "Random.hpp"

Worker::Worker(int id, WorkerType type, Storehouse &store,
               std::atomic<bool> &run_flag)
    : id(id), type(type), storehouse(store), running(run_flag) {
  last_meal_time =
      std::chrono::steady_clock::now(); // Initializes last_meal_time to current
                                        // time
}

void Worker::start() { thread_obj = std::thread(&Worker::life_cycle, this); }

void Worker::join() {
  if (thread_obj.joinable())
    thread_obj.join();
}

bool Worker::is_starving() const {
  // TODO: Lock status_mutex, check if current time - last_meal_time >
  // STARVATION_THRESHOLD and return status
  //       Hint: Refer to the constructor above to see how to obtain the current
  //       time
  std::unique_lock<std::mutex> lock(status_mutex);
  std::chrono::steady_clock::time_point current_time{
      std::chrono::steady_clock::now()};

  return (current_time - last_meal_time) > Config::STARVATION_THRESHOLD;
}

void Worker::life_cycle() {
  while (running) {
    // TODO: Sleep for a random duration between 200-500ms to simulate rest
    std::this_thread::sleep_for(
        std::chrono::milliseconds(Random::get(200, 500)));
    // TODO: Attempt to acquire resources based on worker type using the methods
    // in Storehouse.cpp

    std::pair<int, int> guardian_tools{-1, -1};
    int builder_tool = -1;
    bool acquired{};

    switch (type) {
    case WorkerType::GATHERER:
      acquired = storehouse.acquire_gatherer();
      break;
    case WorkerType::BUILDER:
      builder_tool = storehouse.acquire_builder();
      acquired = (builder_tool != -1);
      break;
    case WorkerType::GUARDIAN:
      guardian_tools = storehouse.acquire_guardian();
      acquired = (guardian_tools.first != -1 && guardian_tools.second != -1);
      break;
    }

    // TODO: If acquisition successful, update last_meal_time, perform work
    // (sleep for 50/150/300ms), record task completion, and release resources
    if (acquired) {

      std::unique_lock<std::mutex> lock(status_mutex);
      last_meal_time = std::chrono::steady_clock::now();
      lock.unlock();

      int work_duration = (type == WorkerType::GATHERER)  ? 50
                          : (type == WorkerType::BUILDER) ? 150
                                                          : 300;

      std::this_thread::sleep_for(std::chrono::milliseconds(work_duration));
      storehouse.record_task_completion();

      switch (type) {
      case WorkerType::GATHERER:
        storehouse.release_gatherer();
        break;
      case WorkerType::BUILDER:
        storehouse.release_builder(builder_tool);
        break;
      case WorkerType::GUARDIAN:
        storehouse.release_guardian(guardian_tools.first,
                                    guardian_tools.second);
        break;
      }
    }
  }
}
