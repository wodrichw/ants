#pragma once

class ClockController {
   public:
    virtual void handleClockPulse() = 0;
    virtual ~ClockController() = default;
};
