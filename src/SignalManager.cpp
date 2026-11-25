#include "SignalManager.hpp"

volatile sig_atomic_t SignalManager::_receivedSignal = 0;

void SignalManager::handler(int signal) {
    _receivedSignal = signal;
}

sig_atomic_t SignalManager::receivedSignal() {
    return _receivedSignal;
}

bool SignalManager::setup() {
    struct sigaction sa {};

    sa.sa_handler = &SignalManager::handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    const int signals[] = {SIGINT, SIGTERM, SIGQUIT, SIGHUP};

    for (int signal : signals) {
        if (sigaction(signal, &sa, nullptr) == -1) {
            return false;
        }
    }

    return true;
}
