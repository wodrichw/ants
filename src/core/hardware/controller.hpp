#pragma once

enum ControllerType { PROGRAM_EXECUTOR };
class ClockController {
   public:
    virtual void handleClockPulse() = 0;
    virtual ~ClockController() = default;
    virtual ControllerType get_type() const = 0;
};
