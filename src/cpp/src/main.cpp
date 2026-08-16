// Usage: ./water <seconds>
// Runs the pump for the given number of seconds, then stops it.
//
// TODO: read the GPIO pin from a config file instead of hardcoding it.

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

#include "Pump.h"

namespace {
constexpr unsigned int kDefaultPumpPin = 18;  // BCM GPIO18
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <seconds>\n";
        return 1;
    }

    int seconds = std::atoi(argv[1]);
    if (seconds <= 0) {
        std::cerr << "Error: seconds must be a positive integer\n";
        return 1;
    }

    try {
        Pump pump(kDefaultPumpPin);
        std::cout << "Pump ON for " << seconds << "s...\n";
        pump.on();
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
        pump.off();
        std::cout << "Pump OFF.\n";
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
