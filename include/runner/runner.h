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

#include "cgroup_manager.h"
#include "domain.h"
#include "mount_manager.h"
#include "seccomp.h"

namespace elans {
    namespace runner {
        class Runner {
        public:
            enum class RunningResult {
                TL = 0,
                IE = 1,
                ML = 2,
                OK = 3,
                RE = 4,
                SE = 5
            };

            struct Params {
                std::string input_stream_file;
                std::string output_stream_file;
                std::vector<std::string> args;
                Limitations lims;
                uid_t user;
                std::string working_directory;
            };

            struct TestingResult {
                RunningResult verdict;
                uint64_t cpu_time; // ms
                uint64_t real_time; // ms
                uint64_t memory; // kb
                int exit_code;
            };

            Runner(std::string path, Params params);

            ~Runner();

            TestingResult GetOutput() const;

        private:
            TestingResult res_;
            pid_t slave_pid_;
            uint32_t runner_number_;
            Params params_;
            std::unique_ptr<CgroupManager> cgroup_manager_;

            [[noreturn]] void SetUpSlave(std::string path);

            pid_t RunKillerByRealTime(uint64_t millis_limit);

            pid_t RunKillerByCpuTime(uint64_t millis_limit);

            void ControlExecution();

            static uint16_t GetRunnerNumber();

            void InitMount();

            void DeinitMount();
        };
    } // namespace runner
} // namespace elans