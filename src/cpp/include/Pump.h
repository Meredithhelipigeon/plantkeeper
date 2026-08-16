#pragma once

#include <gpiod.h>
#include <stdexcept>
#include <string>

// Pump wraps a single GPIO line (via libgpiod v2) that drives a MOSFET/relay
// controlling a 12V pump. RAII: the GPIO line is released automatically
// when the Pump object goes out of scope, so a crash or early return
// can never leave the pump running.
//
// Targets libgpiod 2.x (the API is a ground-up redesign from v1: request
// objects + line-settings/line-config builders instead of gpiod_line_*
// free functions). Confirmed against libgpiod 2.2.1 on Raspberry Pi OS
// (Debian trixie) on a Pi 5, where `gpiodetect` shows the 40-pin header
// lines live on "gpiochip0" (pinctrl-rp1) -- that's the default below.
class Pump {
public:
    // pin: BCM GPIO number (e.g. 18)
    // chipName: gpiochip device name (without "/dev/"), defaults to "gpiochip0"
    explicit Pump(unsigned int pin, const std::string& chipName = "gpiochip0");
    ~Pump();

    // Non-copyable: a GPIO line should only ever be owned by one object.
    Pump(const Pump&) = delete;
    Pump& operator=(const Pump&) = delete;

    void on();
    void off();
    bool isOn() const { return on_; }

private:
    struct gpiod_chip* chip_ = nullptr;
    struct gpiod_line_request* request_ = nullptr;
    unsigned int pin_;
    bool on_ = false;
};
