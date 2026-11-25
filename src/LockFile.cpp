#include "LockFile.hpp"

LockFile::LockFile() : _fd(-1) {}

LockFile::~LockFile() {
    if (_fd != -1) {
        flock(_fd, LOCK_UN);
        close(_fd);
    }
}

bool LockFile::acquire(const std::string& path, std::string& error) {
    if (_fd != -1) {
        error = "Lock already acquired.";
        return false;
    }

    int fd = open(path.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
        error = "Error creating/opening matt_daemon.lock: " + std::string(strerror(errno));
        return false;
    }

    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
        const int savedError = errno;
        close(fd);

        if (savedError == EWOULDBLOCK || savedError == EAGAIN) {
            error = "Another instance of the daemon is already running.";
        } else {
            error = "Error acquiring lock on matt_daemon.lock: " + std::string(strerror(savedError));
        }

        return false;
    }

    _fd = fd;
    return true;
}