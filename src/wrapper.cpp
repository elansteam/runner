#include "../extern/pybind11/include/pybind11/pybind11.h"
#include "runner/runner.h"
namespace py = pybind11;
using namespace std;
using namespace elans::runner;

PYBIND11_MODULE(runner, m) {
    m.doc() = "safe runner python lib";

    py::class_<Runner::TestingResult>(m, "TestingResult")
            .def(py::init<>())
            .def_readwrite("res", &Runner::TestingResult::res)
            .def_readwrite("output", &Runner::TestingResult::output);


    py::class_<Runner>(m, "Runner")
            .def(py::init<std::string, std::string, Runner::Limits>())
            .def("GetOutput", &Runner::GetOutput, "wait for the slave program's end");


    py::class_<Runner::Limits>(m, "Limits", py::dynamic_attr())
            .def(py::init<>())
            .def_readwrite("threads", &Runner::Limits::threads)
            .def_readwrite("memory", &Runner::Limits::memory)
            .def_readwrite("time", &Runner::Limits::time);


    py::enum_<Runner::RunningResult>(m, "ExitStatus")
            .value("TL", Runner::RunningResult::TL)
            .value("ML", Runner::RunningResult::ML)
            .value("OK", Runner::RunningResult::OK)
            .value("SE", Runner::RunningResult::SE)
            .value("RE", Runner::RunningResult::RE)
            .export_values();
}