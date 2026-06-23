#pragma once
#include <thread>
#include <atomic>
#include <mutex>
#include "Storehouse.hpp"

enum class WorkerType { GATHERER, BUILDER, GUARDIAN };

class Worker {
public:
    Worker(int id, WorkerType type, Storehouse& store, std::atomic<bool>& run_flag);
    
    void start();
    void join();
    bool is_starving() const;

private:
    void life_cycle();

    int id;
    WorkerType type;
    Storehouse& storehouse;
    std::atomic<bool>& running;
    std::thread thread_obj;
    
    std::chrono::steady_clock::time_point last_meal_time;
    mutable std::mutex status_mutex;
};