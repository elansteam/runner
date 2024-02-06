#include <iostream>
#include "safe-runner/runner.h"

using namespace std;

int main() {
    elans::runner::SafeRunner runner("/home/pablo/Documents/project/runner/a", "13\n", elans::runner::SafeRunner::Limits{ .threads = 20ll, .memory = 50ull, .time= 2 });
    switch (runner.GetOutput().res) {
        case elans::runner::SafeRunner::RunningResult::RE:
            cout << "RE" << endl;
            break;
        case elans::runner::SafeRunner::RunningResult::OK:
            cout << "OK" << endl;
            break;
        case elans::runner::SafeRunner::RunningResult::ML:
            cout << "ML" << endl;
            break;
        case elans::runner::SafeRunner::RunningResult::SE:
            cout << "SE" << endl;
            break;
        case elans::runner::SafeRunner::RunningResult::TL:
            cout << "TL" << endl;
            break;
    }
    cout << "OUT:" << runner.GetOutput().output << endl;

    cout << runner.GetOutput().output << endl;
}