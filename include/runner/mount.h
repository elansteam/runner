#pragma once
#include <filesystem>
#include <sys/mount.h>

#include "message_assert.h"

namespace runner::mount {
    void InitMount(const std::string &working_directory);

    void DeinitMount(const std::string &working_directory);
} // namespace runner::mount
