#include "ui/serializer_handler.hpp"

#include "app/engine_state.hpp"
#include "spdlog/spdlog.h"

AutoSaveTriggerHandler::AutoSaveTriggerHandler(EngineState& engine,
                                               std::string const save_path)
    : engine(engine), save_path(save_path) {}

void AutoSaveTriggerHandler::operator()(KeyboardEvent const&) {
    SPDLOG_INFO("Starting autosave...");
    Packer p(save_path);
    p << engine;
    SPDLOG_INFO("Autosave completed");
}