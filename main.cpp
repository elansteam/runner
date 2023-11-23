#include <iostream>
#include "safe-runner/runner.h"

using namespace std;

int main() {
    elans::runner::SafeRunner runner("/mnt/c/Users/User/CLionProjects/runner/a", "13\n", elans::runner::SafeRunner::Limits{ .threads = 20ll, .memory = (uint64_t)1e6 });
    if (runner.GetOutput().res == elans::runner::SafeRunner::RunningResult::SE) {
        cout << "WIN!!!" << endl;
    } else {
        cout << "FUCK" << endl;
    }

    cout << runner.GetOutput().output << endl;
}