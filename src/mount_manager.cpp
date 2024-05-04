#include "../include/runner/mount_manager.h"

void Mount(std::string working_dir) {
    {
        std::filesystem::create_directories(working_dir + "/usr");
        std::string path_to_new_usr = working_dir + "/usr";
        message_assert(mount("/usr", path_to_new_usr.data(), "ext4", MS_BIND | MS_RDONLY, nullptr) == 0, "Failed to mount /usr");
    }
    {
        std::filesystem::create_directories(working_dir + "/bin");
        std::string path_to_mounting = working_dir + "/bin";
        message_assert(mount("/bin", path_to_mounting.data(), "ext4", MS_BIND | MS_RDONLY, nullptr) == 0, "Failed to mount /bin");
    }
    {
        std::filesystem::create_directories(working_dir + "/lib");
        std::string path_to_mounting = working_dir + "/lib";
        message_assert(mount("/lib", path_to_mounting.data(), "ext4", MS_BIND | MS_RDONLY, nullptr) == 0, "Failed to mount /lib");
    }
    {
        std::filesystem::create_directories(working_dir + "/lib64");
        std::string path_to_mounting = working_dir + "/lib64";
        message_assert(mount("/lib64", path_to_mounting.data(), "ext4", MS_BIND | MS_RDONLY, nullptr) == 0, "Failed to mount /lib64");
    }
    {
        std::filesystem::create_directories(working_dir + "/proc");
        std::string path_to_mounting = working_dir + "/proc";
        message_assert(mount("/proc", path_to_mounting.data(), "procfs", MS_BIND | MS_RDONLY, nullptr) == 0, "Failed to mount /proc");
    }
}

void Umount(std::string working_dir) {
    {
        std::string path_to_mounting = working_dir + "/usr";
        message_assert(umount(path_to_mounting.data()) != -1, "Failed to umount /usr");
        std::filesystem::remove(path_to_mounting);
    }
    {
        std::string path_to_mounting = working_dir + "/bin";
        message_assert(umount(path_to_mounting.data()) != -1, "Failed to umount /bin");
        std::filesystem::remove(path_to_mounting);
    }
    {
        std::string path_to_mounting = working_dir + "/lib";
        message_assert(umount(path_to_mounting.data()) != -1, "Failed to umount /lib");
        std::filesystem::remove(path_to_mounting);
    }
    {
        std::string path_to_mounting = working_dir + "/lib64";
        message_assert(umount(path_to_mounting.data()) != -1, "Failed to umount /lib64");
        std::filesystem::remove(path_to_mounting);
    }
    {
        std::string path_to_mounting = working_dir + "/proc";
        message_assert(umount(path_to_mounting.data()) != -1, "Failed to umount /proc");
        std::filesystem::remove(path_to_mounting);
    }
}
