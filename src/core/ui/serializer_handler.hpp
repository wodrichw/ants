#pragma once

#include <string>

#include "ui/event_system.hpp"
#include "utils/serializer.hpp"

class Engine;

class AutoSaveTriggerHandler: public Subscriber<KeyboardEvent> {
    Engine const& engine;
    std::string const save_path;
public:
    AutoSaveTriggerHandler(Engine&, std::string const);
    void operator()(KeyboardEvent const&);
};
