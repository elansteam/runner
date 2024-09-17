#include <vector>
#include "../include/runner/mount.h"

using std::string_literals::operator""s;

std::array mounting_dirs = {
    "/usr"s,
    "/bin"s,
    "/lib"s,
    "/lib64"s
};

void Mount(const std::string &from, const std::string &to) {
    std::filesystem::create_directories(to);
    MessageAssert(mount(from.data(), to.data(), "ext4", MS_BIND, nullptr) == 0, "Failed to mount " + from);
}

void runner::mount::Mount(const std::string &working_directory) {
    for (const std::string &dir : mounting_dirs) {
        ::Mount(dir, working_directory + dir);
    }
}

void Umount(const std::string &path) {
    MessageAssert(umount(path.data()) != -1, "Failed to umount");
    std::filesystem::remove(path);
}

void runner::mount::Umount(const std::string &working_directory) {
    for (const std::string &dir : mounting_dirs) {
        ::Umount(working_directory + dir);
    }
}
