#include "../include/runner/message_assert.h"

void MessageAssert(bool cond, std::string_view message, bool call_destructors, std::source_location loc) {
    if (!cond) {
        auto error_message = std::format("Execution failed at {} line, in fucntion {}, of file \"{}\" with message: {}",
                                         loc.line(), loc.function_name(), loc.file_name(), message);
        if (errno != 0) {
            error_message += std::format("\nErrno value: {}", errno);
        }
        if (call_destructors) {
            throw std::runtime_error(error_message);
        } else {
            std::cerr << error_message << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}
