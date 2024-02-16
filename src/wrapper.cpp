#include "../extern/pybind11/include/pybind11/pybind11.h"
#include "runner/runner.h"
namespace py = pybind11;
using namespace std;
using namespace elans::runner;

PYBIND11_MODULE(runner_lib_py, m) {
    m.doc() = "safe runner python lib";


    py::class_<Runner> runner(m, "Runner");

    py::enum_<Runner::RunningResult>(runner, "ExitStatus")
            .value("TL", Runner::RunningResult::TL)
            .value("ML", Runner::RunningResult::ML)
            .value("OK", Runner::RunningResult::OK)
            .value("SE", Runner::RunningResult::SE)
            .value("RE", Runner::RunningResult::RE)
            .value("IE", Runner::RunningResult::IE)
            .export_values();

    py::class_<Runner::TestingResult>(runner, "TestingResult")
            .def(py::init<>())
            .def_readwrite("verdict", &Runner::TestingResult::verdict)
            .def_readwrite("exit_code", &Runner::TestingResult::exit_code)
            .def_readwrite("threads", &Runner::TestingResult::threads)
            .def_readwrite("cpu_time", &Runner::TestingResult::cpu_time)
            .def_readwrite("real_time", &Runner::TestingResult::real_time)
            .def_readwrite("memory", &Runner::TestingResult::memory);

    py::class_<Runner::Limits>(runner, "Limits", py::dynamic_attr())
            .def(py::init<>())
            .def_readwrite("threads", &Runner::Limits::threads)
            .def_readwrite("memory", &Runner::Limits::memory)
            .def_readwrite("cpu_time_limit", &Runner::Limits::cpu_time_limit)
            .def_readwrite("real_time_limit", &Runner::Limits::real_time_limit)
            .def_readwrite("allow_files_read", &Runner::Limits::allow_files_read)
            .def_readwrite("allow_files_write", &Runner::Limits::allow_files_write)
            .def_readwrite("input_stream_file", &Runner::Limits::input_stream_file)
            .def_readwrite("output_stream_file", &Runner::Limits::output_stream_file);

    runner
        .def(py::init<std::string, Runner::Limits>())
        .def("GetOutput", &Runner::GetOutput);
}
