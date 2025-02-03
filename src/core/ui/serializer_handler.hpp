#pragma once

#include <string>
#include "ui/subscriber.hpp"

struct EngineState;

class AutoSaveTriggerHandler : public Subscriber<KeyboardEvent> {
    EngineState const& engine;
    std::string const save_path;

   public:
    AutoSaveTriggerHandler(const AutoSaveTriggerHandler &) = default;
    AutoSaveTriggerHandler(AutoSaveTriggerHandler &&) = default;
    AutoSaveTriggerHandler &operator=(const AutoSaveTriggerHandler &) = delete;
    AutoSaveTriggerHandler &operator=(AutoSaveTriggerHandler &&) = delete;
    AutoSaveTriggerHandler(EngineState &, std::string const);
    void operator()(KeyboardEvent const&);
};
