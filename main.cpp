#include <iostream>
#include "safe-runner/runner.h"

using namespace std;

int main() {
    elans::runner::SafeRunner runner("/home/parat07/CLionProjects/project/runner/a", "13\n");
    while (!runner.IsEnded()) {
//        cout << "waiting..." << endl;
    }
    cout << runner.GetOutput().output << endl;
}