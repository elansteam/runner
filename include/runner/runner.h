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

inline void _message_assert_func(bool cond, size_t line, std::string_view file, std::string_view mess);

#define message_assert(cond, mess) _message_assert_func((cond), __LINE__, __FILE__, (mess))

namespace elans {
    namespace runner {
        class Runner {
        public:
            enum class RunningResult {
                TL = 0,//
                IE = 1,//
                ML = 2,//
                OK = 3,
                RE = 4,
                SE = 5
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

            TestingResult GetOutput();
            
        private:
            TestingResult res_;
            pid_t slave_pid_;
            uint32_t runner_number_;

            [[noreturn]] static void SetUpSlave(std::string path, Params params);

            pid_t RunKillerByRealTime(uint64_t millis_limit);

            pid_t RunKillerByCpuTime(uint64_t millis_limit) {
                pid_t proc_pid = fork();
                message_assert(proc_pid != -1, "Cant create a process");
                if (proc_pid != 0) {
                    return proc_pid;
                } else {
                    while (GetCPUTime() <= millis_limit) {}
                    kill(slave_pid_, SIGKILL);
                    exit(EXIT_SUCCESS);
                }
            }

            void ControlExecution(elans::runner::Runner::Limits lims);

            static void Write(std::string path, std::string data);

            static std::string Read(std::string path);

            static void SigHandler(int);

            uint64_t GetCPUTime();

            void InitCgroups(Limits lims) const;

            uint64_t GetMaxMemoryCgroup() const;

            void DeinitCgroups() const;

            static uint16_t GetRunnerNumber();
        };
    } // namespace runner
} // namespace elans