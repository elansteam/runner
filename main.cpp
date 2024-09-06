#include <iostream>

#include "runner/runner.h"

using namespace std;

int main() {
    for (int _ : views::iota(0, 2)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        std::string in_path = "/home/in.txt";
        std::string out_path = "/home/out.txt";
        runner::Runner::Params params;
        runner::Runner::Limits lims;
        lims.threads = 1;
        lims.memory = 1024;
        lims.cpu_time_limit = 1000;
        lims.real_time_limit = 2000;
        lims.allow_files_write = true;
        lims.allow_files_read = true;
        params.lims = lims;
        params.args = {"/usr/bin/ls"};
        params.input_stream_file = in_path;
        params.output_stream_file = out_path;
        params.working_directory = "/home";
        params.user = 1000;
        runner::Runner runner("/usr/bin/ls", params);
        std::ifstream fin(out_path);
        std::string buf(1024, 'a');
        cout.write(buf.data(), fin.readsome(buf.data(), 1024));
    }
}
