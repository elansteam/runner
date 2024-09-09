#include "../include/runner/mount.h"

void Mount(const std::string &from, const std::string &to) {
    std::filesystem::create_directories(to);
    MessageAssert(mount(from.data(), to.data(), "ext4", MS_BIND, nullptr) == 0, "Failed to mount " + from);
}

void runner::mount::InitMount(const std::string &working_directory) {
    Mount("/usr", working_directory + "/usr");
    Mount("/bin", working_directory + "/bin");
    Mount("/lib", working_directory + "/lib");
    Mount("/lib64", working_directory + "/lib64");
}

void Umount(const std::string &path) {
    MessageAssert(umount(path.data()) != -1, "Failed to umount");
    std::filesystem::remove(path);
}

void runner::mount::DeinitMount(const std::string &working_directory) {
    Umount(working_directory + "/usr");
    Umount(working_directory + "/bin");
    Umount(working_directory + "/lib");
    Umount(working_directory + "/lib64");
}
