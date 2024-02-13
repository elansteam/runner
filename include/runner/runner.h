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

namespace elans {
    namespace runner {
        struct CantOpenExecutable : public std::runtime_error {
            using std::runtime_error::runtime_error;
        };
        template<typename T>
        class SharedMem {
        public:
            SharedMem()
                    : inialized_((bool*)mmap(nullptr, sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0))
                    , ptr_((T*)mmap(nullptr, sizeof(T), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0))
                    , cur_amount_of_objs_((int*)mmap(nullptr, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0))
            {
                *inialized_ = false;
                *cur_amount_of_objs_ = 1;
            }

            SharedMem(const SharedMem<T> &other)
                    : inialized_(other.inialized_)
                    , ptr_(other.ptr_)
                    , cur_amount_of_objs_(other.cur_amount_of_objs_)
            {
                ++*cur_amount_of_objs_;
            }

            ~SharedMem() {
                --*cur_amount_of_objs_;
                if (*cur_amount_of_objs_ == 0) {
                    munmap(inialized_, sizeof(bool));
                    munmap(ptr_, sizeof(T));
                    munmap(cur_amount_of_objs_, sizeof(int));
                }
            }

            SharedMem &operator=(const SharedMem &other) {
                if (&other == this) return *this;
                --*cur_amount_of_objs_;
                if (*cur_amount_of_objs_ == 0) {
                    munmap(inialized_, sizeof(bool));
                    munmap(ptr_, sizeof(T));
                    munmap(cur_amount_of_objs_, sizeof(int));
                }
                inialized_ = other.inialized_;
                ptr_ = other.ptr_;
                cur_amount_of_objs_ = other.cur_amount_of_objs_;
                ++*cur_amount_of_objs_;
                return *this;
            }

            T &operator*() {
                if (!*inialized_) {
                    throw std::runtime_error("operator* for nullptr");
                }
                return *ptr_;
            }

            const T &operator*() const {
                if (!*inialized_) {
                    throw std::runtime_error("operator* for nullptr");
                }
                return *ptr_;
            }

            T *operator->() {
                if (!*inialized_) {
                    throw std::runtime_error("operator-> for nullptr");
                }
                return ptr_;
            }

            const T *operator->() const {
                if (!*inialized_) {
                    throw std::runtime_error("operator-> for nullptr");
                }
                return ptr_;
            }

            template<typename... ArgsT>
            void emplace(ArgsT... args) {
                if (*inialized_) {
                    ptr_->~T();
                }
                new (ptr_) T(args...);
                *inialized_ = true;
            }

            bool has_value() const {
                return *inialized_;
            }
        private:
            bool *inialized_;
            T *ptr_;
            int *cur_amount_of_objs_;
        };

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
                uint64_t tl_cpu_time; // ms
                uint64_t tl_real_time; // ms
                bool files; // can user write to of read from files
            };

            struct TestingResult {
                RunningResult res;
                std::string output_path;
            };

            Runner(const std::string &path, const std::string &input, Limits lims) {
                runner_number_ = GetRunnerNumber();

                std::filesystem::create_directory("/tmp/runner");
                std::string input_path = "/tmp/runner/input" + std::to_string(runner_number_);
                program_input_ = open(input_path.data(), O_RDWR | O_CREAT);
                assert(program_input_ != -1);
                std::string output_path = "/tmp/runner/output" + std::to_string(runner_number_);
                program_output_ = open(output_path.data(), O_RDWR | O_CREAT);

                slave_pid_ = fork();
                if (slave_pid_) {
                    InitCgroups(lims.memory);
                    PtraceProcess(input, lims);
                    DeinitCgroups();
                } else {
                    SetUpSlave(path);
                }
            }

            TestingResult GetOutput() {
                return *res_;
            }
            
        private:
            int program_input_, program_output_;
            SharedMem<TestingResult> res_;
            pid_t slave_pid_;
            uint32_t runner_number_;

            void SetUpSlave(const std::string &path) {
                ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
                dup2(program_input_, STDIN_FILENO);
                dup2(program_output_, STDOUT_FILENO);
                close(program_input_);
                close(program_output_);
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
                    timespec ts(0, millis_limit * 100'000);
                    nanosleep(&ts, &ts);
                    kill(slave, SIGKILL);
                    exit(EXIT_SUCCESS);
                }
            }

            void PtraceProcess(const std::string &input, Limits lims) {
                write(program_input_, input.data(), input.size());

                rlimit lim(lims.tl_cpu_time / 1000 + 1, lims.tl_cpu_time / 1000 + 1);
                prlimit(slave_pid_, RLIMIT_CPU, &lim, nullptr);
                pid_t killer_pid = RunKiller(slave_pid_, lims.tl_real_time);


                clockid_t slave_clock_id;
                clock_getcpuclockid(slave_pid_, &slave_clock_id);
                timespec beg_cpu_time, end_cpu_time;
                clock_gettime(slave_clock_id, &beg_cpu_time);

                // const auto begin_time = std::chrono::high_resolution_clock::now();


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
                                KillInSyscall(state);
                                res_.emplace(RunningResult::SE, "/tmp/runner/output" + std::to_string(runner_number_));
                                return;
                            // files
                            case __NR_open:
                            case __NR_mkdir:
                            case __NR_rmdir:
                            case __NR_creat:
                            case __NR_link:
                            case __NR_symlink:
                            case __NR_chmod:
                            case __NR_chown:
                            case __NR_lchown:
                            case __NR_umask:
                                if (lims.files) {
                                    KillInSyscall(state);
                                    res_.emplace(RunningResult::SE, "/tmp/runner/output" + std::to_string(runner_number_));
                                    return;
                                }
                                break;
                            case __NR_exit:
                            case __NR_exit_group:
                                if (state.rdi != 0 && state.rdi != 137) {
                                    res_.emplace(RunningResult::RE, "/tmp/runner/output" + std::to_string(runner_number_));
                                    return;
                                }
                                clock_gettime(slave_clock_id, &end_cpu_time);
                                break;
                        }
                        ptrace(PTRACE_SYSCALL, slave_pid_, 0, 0);
                        waitpid(slave_pid_, &status, 0);
                    }
                }

                // const auto end_time = std::chrono::high_resolution_clock::now();
                // auto period = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - begin_time);
                // std::cout << "Execution time: " << period << std::endl;
                // auto time_limit_millis = std::chrono::milliseconds(lims.time);
                if (kill(killer_pid, 0)) {
                    kill(killer_pid, SIGKILL);
                } else {
                    res_.emplace(RunningResult::IE, "/tmp/runner/output" + std::to_string(runner_number_));
                    return;
                }

                std::cout << "CPU exec time: " << (end_cpu_time.tv_nsec - beg_cpu_time.tv_nsec) / 1e5 << "ms" << std::endl;
                if ((end_cpu_time.tv_nsec - beg_cpu_time.tv_nsec) / 1e5 > lims.tl_cpu_time) {
                    res_.emplace(RunningResult::TL, "/tmp/runner/output" + std::to_string(runner_number_));
                    return;
                }

                if (WIFSIGNALED(status)) {
                    res_.emplace(RunningResult::ML, "/tmp/runner/output" + std::to_string(runner_number_));
                }

                if (!res_.has_value()) {
                    res_.emplace(RunningResult::OK, "/tmp/runner/output" + std::to_string(runner_number_));
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

            void InitCgroups(uint64_t memory_limit) const {
                std::filesystem::create_directory("/sys/fs/cgroup/group" + std::to_string(runner_number_));
                Write("/sys/fs/cgroup/group" + std::to_string(runner_number_) + "/cgroup.procs", std::to_string(slave_pid_));
                Write("/sys/fs/cgroup/group" + std::to_string(runner_number_) + "/memory.max", std::to_string(memory_limit * 1024));
            }

            void DeinitCgroups() const {
                while (!std::filesystem::remove("/sys/fs/cgroup/group" + std::to_string(runner_number_))) {}
            }

            uint16_t GetRunnerNumber() const {
                static const std::string path_to_runner_num = "/tmp/runner/runner_num";
                std::fstream finout(path_to_runner_num, std::ios::in | std::ios::out | std::ios::trunc);
                uint16_t ans;
                finout >> ans;
                finout << ans + 1;
                return ans;
            }
        };
    } // namespace runner
} // namespace elans