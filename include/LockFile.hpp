#pragma once

#include <string>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <cstring>


class LockFile {
    private:
        int _fd;

    public:
        LockFile();
        ~LockFile();

        LockFile(const LockFile&) = delete;
        LockFile& operator=(const LockFile&) = delete;

        bool acquire(const std::string& path, std::string& error);
};