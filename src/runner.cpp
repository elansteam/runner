#include "runner/runner.h"

elans::runner::Runner::Runner(std::string path, Params params)
    : params_(params)
    , slave_pid_(fork())
{
    message_assert(slave_pid_ != -1, "Can't create process");
    if (slave_pid_) {
        cgroup_manager_ = std::make_unique<CgroupManager>(slave_pid_, params_.lims);
        ControlExecution();
    } else {
        Mount(params_.working_directory);
        SetUpSlave(path);
    }
}

elans::runner::Runner::~Runner() {
    Umount(params_.working_directory);
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
    // message_assert(chmod(path.data(), S_IXGRP | S_IXUSR | S_IXOTH) != -1, "Failed to chmod");

    message_assert(setuid(params_.user) != -1, "Changing user failed");
    message_assert(getuid() == params_.user, "Changing user failed");

    std::vector<char*> args_ptrs(params_.args.size());
    std::transform(params_.args.begin(), params_.args.end(), args_ptrs.begin(), [] (std::string &str) {
        return str.data();
    });

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
    message_assert(WIFEXITED(status) != 0, "WFT?");

    res_.cpu_time = cgroup_manager_->GetMemoryPeak();
    res_.real_time = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - beg_real_time).count();
    res_.memory = cgroup_manager_->GetMemoryPeak();

    if (kill(real_time_killer_pid, SIGKILL) != 0) {
        kill(cpu_time_killer_pid, SIGKILL);
        res_.verdict = RunningResult::IE;
    } else if (kill(cpu_time_killer_pid, SIGKILL) != 0) {
        res_.verdict = RunningResult::TL;
    } else if (cgroup_manager_->GetMemoryPeak() >= params_.lims.memory) {
        res_.verdict = RunningResult::ML;
    } else if (WEXITSTATUS(status) != 0) {
        res_.verdict = RunningResult::RE;
    } else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSYS) {
        res_.verdict = RunningResult::SE;
    } else {
        res_.verdict = RunningResult::OK;
    }
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
        while (cgroup_manager_->GetCpuTime() <= millis_limit) {}
        kill(slave_pid_, SIGKILL);
        exit(EXIT_SUCCESS);
    }
}
