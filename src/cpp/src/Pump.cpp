#include "Pump.h"

namespace {
// v2 requires the request to be built from a settings + line-config object
// rather than requesting a line directly. This helper does that dance for
// a single output line and returns the resulting request (or nullptr).
struct gpiod_line_request* requestSingleOutputLine(struct gpiod_chip* chip,
                                                    unsigned int offset,
                                                    const char* consumer) {
    struct gpiod_line_settings* settings = gpiod_line_settings_new();
    if (!settings) return nullptr;
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);

    struct gpiod_line_config* line_cfg = gpiod_line_config_new();
    if (!line_cfg) {
        gpiod_line_settings_free(settings);
        return nullptr;
    }
    if (gpiod_line_config_add_line_settings(line_cfg, &offset, 1, settings) != 0) {
        gpiod_line_config_free(line_cfg);
        gpiod_line_settings_free(settings);
        return nullptr;
    }

    struct gpiod_request_config* req_cfg = gpiod_request_config_new();
    if (req_cfg) {
        gpiod_request_config_set_consumer(req_cfg, consumer);
    }

    struct gpiod_line_request* request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

    // These builder objects are only needed to construct the request;
    // libgpiod copies what it needs internally, so we free them here.
    if (req_cfg) gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);

    return request;
}
}  // namespace

Pump::Pump(unsigned int pin, const std::string& chipName) : pin_(pin) {
    std::string path = "/dev/" + chipName;
    chip_ = gpiod_chip_open(path.c_str());
    if (!chip_) {
        throw std::runtime_error("Pump: failed to open GPIO chip '" + path +
                                  "' (run `gpiodetect` to confirm the chip name)");
    }

    request_ = requestSingleOutputLine(chip_, pin_, "auto-watering-pump");
    if (!request_) {
        gpiod_chip_close(chip_);
        chip_ = nullptr;
        throw std::runtime_error("Pump: failed to request GPIO line " + std::to_string(pin_) +
                                  " as output (is another process using it? try `sudo`)");
    }
}

Pump::~Pump() {
    // Best-effort: make sure the pump is off before we release the line.
    if (request_) {
        gpiod_line_request_set_value(request_, pin_, GPIOD_LINE_VALUE_INACTIVE);
        gpiod_line_request_release(request_);
    }
    if (chip_) {
        gpiod_chip_close(chip_);
    }
}

void Pump::on() {
    if (!request_) return;
    if (gpiod_line_request_set_value(request_, pin_, GPIOD_LINE_VALUE_ACTIVE) != 0) {
        throw std::runtime_error("Pump: failed to set GPIO line ACTIVE");
    }
    on_ = true;
}

void Pump::off() {
    if (!request_) return;
    if (gpiod_line_request_set_value(request_, pin_, GPIOD_LINE_VALUE_INACTIVE) != 0) {
        throw std::runtime_error("Pump: failed to set GPIO line INACTIVE");
    }
    on_ = false;
}
