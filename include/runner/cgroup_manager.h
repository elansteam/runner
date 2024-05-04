#pragma once

#include <string>
#include <filesystem>
#include <fstream>

#include <fcntl.h>
#include "unistd.h"

#include "domain.h"

class CgroupManager {
public:
    CgroupManager(pid_t pid, Limitations lims);

    uint64_t GetCpuTime() const;

    uint64_t GetMemoryPeak() const;

    ~CgroupManager() {
        std::filesystem::remove("/sys/fs/cgroup/" + GetCgroupName());
    }

private:
    pid_t pid_;

    std::string GetCgroupName() const;

    static void Write(std::string path, std::string data);
};
