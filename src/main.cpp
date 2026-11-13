#include "LockFile.hpp"
#include "Tintin_reporter.hpp"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

bool deamonize(Tintin_reporter& reporter) {
    pid_t pid = fork();

    if (pid == -1) {
        reporter.error(
            "First fork failed: " + std::string(std::strerror(errno))
        );
        return false;
    }

    if (pid > 0) 
        _exit(EXIT_SUCCESS);

    if (setsid() == -1) {
        reporter.error(
            "setsid failed: " + std::string(std::strerror(errno))
        );
        return false;
    }

    pid = fork();

    if (pid == -1) {
        reporter.error(
            "Second fork failed: " + std::string(std::strerror(errno))
        );
        return false;
    }

    if (pid > 0) 
        _exit(EXIT_SUCCESS);

    if (chdir("/") == -1) {
        reporter.error(
            "chdir failed: " + std::string(std::strerror(errno))
        );
        return false;
    }

    umask(0);

    const int nullFd = open("/dev/null", O_RDWR);

    if (nullFd == -1) {
        reporter.error(
            "Failed to open /dev/null: " + std::string(std::strerror(errno))
        );
        return false;
    }

    for (int fd = STDIN_FILENO; fd <= STDERR_FILENO; ++fd) {
        if (dup2(nullFd, fd) == -1) {
            reporter.error(
                "Failed to redirect file descriptor " + std::to_string(fd) +
                " to /dev/null: " + std::string(std::strerror(errno))
            );

            if (nullFd > STDERR_FILENO)
                close(nullFd);

            return false;
        }
    }

    if (nullFd > STDERR_FILENO)
        close(nullFd);

    reporter.info("Daemonization successful.");
    return true;
}

int main() {
    Tintin_reporter reporter("/var/log/matt_deamon/matt_deamon.log");

    if (!reporter.isOpen()) {
        std::cerr << "Failed to open log file: /var/log/matt_deamon/matt_deamon.log" << std::endl;
        return 1;
    }

    reporter.info("Starting Matt_deamon...");

    if (geteuid() != 0) {
        std::cerr << "Matt_deamon must be launched as root." << std::endl;
        return 1;
    }

    LockFile lock;
    std::string error;

    if (!lock.acquire("/var/run/matt_deamon.lock", error)) {
        std::cerr << error << std::endl;
        return 1;
    }

}