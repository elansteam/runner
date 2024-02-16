// #pragma GCC optimize(s"O3")

#include <iostream>
#include "runner/runner.h"

using namespace std;

int main() {
    // fork();fork();fork();fork();//fork();
    elans::runner::Runner runner("/home/pablo/Documents/project/runner/a",
        elans::runner::Runner::Limits{
            .threads = 20ll,
            .memory = 10000,
            .cpu_time_limit = 5'000,
            .real_time_limit = 5'000,
            .input_stream_file = "/home/pablo/Documents/project/runner/input.txt",
            .output_stream_file = "/tmp/runner/out"
            });

    switch (runner.GetOutput().verdict) {
        case elans::runner::Runner::RunningResult::RE:
            cout << "RE" << endl;
            break;
        case elans::runner::Runner::RunningResult::OK:
            cout << "OK" << endl;
            break;
        case elans::runner::Runner::RunningResult::ML:
            cout << "ML" << endl;
            break;
        case elans::runner::Runner::RunningResult::SE:
            cout << "SE" << endl;
            break;
        case elans::runner::Runner::RunningResult::TL:
            cout << "TL" << endl;
            break;
        case elans::runner::Runner::RunningResult::IE:
            cout << "IE" << endl;
            break;
    }

    cout << "Execution time: " << runner.GetOutput().cpu_time << endl;
}