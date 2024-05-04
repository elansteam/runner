#pragma once
#include <string>
#include <filesystem>
#include <sys/mount.h>

#include "domain.h"

void Mount(std::string working_dir);

void Umount(std::string working_dir);
