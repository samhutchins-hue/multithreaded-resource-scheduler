#pragma once
#include <iostream>
#include <mutex>
#include <string>

class Logger {
public:
    static void log(const std::string& msg, bool verbose_only = false) {
        if (verbose_only && !verbose_mode) return;

        std::lock_guard<std::mutex> lock(console_mutex);
        std::cout << msg << std::endl;
    }

    static void set_verbose(bool v) { verbose_mode = v; }

private:
    static inline std::mutex console_mutex;
    static inline bool verbose_mode = false;
};