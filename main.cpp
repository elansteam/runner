#pragma GCC optimize("O3")

#include <iostream>
#include "runner/runner.h"

using namespace std;

int main() {
    elans::runner::Runner runner("/home/pablo/Documents/project/runner/a", "13\n", elans::runner::Runner::Limits{ .threads = 20ll, .memory = 1024, .time = 1'000 });
    switch (runner.GetOutput().res) {
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
    }

    cout << "OUT:" << runner.GetOutput().output_path << endl;
    std::ifstream in(runner.GetOutput().output_path);
    char buf[1024];
    cout.write(buf, in.readsome(buf, 1024));
}