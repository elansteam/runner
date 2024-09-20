#include "../include/runner/message_assert.h"

void MessageAssert(bool cond,
                   std::string_view message,
                   bool call_destructors,
                   std::source_location loc) {
    if (!cond) {
        using std::string_literals::operator""s;
        auto error_message = std::format(
            "{}:{}: {}: {}",
            loc.file_name(), loc.line(), loc.function_name(), message);
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
