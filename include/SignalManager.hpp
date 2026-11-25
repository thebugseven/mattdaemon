#pragma once

#include <csignal>
#include <cstring>

class SignalManager {
    private:
        static volatile sig_atomic_t _receivedSignal;

        static void handler(int signal);

    public:
        static bool setup();

        static sig_atomic_t receivedSignal();
};