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
                ML,
                OK,
                RE,
                SE
            };

            struct Limits {
                uint64_t threads;
                uint64_t memory; // kb
                uint64_t time;// ms
            };

            struct TestingResult {
                RunningResult res;
                std::string output;
            };

            Runner(const std::string &path, const std::string &input, Limits lims) {
                group_number_ = groups_amount_++;
                pipe(program_input_);
                pipe(program_output_);
                slave_pid_ = fork();
                if (slave_pid_) {
                    SetUpCgroups(lims.memory);
                    PtraceProcess(input, lims.time);
                } else {
                    SetUpSlave(path);
                }
            }

            TestingResult GetOutput() {
                return *res_;
            }
        private:
            int program_input_[2], program_output_[2];
            SharedMem<TestingResult> res_;
            pid_t slave_pid_;
            uint32_t group_number_;
            static uint32_t groups_amount_;

            void SetUpSlave(const std::string &path) {
                ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
                dup2(program_input_[0], STDIN_FILENO);
                dup2(program_output_[1], STDOUT_FILENO);
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

            void PtraceProcess(const std::string &input, uint64_t time_limit) {
                const auto begin_time = std::chrono::steady_clock::now();
                std::cout << time_limit / 1000 + (time_limit % 1000 > 0) << std::endl;
                const rlimit *lim = new rlimit(time_limit / 1000 + (time_limit % 1000 > 0), time_limit / 1000 + (time_limit % 1000 > 0));
                prlimit(slave_pid_, RLIMIT_CPU, lim, nullptr);
                write(program_input_[1], input.data(), input.size());
                close(program_input_[1]);
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
                            case __NR_clone:
                            case __NR_fork:
                                KillInSyscall(state);
                                res_.emplace(RunningResult::SE, "");
                                return;
                            case __NR_exit:
                                if (state.rdi == 137) {
                                    res_.emplace(RunningResult::ML, "");
                                } else if (state.rdi != 0) {
                                    if (!res_.has_value()) {
                                        res_.emplace(RunningResult::RE, "");
                                    }
                                } else {
                                    res_.emplace(RunningResult::OK, "");
                                }
                                return;
                        }
                        ptrace(PTRACE_SYSCALL, slave_pid_, 0, 0);
                        waitpid(slave_pid_, &status, 0);
                    }
                }
                const auto end_time = std::chrono::steady_clock::now();
                auto period = end_time - begin_time;
                if (std::chrono::duration_cast<std::chrono::milliseconds>(period) >= std::chrono::milliseconds(time_limit)) {
                    res_.emplace(RunningResult::TL, "");
                    return;
                }
                if (WIFSIGNALED(status)) {
                    std::cout << WTERMSIG(status) << std::endl;
                    res_.emplace(RunningResult::ML, "");
                }
                if (!res_.has_value()) {
                    std::string output(1024, 'a');
                    output.resize(read(program_output_[0], output.data(), 1024));
                    res_.emplace(RunningResult::OK, output);
                }
            }

            void Write(std::string path, std::string data) {
                int fd = open(path.data(), O_WRONLY);
                write(fd, data.data(), data.size());
                close(fd);
            }

            void SetUpCgroups(uint64_t memory_limit) {
                std::filesystem::create_directory("/sys/fs/cgroup/group" + std::to_string(group_number_));
                Write("/sys/fs/cgroup/group" + std::to_string(group_number_) + "/cgroup.procs", std::to_string(slave_pid_));
                Write("/sys/fs/cgroup/group" + std::to_string(group_number_) + "/memory.max", std::to_string(memory_limit * 1024));
            }
        };
        uint32_t Runner::groups_amount_ = 0;
    } // namespace runner
} // namespace elans