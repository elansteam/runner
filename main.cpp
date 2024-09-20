#include <iostream>
#include <cassert>

#include "runner/runner.h"


int main() {
    for (int _ : std::views::iota(0, 100)) {
        std::string in_path = "/home/in.txt";
        std::string out_path = "/home/out.txt";
        runner::Runner::Params params;
        runner::Runner::Limits lims;
        lims.threads = 1;
        lims.memory_kb = 1024;
        lims.cpu_time_limit_ms = 1000;
        lims.real_time_limit_ms = 2000;
        lims.allow_files_write = true;
        lims.allow_files_read = true;
        params.lims = lims;
        params.args = { "/usr/bin/ping", "google.com" };
        params.input_stream_file = in_path;
        params.output_stream_file = out_path;
        params.working_directory = "/home";
        params.user = 1000;
        runner::Runner runner("/usr/bin/ping", params);
        switch (runner.GetOutput().verdict) {
            case runner::Runner::RunningResult::OK:
                std::cout << "Test passed" << std::endl;
                break;
            case runner::Runner::RunningResult::SE:
                std::cout << "Test failed: SE" << std::endl;
                break;
            case runner::Runner::RunningResult::TL:
                std::cout << "Test failed: TE" << std::endl;
                break;
            case runner::Runner::RunningResult::RE:
                std::cout << "Test failed: RE" << std::endl;
                break;
            case runner::Runner::RunningResult::ML:
                std::cout << "Test failed: ME" << std::endl;
                break;
            case runner::Runner::RunningResult::IE:
                std::cout << "Test failed: IE" << std::endl;
                break;
            default:
                assert(false);
                break;
        }
        std::ifstream fin(out_path);
        std::string buf(1024, 'a');
        std::cout.write(buf.data(), fin.readsome(buf.data(), 1024));
    }
}
