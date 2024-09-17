#pragma once
#include <filesystem>
#include <sys/mount.h>

#include "message_assert.h"

namespace runner::mount {
    void Mount(const std::string &working_directory);
    void Umount(const std::string &working_directory);
} // namespace runner::mount
