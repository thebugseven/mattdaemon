#include "Tintin_reporter.hpp"

Tintin_reporter::Tintin_reporter() {
    const std::filesystem::path directory("/var/log/matt_");

    std::error_code error;
    std::filesystem::create_directories(directory, error);

    if (error)
        return;

    _file.open(
        directory / "matt_daemon.log",
        std::ios::out | std::ios::app
    );
}

bool Tintin_reporter::isOpen() const {
    return _file.is_open();
}

void Tintin_reporter::info(const std::string& message) {
    write("INFO", message);
}

void Tintin_reporter::warning(const std::string& message) {
    write("WARNING", message);
}

void Tintin_reporter::error(const std::string& message) {
    write("ERROR", message);
}

void Tintin_reporter::write(const std::string& level, const std::string& message) {
    std::time_t now = std::time(nullptr);

    std::tm localTime{};

    if (localtime_r(&now, &localTime) == nullptr || !_file.is_open())
        return;

    _file << "[ " << std::put_time(&localTime, "%d / %m / %Y - %H : %M : %S") << "] ["
        << level << "] " << message << std::endl;
}