#include <iostream>
#include "safe-runner/runner.h"

using namespace std;

int main() {
    elans::runner::SafeRunner runner("/home/pablo/Documents/project/runner/a", "13\n", elans::runner::SafeRunner::Limits{ .threads = 20ll, .memory = (uint64_t)50 });
    if (runner.GetOutput().res == elans::runner::SafeRunner::RunningResult::ML) {
        cout << "WIN!!!" << endl;
    } else {
        cout << "FUCK" << endl;
    }
    cout << "OUT:" << runner.GetOutput().output << endl;

    cout << runner.GetOutput().output << endl;
}