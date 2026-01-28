#include <gtest/gtest.h>

#include <pybind11/embed.h>
#include <Python.h>

#include <memory>

namespace py = pybind11;

extern "C" PyObject* PyInit_ant_core();

class PyWrapperMethodTest : public ::testing::TestWithParam<const char*> {
protected:
    static std::unique_ptr<py::scoped_interpreter> guard;
    static py::object module;

    static void SetUpTestSuite() {
        guard = std::make_unique<py::scoped_interpreter>();
        PyObject* mod = PyInit_ant_core();
        ASSERT_NE(mod, nullptr);
        module = py::reinterpret_steal<py::object>(mod);
    }

    static void TearDownTestSuite() {
        module = py::object();
        guard.reset();
    }
};

std::unique_ptr<py::scoped_interpreter> PyWrapperMethodTest::guard = nullptr;
py::object PyWrapperMethodTest::module = py::object();

TEST_P(PyWrapperMethodTest, AntGameFacadeExposesMethods) {
    auto facade_cls = module.attr("AntGameFacade");
    EXPECT_TRUE(py::hasattr(facade_cls, GetParam()));
}

INSTANTIATE_TEST_SUITE_P(
    PyWrapperMethodCases,
    PyWrapperMethodTest,
    ::testing::Values(
        "update",
        "engine_update",
        "start_replay_recording",
        "start_replay_playback",
        "stop_replay_recording",
        "replay_has_error",
        "replay_last_error",
        "replay_done",
        "action_move_player",
        "action_dig"
    )
);
