#pragma once

#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>

class Tintin_reporter {
    private:
        std::ofstream _file;

        void write(const std::string& level, const std::string& message);

    public:
        explicit Tintin_reporter();

        bool isOpen() const;

        void info(const std::string& message);
        void warning(const std::string& message);
        void error(const std::string& message);
};