#include "LockFile.hpp"
#include "Tintin_reporter.hpp"
#include "SignalManager.hpp"
#include "TcpServer.hpp"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

bool daemonize(Tintin_reporter& reporter) {
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
    if (geteuid() != 0) {
        std::cerr << "Matt_daemon must be launched as root." << std::endl;
        return 1;
    }

    LockFile lock;
    std::string error;

    if (!lock.acquire("/var/lock/matt_daemon.lock", error)) {
        std::cerr << error << std::endl;
        return 1;
    }

    Tintin_reporter reporter;

    if (!reporter.isOpen()) {
        std::cerr << "Failed to open log file: /var/log/matt_daemon/matt_daemon.log" << std::endl;
        return 1;
    }

    reporter.info("Starting Matt_daemon...");

    if (!SignalManager::setup()) {
        reporter.error("Failed to set up signal handlers.");
        return 1;
    }

    if (!daemonize(reporter))
        return 1;
    
    bool running = true;

    TcpServer server(4242, 3, [&reporter, &running](const std::string& message) {
        if (message == "quit") {
            reporter.info("Received quit command. Shutting down.");
            running = false;
            return std::string ("Matt_daemon is shutting down.\n");
        }

        reporter.info("Received message: " + message);
        return std::string("Message logged.");
    });

    if (!server.start(error)) {
        reporter.error("Failed to start TCP server: " + error);
        return 1;
    }

    reporter.info("TCP server started on port 4242.");

    while (running) {
        const sig_atomic_t signal = SignalManager::receivedSignal();

        if (signal != 0) {
            reporter.info("Received signal: " + std::to_string(static_cast<int>(signal)) + ". Shutting down.");
            break;
        }

        server.update();
        poll(nullptr, 0, 100);
    }



}