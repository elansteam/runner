#include <iostream>
#include "runner/runner.h"

using namespace std;

int main() {
    std::string in_path = "/home/pablo/Documents/project/runner/in.txt";
    std::string out_path = "/home/pablo/Documents/project/runner/out.txt";
    elans::runner::Runner::Params params;
    Limitations lims;
    lims.threads = 1;
    lims.memory = 1024;
    lims.cpu_time_limit = 1000;
    lims.real_time_limit = 2000;
    lims.allow_files_write = true;
    lims.allow_files_read = true;
    params.lims = lims;
    // params.args = { "/usr/bin/curl", "google.com" };
    params.args = { "/a.out" };
    params.input_stream_file = in_path;
    params.output_stream_file = out_path;
    params.user = 1000;
    params.working_directory = "/tmp/tmp.N8QGAlSWyX/working_dir";
    // elans::runner::Runner runner("/usr/bin/curl", params);
    elans::runner::Runner runner("/a.out", params);

    switch (runner.GetOutput().verdict) {
        case elans::runner::Runner::RunningResult::OK:
            cout << "OK" << endl;
            break;
        case elans::runner::Runner::RunningResult::TL:
            cout << "TL" << endl;
            break;
        case elans::runner::Runner::RunningResult::ML:
            cout << "ML" << endl;
            break;
        case elans::runner::Runner::RunningResult::RE:
            cout << "RE " << runner.GetOutput().exit_code << endl;
            break;
        case elans::runner::Runner::RunningResult::IE:
            cout << "IE" << endl;
            break;
        case elans::runner::Runner::RunningResult::SE:
            cout << "SE" << endl;
            break;
    }

    std::ifstream fin(out_path);
    std::string buf(1024, 'a');
    cout.write(buf.data(), fin.readsome(buf.data(), 1024));
}
