#include "runner/runner.h"

#define LOG() do { std::cerr << __LINE__ << std::endl; } while (false)

elans::runner::Runner::Runner(std::string path, elans::runner::Runner::Params params)
    : params_(params)
{
    runner_number_ = GetRunnerNumber();

    slave_pid_ = fork();
    message_assert(slave_pid_ != -1, "Can't create process");
    if (slave_pid_) {
        InitCgroups();
        InitMount();
        ControlExecution();
    } else {
        SetUpSlave(path);
    }
}

elans::runner::Runner::~Runner() {
    DeinitCgroups();
    DeinitMount();
}

void elans::runner::Runner::SetUpSlave(std::string path) {
    {
        int input = open(params_.input_stream_file.data(), O_RDONLY);
        int output = open(params_.output_stream_file.data(), O_WRONLY | O_TRUNC);
        dup2(input, STDIN_FILENO);
        dup2(output, STDOUT_FILENO);
        close(input);
        close(output);
    }
    message_assert(chdir(params_.working_directory.data()) != -1, "Chdir failed");
    message_assert(chroot(params_.working_directory.data()) != -1, "Chroot failed");

    EPERM;
    message_assert(chmod(path.data(), S_IXGRP | S_IXUSR | S_IXOTH) != -1, "Failed to chmod");
    message_assert(setuid(params_.user) != -1, "Changing user failed");
    message_assert(getuid() == params_.user, "Changing user failed");

    std::vector<char*> args_ptrs(params_.args.size());
    std::transform(params_.args.begin(), params_.args.end(), args_ptrs.begin(), [] (std::string &str) {
        return str.data();
    });

    EACCES;
    message_assert(execv(path.data(), args_ptrs.data()) != -1, "Failed to execute");
}

pid_t elans::runner::Runner::RunKillerByRealTime(uint64_t millis_limit) {
    pid_t killer_pid = fork();
    message_assert(killer_pid != -1, "Can't create a process");
    if (killer_pid) {
        return killer_pid;
    } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(millis_limit));
        kill(slave_pid_, SIGKILL);
        exit(EXIT_SUCCESS);
    }
}

void elans::runner::Runner::ControlExecution() {
    auto beg_real_time = std::chrono::high_resolution_clock::now();
    pid_t real_time_killer_pid = RunKillerByRealTime(params_.lims.real_time_limit);
    pid_t cpu_time_killer_pid = RunKillerByCpuTime(params_.lims.cpu_time_limit);

    int status;
    message_assert(waitpid(slave_pid_, &status, 0) != -1, "Error while waiting for slave");

    res_.cpu_time = GetCPUTime();
    res_.real_time = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - beg_real_time).count();
    res_.memory = GetMaxMemoryCgroup();

    if (kill(real_time_killer_pid, SIGKILL) != 0) {
        kill(cpu_time_killer_pid, SIGKILL);
        res_.verdict = RunningResult::IE;
    } else if (kill(cpu_time_killer_pid, SIGKILL) != 0) {
        res_.verdict = RunningResult::TL;
    } else if (GetMaxMemoryCgroup() >= params_.lims.memory) {
        res_.verdict = RunningResult::ML;
    } else if (WEXITSTATUS(status) != 0) {
        res_.verdict = RunningResult::RE;
    } else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSYS) {
        res_.verdict = RunningResult::SE;
    } else {
        res_.verdict = RunningResult::OK;
    }
}

void elans::runner::Runner::Write(std::string path, std::string data) {
    int fd = open(path.data(), O_WRONLY | O_TRUNC);
    write(fd, data.data(), data.size());
    close(fd);
}

std::string elans::runner::Runner::Read(std::string path) {
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

uint64_t elans::runner::Runner::GetCPUTime() {
    std::ifstream fin("/sys/fs/cgroup/group" + std::to_string(runner_number_) + "/cgroup.procs");
    uint64_t ans_usec;
    fin >> ans_usec;
    return ans_usec / 1000;
}

void elans::runner::Runner::InitCgroups() const {
    std::filesystem::create_directory("/sys/fs/cgroup/group" + std::to_string(runner_number_));
    Write("/sys/fs/cgroup/group" + std::to_string(runner_number_) + "/cgroup.procs", std::to_string(slave_pid_));
    Write("/sys/fs/cgroup/group" + std::to_string(runner_number_) + "/memory.max", std::to_string(params_.lims.memory * 1024));
    Write("/sys/fs/cgroup/group" + std::to_string(runner_number_) + "/memory.low", std::to_string(params_.lims.memory * 1024 - 1));
    Write("/sys/fs/cgroup/group" + std::to_string(runner_number_) + "/pids.max", std::to_string(params_.lims.threads));
}

uint64_t elans::runner::Runner::GetMaxMemoryCgroup() const {
    std::ifstream fin("/sys/fs/cgroup/group" + std::to_string(runner_number_) + "/memory.peak");
    uint64_t ans;
    fin >> ans;
    return ans / 1024;
}

void elans::runner::Runner::DeinitCgroups() const {
    std::filesystem::remove("/sys/fs/cgroup/group" + std::to_string(runner_number_));
}

uint16_t elans::runner::Runner::GetRunnerNumber() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return gen();
}

pid_t elans::runner::Runner::RunKillerByCpuTime(uint64_t millis_limit) {
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

void elans::runner::Runner::InitMount() {
    std::filesystem::create_directories(params_.working_directory + "/usr");
    std::string path_to_new_usr = params_.working_directory + "/usr";
    ENODEV;
    message_assert(mount("/usr", path_to_new_usr.data(), "ext4", MS_BIND, nullptr) == 0, "Failed to mount");
}

void elans::runner::Runner::DeinitMount() {
    std::string path_to_new_usr = params_.working_directory + "/usr";
    message_assert(umount(path_to_new_usr.data()) != -1, "Failed to umount");
}

void _message_assert_func(bool cond, size_t line, std::string_view file, std::string_view mess) {
    if (!cond) {
        std::cerr << "Assertation failed at line: " << line << " of file: \"" << file << "\" with message: \"" << mess << "\"" << std::endl;
        if (errno != 0) {
            std::cerr << "Errno value: " << errno << std::endl;
        }
        exit(EXIT_FAILURE);
    }
}
