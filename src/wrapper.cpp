#include "../extern/pybind11/include/pybind11/pybind11.h"
#include "safe-runner/runner.h"
namespace py = pybind11;
using namespace std;
using namespace elans::runner;

PYBIND11_MODULE(safe_runner_lib, m) {
    m.doc() = "safe runner python lib";
    py::class_<SafeRunner> runner(m, "SafeRunner");

    py::class_<SafeRunner::TestingResult>(runner, "TestingResult", py::dynamic_attr())
            .def(py::init<>())
            .def_readwrite("res", &SafeRunner::TestingResult::res)
            .def_readwrite("output", &SafeRunner::TestingResult::output);

    runner  .def(py::init<std::string, std::string>(), "gets path and input")
            .def("GetOutput", &SafeRunner::GetOutput, "wait for the slave program's end")
            .def("IsEnded", &SafeRunner::IsEnded, "return if slave program ended or not");

    py::enum_<SafeRunner::RunningResult>(runner, "ExitStatus")
            .value("TL", SafeRunner::RunningResult::TL)
            .value("ML", SafeRunner::RunningResult::ML)
            .value("OK", SafeRunner::RunningResult::OK)
            .value("SE", SafeRunner::RunningResult::SE)
            .value("RE", SafeRunner::RunningResult::RE)
            .export_values();
}