#include <optional>
#include <csignal>
#include <exception>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <unistd.h>
#include <bits/stdc++.h>

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

        class SafeRunner {
            void set_up_slave(const std::string &path) {
                ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
                dup2(program_input[0], STDIN_FILENO);
                dup2(program_output[1], STDOUT_FILENO);
                execl(path.data(), "", nullptr);
                throw CantOpenExecutable(path);
            }

            void kill_in_syscall(user_regs_struct &state) {
                state.orig_rax = __NR_kill;
                state.rdi = slave_pid;
                state.rsi = SIGKILL;
                ptrace(PTRACE_SETREGS, slave_pid, 0, &state);
                ptrace(PTRACE_CONT, slave_pid, 0, 0);
                waitpid(slave_pid, nullptr, 0);
            }

            void ptrace_process(const std::string &input) {
                write(program_input[1], input.data(), input.size());
                close(program_input[1]);
                int status;
                waitpid(slave_pid, &status, 0);
                while (!WIFEXITED(status)) {
                    user_regs_struct state;
                    ptrace(PTRACE_SYSCALL, slave_pid, 0, 0);
                    waitpid(slave_pid, &status, 0);
                    switch (state.orig_rax) {
                        case __NR_fork:
                            kill_in_syscall(state);
                            res.emplace(RunningResult::SE, "");
                            break;
                        case __NR_exit:
                            if (state.rdi != 0) {
                                res.emplace(RunningResult::RE, "");
                            } else {
                                res.emplace(RunningResult::OK, "");
                            }
                    }
                }
                if (!res.has_value()) {
                    std::string output(1024, 'a');
                    output.resize(read(program_output[0], output.data(), 1024));
                    res.emplace(RunningResult::OK, output);
                }
                *slave_ended = true;
            }
        public:
            SafeRunner(const std::string &path, const std::string &input) {
                slave_ended.emplace(false);
                pipe(program_input);
                pipe(program_output);
                slave_pid = fork();
                if (slave_pid) {
                    ptrace_process(input);
                } else {
                    set_up_slave(path);
                }
            }
            enum class RunningResult {
                TL,
                ML,
                OK,
                RE,
                SE
            };

            struct TestingResult {
                RunningResult res;
                std::string output;
            };
            TestingResult GetOutput() {
                return *res;
            }
            bool IsEnded() const {
                return *slave_ended;
            }
        private:
            int program_input[2], program_output[2];
            SharedMem<TestingResult> res;
            SharedMem<bool> slave_ended;
            pid_t slave_pid;
        };
    } // namespace runner
} // namespace elans