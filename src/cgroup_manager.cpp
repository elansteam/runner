#include "../include/runner/cgroup_manager.h"

CgroupManager::CgroupManager(pid_t pid, Limitations lims): pid_(pid) {
    std::filesystem::create_directory("/sys/fs/cgroup/" + GetCgroupName());

    Write("/sys/fs/cgroup/" + GetCgroupName() + "/cgroup.procs", std::to_string(pid_));
    Write("/sys/fs/cgroup/" + GetCgroupName() + "/memory.max", std::to_string(lims.memory * 1024));
    Write("/sys/fs/cgroup/" + GetCgroupName() + "/memory.low", std::to_string(lims.memory * 1024 - 1));
    Write("/sys/fs/cgroup/" + GetCgroupName() + "/pids.max", std::to_string(lims.threads));
}

uint64_t CgroupManager::GetCpuTime() const {
    std::ifstream fin("/sys/fs/cgroup/" + GetCgroupName() + "/cgroup.procs");
    uint64_t ans_usec;
    fin >> ans_usec;
    return ans_usec / 1000;
}

uint64_t CgroupManager::GetMemoryPeak() const {
    std::ifstream fin("/sys/fs/cgroup/" + GetCgroupName() + "/memory.peak");
    uint64_t ans_bytes;
    fin >> ans_bytes;
    return ans_bytes / 1024;
}

std::string CgroupManager::GetCgroupName() const {
    return "runner_cgroup_" + std::to_string(pid_);
}

void CgroupManager::Write(std::string path, std::string data) {
    int fd = open(path.data(), O_WRONLY | O_TRUNC);
    write(fd, data.data(), data.size());
    close(fd);
}
