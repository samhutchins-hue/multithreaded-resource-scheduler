#include "Auditor.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include <sstream>

Auditor::Auditor(Storehouse &store, std::vector<std::unique_ptr<Worker>> &w,
                 std::atomic<bool> &r)
    : storehouse(store), workers(w), running(r) {}

void Auditor::start() { thread_obj = std::thread(&Auditor::audit_loop, this); }

void Auditor::join() {
  if (thread_obj.joinable())
    thread_obj.join();
}

void Auditor::audit_loop() {
  while (running) {
    // TODO: Sleep for Config::AUDITOR_RATE, then check the running flag and
    // break if simulation has ended
    using namespace std::chrono;
    std::this_thread::sleep_for(milliseconds(Config::AUDITOR_RATE));
    if (!running)
      break;

    // TODO: Fetch stats from the Storehouse using get_statistics()
    Storehouse::Stats stats{storehouse.get_statistics()};

    // TODO: Compute the integrity check.
    //       The invariant is: (Config::INITIAL_ENERGY + total_produced) ==
    //       (total_consumed + current_energy) Build a log string using a
    //       stringstream in this format:
    //         "[AUDIT] Tasks: X | Energy: Y | Integrity: PASS"
    //	 Note that you should only add "PASS" if the integrity check is
    // sucessfull.
    //       If the check fails, also log these two lines immediately (before
    //       the main audit line):
    //         "FAIL (Exp: X vs Act: Y)"
    //         "CRITICAL FAILURE: INTEGRITY CHECK FAILED!"

    std::stringstream logStream{};

    const int expected_energy{Config::INITIAL_ENERGY + stats.total_produced};
    const int actual_energy{stats.total_consumed + stats.current_energy};
    const bool invariant_success{expected_energy == actual_energy};

    if (!invariant_success) {
      logStream << "FAIL (Exp: " << expected_energy
                << " vs Act: " << actual_energy << ")\n";
      logStream << "CRITICAL FAILURE: INTEGRITY CHECK FAILED!\n";
    }

    logStream << "[AUDIT] Tasks: " << stats.tasks_completed << " | "
              << "Energy: " << stats.current_energy << " | "
              << "Integrity: " << (invariant_success ? "PASS" : "FAIL");

    // TODO: Count how many workers are starving using w->is_starving().
    //       Append the count to the audit log string in this format:
    //         " | Starved: N"
    //       Then log the completed audit string with Logger::log()
    int starved_count{};
    for (const auto &worker : workers) {
      if (worker->is_starving())
        starved_count++;
    }
    logStream << " | Starved: " << starved_count << '\n';
    Logger::log(logStream.str());

    // TODO: If starved_count > Config::MAX_STARVED_WORKERS, log the following
    // line and set running = false:
    //         "FAIL - GAME OVER: TOO MANY STARVED WORKERS"
    if (starved_count > Config::MAX_STARVED_WORKERS) {
      Logger::log("FAIL - GAME OVER: TOO MANY STARVED WORKERS");
      running = false;
    }
  }
}
