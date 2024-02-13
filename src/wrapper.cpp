#include "../extern/pybind11/include/pybind11/pybind11.h"
#include "runner/runner.h"
namespace py = pybind11;
using namespace std;
using namespace elans::runner;

PYBIND11_MODULE(runner_lib_py, m) {
    m.doc() = "safe runner python lib";


    py::class_<Runner> runner(m, "Runner");
    runner
        .def(py::init<std::string, std::string, Runner::Limits>())
            .def("GetOutput", &Runner::GetOutput, "wait for the slave program's end");

    py::class_<Runner::TestingResult>(runner, "TestingResult")
            .def(py::init<>())
            .def_readwrite("res", &Runner::TestingResult::res)
            .def_readwrite("output", &Runner::TestingResult::output_path);


    py::class_<Runner::Limits>(runner, "Limits", py::dynamic_attr())
            .def(py::init<>())
            .def_readwrite("threads", &Runner::Limits::threads)
            .def_readwrite("memory", &Runner::Limits::memory)
            .def_readwrite("time", &Runner::Limits::tl_cpu_time);


    py::enum_<Runner::RunningResult>(runner, "ExitStatus")
            .value("TL", Runner::RunningResult::TL_CPU)
            .value("ML", Runner::RunningResult::ML)
            .value("OK", Runner::RunningResult::OK)
            .value("SE", Runner::RunningResult::SE)
            .value("RE", Runner::RunningResult::RE)
            .export_values();
}
