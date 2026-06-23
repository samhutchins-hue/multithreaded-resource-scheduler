#pragma once
#include <thread>
#include <vector>
#include <atomic>
#include <memory>
#include "Storehouse.hpp"
#include "Worker.hpp"

class Auditor {
public:
    Auditor(Storehouse& store, std::vector<std::unique_ptr<Worker>>& workers, std::atomic<bool>& run_flag);
    void start();
    void join();

private:
    void audit_loop();
    
    Storehouse& storehouse;
    std::vector<std::unique_ptr<Worker>>& workers;
    std::atomic<bool>& running;
    std::thread thread_obj;
};