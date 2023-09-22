namespace elans {
namespace runner {
class SafeRunner {
public:
    SafeRunner(std::string path, std::string input) {}
    enum class RunningResult {
        TL,
        ML,
        OK,
        RE,
        SE
    };

    struct TestingResult {
        RunningResult res;
        std::string output;
    };
    TestingResult GetOutput() {
        return { RunningResult::OK, "test output" };
    }
    bool IsEnded() {
        return random() & 1;
    }
};
} // namespace runner
} // namespace elans