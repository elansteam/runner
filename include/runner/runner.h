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

void SignalHandler(int sig) {
    assert(sig == SIGXCPU);
    exit(TL_EXIT_CODE);
}

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
            };

            Runner(std::string path, Limits lims) {
                runner_number_ = GetRunnerNumber();

                slave_pid_ = fork();
                if (slave_pid_) {
                    InitCgroups(lims.memory);
                    PtraceProcess(lims);
                } else {
                    SetUpSlave(path, lims);
                }
            }

            ~Runner() {
                DeinitCgroups();
            }

            TestingResult GetOutput() {
                return *res_;
            }
            
        private:
            std::optional<TestingResult> res_;
            pid_t slave_pid_;
            uint32_t runner_number_;

            void SetUpSlave(const std::string &path, Limits lims) {
                int output = open(lims.output_stream_file.data(), O_RDWR);
                int input = open(lims.output_stream_file.data(), O_RDWR);
                dup2(input, STDIN_FILENO);
                dup2(output, STDOUT_FILENO);
                close(input);
                close(output);
                signal(SIGXCPU, SigHandler);

                ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
                execl(path.data(), "", nullptr);
                throw CantOpenExecutable(path);
            }

            void KillInSyscall(user_regs_struct &state) {
                state.orig_rax = __NR_kill;
                state.rdi = slave_pid_;
                state.rsi = SIGKILL;
                ptrace(PTRACE_SETREGS, slave_pid_, 0, &state);
                ptrace(PTRACE_CONT, slave_pid_, 0, 0);
            }

            pid_t RunKiller(pid_t slave, uint64_t millis_limit) {
                pid_t killer_pid = fork();
                if (killer_pid) {
                    return killer_pid;
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(millis_limit));
                    kill(slave, SIGKILL);
                    exit(EXIT_SUCCESS);
                }
            }

            void PtraceProcess(Limits lims) {
                rlimit lim(lims.cpu_time_limit / 1000 + 1, RLIM_INFINITY);

                pid_t killer_pid = RunKiller(slave_pid_, lims.real_time_limit);
                prlimit(slave_pid_, RLIMIT_CPU, &lim, nullptr);
                uint64_t cpu_time_ejudge_beg = GetCPUTime(slave_pid_);
                auto beg_real_time = std::chrono::high_resolution_clock::now();
                uint64_t cpu_time_ejudge_end;

                int status;
                waitpid(slave_pid_, &status, 0);
                ptrace(PTRACE_SETOPTIONS, slave_pid_, 0, PTRACE_O_TRACESYSGOOD);
                while (!WIFEXITED(status) && !WIFSIGNALED(status)) {
                    user_regs_struct state{};
                    ptrace(PTRACE_SYSCALL, slave_pid_, 0, 0);
                    waitpid(slave_pid_, &status, 0);
                    if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80) {
                        ptrace(PTRACE_GETREGS, slave_pid_, 0, &state);
                        switch (state.orig_rax) {
                            // rasstrel
                            case __NR_clone:
                            case __NR_fork:
                            case __NR_execve:
                            case __NR_ptrace:
                            case __NR_setgid:
                            case __NR_setgroups:
                            case __NR_signalfd:
                            case __NR_sigaltstack:
                                kill(killer_pid, SIGKILL);
                                KillInSyscall(state);
                                res_ = TestingResult{   .verdict = RunningResult::SE,
                                                        .exit_code =  0,
                                                        .threads = 1,
                                                        .cpu_time = GetCPUTime(slave_pid_) - cpu_time_ejudge_beg,
                                                        .real_time = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - beg_real_time).count(),
                                                        .memory = GetMaxMemoryCgroup()
                                                    };
                                return;
                            case __NR_mkdir:
                            case __NR_rmdir:
                            case __NR_creat:
                            case __NR_link:
                            case __NR_symlink:
                            case __NR_chmod:
                            case __NR_chown:
                            case __NR_lchown:
                            case __NR_umask:
                                if (!lims.allow_files_write) {
                                    kill(killer_pid, SIGKILL);
                                    KillInSyscall(state);
                                    res_ = TestingResult{   .verdict = RunningResult::SE,
                                                            .exit_code =  0,
                                                            .threads = 1,
                                                            .cpu_time = GetCPUTime(slave_pid_) - cpu_time_ejudge_beg,
                                                            .real_time = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - beg_real_time).count(),
                                                            .memory = GetMaxMemoryCgroup()
                                                        };
                                    return;
                                }
                                break;
                            case __NR_exit:
                            case __NR_exit_group:
                                cpu_time_ejudge_end = GetCPUTime(slave_pid_);
                                if (state.rdi != 0 && state.rdi != 137) {
                                    kill(killer_pid, SIGKILL);
                                    res_ = TestingResult{   .verdict = RunningResult::RE,
                                                            .exit_code =  (int)state.rdi,
                                                            .threads = 1,
                                                            .cpu_time = cpu_time_ejudge_end - cpu_time_ejudge_beg,
                                                            .real_time = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - beg_real_time).count(),
                                                            .memory = GetMaxMemoryCgroup()
                                                        };
                                    return;
                                } else if (state.rdi == 137) {
                                    throw std::runtime_error("IT CAN HAPPEN!!!");
                                }
                                break;
                        }
                        ptrace(PTRACE_SYSCALL, slave_pid_, 0, 0);
                        waitpid(slave_pid_, &status, 0);
                    }
                }

                if (kill(killer_pid, 0) == 0) {
                    kill(killer_pid, SIGKILL);
                } else {
                    res_ = TestingResult{   .verdict = RunningResult::IE,
                                            .exit_code =  0,
                                            .threads = 1,
                                            .cpu_time = cpu_time_ejudge_end - cpu_time_ejudge_beg,
                                            .real_time = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - beg_real_time).count(),
                                            .memory = GetMaxMemoryCgroup()
                                        };
                    return;
                }

                if (GetMaxMemoryCgroup() >= lims.memory) {
                    res_ = TestingResult{   .verdict = RunningResult::ML,
                                            .exit_code =  0,
                                            .threads = 1,
                                            .cpu_time = (uint64_t)-1,
                                            .real_time = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - beg_real_time).count(),
                                            .memory = GetMaxMemoryCgroup()
                                        };
                    return;
                }

                if (cpu_time_ejudge_end - cpu_time_ejudge_beg > lims.cpu_time_limit) {
                    res_ = TestingResult{   .verdict = RunningResult::TL,
                                            .exit_code =  0,
                                            .threads = 1,
                                            .cpu_time = cpu_time_ejudge_end - cpu_time_ejudge_beg,
                                            .real_time = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - beg_real_time).count(),
                                            .memory = GetMaxMemoryCgroup()
                                        };
                    return;
                }

                if (!res_.has_value()) {
                    res_ = TestingResult{   .verdict = RunningResult::OK,
                                            .exit_code =  0,
                                            .threads = 1,
                                            .cpu_time = cpu_time_ejudge_end - cpu_time_ejudge_beg,
                                            .real_time = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - beg_real_time).count(),
                                            .memory = GetMaxMemoryCgroup()
                                        };
                }
            }

            static void Write(std::string path, std::string data) {
                int fd = open(path.data(), O_WRONLY | O_TRUNC);
                write(fd, data.data(), data.size());
                close(fd);
            }

            static std::string Read(std::string path) {
                int fd = open(path.data(), O_RDONLY);
                std::string s;
                std::string buf(1024, 0);
                while (true) {
                    size_t read_amount = read(fd, buf.data(), 1024);
                    if (read_amount == 1024) {
                        s += buf;
                    } else {
                        buf.resize(read_amount);
                        s += buf;
                        break;
                    }
                }
                close(fd);
                return s;
            }

            static void SigHandler(int) {
                exit(52);
            }

            static uint64_t GetCPUTime(pid_t pid) {
                std::ifstream fin("/proc/" + std::to_string(pid) + "/stat");
                for (int i = 0; i < 13; ++i) {
                    std::string trash;
                    fin >> trash;
                }
                uint64_t utime, stime;
                fin >> utime >> stime;
                return (utime + stime) * 1'000 / sysconf(_SC_CLK_TCK);
            }

            void InitCgroups(uint64_t memory_limit) const {
                std::filesystem::create_directory("/sys/fs/cgroup/group" + std::to_string(runner_number_));
                Write("/sys/fs/cgroup/group" + std::to_string(runner_number_) + "/cgroup.procs", std::to_string(slave_pid_));
                Write("/sys/fs/cgroup/group" + std::to_string(runner_number_) + "/memory.max", std::to_string(memory_limit * 1024));
            }

            uint64_t GetMaxMemoryCgroup() const {
                std::ifstream fin("/sys/fs/cgroup/group" + std::to_string(runner_number_) + "/memory.peak");
                uint64_t ans;
                fin >> ans;
                return ans / 1024;
            }

            void DeinitCgroups() const {
                assert(std::filesystem::remove("/sys/fs/cgroup/group" + std::to_string(runner_number_)));
            }

            uint16_t GetRunnerNumber() const {
                static std::random_device rd;
                static std::mt19937 gen(rd());
                return gen();
            }
        };
    } // namespace runner
} // namespace elans