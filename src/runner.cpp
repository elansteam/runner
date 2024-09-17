#include "runner/runner.h"

runner::Runner::Runner(std::string path, runner::Runner::Params params)
    : params_(std::move(params)) {
    runner::mount::Mount(params_.working_directory);
    slave_pid_ = fork();
    MessageAssert(slave_pid_ != -1, "Can't create process");
    if (slave_pid_) {
        InitCgroups();
        ControlExecution();
    } else {
        SetupSlave(std::move(path));
    }
}

runner::Runner::~Runner() {
    DeinitCgroups();
    runner::mount::Umount(params_.working_directory);
}

void runner::Runner::SetupSlave(std::string path) {
    {
        int input = open(params_.input_stream_file.data(), O_RDONLY);
        int output = open(params_.output_stream_file.data(), O_WRONLY | O_TRUNC);
        dup2(input, STDIN_FILENO);
        dup2(output, STDOUT_FILENO);
        close(input);
        close(output);
    }
    MessageAssert(chdir(params_.working_directory.data()) != -1, "Chdir failed", false);
    MessageAssert(chroot(params_.working_directory.data()) != -1, "Chroot failed", false);

    MessageAssert(chmod(path.data(), S_IXGRP | S_IXUSR | S_IXOTH) != -1, "Failed to chmod", false);

    MessageAssert(setuid(params_.user) != -1, "Changing user failed", false);
    MessageAssert(getuid() == params_.user, "Changing user failed", false);

    std::vector<char*> args_ptrs(params_.args.size());
    std::transform(params_.args.begin(), params_.args.end(), args_ptrs.begin(), [] (std::string &str) {
        return str.data();
    });
    args_ptrs.push_back(nullptr);

    MessageAssert(execvp(path.data(), args_ptrs.data()) != -1, "Failed to execute", false);
}

pid_t runner::Runner::RunKillerByRealTime(uint64_t millis_limit) {
    pid_t killer_pid = fork();
    MessageAssert(killer_pid != -1, "Can't create a process");
    if (killer_pid) {
        return killer_pid;
    } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(millis_limit));
        kill(slave_pid_, SIGKILL);
        exit(EXIT_SUCCESS);
    }
}

void runner::Runner::ControlExecution() {
    auto beg_real_time = std::chrono::high_resolution_clock::now();
    pid_t real_time_killer_pid = RunKillerByRealTime(params_.lims.real_time_limit);
    pid_t cpu_time_killer_pid = RunKillerByCpuTime(params_.lims.cpu_time_limit);

    int status;
    MessageAssert(waitpid(slave_pid_, &status, 0) != -1, "Error while waiting for slave");

    auto end_real_time = std::chrono::high_resolution_clock::now();
    res_.cpu_time = GetCPUTimeMs();
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

void runner::Runner::Write(std::string path, std::string data) {
    int fd = open(path.data(), O_WRONLY | O_TRUNC);
    write(fd, data.data(), data.size());
    close(fd);
}

std::string runner::Runner::Read(std::string path) {
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

uint64_t runner::Runner::GetCPUTimeMs() {
    std::ifstream fin("/sys/fs/cgroup/group" + std::to_string(slave_pid_) + "/cpu.stat");
    uint64_t ans_usec;
    fin >> ans_usec;
    return ans_usec / 1000;
}

void runner::Runner::InitCgroups() const {
    std::filesystem::create_directory("/sys/fs/cgroup/group" + std::to_string(slave_pid_));
    Write("/sys/fs/cgroup/group" + std::to_string(slave_pid_) + "/cgroup.procs", std::to_string(slave_pid_));
    Write("/sys/fs/cgroup/group" + std::to_string(slave_pid_) + "/memory.max", std::to_string(params_.lims.memory * 1024));
    Write("/sys/fs/cgroup/group" + std::to_string(slave_pid_) + "/memory.low", std::to_string(params_.lims.memory * 1024 - 1));
    Write("/sys/fs/cgroup/group" + std::to_string(slave_pid_) + "/pids.max", std::to_string(params_.lims.threads));
}

uint64_t runner::Runner::GetMaxMemoryCgroup() const {
    std::ifstream fin("/sys/fs/cgroup/group" + std::to_string(slave_pid_) + "/memory.peak");
    uint64_t ans;
    fin >> ans;
    return ans / 1024;
}

void runner::Runner::DeinitCgroups() const {
    std::filesystem::remove("/sys/fs/cgroup/group" + std::to_string(slave_pid_));
}

pid_t runner::Runner::RunKillerByCpuTime(uint64_t millis_limit) {
    pid_t proc_pid = fork();
    MessageAssert(proc_pid != -1, "Cant create a process");
    if (proc_pid != 0) {
        return proc_pid;
    } else {
        while (GetCPUTimeMs() <= millis_limit) {}
        kill(slave_pid_, SIGKILL);
        exit(EXIT_SUCCESS);
    }
}

const runner::Runner::TestingResult &runner::Runner::GetOutput() const {
    return res_;
}
