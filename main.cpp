#include <iostream>
#include "runner/runner.h"

using namespace std;

int main() {
    std::string in_path = "/home/pablo/Documents/project/runner/in.txt";
    std::string out_path = "/home/pablo/Documents/project/runner/out.txt";
    elans::runner::Runner::Params params;
    elans::runner::Runner::Limits lims;
    lims.threads = 1;
    lims.memory = 1024;
    lims.cpu_time_limit = 1000;
    lims.real_time_limit = 2000;
    lims.allow_files_write = true;
    lims.allow_files_read = true;
    params.lims = lims;
    params.args = { "/usr/bin/ls" };
    params.input_stream_file = in_path;
    params.output_stream_file = out_path;
    params.user = 1000;
    params.working_directory = "/tmp/tmp.N8QGAlSWyX/working_dir";
    elans::runner::Runner runner("/usr/bin/ls", params);
    std::ifstream fin(out_path);
    std::string buf(1024, 'a');
    cout.write(buf.data(), fin.readsome(buf.data(), 1024));
}
