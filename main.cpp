#include <iostream>

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
        params.args = { "/usr/bin/echo", "MESSAGE" };
        params.input_stream_file = in_path;
        params.output_stream_file = out_path;
        params.working_directory = "/home";
        params.user = 1000;
        runner::Runner runner("/usr/bin/echo", params);
        std::ifstream fin(out_path);
        std::string buf(1024, 'a');
        std::cout.write(buf.data(), fin.readsome(buf.data(), 1024));
    }
}
