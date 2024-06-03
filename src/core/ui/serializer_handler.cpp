#include "app/engine.hpp"
#include "ui/serializer_handler.hpp"
#include "spdlog/spdlog.h"

AutoSaveTriggerHandler::AutoSaveTriggerHandler(Engine& engine, std::string const save_path): engine(engine), save_path(save_path) {}

void AutoSaveTriggerHandler::operator()(KeyboardEvent const&) {
    SPDLOG_INFO("Starting autosave...");
    Packer p(save_path);
    p << engine;
    SPDLOG_DEBUG("Autosave completed");
}