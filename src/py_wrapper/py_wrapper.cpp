#include "app/facade.hpp"

#include <pybind11/pybind11.h>
namespace py = pybind11;

PYBIND11_MODULE(ant_core, m){
    m.doc() = "the interface for the core ant engine"; // Optional docstring

    py::class_<AntGameFacade>(m, "AntGameFacade")
        .def(py::init<>())   // our constructor
        .def("update", &AntGameFacade::update);      // Expose member methods
}