#include <optional>
#include <csignal>
#include <exception>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <unistd.h>
#include <sys/resource.h>

#include <bits/stdc++.h>
#include <fcntl.h>
constexpr int TL_EXIT_CODE = 132;

void SignalHandler(int sig);

namespace elans {
    namespace runner {
        struct CantOpenExecutable : public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        class Runner {
        public:
            static constexpr int TL_EXIT_CODE = 132;

            enum class RunningResult {
                TL = 0,
                IE = 1,
                ML = 2,
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
                std::string input_stream_file;
                std::string output_stream_file;
            };

            struct TestingResult {
                RunningResult verdict;
                int exit_code;
                uint16_t threads;
                uint64_t cpu_time; // ms
                uint64_t real_time; // ms
                uint64_t memory; // bytes
                std::string output;
            };

            Runner(std::string path, Limits lims);

            ~Runner();

            TestingResult GetOutput();
            
        private:
            std::optional<TestingResult> res_;
            pid_t slave_pid_;
            uint32_t runner_number_;
            int out_fd_[2];

            void SetUpSlave(std::string path, Limits lims) const;

            void KillInSyscall(user_regs_struct &state);

            pid_t RunKiller(pid_t slave, uint64_t millis_limit);

            void PtraceProcess(Limits lims);

            static void Write(std::string path, std::string data);

            static std::string Read(std::string path);

            static void SigHandler(int);

            static uint64_t GetCPUTime(pid_t pid);

            void InitCgroups(Limits lims) const;

            uint64_t GetMaxMemoryCgroup() const;

            void DeinitCgroups() const;

            static uint16_t GetRunnerNumber();
        };
    } // namespace runner
} // namespace elans