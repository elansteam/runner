#pragma once

#include <cstdint>

struct Limitations {
    uint64_t threads;
    uint64_t memory; // kb
    uint64_t cpu_time_limit; // ms
    uint64_t real_time_limit; // ms
    bool allow_files_write;
    bool allow_files_read;
};

inline void _message_assert_func(bool cond, size_t line, std::string_view file, std::string_view mess) {
    if (!cond) {
        std::cerr << "Assertation failed at line: " << line << " of file: \"" << file << "\" with message: \"" << mess << "\"" << std::endl;
        if (errno != 0) {
            std::cerr << "Errno value: " << errno << std::endl;
        }
        exit(EXIT_FAILURE);
    }
}

#define message_assert(cond, mess) _message_assert_func((cond), __LINE__, __FILE__, (mess))
