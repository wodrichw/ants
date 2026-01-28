#include "app/facade.hpp"

#include <pybind11/pybind11.h>
namespace py = pybind11;

PYBIND11_MODULE(ant_core, m){
    m.doc() = "the interface for the core ant engine"; // Optional docstring

    py::class_<ReplayError>(m, "ReplayError")
        .def_readonly("message", &ReplayError::message)
        .def_readonly("expected_version", &ReplayError::expected_version)
        .def_readonly("actual_version", &ReplayError::actual_version)
        .def_readonly("frame_index", &ReplayError::frame_index)
        .def_readonly("event_index", &ReplayError::event_index)
        .def_readonly("event_kind", &ReplayError::event_kind);

    py::class_<AntGameFacade>(m, "AntGameFacade")
        .def(py::init<>())   // our constructor
        .def("update", &AntGameFacade::update)
        .def("engine_update", &AntGameFacade::engine_update)
        .def("start_replay_recording",
             [](AntGameFacade& facade, const std::string& path) {
                 return facade.start_replay_recording(path).ok;
             })
        .def("start_replay_playback",
             [](AntGameFacade& facade, const std::string& path) {
                 return facade.start_replay_playback(path).ok;
             })
        .def("stop_replay_recording", &AntGameFacade::stop_replay_recording)
        .def("replay_has_error", &AntGameFacade::replay_has_error)
        .def("replay_last_error", &AntGameFacade::replay_last_error)
        .def("replay_done", &AntGameFacade::replay_done)
        .def("action_move_player", &AntGameFacade::action_move_player)
        .def("action_dig", &AntGameFacade::action_dig)
        .def("action_create_ant", &AntGameFacade::action_create_ant)
        .def("action_add_program_lines",
             &AntGameFacade::action_add_program_lines)
        .def("action_assign_program_to_ant",
             &AntGameFacade::action_assign_program_to_ant)
        .def("action_go_up", &AntGameFacade::action_go_up)
        .def("action_go_down", &AntGameFacade::action_go_down);
}