#pragma once
#include <string_view>
#include <source_location>
#include <iostream>
#include <format>

void MessageAssert(bool cond, std::string_view message, bool call_destructors = true, std::source_location loc = std::source_location::current());
