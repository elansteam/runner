#include <optional>
#include <csignal>
#include <exception>
#include <cstdint>
#include <chrono>
#include <thread>
#include <ranges>
#include <filesystem>
#include <fstream>
#include <random>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/resource.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include "mount.h"

namespace runner {
    class Runner {
    public:
        enum class RunningResult {
            TL,
            IE,
            ML,
            OK,
            RE,
            SE
        };

        struct Limits {
            uint64_t threads;
            uint64_t memory; // kb
            uint64_t cpu_time_limit; // ms
            uint64_t real_time_limit; // ms
            bool allow_files_write;
            bool allow_files_read;
        };

        struct Params {
            std::string input_stream_file;
            std::string output_stream_file;
            std::vector<std::string> args;
            Limits lims;
            uid_t user;
            std::string working_directory;
        };

        struct TestingResult {
            RunningResult verdict;
            uint64_t cpu_time; // ms
            uint64_t real_time; // ms
            uint64_t memory; // kb
        };

        Runner(std::string path, Params params);

        ~Runner();

        const TestingResult &GetOutput() const;

    private:
        TestingResult res_;
        pid_t slave_pid_;
        Params params_;

        void SetupSlave(std::string path);

        pid_t RunKillerByRealTime(uint64_t millis_limit);

        pid_t RunKillerByCpuTime(uint64_t millis_limit);

        void ControlExecution();

        static void Write(std::string path, std::string data);

        static std::string Read(std::string path);

        uint64_t GetCPUTimeMs();

        void InitCgroups() const;

        uint64_t GetMaxMemoryCgroup() const;

        void DeinitCgroups() const;
    };
} // namespace runner
